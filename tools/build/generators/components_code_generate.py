#!/usr/bin/python3

from generators.shared_functions import *

def get_base_data_dictionary(data_description):
    return {
        "component_name": data_description["component_name"][0].lower() + data_description["component_name"][1:],
        "component_name_capital": data_description["component_name"],
        "class_name": "%sComponent" % data_description["component_name"],
        "component_tolower" : data_description["component_name"].lower(),
        "component_description": data_description["description"],
        "component_flags": data_description["flags"] if ("flags" in data_description) else []
    }


def get_attribute_data_dictionary(attribute, attribute_optional_fields):
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
        if not any(x in component["component_flags"] for x in template_data["whitelist"]):
            return False

    return True


def append_attributes_data_dictionary(data_dictionary, data_description, attribute_templates, templates_dir, delimiter_dictionary, empty_delimiter_dictionary):
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


def get_full_data_dictionary(data_description, attribute_templates, templates_dir, delimiter_dictionary, empty_delimiter_dictionary):
    full_data_dictionary = get_base_data_dictionary(data_description)
    full_data_dictionary = append_attributes_data_dictionary(full_data_dictionary, data_description, attribute_templates, templates_dir, delimiter_dictionary, empty_delimiter_dictionary)
    return full_data_dictionary


def generate_cpp_file(template_name, destination_dir, file_name_template, filled_templates, templates_dir):
    template = read_template(template_name, templates_dir)
    generated_content = replace_content(template, filled_templates)
    file_name = replace_content(file_name_template, filled_templates)

    if not os.path.exists(destination_dir):
        os.makedirs(destination_dir)

    out_file_path = path.join(destination_dir, file_name)
    write_file(out_file_path, generated_content)
    return out_file_path


def generate_per_attribute_cpp_files(data_description, template_name, destination_dir, file_name_template, blacklist, full_data_dictionary, templates_dir):
    generated_files = []
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

        out_file_path = path.join(destination_dir, file_name)
        write_file(out_file_path, generated_content)
        generated_files.append(out_file_path)

    return generated_files


def generate_files(file_infos, data_description, full_data_dict, output_base_dir, templates_dir):
    generated_files = []
    for file_info in file_infos:
        if "flags" in file_info and "per_attribute" in file_info["flags"]:
            generated_files += generate_per_attribute_cpp_files(data_description, file_info["template"],
                path.join(output_base_dir, file_info["output_dir"]),
                file_info["name_template"],
                file_info["blacklist"],
                full_data_dict,
                templates_dir)
        elif "flags" in file_info and "attribute_list" in file_info["flags"]:
            pass # generated in another function
        elif "flags" in file_info and "list" in file_info["flags"]:
            pass # generated in another function
        else:
            if does_component_pass_filters(full_data_dict, file_info):
                out_file_path = generate_cpp_file(file_info["template"],
                    path.join(output_base_dir, file_info["output_dir"]),
                    file_info["name_template"],
                    full_data_dict,
                    templates_dir)
                generated_files.append(out_file_path)

    return generated_files


def load_component_data_description(file_path, attribute_optional_fields):
    component_data = load_json(file_path)

    for attribute in component_data["attributes"]:
        attribute["data_dict"] = get_attribute_data_dictionary(attribute, attribute_optional_fields)

    return component_data


def generate_component_list_descriptions(components, component_templates, templates_dir, delimiter_dictionary, empty_delimiter_dictionary):
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


def generate_attribute_list_descriptions(components, attribute_templates, templates_dir, delimiter_dictionary, empty_delimiter_dictionary):
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
            is_last_component = component is components[len(components) - 1]
            for attribute in attributes:
                # skip blacklisted attributes
                if blacklist is not None:
                    if any(x in attribute["data_dict"]["attribute_flags"] for x in blacklist):
                        continue

                # skip delimiters for the last item
                if is_last_component and attribute is attributes[len(attributes) - 1]:
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


def get_component_name_from_file_name(file_name):
    name = file_name[:-5] if file_name.endswith('.json') else file_name
    name = name[:-9] if name.endswith('Component') else name
    return name


def generate_all(generator):
    configs_dir = generator["configs_dir"]
    templates_dir = path.join(configs_dir, generator["templates_dir"])
    descriptions_dir = generator["descriptions_dir"]
    output_dir_base = generator["output_dir_base"]

    delimiter_dictionary = load_json(path.join(configs_dir, "delimiter_dictionary.json"))

    empty_delimiter_dictionary = {key: "" for key in delimiter_dictionary.keys()}

    attribute_templates = load_json(path.join(configs_dir, "attribute_templates.json"))

    component_templates = load_json(path.join(configs_dir, "component_templates.json"))

    files_to_generate = load_json(path.join(configs_dir, "files_to_generate.json"))

    attribute_optional_fields = load_json(path.join(configs_dir, "attribute_optional_fields.json"))


    generated_files = []
    components = []
    raw_components = []
    for file_name in os.listdir(descriptions_dir):
        component = load_component_data_description(path.join(descriptions_dir, file_name), attribute_optional_fields)
        component["component_name"] = get_component_name_from_file_name(file_name)
        full_data_dict = get_full_data_dictionary(component, attribute_templates, templates_dir, delimiter_dictionary, empty_delimiter_dictionary)
        generated_files += generate_files(files_to_generate, component, full_data_dict, output_dir_base, templates_dir)
        component["data_dict"] = full_data_dict
        raw_components.append(component)
        components.append(full_data_dict)

    # generate component lists
    component_filled_templates = generate_component_list_descriptions(components, component_templates, templates_dir, delimiter_dictionary, empty_delimiter_dictionary)
    for file_info in files_to_generate:
        if "flags" in file_info and "list" in file_info["flags"]:
            out_file_path = generate_cpp_file(file_info["template"],
                path.join(output_dir_base, file_info["output_dir"]),
                file_info["name_template"],
                component_filled_templates, templates_dir)
            generated_files.append(out_file_path)

    # generate attributes lists
    attribute_filled_templates = generate_attribute_list_descriptions(raw_components, attribute_templates, templates_dir, delimiter_dictionary, empty_delimiter_dictionary)
    for file_info in files_to_generate:
        if "flags" in file_info and "attribute_list" in file_info["flags"]:
            out_file_path = generate_cpp_file(file_info["template"],
                path.join(output_dir_base, file_info["output_dir"]),
                file_info["name_template"],
                attribute_filled_templates, templates_dir)
            generated_files.append(out_file_path)

    return generated_files
