/*
 * Functions to print information and debug.
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
#include <tree.h>
#include <gimple.h>
#include <gimple-iterator.h>
#include <diagnostic-core.h>

#include "print.h"
#include "mpicoll.h"
#include "frontier.h"

/*
 * Prints bb’s direct (post-)dominators (depending on dir).
 */
static void print_dominance(const enum cdi_direction dir, const basic_block bb)
{
        auto_vec<basic_block> dom = get_all_dominated_blocks(dir, bb);
        unsigned int i;

        for (i = 0U; i < dom.length(); ++i) {
                if (dom[i]->index != bb->index)
                        printf("\tNode %d\n", dom[i]->index);
        }
}

/*
 * Prints fun’s name and returns it.
 */
const char *print_function_name(function *const fun)
{
        const char *fname = function_name(fun);

        /* printf("Current function: %s()\n", fndecl_name(cfun->decl)); */
        printf("Current function: %s()\n", fname);

        return fname;
}

/*
 * Prints all basic blocks in fun.
 */
void print_blocks(const function *const fun)
{
        basic_block bb;

        FOR_EACH_BB_FN(bb, fun)
                printf("\tBasic block %d, line %d\n", bb->index,
                       gimple_lineno(gsi_stmt(gsi_start_bb(bb))));
}

/*
 * Prints all called functions in bb if any.
 */
void print_called_functions(const basic_block bb)
{
        gimple_stmt_iterator gsi;
        gimple *stmt;

        for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                stmt = gsi_stmt(gsi);

                if (is_gimple_call(stmt))
                        printf("\t\tCall %s()\n", IDENTIFIER_POINTER(DECL_NAME(
                                                  gimple_call_fndecl(stmt))));
        }
}

/*
 * Prints the function’s name called in stmt if it is a MPI collective.
 */
void print_mpicoll_name(const gimple *const stmt)
{
        const char *fname;
        int i;

        if (is_gimple_call(stmt)) {
                fname = IDENTIFIER_POINTER(DECL_NAME(gimple_call_fndecl(stmt)));

                for (i = 0; i < 5; ++i) {
                        if (strncmp(fname, MPI_COLLECTIVE_NAME[i],
                                    strlen(MPI_COLLECTIVE_NAME[i])) == 0)
                                printf("\t\tCall %s()\n", fname);
                }
        }
}

/*
 * Prints basic blocks domination in fun. If the dominance information is not
 * computed, the behaviour is undefined.
 *
 * See calculate_dominance_info() for details.
 */
void print_dominators(const function *const fun)
{
        basic_block bb;

        FOR_ALL_BB_FN(bb, fun) {
                printf("Node %d dominates:\n", bb->index);
                print_dominance(CDI_DOMINATORS, bb);
        }
}

/*
 * Prints basic blocks post-domination in fun. If the dominance information is
 * not computed, the behaviour is undefined.
 *
 * See calculate_dominance_info() for details.
 */
void print_post_dominators(const function *const fun)
{
        basic_block bb;

        FOR_ALL_BB_FN(bb, fun) {
                printf("Node %d post-dominates:\n", bb->index);
                print_dominance(CDI_POST_DOMINATORS, bb);
        }
}

/*
 * Prints the post-dominance frontiers in fun.
 */
void print_post_dominance_frontiers(const function *const fun,
                                    const bitmap frontiers)
{
        basic_block bb;

        FOR_ALL_BB_FN(bb, fun) {
                printf("Node %d post-dominance frontier: ", bb->index);
                bitmap_print(stdout, &(frontiers[bb->index]), "", "\n");
        }
}

/*
 * Prints fun’s cfg.
 */
void print_cfg(const function *const fun, const bitmap cfg)
{
        basic_block bb;

        FOR_ALL_BB_FN(bb, fun) {
                printf("Node %d successors: ", bb->index);
                bitmap_print(stdout, &(cfg[bb->index]), "", "\n");
        }
}

/*
 * Prints a warning if a possible MPI deadlock is detected in fun. A deadlock
 * might be possible if pdf is set for at least 1 basic block in fun.
 */
void print_warning(function *const fun, const bitmap groups, const bitmap pdf)
{
        basic_block bb;
        bitmap_iterator bi;
        unsigned int bb_index;
        int i;

        FOR_EACH_BITMAP(groups, 0, i) {
                if (!bitmap_empty_p(&(pdf[i]))) {
                        EXECUTE_IF_SET_IN_BITMAP(&(groups[i]), 0, bb_index, bi) {
                                bb = BASIC_BLOCK_FOR_FN(fun, bb_index);
                                warning_at(mpicoll_location(bb), 0,
                                           "possible MPI deadlock");
                        }

                        EXECUTE_IF_SET_IN_BITMAP(&(pdf[i]), 0, bb_index, bi)
                                inform(gimple_location(gsi_stmt(gsi_last_bb(
                                       BASIC_BLOCK_FOR_FN(fun, bb_index)))),
                                       "fork here");
                }
        }
}
