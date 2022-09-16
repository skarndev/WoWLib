from cxx_type import CxxNumericType
from utils import CTypeInfo

import ctypes

u8 = CxxNumericType('uint8_t'
                    , CTypeInfo(ctypes.c_uint8).min
                    , CTypeInfo(ctypes.c_uint8).max)

u16 = CxxNumericType('uint16_t'
                     , CTypeInfo(ctypes.c_uint16).min
                     , CTypeInfo(ctypes.c_uint16).max)

u32 = CxxNumericType('uint32_t'
                     , CTypeInfo(ctypes.c_uint32).min
                     , CTypeInfo(ctypes.c_uint32).max)

u64 = CxxNumericType('uint64_t'
                     , CTypeInfo(ctypes.c_uint64).min
                     , CTypeInfo(ctypes.c_uint64).max)

i8 = CxxNumericType('int8_t'
                    , CTypeInfo(ctypes.c_int8).min
                    , CTypeInfo(ctypes.c_int8).max)

i16 = CxxNumericType('int16_t'
                     , CTypeInfo(ctypes.c_int16).min
                     , CTypeInfo(ctypes.c_int16).max)

i32 = CxxNumericType('int32_t'
                     , CTypeInfo(ctypes.c_int32).min
                     , CTypeInfo(ctypes.c_int32).max)

i64 = CxxNumericType('int64_t'
                     , CTypeInfo(ctypes.c_int64).min
                     , CTypeInfo(ctypes.c_int64).max)


f32 = CxxNumericType('float'
                     , CTypeInfo(ctypes.c_float).min
                     , CTypeInfo(ctypes.c_float).max
                     , headers=set()
                     , namespace='')

f64 = CxxNumericType('double'
                     , CTypeInfo(ctypes.c_double).min
                     , CTypeInfo(ctypes.c_double).max
                     , headers=set()
                     , namespace='')

__all__ = ('u8', 'u16', 'u32', 'u64', 'i8', 'i16', 'i32', 'i64', 'f32', 'f64')

# test
if __name__ == '__main__':
    types = (u8, u16, u32, u64, i8, i16, i32, i64, f32, f64)

    for t in types:
        print(f"Type: {t.full_typename}, size: {t.size}, min: {t.min}, max: {t.max}")

