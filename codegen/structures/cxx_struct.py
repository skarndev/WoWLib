from cxx_type import CxxType
from config import ClientVersions
from builtin_types import *

from typing import Tuple, Type, Union, Set

import inspect


class Field:
    """
    Field descriptor to be used in CxxStruct.
    """
    __slots__ = ('value_type', 'name', 'default', 'comment', 'bit')

    value_type: Type[CxxType] | str
    """ Cxx type of field's value. """

    name: str
    """ Name of field. """

    default: Tuple[Union[int, float]] | Union[int, float, None]
    """ Default value of field. Can either be singular, or tuple (in that case results into aggregate initializer)."""

    comment: str
    """ Optional commentary string that will be prefixed with Doxygen-styled comment automatically. """

    bit: int | None
    """ Bit value used in order to define bitfields (optional). """

    def __init__(self
                 , value_type: CxxType | str
                 , name: str
                 , bit = None
                 , default: Tuple[Union[int, float]] | Union[int, float, None] = None
                 , comment=""):
        """
        Initialize Field class.
        :param value_type: Type of field.
        :param name: Name of field.
        :param bit: Optional bit-size of a field.
        :param default: Default value of a field.
        :param comment: Optional comment for a field.
        """

        self.value_type = value_type
        self.name = name
        self.default = default
        self.comment = comment
        self.bit = bit

    def generate_code(self) -> str:
        """
        Generate C++ code for the field.
        :return: C++ code string.
        """

        line = f"{self.value_type.full_typename} {self.name} "

        if self.bit is not None:
            line += f": {self.bit} "

        if self.default is not None:

            if isinstance(self.default, tuple):
                value = ", ".join(val for val in self.default)
                line += f"= {{ { value } }}"
            else:
                line += f"= {self.default}"

        line += '; '

        if self.comment:
            line += f"///> {self.comment}"

        line += '\n'

        return line

    def generate_default_init_code(self) -> str | None:
        """
        Generate code for default initializer for this field.
        :return: C++ code string.
        """

        if self.default is None:
            return None

        line = f"ret.{self.name} = "

        if isinstance(self.default, tuple):
            value = ", ".join(val for val in self.default)
            line += f"{{ {value} }}"
        else:
            line += str(self.default)

        line += ';\n'

        return line


class VersionedBlock:
    """
    Used to defined versioned field blocks for structures.
    """
    __slots__ = ('version_range', 'fields')
    version_range: Tuple[int, int]
    """ Range of versions this block will generate fields for. """

    fields: Tuple[Union[Field, 'VersionedBlock']]
    """ Fields of this block. """

    def __init__(self
                 , *
                 , version_range: Tuple[ClientVersions, ClientVersions]
                 , fields: Tuple[Union[Field, 'VersionedBlock']]):
        """
        Initializes VersionBlock
        :param version_range: Range of versions this block will generate fields for.
        :param fields: Fields of this block.
        """

        self.version_range = version_range
        self.fields = fields

    def get_fields(self, client_version: ClientVersions) -> Tuple[Field]:
        """
        Returns a tuple of fields used by this block given a specific version.
        :param client_version: Version of game client.
        :return: Tuple of fields.
        """

        if not (self.version_range[0] <= client_version <= self.version_range[1]):
            return tuple()

        final_fields = []
        for entry in self.fields:
            if isinstance(entry, Field):
                final_fields.append(entry)
            else:
                final_fields.extend(entry.get_fields(client_version))

        return tuple(final_fields)


