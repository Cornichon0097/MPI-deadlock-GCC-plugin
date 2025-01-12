/*
 * Functions dealing with CFG visualization.
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

#include <stdio.h>

#include "cfgviz.h"
#include "mpi_call.h"

/*
 * Builds a filename (as a string) based on fun’s name and suffix.
 */
static char *cfgviz_generate_filename(function *const fun,
                                      const char *const suffix)
{
        char *target_filename = XNEWVEC(char, 1024);

        snprintf(target_filename, 1024, "%s_%s_%d_%s.dot", function_name(fun),
                 basename(LOCATION_FILE(fun->function_start_locus)),
                 LOCATION_LINE(fun->function_start_locus), suffix);

        return target_filename;
}

/*
 * Dumps the graphviz CFG representation of bb’s edges.
 */
static void cfgviz_edge_dump(const basic_block bb, FILE *const out)
{
        edge e;
        edge_iterator ei;
        const char *label = "";

        FOR_EACH_EDGE(e, ei, bb->succs) {
                if (e->flags == EDGE_TRUE_VALUE)
                        label = "true";
                else if (e->flags == EDGE_FALSE_VALUE)
                        label = "false";

                fprintf(out, "\tN%d -> N%d [color=red label=\"%s\"]\n",
                        e->src->index, e->dest->index, label);
        }
}

/*
 * Dumps the graphviz CFG representation of fun in out stream.
 */
static void cfgviz_internal_dump(const function *const fun, FILE *const out)
{
        basic_block bb;

        fprintf(out, "Digraph G{\n");

        FOR_ALL_BB_FN(bb, fun) {
                if (bb->aux != (void *) LAST_AND_UNUSED_MPI_COLLECTIVE_CODE)
                        fprintf(out, "\tN%d [label=\"%s\" shape=ellipse]\n",
                                bb->index,
                                MPI_COLLECTIVE_NAME[(unsigned long) (bb->aux)]);
                else
                        fprintf(out, "\tN%d [label=\"%d\" shape=ellipse]\n",
                                bb->index, bb->index);

                cfgviz_edge_dump(bb, out);
        }

        fprintf(out, "}\n");
}

/*
 * Dumps the graphviz CFG representation of fun in a file.
 */
void cfgviz_dump(function *const fun, const char *const suffix)
{
        char *target_filename;
        FILE *out;

        target_filename = cfgviz_generate_filename(fun, suffix);
        out = fopen(target_filename, "w");

        cfgviz_internal_dump(fun, out);

        fclose(out);
        free(target_filename);
}

/*
 * Dumps the graphviz CFG representation of bb’s edges.
 */
static void cfgviz_edge_dump_bis(const basic_block bb,
                                 FILE *const out, const bitmap cfg)
{
        edge e;
        edge_iterator ei;
        const char *label = "";

        FOR_EACH_EDGE(e, ei, bb->succs) {
                if (bitmap_bit_p(&(cfg[bb->index]), e->dest->index)) {
                        if (e->flags == EDGE_TRUE_VALUE)
                                label = "true";
                        else if (e->flags == EDGE_FALSE_VALUE)
                                label = "false";

                        fprintf(out, "\tN%d -> N%d [color=red label=\"%s\"]\n",
                                e->src->index, e->dest->index, label);
                }
        }
}

/*
 * Dumps the graphviz CFG representation of fun from cfg in a file.
 */
static void cfgviz_internal_dump_bis(const function *const fun,
                                     FILE *const out, const bitmap cfg)
{
        basic_block bb;

        fprintf(out, "Digraph G{\n");

        FOR_ALL_BB_FN(bb, fun) {
                if (bb->aux != (void *) LAST_AND_UNUSED_MPI_COLLECTIVE_CODE)
                        fprintf(out, "\tN%d [label=\"%s\" shape=ellipse]\n",
                                bb->index,
                                MPI_COLLECTIVE_NAME[(unsigned long) (bb->aux)]);
                else
                        fprintf(out, "\tN%d [label=\"%d\" shape=ellipse]\n",
                                bb->index, bb->index);

                cfgviz_edge_dump_bis(bb, out, cfg);
        }

        fprintf(out, "}\n");
}

/*
 * Dumps the graphviz CFG representation of fun from cfg in a file.
 */
void cfgviz_dump_cfg(function *const fun,
                     const char *const suffix, const bitmap cfg)
{
        char *target_filename;
        FILE *out;

        target_filename = cfgviz_generate_filename(fun, suffix);
        out = fopen(target_filename, "w");

        cfgviz_internal_dump_bis(fun, out, cfg);

        fclose(out);
        free(target_filename);
}
