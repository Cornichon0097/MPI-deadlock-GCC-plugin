/*
 * MPI deadlock GCC plugin.
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
#include <plugin-version.h>
#include <tree-pass.h>
#include <context.h>

#include "print.h"
#include "cfgviz.h"
#include "mpicoll.h"
#include "frontier.h"
#include "pragma.h"

/*
 * Ensures the plugin is build for GCC 12.2.0.
 */
#if GCCPLUGIN_VERSION != 12002
#error This GCC plugin is for GCC 12.2.0
#endif

/*
 * If the symbol plugin_is_GPL_compatible does not exist, the compiler will emit
 * a fatal error and exit. The compiler merely asserts that the symbol exists in
 * the global scope.
 */
int plugin_is_GPL_compatible;

/*
 * The metadata of the MPI pass, non-varying across all instances of a pass.
 */
static const pass_data mpi_pass_data = {
        GIMPLE_PASS,
        "mpi_pass",
        OPTGROUP_NONE,
        TV_OPTIMIZE,
        0U,
        0U,
        0U,
        0U,
        0U,
};

/*
 * The MPI pass class.
 */
class mpi_pass: public opt_pass
{
public:
        /*
         * MPI pass constructor.
         */
        mpi_pass(gcc::context *ctxt): opt_pass(mpi_pass_data, ctxt) {}

        /*
         * Creates a copy of this pass.
         */
        mpi_pass *clone(void)
        {
                return new mpi_pass(g);
        }

        /*
         * This pass is executed only if this function returns true.
         */
        bool gate(function *const fun)
        {
                return is_set_pragma_mpicoll(fun);
        }

        /*
         * This is the code to run when the pass is executed. The return value
         * contains TODOs to execute in addition to those in TODO_flags_finish.
         */
        unsigned int execute(function *const fun)
        {
                /* bitmap_head *frontiers; */
                bitmap_head *cfg, *ranks, *groups, *pdf;

                /* print_function_name(fun); */

                while (mpicoll_check(fun))
                        mpicoll_split(fun);

                mpicoll_mark_code(fun);

                /* print_blocks(fun); */
                /* cfgviz_dump(fun, "cfg"); */

                calculate_dominance_info(CDI_POST_DOMINATORS);

                /* print_dominators(fun); */
                /* print_post_dominators(fun); */

                /* frontiers = frontier_compute_post_dominance(fun);
                print_post_dominance_frontiers(fun, frontiers); */

                cfg = frontier_compute_cfg_bis(fun);

                /* print_cfg(fun, cfg); */
                /* cfgviz_dump_cfg(fun, "bis", cfg); */

                ranks = mpicoll_ranks(fun, cfg);
                groups = frontier_make_groups(fun, ranks);

                /* pdf = frontier_compute_post_dominance(fun);
                print_post_dominance_frontiers(fun, pdf);
                free(pdf); */

                /* pdf = frontier_compute_groups_post_dominance(fun, groups); */
                pdf = frontier_compute_groups_iter_post_dominance(fun, groups);

                print_warning(fun, groups, pdf);

                free(pdf);
                free(groups);
                free(ranks);
                free(cfg);

                free_dominance_info(CDI_POST_DOMINATORS);
                mpicoll_sanitize(fun);

                return 0U;
        }
};

/*
 * The function plugin_init() is responsible for registering all the callbacks
 * required by the plugin and do any other required initialization. It returns
 * a non-zero value if initialization fails, 0 otherwise.
 */
int plugin_init(struct plugin_name_args *const plugin_info,
                struct plugin_gcc_version *const version)
{
        struct register_pass_info mpi_pass_info;
        mpi_pass mpi_pass(g);

        if (!plugin_default_version_check(version, &gcc_version))
                return 1;

        mpi_pass_info.pass = &mpi_pass;
        mpi_pass_info.reference_pass_name = "cfg";
        mpi_pass_info.ref_pass_instance_number = 0;
        mpi_pass_info.pos_op = PASS_POS_INSERT_AFTER;

        register_callback(plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP,
                          NULL, &mpi_pass_info);
        register_callback(plugin_info->base_name, PLUGIN_PRAGMAS,
                          &register_pragma_mpicoll, NULL);

        return 0;
}
