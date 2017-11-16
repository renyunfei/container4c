set(CMAKE_MACOSX_RPATH 1)

set(LIB_SRC
    list_test.c)

set(LIB_INCLUDE
    list.h)

add_executable(list_test ${LIB_SRC} ${LIB_INCLUDE})
