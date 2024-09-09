import os


allowed_extensions = [".cpp", ".h", ".hpp", ".c", ".cc", ".hh", ".cxx", ".hxx"]
ignored_extensions = [".generated.cpp", ".generated.h"]


def is_valid_file(file):
    if any(file.endswith(ext) for ext in allowed_extensions):
        if not any(file.endswith(ext) for ext in ignored_extensions):
            return True

    return False


def format_one_file(clang_format_path, file):
    with open(file, "r") as f:
        original_content = f.read()

    os.system("clang-format -i -style=file:"+clang_format_path+" " + file)

    with open(file, "r") as f:
        formatted_content = f.read()

    if original_content != formatted_content:
        print("Reformatted " + file)
        return True

    return False


def format_for_all_arguments():
    files = os.sys.argv[1:]

    if not files:
        print("Usage: run-clang-format.py <source0> <source1> ...")
        os.sys

    clang_format_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../.clang-format"))
    if not os.path.exists(clang_format_path):
        print("Could not find .clang-format file at " + clang_format_path)
        os.sys.exit(1)

    changed_files_count = 0

    for file in files:
        if os.path.isfile(file) and is_valid_file(file) and format_one_file(clang_format_path, file):
            changed_files_count += 1
        elif os.path.isdir(file):
            for root, _, files in os.walk(file):
                for file in files:
                    if is_valid_file(file) and format_one_file(clang_format_path, os.path.join(root, file)):
                        changed_files_count += 1

    if changed_files_count > 0:
        print("Reformatted " + str(changed_files_count) + " files")


format_for_all_arguments()
