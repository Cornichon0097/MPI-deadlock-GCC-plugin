/*
 * Functions dealing with the post-dominance frontiers.
 * Copyright (C) 2023-2025 Antoni Blanche
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <gcc-plugin.h>

#include "frontier.h"

/*
 * Computes the post-dominance frontiers for basic blocks in fun. If the
 * dominance information is not set, the behaviour is undefined.
 *
 * See calculate_dominance_info() for details
 *
 * The Post-Dominance-Frontier Algorithm:
 * for all nodes, b
 *     if the number of successors of b >= 2
 *         for all successors, p, of b
 *             runner <- p
 *             while runner != pdoms[b]
 *                 add b to runner’s post-dominance frontier set
 *                 runner = pdoms[runner]
 */
bitmap frontier_compute_post_dominance(const function *const fun)
{
        bitmap_head *frontiers;
        basic_block bb, runner;
        edge e;
        edge_iterator ei;

        frontiers = XNEWVEC(bitmap_head, last_basic_block_for_fn(fun));

        FOR_ALL_BB_FN(bb, fun)
                bitmap_initialize(&(frontiers[bb->index]),
                                  &bitmap_default_obstack);

        FOR_ALL_BB_FN(bb, fun) {
                if (EDGE_COUNT(bb->succs) >= 2) {
                        FOR_EACH_EDGE(e, ei, bb->succs) {
                                for (runner = e->dest;
                                     runner != get_immediate_dominator(CDI_POST_DOMINATORS, bb);
                                     runner = get_immediate_dominator(CDI_POST_DOMINATORS, runner))
                                        bitmap_set_bit(&(frontiers[runner->index]), bb->index);
                        }
                }
        }

        return frontiers;
}

/*
 * Computes CFG’, a part of fun’s CFG without loop backedge. A loop backedge can
 * be found when a successor of a basic block is also one of its predecessors.
 */
bitmap frontier_compute_cfg_bis(const function *const fun)
{
        auto_vec<basic_block> bb_queue;
        bitmap_head *cfg, *visited_blocks;
        basic_block bb;
        edge e;
        edge_iterator ei;

        cfg = XNEWVEC(bitmap_head, last_basic_block_for_fn(fun));
        visited_blocks = XNEWVEC(bitmap_head, last_basic_block_for_fn(fun));

        FOR_ALL_BB_FN(bb, fun) {
                bitmap_initialize(&(cfg[bb->index]), &bitmap_default_obstack);
                bitmap_initialize(&(visited_blocks[bb->index]),
                                  &bitmap_default_obstack);
        }

        bb_queue.safe_push(ENTRY_BLOCK_PTR_FOR_FN(fun));

        while (!bb_queue.is_empty()) {
                bb = bb_queue.pop();
                bitmap_set_bit(&(visited_blocks[bb->index]), bb->index);

                FOR_EACH_EDGE(e, ei, bb->succs) {
                        if (!bitmap_bit_p(&(visited_blocks[bb->index]),
                                          e->dest->index)) {
                                bitmap_set_bit(&(cfg[bb->index]),
                                               e->dest->index);
                                bitmap_copy(&(visited_blocks[e->dest->index]),
                                            &(visited_blocks[bb->index]));
                                bb_queue.safe_push(e->dest);
                        }
                }
        }

        free(visited_blocks);

        return cfg;
}

/*
 * Finds the right set of groups for bb. If there is no group accepting bb, then
 * add it in a new one. This function returns the group number where bb arrived.
 */
static int frontier_find_group(const function *const fun, const basic_block bb,
                               bitmap groups, const int first_group)
{
        basic_block g;
        int i;

        FOR_EACH_BITMAP(groups, first_group, i) {
                g = BASIC_BLOCK_FOR_FN(fun, bitmap_first_set_bit(&(groups[i])));

                if (bb->aux == g->aux) {
                        bitmap_set_bit(&(groups[i]), bb->index);
                        return i;
                }
        }

        bitmap_set_bit(&(groups[i]), bb->index);

        return i;
}

/*
 * Builds groups of basic blocks with the same rank and MPI collective. MPI
 * collective codes must be set in basic blocks’s aux field before calling this
 * function.
 *
 * See mpicoll_mark_code() for details.
 */
bitmap frontier_make_groups(const function *const fun, const bitmap ranks)
{
        bitmap_head *groups;
        basic_block bb;
        bitmap_iterator bi;
        unsigned int bb_index;
        int nb_groups, first_group;
        int res;
        int i;

        groups = XNEWVEC(bitmap_head, last_basic_block_for_fn(fun));

        FOR_ALL_BB_FN(bb, fun)
                bitmap_initialize(&(groups[bb->index]),
                                  &bitmap_default_obstack);

        nb_groups = 0;

        FOR_EACH_BITMAP(ranks, 0, i) {
                first_group = nb_groups;

                EXECUTE_IF_SET_IN_BITMAP(&(ranks[i]), 0, bb_index, bi) {
                        bb = BASIC_BLOCK_FOR_FN(fun, bb_index);
                        res = frontier_find_group(fun, bb, groups, first_group);

                        if (res == nb_groups)
                                nb_groups = nb_groups + 1;
                }
        }

        return groups;
}

