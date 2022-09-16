import ctypes
import struct

from typing import Type

FLOAT_MAX = struct.unpack('>f', b'\x7f\x7f\xff\xff')[0]
FLOAT_MIN = struct.unpack('>f', b'\xff\x7f\xff\xff')[0]
DOUBLE_MAX = struct.unpack('>d', b'\x7f\xef\xff\xff\xff\xff\xff\xff')[0]
DOUBLE_MIN = struct.unpack('>d', b'\xff\xef\xff\xff\xff\xff\xff\xff')[0]

SIGNED_MAP = \
    {
      ctypes.c_int8: ctypes.c_uint8,
      ctypes.c_int16: ctypes.c_uint16,
      ctypes.c_int32: ctypes.c_uint32,
      ctypes.c_int64: ctypes.c_uint64
    }


class CTypeInfo:
    """ Type info inspector for built-in cxx types. """

    __slots__ = ('val_type',)

    val_type: Type[ctypes.c_int.__base__]
    """ ctypes object representing the corresponding cxx type. """

    def __init__(self, val_type: Type[ctypes.c_int.__base__]):
        """
        Initialize CTypeInfo.
        :param val_type: Child of _ctypes._SimpleCData.
        """
        self.val_type = val_type

    def _get_signed_min(self) -> int:
        """ Gets minimum value of a signed integral type. Requires _is_integral() and _is_signed() to be True. """
        return (-SIGNED_MAP.get(self.val_type)(-1).value) // 2

    def _get_signed_max(self) -> int:
        """ Gets maximum value of a signed integral type. Requires _is_integral() and _is_signed() to be True. """
        return SIGNED_MAP.get(self.val_type)(-1).value // 2

    def _get_unsigned_max(self) -> int:
        """ Gets maximum value of an unsigned integral type. Requires _is_integral() to be True. """
        return self.val_type(-1).value

    def _is_integral(self) -> bool:
        """ Returns True if underlying ctypes type is integral. """
        return isinstance(self.val_type(0).value, int)

    def _is_signed(self) -> bool:
        """ Returns true if underlying type is signed. Requires _is_integral() to be True. """
        return self.val_type(-1).value < 0

    @property
    def size(self) -> int:
        """
        Size of underlying type.
        :return: Size of cxx type in bytes.
        """
        return ctypes.sizeof(self.val_type)

    @property
    def min(self) -> int | float:
        """
        Minimum value the type can hold.
        :return: Minimum value of the cxx type.
        """
        if self._is_integral():
            if self._is_signed():
                return self._get_signed_min()
            else:
                return 0
        else:
            return FLOAT_MIN if self.val_type == ctypes.c_float else DOUBLE_MIN

    @property
    def max(self) -> int | float:
        """
        Maximum value the type can hold.
        :return: Maximum value of the cxx type.
        """
        if self._is_integral():
            if self._is_signed():
                return self._get_signed_max()
            else:
                return self._get_unsigned_max()
        else:
            return FLOAT_MAX if self.val_type == ctypes.c_float else DOUBLE_MAX


# test
if __name__ == '__main__':
    print(CTypeInfo(ctypes.c_uint8).min)
    print(CTypeInfo(ctypes.c_uint8).max)
    print(CTypeInfo(ctypes.c_int16).min)
    print(CTypeInfo(ctypes.c_int16).max)
    print(CTypeInfo(ctypes.c_double).min)
    print(CTypeInfo(ctypes.c_double).max)
    print(CTypeInfo(ctypes.c_float).min)
    print(CTypeInfo(ctypes.c_float).max)
