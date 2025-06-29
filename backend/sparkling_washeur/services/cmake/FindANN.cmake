FIND_PATH(ANN_INCLUDE_DIR ANN/ANN.h 
  ${GLOBAL_BUILD_DIR}/extern/ann_1.1.2/include/
)

FIND_LIBRARY(ANN_LIBRARIES libANN.a
  ${GLOBAL_BUILD_DIR}/extern/ann_1.1.2/lib/
)


IF (ANN_INCLUDE_DIR)
  SET(ANN_INCLUDE_FOUND "YES")
  MESSAGE(STATUS "Found ANN include dir: ${ANN_INCLUDE_DIR}")
ENDIF (ANN_INCLUDE_DIR)

IF (ANN_LIBRARIES)
  SET(ANN_LIB_FOUND "YES")
  MESSAGE(STATUS "Found ANN lib: ${ANN_LIBRARIES}")
ENDIF (ANN_LIBRARIES)

IF(ANN_LIB_FOUND AND ANN_INCLUDE_FOUND)
  SET(ANN_FOUND "YES")
ENDIF(ANN_LIB_FOUND AND ANN_INCLUDE_FOUND)
