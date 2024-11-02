#!/usr/bin/python3

import re

from generators.shared_functions import *


def generate_string_ids_cpp_file(template_name, destination_dir, file_name_template, component_filled_templates, templates_dir):
    template = read_template(template_name, templates_dir)
    generated_content = replace_content(template, component_filled_templates)
    file_name = replace_content(file_name_template, component_filled_templates)

    if not os.path.exists(destination_dir):
        os.makedirs(destination_dir)

    out_file_path = path.join(destination_dir, file_name)
    write_file(out_file_path, generated_content)
    return out_file_path


def generate_ids_list_descriptions(components, element_templates, templates_dir, delimiter_dictionary, empty_delimiter_dictionary):
    component_filled_templates = {}
    for component_template in element_templates:
        template = read_template(component_template["name"], templates_dir)
        filled_template = ""
        for component in components:
            # skip delimiters for the last item
            if component is components[len(components) - 1]:
                delimiter_dict = empty_delimiter_dictionary
            else:
                delimiter_dict = delimiter_dictionary

            filled_template = filled_template + replace_content(template, {
                **component,
                **delimiter_dict
            })
        component_filled_templates[component_template["name"]] = filled_template
    return component_filled_templates


def fnv64a(str):
    hval = 0xcbf29ce484222325
    fnv_64_prime = 0x100000001b3
    uint64_max = 2 ** 64
    for s in str:
        hval = hval ^ ord(s)
        hval = (hval * fnv_64_prime) % uint64_max
    return hval


class HashCollisionError(Exception):
    def __init__(self, message):
        self.message = message


def detect_duplicates(string_data):
    string_data.sort(key=lambda x: x["string_hash"])

    for i in range (len (string_data) -1):
        if string_data[i]["string_hash"] == string_data[i+1]["string_hash"]:
            raise HashCollisionError("Hash collision between %s and %s: %u" % (string_data[i]["string_text"], string_data[i+1]["string_text"], string_data[i]["string_hash"]))


def gather_string_data(sources_dir, string_literal_pattern):
    string_literals = []

    for root, directories, filenames in os.walk(sources_dir):
        for filename in filenames:
            with open(path.join(root, filename), 'r') as f:
                string_literals.extend(re.findall(string_literal_pattern, f.read()))

    #remove duplicates
    string_literals = list(dict.fromkeys(string_literals))

    result = []
    for string_literal in string_literals:
        result.append({"string_text": string_literal, "string_hash": fnv64a(string_literal)})

    detect_duplicates(result)

    return result


def generate_all(generator):
    configs_dir = generator["configs_dir"]
    output_dir_base = generator["output_dir_base"]
    sources_dir = path.join(output_dir_base, "src")
    templates_dir = path.join(configs_dir, generator["templates_dir"])

    string_literal_pattern = r'\WSTR_TO_ID\(\s*"([^"]*)"\s*\)'

    delimiter_dictionary = load_json(path.join(configs_dir, "delimiter_dictionary.json"))

    empty_delimiter_dictionary = {key: "" for key in delimiter_dictionary.keys()}

    element_templates = load_json(path.join(configs_dir, "element_templates.json"))

    files_to_generate = load_json(path.join(configs_dir, "files_to_generate.json"))

    components = gather_string_data(sources_dir, string_literal_pattern)
    generated_files = []

    # generate file with string ids
    component_filled_templates = generate_ids_list_descriptions(components, element_templates, templates_dir, delimiter_dictionary, empty_delimiter_dictionary)
    for file_info in files_to_generate:
        if "flags" in file_info and "list" in file_info["flags"]:
            out_file_name = generate_string_ids_cpp_file(file_info["template"],
                path.join(output_dir_base, file_info["output_dir"]),
                file_info["name_template"],
                component_filled_templates,
                templates_dir)

            generated_files.append(out_file_name)

    return generated_files
