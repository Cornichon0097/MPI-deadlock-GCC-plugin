/*
 * Function dealing with MPI collective pragmas.
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
 * Returns the function name from a tree node.
 */
#define FNAME(t) IDENTIFIER_POINTER(TREE_VALUE(t))

/*
 * All functions tagged by #pragma mpicoll check.
 */
static auto_vec<tree> fnames; /* Global variable, yuck */

/*
 * Returns true if fnames contains t, false otherwise.
 */
static bool contains_pragma_mpicoll(const tree t)
{
        unsigned int i;

        if (fnames.is_empty())
                return false;

        for (i = 0U; i < fnames.length(); ++i) {
                if (FNAME(t) == FNAME(fnames[i]))
                        return true;
        }

        return false;
}

/*
 * Parses #pragma mpicoll check args and put functions in fnames.
 */
static void parse_pragma_mpicoll(const tree args)
{
        tree iter;

        for (iter = args; iter; iter = TREE_CHAIN(iter)) {
                if (contains_pragma_mpicoll(iter))
                        warning(OPT_Wpragmas, "%<#pragma mpicoll check%> tags "
                                "%<%s%> function several times", FNAME(iter));
                else
                        fnames.safe_push(iter);
        }
}

/*
 * Reads a #pragma mpicoll check line and retrieve function names.
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
                warning_at(loc, OPT_Wpragmas,
                           "malformed %<#pragma mpicoll check%>, ignored");
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
 * Registers #pragma mpicoll check.
 */
void register_pragma_mpicoll(void *const event_data ATTRIBUTE_UNUSED,
                             void *const data ATTRIBUTE_UNUSED)
{
        /* warning(0, G_("Callback to register pragmas")); */
        c_register_pragma("mpicoll", "check", &handle_pragma_mpicoll_check);
}

/*
 * Prints a warning for each undefined function in fnames.
 */
void undefined_pragma_mpicoll(void *const event_data ATTRIBUTE_UNUSED,
                              void *const data ATTRIBUTE_UNUSED)
{
        unsigned int i;

        for (i = 0U; i < fnames.length(); ++i)
                warning(OPT_Wpragmas, "no matching function for "
                        "%<#pragma mpicoll check %s%>", FNAME(fnames[i]));
}

/*
 * Returns true if fun is tagged by #pragma mpicoll check, false otherwise.
 */
bool is_set_pragma_mpicoll(function *const fun)
{
        unsigned int i;

        for (i = 0U; i < fnames.length(); ++i) {
                if (strcmp(FNAME(fnames[i]), function_name(fun)) == 0) {
                        fnames.unordered_remove(i);
                        return true;
                }
        }

        return false;
}
