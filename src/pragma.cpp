/*
 *
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
#include <diagnostic-core.h>
#include <c-family/c-pragma.h>
#include <intl.h>

#include "pragma.h"

static void handle_pragma_mpicoll(cpp_reader *)
{
        /* location_t loc;
        enum cpp_ttype token;
        tree x, args;
        bool close_paren_needed_p = false; */

        if (cfun) {
                error("%<#pragma mpicoll option%> is not allowed inside functions");
                return;
        }

        /* token = pragma_lex(&x, &loc);

        if (token == CPP_OPEN_PAREN) {
                close_paren_needed_p = true;
                token = pragma_lex(&x, &loc);
        }

        if (tokne != CPP_STRING)
                GCC_BAD_AT(loc, "%<#pragma mpicoll option%> is not a string");
        else {
                args = NULL_TREE;

                do {
                        if (TREE_STRING_LENGTH(x) > 0)
                                args = tree_cons(NULL_TREE, x, args);

                        token = pragma_lex(&x);
                }
        } */
}

void register_pragma_mpicoll(void *event_data, void *data)
{
        /* warning(0, G_("Callback to register pragmas")); */
        c_register_pragma("mpicoll", "check", &handle_pragma_mpicoll);
}
