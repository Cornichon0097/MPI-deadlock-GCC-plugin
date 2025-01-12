/*
 * Declarations and definitions dealing with the post-dominance frontiers.
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
#ifndef FRONTIER_H
#define FRONTIER_H

#include <coretypes.h>

#define FOR_EACH_BITMAP(map, start, iter) \
        for ((iter) = start; !bitmap_empty_p(&((map)[(iter)])); ++(iter))

/*
 * Computes the post-dominance frontiers for basic blocks in fun. If the
 * dominance information is not computed, the behaviour is undefined.
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
bitmap frontier_compute_post_dominance(const function *fun);

/*
 * Computes CFG’, a part of fun’s CFG without loop backedge. A loop backedge can
 * be found when a successor of a basic block is also one of its predecessors.
 */
bitmap frontier_compute_cfg_bis(const function *fun);

/*
 * Builds groups of basic blocks with the same rank and MPI collective. MPI
 * collective codes must be set in basic blocks’s aux field before calling this
 * function.
 *
 * See mpicoll_mark_code() for details.
 */
bitmap frontier_make_groups(const function *fun, bitmap ranks);

/*
 * Computes the post-dominance frontier for groups. A group post-dominance
 * frontier contains all basic blocks that are not post-dominated by the group
 * but have at least 1 of its successors that it is. If the dominance
 * information is not computed, the behaviour is undefined.
 *
 * See calculate_dominance_info() for details
 */
bitmap frontier_compute_groups_post_dominance(const function *fun,
                                              bitmap groups);

/*
 * Computes the itered post-dominance frontier for groups. If the dominance
 * information is not computed, the behaviour is undefined.
 *
 * See calculate_dominance_info() for details
 */
bitmap frontier_compute_groups_iter_post_dominance(const function *fun,
                                                   bitmap groups);

#endif /* frontier.h */