class CxxStruct(CxxType):
    """
    When inherited from gives derived class the functionality to generate C++ code struct definitions.
    Define the following attributes, prefixed and postfixed with __ to define struct's content.
    """

    __cxx_version_range__: Tuple[ClientVersions, ClientVersions] | None = ()
    """ Range of versions this struct has specialization for. If none, struct is not versioned. """

    __cxx_fields__: Tuple[Field | VersionedBlock] = ()
    """ Fields of the structure. """

    __cxx_namespace__: str = ""
    """ Namespace this structure is defined in . """

    __cxx__docstring__: str = ""
    """ Docstring (optional). """

    _is_versioned: bool

    def __init__(self):
        super().__init__(self.__class__.__name__, namespace=self.__cxx_namespace__)
        self.is_versioned = self.__cxx_version_range__ \
            or any(isinstance(f, VersionedBlock) for f in self.__cxx_fields__)

    def is_version_enabled(self, version: ClientVersions) -> bool:
        """
        Check if structure exists for the given version.
        :param version: Game client version.
        :return: True if version is enabled, else False.
        """
        return not (self.__cxx_version_range__ is not None
                    and (version < self.__cxx_version_range__[0] or version > self.__cxx_version_range__[1]))

    def generate_code(self) -> str:
        """
        Generate code for structure definition.
        :return: C++ code string.
        """

        code = ""

        if self.__cxx__docstring__:
            code += self.__cxx__docstring__
            code += '\n'

        # versioned structure
        if self.is_versioned:
            # generate template declaration

            code += f"template<IO::Common::ClientVersions client_version>\nstruct {self.__class__.__name__};\n\n"

            for version in ClientVersions:
                if not self.is_version_enabled(version):
                    continue

                code += f"template<>\nstruct {self.__class__.__name__}" \
                        f"<IO::Common::ClientVersions::{version.name}>\n{{\n"

                fields = []
                for field_or_block in self.__cxx_fields__:
                    if isinstance(field_or_block, Field):
                        fields.append(field_or_block)
                    else:
                        fields.extend(field_or_block.get_fields(version))

                for field in fields:
                    code += f"  {field.generate_code()}"

                # generate default initializer
                code += f"\n  static {self.__class__.__name__} DefaultInit()\n  {{\n"
                code += f"    auto ret = {self.__class__.__name__}{{}};\n"
                for field in fields:
                    if field.default is None:
                        continue

                    code += f"    {field.generate_default_init_code()}"

                code += f"    return ret;\n"

                code += '  }\n};\n\n'

        # static structure
        else:
            code += f"struct {self.__class__.__name__}{{\n"

            for field in self.__cxx_fields__:
                code += f"  {field.generate_code()}"

            # generate default initializer
            code += f"\n  static {self.__class__.__name__} DefaultInit()\n  {{\n"
            code += f"    auto ret = {self.__class__.__name__}{{}};\n"
            for field in self.__cxx_fields__:
                if field.default is None:
                    continue

                code += f"    {field.generate_default_init_code()}"

            code += f"    return ret;\n"

            code += '  }\n};\n\n'

        return code

    def generate_reflection_code(self) -> str:
        """
        Generate reflection descriptor code.
        :return: C++ code string.
        """
        if self.is_versioned:
            code = ""
            for version in ClientVersions:
                if not self.is_version_enabled(version):
                    continue

                field_names = []

                for field_or_block in self.__cxx_fields__:
                    if isinstance(field_or_block, Field):
                        field_names.append(field_or_block.name)
                    else:
                        field_names.extend(field.name for field in field_or_block.get_fields(version))

                field_names = "\n  , ".join(name for name in field_names)
                code += f"REFLECTION_DESCRIPTOR(\n  {self.full_typename}<IO::Common::ClientVersions::{version.name}>\n"\
                        f"  , { field_names}\n);\n\n"

            return code
        else:
            field_names = "  \n, ".join(field.name for field in self.__cxx_fields__)

        return f"REFLECTION_DESCRIPTOR(\n  {self.full_typename}\n, {field_names}\n);\n\n"

    @property
    def type_headers(self) -> Set[str]:
        """
        :return: Set of headers used by nested types.
        """

        headers = set()
        for entry in self.__cxx_fields__:
            if isinstance(entry, Field):
                if isinstance(entry.value_type, CxxType):
                    headers += entry.value_type.headers
            else:
                for version in ClientVersions:
                    if not self.is_version_enabled(version):
                        continue

                    for field in entry.get_fields(version):
                        if isinstance(field, Field):
                            headers += field.value_type.headers

        headers += super().headers
        return headers

