set(CMAKE_MACOSX_RPATH 1)

set(LIB_SRC
    list.c)

set(LIB_INCLUDE
    list.h)

ADD_LIBRARY(container SHARED ${LIB_SRC} ${LIB_INCLUDE})
