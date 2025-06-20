SET(DOUBLE_CONV_INCLUDE_DIR ${EXTERN_PROJECT_SRC_DIR}/double-conv/double-conversion/)

FIND_PATH(DOUBLE_CONV_INCLUDE_DIR string-to-double.h double-to-string.h
  ${EXTERN_PROJECT_SRC_DIR}/double-conv/double-conversion/
)

FIND_LIBRARY(DOUBLE_CONV_LIBRARIES libdouble-conversion.a
  ${EXTERN_PROJECT_SRC_DIR}/double-conv/build/
)

IF (DOUBLE_CONV_INCLUDE_DIR)
  SET(DOUBLE_CONV_INCLUDE_FOUND "YES")
  MESSAGE(STATUS "Found DOUBLE_CONV include dir: ${DOUBLE_CONV_INCLUDE_DIR}")
ELSE(DOUBLE_CONV_INCLUDE_DIR)
 MESSAGE(STATUS "/!\ /!\ Not Found : DOUBLE_CONV include dir ")
ENDIF (DOUBLE_CONV_INCLUDE_DIR)

IF (DOUBLE_CONV_LIBRARIES)
  SET(DOUBLE_CONV_LIB_FOUND "YES")
  MESSAGE(STATUS "Found DOUBLE_CONV lib: ${DOUBLE_CONV_LIBRARIES}")
ELSE(DOUBLE_CONV_LIBRARIES) 
 MESSAGE(STATUS "/!\\ /!\\ Not Found : DOUBLE_CONV libs in : ${GLOBAL_BUILD_DIR}/lib/")
ENDIF (DOUBLE_CONV_LIBRARIES)


IF(DOUBLE_CONV_LIB_FOUND AND DOUBLE_CONV_INCLUDE_FOUND)
  SET(DOUBLE_CONV_FOUND "YES")
ENDIF(DOUBLE_CONV_LIB_FOUND AND DOUBLE_CONV_INCLUDE_FOUND)
