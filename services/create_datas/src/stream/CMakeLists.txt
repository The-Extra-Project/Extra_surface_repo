cmake_minimum_required (VERSION 2.6)

# --------- Includes -----------
include(${CGAL_USE_FILE})
include_directories(${INCLUDE_DIR})
include_directories(SYSTEM ${TINYPLY_INCLUDE_DIR})
#include_directories(SYSTEM ${PUGIXML_INCLUDE_DIR})
include_directories(SYSTEM ${DDTLIB_INCLUDE_DIR})
include_directories(SYSTEM ${ANN_INCLUDE_DIR})
include_directories(SYSTEM ${GRAPHCUT_INCLUDE_DIR})
include_directories(SYSTEM ${CIMG_INCLUDE_DIR})
include_directories(SYSTEM ${GLOBAL_BUILD_DIR})
include_directories(SYSTEM ${DOUBLE_CONV_INCLUDE_DIR})
include_directories(../../../ddt-spark/include)

link_directories(${GLOBAL_BUILD_DIR})
set(create_data_SOURCES
  main_create_data.cpp
  )
add_executable(create_data-stream-exe ${create_data_SOURCES})
set_property(TARGET create_data-stream-exe PROPERTY CXX_STANDARD 11)
target_link_libraries(create_data-stream-exe
  # intern dependencies
  ddt
  ${CREATE_DATA_EXE_LIB_DEP}
    ${CIMG_LIB}
  )


 
if(CMAKE_VERBOSITY GREATER 0)
message(STATUS "Added target create_data (exe)")
endif(CMAKE_VERBOSITY GREATER 0)
