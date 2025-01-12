/*
 * Declarations and definitions dealing with MPI calls in basic blocks.
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
#ifndef MPI_CALL
#define MPI_CALL

#include <coretypes.h>

/*
 * Code of each MPI collective.
 */
#define DEF_MPI_COLLECTIVES(CODE, NAME) CODE,
enum mpi_collective_code {
#include "MPI_collectives.def"
        LAST_AND_UNUSED_MPI_COLLECTIVE_CODE
};
#undef DEF_MPI_COLLECTIVES

/*
 * Name of each MPI collective.
 */
#define DEF_MPI_COLLECTIVES(CODE, NAME) NAME,
const char *const MPI_COLLECTIVE_NAME[] = {
#include "MPI_collectives.def"
};
#undef DEF_MPI_COLLECTIVES

/*
 * Puts MPI collective code in basic blocks’s aux field if they contain a MPI
 * call defined in MPI_collectives.def, or LAST_AND_UNUSED_MPI_COLLECTIVE_CODE
 * otherwise.
 *
 * See include/MPI_collectives.def for details.
 */
void mpicoll_mark_code(const function *fun);

/*
 * Sanitises basic blocks’s fields in fun.
 */
void mpicoll_sanitize(const function *fun);

/*
 * Returns true if at least one basic block in fun contains at least 2 MPI
 * collectives, false otherwise.
 */
bool mpicoll_check(const function *fun);

/*
 * Splits each basic block in fun that contains at least 2 MPI collectives.
 */
void mpicoll_split(const function *fun);

/*
 * Returns MPI collectives’s rank in fun using cfg. Loop backedges in cfg must
 * be removed from cfg before calling this function to avoid an infinite loop.
 *
 * See frontier_compute_cfg_bis() for details.
 */
bitmap mpicoll_ranks(const function *fun, bitmap cfg);

/*
 * Returns MPI collective location in bb. The basic block must contain only one
 * MPI collective gimple statement. If bb does not contain MPI collective, this
 * function returns UNKNOWN_LOCATION.
 *
 * See mpicoll_split() for details.
 */
location_t mpicoll_location(basic_block bb);

#endif /* mpicoll.h */
