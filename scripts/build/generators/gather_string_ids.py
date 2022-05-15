#!/usr/bin/python3

import json
import os
from os import path
import re

from generators.shared_functions import *

import sys

if len(sys.argv) > 1:
    working_dir = sys.argv[1]
else:
    working_dir = os.getcwd()


configs_dir = path.join(working_dir, "config/code_generation/string_ids")
sources_dir = path.join(working_dir, "src")
templates_dir = path.join(configs_dir, "templates")

string_literal_pattern = r'\WSTR_TO_ID\(\s*"([^"]*)"\s*\)'


def generate_component_cpp_file(template_name, destination_dir, file_name_template, full_data_dictionary):
    template = read_template(template_name, templates_dir)
    generated_content = replace_content(template, full_data_dictionary)
    file_name = replace_content(file_name_template, full_data_dictionary)

    if not os.path.exists(destination_dir):
        os.makedirs(destination_dir)

    write_file(path.join(destination_dir, file_name), generated_content)


def generate_component_list_cpp_file(template_name, destination_dir, file_name_template, component_filled_templates):
    template = read_template(template_name, templates_dir)
    generated_content = replace_content(template, component_filled_templates)
    file_name = replace_content(file_name_template, component_filled_templates)

    if not os.path.exists(destination_dir):
        os.makedirs(destination_dir)

    write_file(path.join(destination_dir, file_name), generated_content)


def generate_files(file_infos, data_description, full_data_dict):
    for file_info in file_infos:
        if "flags" in file_info and "list" in file_info["flags"]:
            pass # generated in another function
        else:
            generate_component_cpp_file(file_info["template"],
                                        path.join(working_dir, file_info["output_dir"]),
                                        file_info["name_template"],
                                        full_data_dict)


def load_component_data_description(file_path):
    return load_json(file_path)


def generate_component_list_descriptions(components):
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


def gather_string_data():
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


def generate_all():
    components = gather_string_data()

    # generate component lists
    component_filled_templates = generate_component_list_descriptions(components)
    for file_info in files_to_generate:
        if "flags" in file_info and "list" in file_info["flags"]:
            generate_component_list_cpp_file(file_info["template"],
                                             path.join(working_dir, file_info["output_dir"]),
                                             file_info["name_template"],
                                             component_filled_templates)


delimiter_dictionary = load_json(path.join(configs_dir, "delimiter_dictionary.json"))

empty_delimiter_dictionary = {key: "" for key in delimiter_dictionary.keys()}

element_templates = load_json(path.join(configs_dir, "element_templates.json"))

files_to_generate = load_json(path.join(configs_dir, "files_to_generate.json"))

generate_all()