/*
 * Computes the post-dominance for groups. A group post-dominates a basic block
 * if all of the basic blocks in the group dominate it. If the dominance
 * information is not set, the behaviour is undefined.
 *
 * See calculate_dominance_info() for details
 *
 * The Iterative Post-Dominator Algorithm:
 * for all nodes, n
 *     PDOM[n] <- {1...N}
 * Changed <- true
 * while (Changed)
 *     Changed <- false
 *     for all nodes, n, in reverse postorder
 *         new_set <- (p in succs(n), intersect(PDOM[p])) U {n}
 *         if (new_set != PDOM[n])
 *             PDOM[n] <- new_set
 *             Changed <- true
 */
static bitmap frontier_get_groups_post_dominated(const function *const fun,
                                                  const bitmap groups)
{
        bitmap_head *pdom, new_set;
        auto_vec<basic_block> dom;
        basic_block bb, elt;
        edge e;
        bitmap_iterator bi;
        edge_iterator ei;
        unsigned int bb_index;
        unsigned int i, j;
        bool changed;

        pdom = XNEWVEC(bitmap_head, last_basic_block_for_fn(fun));

        FOR_ALL_BB_FN(bb, fun)
                bitmap_initialize(&(pdom[bb->index]), &bitmap_default_obstack);

        FOR_EACH_BITMAP(groups, 0, i) {
                EXECUTE_IF_SET_IN_BITMAP(&(groups[i]), 0, bb_index, bi) {
                        bb = BASIC_BLOCK_FOR_FN(fun, bb_index);
                        dom = get_all_dominated_blocks(CDI_POST_DOMINATORS, bb);

                        FOR_EACH_VEC_ELT(dom, j, elt)
                                bitmap_set_bit(&(pdom[elt->index]), i);
                }
        }

        bitmap_initialize(&new_set, &bitmap_default_obstack);

        for (changed = true; changed;) {
                changed = false;

                FOR_ALL_BB_FN(bb, fun) {
                        bitmap_clear(&new_set);

                        FOR_EACH_EDGE(e, ei, bb->succs) {
                                if (bitmap_empty_p(&new_set))
                                        bitmap_copy(&new_set,
                                                    &(pdom[e->dest->index]));
                                else
                                        bitmap_and_into(&new_set,
                                                       &(pdom[e->dest->index]));
                        }

                        bitmap_ior_into(&new_set, &(pdom[bb->index]));

                        if (!bitmap_equal_p(&new_set, &(pdom[bb->index]))) {
                                bitmap_copy(&(pdom[bb->index]), &new_set);
                                changed = true;
                        }
                }
        }

        return pdom;
}

/*
 * Computes the post-dominance frontier for groups. A group post-dominance
 * frontier contains all basic blocks that are not post-dominated by the group
 * but have at least 1 of its successors that it is. If the dominance
 * information is not set, the behaviour is undefined.
 *
 * See calculate_dominance_info() for details
 */
bitmap frontier_compute_groups_post_dominance(const function *const fun,
                                              const bitmap groups)
{
        bitmap_head *pdom, *frontiers;
        basic_block bb;
        edge e;
        edge_iterator ei;
        int i;

        frontiers = XNEWVEC(bitmap_head, last_basic_block_for_fn(fun));

        FOR_ALL_BB_FN(bb, fun)
                bitmap_initialize(&(frontiers[bb->index]),
                                  &bitmap_default_obstack);

        pdom = frontier_get_groups_post_dominated(fun, groups);

        FOR_ALL_BB_FN(bb, fun) {
                FOR_EACH_BITMAP(groups, 0, i) {
                        if (!bitmap_bit_p(&(pdom[bb->index]), i)) {
                                FOR_EACH_EDGE(e, ei, bb->succs) {
                                            if (bitmap_bit_p(&(pdom[e->dest->index]), i))
                                                        bitmap_set_bit(&(frontiers[i]), bb->index);
                                }
                        }
                }
        }

        free(pdom);

        return frontiers;
}

/*
 * Computes the itered post-dominance frontier for groups. If the dominance
 * information is not set, the behaviour is undefined.
 *
 * See calculate_dominance_info() for details
 */
bitmap frontier_compute_groups_iter_post_dominance(const function *const fun,
                                                   bitmap groups)
{
        bitmap_head *grp_frontiers, *bb_frontiers;
        bitmap_iterator bi1, bi2;
        unsigned int bb_index1, bb_index2;
        int i;

        grp_frontiers = frontier_compute_groups_post_dominance(fun, groups);
        bb_frontiers = frontier_compute_post_dominance(fun);

        FOR_EACH_BITMAP(groups, 0, i) {
                EXECUTE_IF_SET_IN_BITMAP(&(grp_frontiers[i]), 0, bb_index1, bi1) {
                        EXECUTE_IF_SET_IN_BITMAP(&(bb_frontiers[bb_index1]), 0, bb_index2, bi2) {
                                bitmap_set_bit(&(grp_frontiers[i]), bb_index2);
                        }
                }
        }

        return grp_frontiers;
}
