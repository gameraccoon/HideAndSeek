#!/usr/bin/python3

import json
import os
from os import path

from generators.shared_functions import *

import sys

if len(sys.argv) > 1:
    working_dir = sys.argv[1]
else:
    working_dir = os.getcwd()

configs_dir = path.join(working_dir, "config/code_generation/enums")
templates_dir = path.join(configs_dir, "templates")
descriptions_dir = path.join(working_dir, "config/class_descriptions/Enums")


delimiter_dictionary = load_json(path.join(configs_dir, "delimiter_dictionary.json"))

empty_delimiter_dictionary = {key: "" for key in delimiter_dictionary.keys()}

attribute_templates = load_json(path.join(configs_dir, "attribute_templates.json"))

files_to_generate = load_json(path.join(configs_dir, "files_to_generate.json"))

def get_base_data_dictionary(data_description):
    return {
        "enum_name": data_description["enum"]["name"],
        "enum_values": data_description["enum"]["values"]
    }

def get_attribute_data_dictionary(attribute):
    attribute_data_dictionary = {}

    for field_name, field_value in attribute.items():
        attribute_data_dictionary["attribute_" + field_name] = field_value

    attribute_data_dictionary["attribute_name_capital"] = capitalize(attribute["name"])

    attribute_data_dictionary["attribute_include_full"] = ""
    if "includes" in attribute:
        for include in attribute["includes"]:
            attribute_data_dictionary["attribute_include_full"] += ("#include %s\n" % include)
    attribute_data_dictionary["attribute_include_full"] = attribute_data_dictionary["attribute_include_full"].rstrip("\n")


def append_attributes_data_dictionary(data_dictionary, data_description):
    for template_name in attribute_templates:

        template = read_template(template_name, templates_dir)
        replacement_content = ""
        enum_values = data_dictionary["enum_values"]

        # generate content
        for enum_value in enum_values:
            # skip delimiters for the last item
            if enum_value is enum_values[len(enum_values) - 1]:
                delimiter_dict = empty_delimiter_dictionary
            else:
                delimiter_dict = delimiter_dictionary

            new_replacement_line = replace_content(template, {
                **data_dictionary,
                **{"enum_value": enum_value},
                **delimiter_dict
            })

            replacement_content += new_replacement_line

        data_dictionary[template_name] = replacement_content
    return data_dictionary


def get_full_data_dictionary(data_description):
    full_data_dictionary = get_base_data_dictionary(data_description)
    full_data_dictionary = append_attributes_data_dictionary(full_data_dictionary, data_description)
    return full_data_dictionary


def generate_enum_cpp_file(template_name, destination_dir, file_name_template, full_data_dictionary):
    template = read_template(template_name, templates_dir)
    generated_content = replace_content(template, full_data_dictionary)
    file_name = replace_content(file_name_template, full_data_dictionary)

    if not os.path.exists(destination_dir):
        os.makedirs(destination_dir)

    out_file_name = path.join(destination_dir, file_name)
    write_file(out_file_name, generated_content)
    return out_file_name


def generate_files(file_infos, data_description):
    generated_files = []

    full_data_dict = get_full_data_dictionary(data_description)
    for file_info in file_infos:
        out_file_name = generate_enum_cpp_file(file_info["template"],
            path.join(working_dir, file_info["output_dir"]),
            file_info["name_template"],
            full_data_dict)

        generated_files.append(out_file_name)

    return generated_files


def generate_all():
    generated_files = []

    for file_name in os.listdir(descriptions_dir):
        enum_data = load_json(path.join(descriptions_dir, file_name))
        generated_files += generate_files(files_to_generate, enum_data)

    return generated_files
