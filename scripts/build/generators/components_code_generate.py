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


configs_dir = path.join(working_dir, "config/code_generation/components")
templates_dir = path.join(configs_dir, "templates")
descriptions_dir = path.join(working_dir, "config/class_descriptions/Components")


def get_base_data_dictionary(data_description):
    return {
        "component_name": data_description["component"],
        "component_name_capital": capitalize(data_description["component"]),
        "class_name": "%sComponent" % capitalize(data_description["component"]),
        "component_tolower" : data_description["component"].lower(),
        "component_description": data_description["description"],
        "component_flags": data_description["flags"] if ("flags" in data_description) else []
    }


def get_attribute_data_dictionary(attribute):
    attribute_data_dictionary = {}

    for field_name, field_value in attribute.items():
        attribute_data_dictionary["attribute_" + field_name] = field_value

    attribute_data_dictionary["attribute_name_capital"] = capitalize(attribute["name"])
    attribute_data_dictionary["attribute_tolower"] = attribute["name"].lower()

    attribute_data_dictionary["attribute_include_full"] = ""
    if "includes" in attribute:
        for include in attribute["includes"]:
            attribute_data_dictionary["attribute_include_full"] += ("#include %s\n" % include)
    attribute_data_dictionary["attribute_include_full"] = attribute_data_dictionary["attribute_include_full"].rstrip("\n")

    # fill missing fields from defaults
    for field_name, field_value in attribute_optional_fields.items():
        if ("attribute_" + field_name) not in attribute_data_dictionary:
            attribute_data_dictionary["attribute_" + field_name] = field_value

    return attribute_data_dictionary


def does_attribute_pass_filters(attribute, template_params, attribute_template_data):
    # skip attributes with empty values, if requested
    if "not_empty" in template_params:
        if len(attribute["data_dict"][attribute_template_data["value_to_empty_test"]]) == 0:
            return False

    # skip blacklisted attributes
    if "blacklist" in attribute_template_data:
        if any(x in attribute["data_dict"]["attribute_flags"] for x in attribute_template_data["blacklist"]):
            return False

    # if we have whitelist, skip attributes without whitelisted flags
    if "whitelist" in attribute_template_data:
        if not any(x in attribute["data_dict"]["attribute_flags"] for x in attribute_template_data["whitelist"]):
            return False

    return True


def does_component_pass_filters(component, template_data):
    # skip blacklisted attributes
    if "blacklist" in template_data:
        if any(x in component["component_flags"] for x in template_data["blacklist"]):
            return False

    # if we have whitelist, skip attributes without whitelisted flags
    if "whitelist" in template_data:
        if not any(x in attribute["component_flags"] for x in template_data["whitelist"]):
            return False

    return True


def append_attributes_data_dictionary(data_dictionary, data_description):
    for attribute_template_data in attribute_templates:
        template_name = attribute_template_data["name"]

        template_params = set(attribute_template_data["params"])

        template = read_template(template_name, templates_dir)
        replacement_content = ""
        replacement_content_elements = []

        # filter attributes
        for attribute in data_description["attributes"]:
            if does_attribute_pass_filters(attribute, template_params, attribute_template_data):
                replacement_content_elements.append(attribute["data_dict"])

        # generate content
        for replace_content_dict in replacement_content_elements:
            # skip delimiters for the last item
            if replace_content_dict is replacement_content_elements[len(replacement_content_elements) - 1]:
                delimiter_dict = empty_delimiter_dictionary
            else:
                delimiter_dict = delimiter_dictionary

            new_replacement_line = replace_content(template, {
                **data_dictionary,
                **replace_content_dict,
                **delimiter_dict
            })

            replacement_content += new_replacement_line

        if "remove_duplicates" in template_params:
            replacement_content = "\n".join(set(replacement_content.split("\n")))

        if "sort" in template_params:
            replacement_content = "\n".join(sorted(replacement_content.split("\n")))

        data_dictionary[template_name] = replacement_content
    return data_dictionary


def get_full_data_dictionary(data_description):
    full_data_dictionary = get_base_data_dictionary(data_description)
    full_data_dictionary = append_attributes_data_dictionary(full_data_dictionary, data_description)
    return full_data_dictionary


def generate_cpp_file(template_name, destination_dir, file_name_template, filled_templates):
    template = read_template(template_name, templates_dir)
    generated_content = replace_content(template, filled_templates)
    file_name = replace_content(file_name_template, filled_templates)

    if not os.path.exists(destination_dir):
        os.makedirs(destination_dir)

    write_file(path.join(destination_dir, file_name), generated_content)


