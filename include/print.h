/*
 * Declarations and definitions to print information and debug.
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
#ifndef PRINT_H
#define PRINT_H

#include <coretypes.h>

/*
 * Prints fun’s name and returns it.
 */
const char *print_function_name(function *fun);

/*
 * Prints all basic blocks in fun.
 */
void print_blocks(const function *fun);

/*
 * Prints all called functions in bb if any.
 */
void print_called_functions(basic_block bb);

/*
 * Prints the function’s name called in stmt if it is a MPI collective.
 */
void print_mpi_call_name(const gimple *stmt);

/*
 * Prints basic blocks domination in fun. If the dominance information is not
 * computed, the behaviour is undefined.
 *
 * See calculate_dominance_info() for details.
 */
void print_dominators(const function *fun);

/*
 * Prints basic blocks post-domination in fun. If the dominance information is
 * not computed, the behaviour is undefined.
 *
 * See calculate_dominance_info() for details.
 */
void print_post_dominators(const function *fun);

/*
 * Prints the post-dominance frontiers in fun.
 */
void print_post_dominance_frontiers(const function *fun, bitmap frontiers);

/*
 * Prints fun’s cfg.
 */
void print_cfg(const function *fun, bitmap cfg);

/*
 * Prints a warning if a possible MPI deadlock is detected in fun. A deadlock
 * might be possible if pdf is set for at least 1 basic block in fun.
 */
void print_warning(function *fun, bitmap groups, bitmap pdf);

#endif /* print.h */
