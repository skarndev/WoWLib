from cxx_struct import CxxStruct

from typing import Tuple


def generate_file(structs: Tuple[CxxStruct]) -> str:
    code = "/*\n" \
           "This file is autogenerated by codegen script. Do not edit manually, changes will be overriden.\n" \
           "Structures are defined in Python files under \"/codegen/structures/definitions/\".\n" \
           "*/\n" \
           "#include <IO/Common.hpp>\n" \
           "#include <Utils/Meta/Reflection.hpp>\n\n"

    cur_namespace = None
    for struct in sorted(structs, key=lambda x: x.__cxx_namespace__):
        if cur_namespace is None:
            cur_namespace = struct.__cxx_namespace__

            code += f"namespace {cur_namespace}\n{{\n\n"
        elif cur_namespace != struct.__cxx_namespace__:

            code += f"}}\n\nnamespace {cur_namespace} {{\n"
            cur_namespace = struct.__cxx_namespace__

        code += struct.generate_code()

    code += f"}} // namespace {cur_namespace}\n\n"

    # generate reflection descriptors

    for struct in structs:
        code += struct.generate_reflection_code()

    return code


