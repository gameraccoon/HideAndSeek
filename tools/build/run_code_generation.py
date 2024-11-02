#!/usr/bin/python3

import argparse
import json
import os

import generators.components_code_generate
import generators.enums_code_generate
import generators.gather_string_ids


def load_generators(generators_json_path):
    with open(generators_json_path, "r") as f:
        generators_json = json.load(f)

    return generators_json["generators"], generators_json["groups"]


def get_arguments_parser():
    parser = argparse.ArgumentParser(description="Run generators")
    parser.add_argument("--validate-all", action="store_true", help="validate all generators")
    parser.add_argument("--generator", help="run specific generator")
    parser.add_argument("--group", help="run all generators in a group")
    parser.add_argument("--quiet", action="store_true", help="suppress output")
    parser.add_argument("--caches-dir", help="directory to store caches")
    parser.add_argument("--working-dir", help="working directory")
    parser.add_argument("--config", help="path to generators.json")

    return parser


def validate_arguments(parser, parsed_args, generator_list, group_list):
    # validate arguments
    if not parsed_args.validate_all and not parsed_args.generator and not parsed_args.group:
        parser.error("at least one of --validate-all, --generator, --group is required")

    if parsed_args.generator and parsed_args.group:
        parser.error("--generator and --group cannot be used together")

    if parsed_args.generator and parsed_args.generator not in generator_list:
        parser.error("generator '{}' not found\n\navailable generators: {}".format(parsed_args.generator, ", ".join(generator_list)))

    if parsed_args.group and parsed_args.group not in group_list:
        parser.error("group '{}' not found\n\navailable groups: {}".format(parsed_args.group, ", ".join(group_list)))


def run_generator(generator_id, generator_list, args):
    if not args.quiet:
        print("Running generator: " + generator_id)
    generator = generator_list[generator_id]
    # this is temporary, ideally we should have one endpoint that runs all types generators based only on parameters
    if generator_id == "components":
        return generators.components_code_generate.generate_all(generator)
    if generator_id == "enums":
        return generators.enums_code_generate.generate_all(generator)
    if generator_id == "string_ids":
        return generators.gather_string_ids.generate_all(generator)

    return []


def remove_old_files(generated_files, args):
    generators_config_path = "generators.json"
    default_caches_dir = "caches"
    previously_generated_files_cache_path = "previously_generated_files.txt"

    if args.caches_dir:
        caches_dir = args.caches_dir
    else:
        caches_dir = os.path.join(os.path.dirname(__file__), default_caches_dir)

    if not os.path.exists(caches_dir):
        os.makedirs(caches_dir)

    previously_generated_files_file_path = os.path.join(caches_dir, previously_generated_files_cache_path)

    if os.path.isfile(previously_generated_files_file_path):
        with open(previously_generated_files_file_path, "r") as f:
            old_generated_files = f.read().splitlines()

        if generated_files != old_generated_files:
            generated_files_set = set(generated_files)
            old_generated_files_set = set(old_generated_files)

            removed_files = old_generated_files_set - generated_files_set
            for file in removed_files:
                if os.path.isfile(file):
                    os.remove(file)
                    print("Removed file: " + file)

            with open(previously_generated_files_file_path, "w") as f:
                f.write("\n".join(generated_files))
    else:
        with open(previously_generated_files_file_path, "w") as f:
            f.write("\n".join(generated_files))


def execute():
    generators_default_config_path = "generators.json"

    arguments_parser = get_arguments_parser()
    arguments = arguments_parser.parse_args()

    if arguments.config:
        generators_config_path = arguments.config
    else:
        generators_config_path = os.path.join(os.path.dirname(__file__), generators_default_config_path)

    generator_list, group_list = load_generators(generators_config_path)
    validate_arguments(arguments_parser, arguments, generator_list, group_list)

    if arguments.working_dir:
        os.chdir(arguments.working_dir)

    if arguments.validate_all:
        for generator in generator_list:
            if not arguments.quiet:
                print("Validating generator: " + generator)
            os.system("python3 generators/" + generator + ".py --validate")

    if arguments.generator or arguments.group:
        generated_files = []
        if arguments.generator:
            generated_files += run_generator(arguments.generator, generator_list, arguments)

        if arguments.group:
            # execute generators in the same order as they are defined in the json
            for generator_id in group_list[arguments.group]:
                generated_files += run_generator(generator_id, generator_list, arguments)

        # remove files that were generated before but are not generated now
        remove_old_files(generated_files, arguments)


execute()
