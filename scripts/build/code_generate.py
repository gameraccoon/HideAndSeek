#!/usr/bin/python3

import os

import generators.components_code_generate
import generators.enums_code_generate
import generators.gather_string_ids

generated_files = []
generated_files += generators.components_code_generate.generate_all()
generated_files += generators.enums_code_generate.generate_all()
generated_files += generators.gather_string_ids.generate_all()

# path relative to the script
previously_generated_files_file_path_relative = "../temp_data/previously_generated_files.txt"

previously_generated_files_file_path = os.path.join(os.path.dirname(__file__), previously_generated_files_file_path_relative)

# remove old generated files
if os.path.isfile(previously_generated_files_file_path):
    with open(previously_generated_files_file_path, "r") as f:
        old_generated_files = f.read().splitlines()

    if generated_files != old_generated_files:
        generated_files_set = set(generated_files)
        old_generated_files_set = set(old_generated_files)

        removed_files = old_generated_files_set - generated_files_set
        for file in removed_files:
            os.remove(file)
            print("Removed file: " + file)

        with open(previously_generated_files_file_path, "w") as f:
            f.write("\n".join(generated_files))
else:
    with open(previously_generated_files_file_path, "w") as f:
        f.write("\n".join(generated_files))
