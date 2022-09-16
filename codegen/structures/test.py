from codegen import generate_file
from cxx_struct import CxxStruct, VersionedBlock, Field
from config import ClientVersions
from builtin_types import *

import inspect

if __name__ == '__main__':
    class TestStruct(CxxStruct):
        __cxx_version_range__ = (ClientVersions.CATA, ClientVersions.MOP)
        __cxx_namespace__ = "ADT::DataStructures"
        __cxx__docstring__ = inspect.cleandoc("""
            /** Test structure.
            *   Continuation line.
            **/
        """)

        __cxx_fields__ = (
            Field(u8, "test_field", default=0, comment="Test comment"),
            Field(u16, "test_field1", default=0, comment="Test comment 1"),
            VersionedBlock(version_range=(ClientVersions.CATA, ClientVersions.CATA),
                           fields=(
                               Field(f64, "test_optional", default=0.0, comment="Double field"),
                           ))
        )

    s = TestStruct()

    print(generate_file((s, )))


