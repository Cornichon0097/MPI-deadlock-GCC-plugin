/*
 * Declarations and definitions dealing with CFG visualization.
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
#ifndef CFGVIZ_H
#define CFGVIZ_H

#include <coretypes.h>

/*
 * Dumps the graphviz CFG representation of fun in a file.
 */
void cfgviz_dump(function *fun, const char *suffix);

/*
 * Dumps the graphviz CFG representation of fun from cfg in a file.
 */
void cfgviz_dump_cfg(function *fun, const char *suffix, bitmap cfg);

#endif /* cfgviz.h */
