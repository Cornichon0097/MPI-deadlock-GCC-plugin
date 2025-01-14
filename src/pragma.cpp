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
#include <tree.h>
#include <intl.h>

#include <string.h>

#include "pragma.h"

/*
 *
 */
static auto_vec<const char *> fnames;

/*
 *
 */
static void pragma_put_fnames(tree args)
{
        // const char *elt1, *elt2;
        // unsigned int ix1, ix2;

        // FOR_EACH_VEC_ELT(args, ix1, elt1) {
        //         FOR_EACH_VEC_ELT(fnames, ix2, elt2) {
        //                 if (strcmp(elt1, elt2) == 0)
        //                         warning(OPT_Wpragmas, "%s already checked", elt1);
        //                 else
        //                         fnames.safe_push(elt1);
        //         }
        // }
}

/*
 *
 */
static void handle_pragma_mpicoll_check(cpp_reader *r)
{
        location_t loc;
        enum cpp_ttype token;
        tree x, args = NULL_TREE;
        bool close_paren_needed_p = false;

        if (cfun) {
                error("%<#pragma mpicoll check%> is not allowed inside functions");
                return;
        }

        token = pragma_lex(&x, &loc);

        if (token == CPP_OPEN_PAREN) {
                close_paren_needed_p = true;
                token = pragma_lex(&x, &loc);
        }

        if (token != CPP_NAME) {
                warning_at(OPT_Wpragmas, loc, "malformed %<#pragma mpicoll check%>, ignored");
                return;
        }

        do {
                args = tree_cons(NULL_TREE, x, args);
                warning(OPT_Wpragmas, "%s added", IDENTIFIER_POINTER(x));

                do
                        token = pragma_lex(&x);
                while (token == CPP_COMMA);
        } while (token == CPP_NAME);

        if (close_paren_needed_p) {
                if (token == CPP_CLOSE_PAREN)
                        token = pragma_lex(&x);
                else {
                        warning(OPT_Wpragmas, "%<#pragma mpicoll check "
                                "(function [,function]...)%> does "
                                "not have a final %<)%>");
                        return;
                }
        }

        if (token != CPP_EOF) {
                error("%<#pragma mpicoll check%> string is badly formed");
                return;
        }

        pragma_put_fnames(args);
}

/*
 *
 */
void register_pragma_mpicoll(void *const event_data, void *const data)
{
        /* warning(0, G_("Callback to register pragmas")); */
        c_register_pragma("mpicoll", "check", &handle_pragma_mpicoll_check);
}

bool is_set_pragma_mpicoll(const function *const fun)
{
        return true;
}
