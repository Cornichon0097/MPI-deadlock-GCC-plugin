/*
 * Functions dealing with MPI calls in basic blocks.
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

#include <string.h>

#include "mpi_call.h"

/*
 * Returns the MPI collective code if stmt is a call to an MPI function defined
 * in MPI_collectives.def, or LAST_AND_UNUSED_MPI_COLLECCTIVE_CODE otherwise.
 *
 * See include/MPI_collectives.def for details.
 */
static enum mpi_collective_code mpi_call_code(const gimple *const stmt)
{
        const char *fname;
        int i;

        if (is_gimple_call(stmt)) {
                fname = IDENTIFIER_POINTER(DECL_NAME(gimple_call_fndecl(stmt)));

                for (i = 0; i < 5; ++i) {
                        if (strncmp(fname, MPI_COLLECTIVE_NAME[i],
                                    strlen(MPI_COLLECTIVE_NAME[i])) == 0)
                                return (enum mpi_collective_code) i;
                }
        }

        return LAST_AND_UNUSED_MPI_COLLECTIVE_CODE;
}

/*
 * Puts MPI collective code in basic blocks’s aux field if they contain a MPI
 * call defined in MPI_collectives.def, or LAST_AND_UNUSED_MPI_COLLECTIVE_CODE
 * otherwise.
 *
 * See include/MPI_collectives.def for details.
 */
void mpi_call_mark_code(const function *const fun)
{
        basic_block bb;
        gimple_stmt_iterator gsi;
        enum mpi_collective_code code;

        FOR_ALL_BB_FN(bb, fun) {
                bb->aux = (void *) LAST_AND_UNUSED_MPI_COLLECTIVE_CODE;

                for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                        code = mpi_call_code(gsi_stmt(gsi));

                        if (code != LAST_AND_UNUSED_MPI_COLLECTIVE_CODE)
                                bb->aux = (void *) code;
                }
        }
}

/*
 * Sanitises basic blocks’s fields in fun.
 */
void mpi_call_sanitize(const function *const fun)
{
        basic_block bb;

        FOR_ALL_BB_FN(bb, fun)
                bb->aux = NULL;
}

/*
 * Returns the number of MPI collectives in bb.
 */
static int mpi_call_nb(const basic_block bb)
{
        gimple_stmt_iterator gsi;
        gimple *stmt;
        int count = 0;

        for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                stmt = gsi_stmt(gsi);

                if (mpi_call_code(stmt) != LAST_AND_UNUSED_MPI_COLLECTIVE_CODE)
                        count = count + 1;
        }

        return count;
}

/*
 * Returns true if at least one basic block in fun contains at least 2 MPI
 * collectives, false otherwise.
 */
bool mpi_call_check(const function *const fun)
{
        basic_block bb;

        FOR_EACH_BB_FN(bb, fun) {
                if (mpi_call_nb(bb) >= 2)
                        return true;
        }

        return false;
}

/*
 * Splits bb at the first MPI collective statement in basic block. If no MPI
 * collective statement is found in the basic block, then bb stays unchanged.
 */
static void mpi_call_split_block(basic_block bb)
{
        gimple_stmt_iterator gsi;
        gimple *stmt;

        for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                stmt = gsi_stmt(gsi);

                if (mpi_call_code(stmt) != LAST_AND_UNUSED_MPI_COLLECTIVE_CODE)
                        split_block(bb, stmt);
        }
}

/*
 * Splits each basic block in fun that contains at least 2 MPI collectives.
 */
void mpi_call_split(const function *const fun)
{
        basic_block bb;

        FOR_EACH_BB_FN(bb, fun) {
                if (mpi_call_nb(bb) >= 2)
                        mpi_call_split_block(bb);
        }
}

/*
 * Ranks bb and all of its successors using cfg if they contain a MPI
 * collective. If bb do not contain a MPI collective, then its rank is 0.
 */
static void mpi_call_rank_next(const bitmap cfg, const basic_block bb,
                               bitmap ranks, int current_rank)
{
        edge e;
        edge_iterator ei;

        if (bb->aux != (void *) LAST_AND_UNUSED_MPI_COLLECTIVE_CODE) {
                bitmap_set_bit(&(ranks[current_rank]), bb->index);
                current_rank = current_rank + 1;
        }

        FOR_EACH_EDGE(e, ei, bb->succs) {
                if (bitmap_bit_p(&(cfg[bb->index]), e->dest->index))
                        mpi_call_rank_next(cfg, e->dest, ranks, current_rank);
        }
}

/*
 * Returns MPI collectives’s rank in fun using cfg. Loop backedges in cfg must
 * be removed from cfg before calling this function to avoid an infinite loop.
 */
bitmap mpi_call_ranks(const function *const fun, const bitmap cfg)
{
        bitmap_head *ranks = XNEWVEC(bitmap_head, last_basic_block_for_fn(fun));
        basic_block bb;

        FOR_ALL_BB_FN(bb, fun)
                bitmap_initialize(&(ranks[bb->index]), &bitmap_default_obstack);

        mpi_call_rank_next(cfg, ENTRY_BLOCK_PTR_FOR_FN(fun), ranks, 0);

        return ranks;
}

/*
 * Returns MPI collective location in bb. The basic block must contain only one
 * MPI collective gimple statement. If bb does not contain MPI collective, this
 * function returns UNKNOWN_LOCATION.
 *
 * See mpi_call_split() for details.
 */
location_t mpi_call_location(const basic_block bb)
{
        gimple_stmt_iterator gsi;
        gimple *stmt;
        const char *fname;
        int i;

        for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                stmt = gsi_stmt(gsi);

                if (is_gimple_call(stmt)) {
                        fname = IDENTIFIER_POINTER(DECL_NAME(
                                                     gimple_call_fndecl(stmt)));

                        for (i = 0; i < 5; ++i) {
                                if (strncmp(fname, MPI_COLLECTIVE_NAME[i],
                                           strlen(MPI_COLLECTIVE_NAME[i])) == 0)
                                        return gimple_location(stmt);
                        }
                }
        }

        return UNKNOWN_LOCATION;
}
