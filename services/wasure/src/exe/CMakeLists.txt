cmake_minimum_required (VERSION 2.6)

# --------- Includes -----------
include(${CGAL_USE_FILE})
include_directories(${INCLUDE_DIR})
include_directories(SYSTEM ${TINYPLY_INCLUDE_DIR})
include_directories(SYSTEM ${DDTLIB_INCLUDE_DIR})
include_directories(SYSTEM ${ANN_INCLUDE_DIR})
include_directories(SYSTEM ${GRAPHCUT_INCLUDE_DIR})
include_directories(SYSTEM ${CIMG_INCLUDE_DIR})
include_directories(SYSTEM ${GLOBAL_BUILD_DIR})
include_directories(SYSTEM ${HDFS_INCLUDE_DIR})
include_directories(SYSTEM ${DOUBLE_CONV_INCLUDE_DIR})
include_directories(../../../ddt/include/)
find_package(ZLIB)

link_directories(${GLOBAL_BUILD_DIR})
set(spark-wasure_SOURCES
  main_wasure_stream.cpp
  )
add_executable(wasure-stream-exe ${spark-wasure_SOURCES})
set_property(TARGET wasure-stream-exe PROPERTY CXX_STANDARD 11)
target_link_libraries(wasure-stream-exe
  # intern dependencies
  ddt
  tbmrf
  ${WASURE_EXE_LIB_DEP}
  ${HDFS_LIBRARIES}
  ${DOUBLE_CONV_LIBRARIES}
  )


 
if(CMAKE_VERBOSITY GREATER 0)
message(STATUS "Added target wasure (exe)")
endif(CMAKE_VERBOSITY GREATER 0)
