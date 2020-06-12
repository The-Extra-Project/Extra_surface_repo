cmake_minimum_required (VERSION 2.6)

# --------- Includes -----------
include(${CGAL_USE_FILE})
include_directories(${INCLUDE_DIR})
include_directories(SYSTEM ${TINYPLY_INCLUDE_DIR})
include_directories(SYSTEM ${DDTLIB_INCLUDE_DIR})
include_directories(SYSTEM ${ANN_INCLUDE_DIR})
include_directories(SYSTEM ${GRAPHCUT_INCLUDE_DIR})
include_directories(SYSTEM ${CIMG_INCLUDE_DIR})
include_directories(SYSTEM ${QPBO_INCLUDE_DIR})
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


link_directories(${GLOBAL_BUILD_DIR})
set(spark-wasure_SOURCES
  main_wasure_local.cpp
  )
add_executable(wasure-local-exe ${spark-wasure_SOURCES})
set_property(TARGET wasure-local-exe PROPERTY CXX_STANDARD 11)
target_link_libraries(wasure-local-exe
  # intern dependencies
  ddt
  tbmrf
  ${QPBO_LIBRARIES}
  ${WASURE_EXE_LIB_DEP}
  ${HDFS_LIBRARIES}
  ${DOUBLE_CONV_LIBRARIES}
  )

set(spark-wasure_SOURCES
  main_qpbo.cpp
  )
add_executable(opt-qpbo-exe ${spark-wasure_SOURCES})
set_property(TARGET opt-qpbo-exe PROPERTY CXX_STANDARD 11)
target_link_libraries(opt-qpbo-exe
  ${QPBO_LIBRARIES}
  )




include_directories("../../../extern/opengm/include/")
# add_executable(example-grid-potts grid_potts.cxx)
# find_library(RT_LIBRARY rt)
# target_link_libraries(example-grid-potts rt)



# set(spark-wasure_SOURCES
#   main_polyfit.cpp
#   )
# add_executable(polyfit-exe ${spark-wasure_SOURCES})
# set_property(TARGET polyfit-exe PROPERTY CXX_STANDARD 11)
# target_link_libraries(polyfit-exe
#   # intern dependencies
#   ddt
#   tbmrf
#   ${WASURE_EXE_LIB_DEP}
#   ${HDFS_LIBRARIES}
#   ${DOUBLE_CONV_LIBRARIES}
#   )


 
if(CMAKE_VERBOSITY GREATER 0)
message(STATUS "Added target wasure (exe)")
endif(CMAKE_VERBOSITY GREATER 0)