def generate_per_attribute_cpp_files(data_description, template_name, destination_dir, file_name_template, blacklist, full_data_dictionary):
    template = read_template(template_name, templates_dir)
    for attribute in data_description["attributes"]:
        attribute_dict = {
            **full_data_dictionary,
            **attribute["data_dict"]
        }

        # skip blacklisted attributes
        if blacklist is not None:
            if any(x in attribute["data_dict"]["attribute_flags"] for x in blacklist):
                continue

        generated_content = replace_content(template, attribute_dict)
        file_name = replace_content(file_name_template, attribute_dict)

        if not os.path.exists(destination_dir):
            os.makedirs(destination_dir)

        write_file(path.join(destination_dir, file_name), generated_content)


def generate_files(file_infos, data_description, full_data_dict):
    for file_info in file_infos:
        if "flags" in file_info and "per_attribute" in file_info["flags"]:
            generate_per_attribute_cpp_files(data_description, file_info["template"],
                path.join(working_dir, file_info["output_dir"]),
                file_info["name_template"],
                file_info["blacklist"],
                full_data_dict)
        elif "flags" in file_info and "attribute_list" in file_info["flags"]:
            pass # generated in another function
        elif "flags" in file_info and "list" in file_info["flags"]:
            pass # generated in another function
        else:
            if does_component_pass_filters(full_data_dict, file_info):
                generate_cpp_file(file_info["template"],
                    path.join(working_dir, file_info["output_dir"]),
                    file_info["name_template"],
                    full_data_dict)


def load_component_data_description(file_path):
    component_data = load_json(file_path)

    for attribute in component_data["attributes"]:
        attribute["data_dict"] = get_attribute_data_dictionary(attribute)

    return component_data


def generate_component_list_descriptions(components):
    component_filled_templates = {}
    for component_template in component_templates:
        template = read_template(component_template["name"], templates_dir)
        filled_template = ""

        filtered_components = []
        for component in components:
            if does_component_pass_filters(component, component_template):
                filtered_components.append(component)

        for component in filtered_components:
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


def generate_attribute_list_descriptions(components):
    attribute_filled_templates = {}
    for attribute_template in attribute_templates:
        template = read_template(attribute_template["name"], templates_dir)

        blacklist = None
        if "blacklist" in attribute_template:
            blacklist = attribute_template["blacklist"]

        filled_template = ""
        for component in components:
            full_data_dict = component["data_dict"]
            attributes = component["attributes"]
            is_lastComponent = component is components[len(components) - 1]
            for attribute in attributes:
                # skip blacklisted attributes
                if blacklist is not None:
                    if any(x in attribute["data_dict"]["attribute_flags"] for x in blacklist):
                        continue

                # skip delimiters for the last item
                if is_lastComponent and attribute is attributes[len(attributes) - 1]:
                    delimiter_dict = empty_delimiter_dictionary
                else:
                    delimiter_dict = delimiter_dictionary

                filled_template = filled_template + replace_content(template, {
                    **full_data_dict,
                    **attribute["data_dict"],
                    **delimiter_dict
                })
        attribute_filled_templates[attribute_template["name"]] = filled_template
    return attribute_filled_templates


def generate_all():
    components = []
    raw_components = []
    for file_name in os.listdir(descriptions_dir):
        component = load_component_data_description(path.join(descriptions_dir, file_name))
        full_data_dict = get_full_data_dictionary(component)
        generate_files(files_to_generate, component, full_data_dict)
        component["data_dict"] = full_data_dict
        raw_components.append(component)
        components.append(full_data_dict)

    # generate component lists
    component_filled_templates = generate_component_list_descriptions(components)
    for file_info in files_to_generate:
        if "flags" in file_info and "list" in file_info["flags"]:
            generate_cpp_file(file_info["template"],
                path.join(working_dir, file_info["output_dir"]),
                file_info["name_template"],
                component_filled_templates)

    # generate attributes lists
    attribute_filled_templates = generate_attribute_list_descriptions(raw_components)
    for file_info in files_to_generate:
        if "flags" in file_info and "attribute_list" in file_info["flags"]:
            generate_cpp_file(file_info["template"],
                path.join(working_dir, file_info["output_dir"]),
                file_info["name_template"],
                attribute_filled_templates)


delimiter_dictionary = load_json(path.join(configs_dir, "delimiter_dictionary.json"))

empty_delimiter_dictionary = {key: "" for key in delimiter_dictionary.keys()}

attribute_templates = load_json(path.join(configs_dir, "attribute_templates.json"))

component_templates = load_json(path.join(configs_dir, "component_templates.json"))

files_to_generate = load_json(path.join(configs_dir, "files_to_generate.json"))

attribute_optional_fields = load_json(path.join(configs_dir, "attribute_optional_fields.json"))

generate_all()
