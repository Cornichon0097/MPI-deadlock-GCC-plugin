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
static void parse_pragma_mpicoll(const tree args)
{
        const char *name;
        unsigned int i;
        tree iter;

        for (iter = args; iter; iter = TREE_CHAIN(iter)) {
                name = IDENTIFIER_POINTER(TREE_VALUE(iter));
                fprintf(stderr, "%s\n", name);

                if (fnames.is_empty())
                        fnames.safe_push(name);
                else {
                        for (i = 0U; i < fnames.length(); ++i) {
                                if (strcmp(fnames[i], name) != 0)
                                        fnames.safe_push(name);
                                // else
                                //         warning(OPT_Wpragmas, "%<#pragma mpicoll check%> adds function %<%s%> multiple times", name);
                        }
                }
        }
}

/*
 *
 */
static void handle_pragma_mpicoll_check(cpp_reader *const r ATTRIBUTE_UNUSED)
{
        location_t loc;
        enum cpp_ttype token;
        tree x, args = NULL_TREE;
        bool close_paren_needed_p = false;

        if (cfun != NULL) {
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

        parse_pragma_mpicoll(args);
}

/*
 *
 */
void register_pragma_mpicoll(void *const event_data ATTRIBUTE_UNUSED,
                             void *const data ATTRIBUTE_UNUSED)
{
        /* warning(0, G_("Callback to register pragmas")); */
        c_register_pragma("mpicoll", "check", &handle_pragma_mpicoll_check);
}

/*
 *
 */
void undefined_pragma_mpicoll(void *const event_data ATTRIBUTE_UNUSED,
                              void *const data ATTRIBUTE_UNUSED)
{

}

/*
 *
 */
bool is_set_pragma_mpicoll(function *const fun)
{
        const char *elt;
        unsigned int ix;

        FOR_EACH_VEC_ELT(fnames, ix, elt) {
                if (strcmp(elt, function_name(fun)) == 0)
                        return false;
        }

        return false;
}
