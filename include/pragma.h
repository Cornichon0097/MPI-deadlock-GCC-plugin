/*
 * Declarations and definitions dealing with MPI collective pragmas.
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
#ifndef PRAGMA_H
#define PRAGMA_H

#include <coretypes.h>

/*
 * Registers #pragma mpicoll check.
 */
void register_pragma_mpicoll(void *event_data ATTRIBUTE_UNUSED,
                             void *data ATTRIBUTE_UNUSED);

/*
 * Prints a warning for each undefined function in fnames.
 */
void undefined_pragma_mpicoll(void *event_data ATTRIBUTE_UNUSED,
                              void *data ATTRIBUTE_UNUSED);

/*
 * Returns true if fun is tagged by #pragma mpicoll check, false otherwise.
 */
bool is_set_pragma_mpicoll(function *fun);

#endif /* pragma.h */
