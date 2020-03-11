cmake_minimum_required(VERSION 3.0)
project(abacus)

# version
set(ABACUS_VERSION_MAJOR 0)
set(ABACUS_VERSION_MINOR 1)
set(ABACUS_VERSION_PATCH 0)
configure_file("${PROJECT_SOURCE_DIR}/include/abacus/version.hh.in"
               "${PROJECT_SOURCE_DIR}/include/abacus/version.hh")

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_Release "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_Release} -O2")

# compiling macros
option(ABACUS_DEBUG "debug mode" OFF)
if(ABACUS_DEBUG)
  add_definitions(-DABACUS_DEBUG)
endif(ABACUS_DEBUG)

# header files path
include_directories(include)

# abacus shared libraries
file(GLOB ABACUS_SRC_LIST src/abacus/*.cc)
add_library(abacus SHARED ${ABACUS_SRC_LIST})
target_link_libraries(abacus m)

# a clone of bc
file(GLOB ABACUS_BC_LIST src/bc/*.cc)
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath ${CMAKE_INSTALL_PREFIX}/lib")
add_executable(bc ${ABACUS_BC_LIST})
target_link_libraries(bc abacus)

# installation
install(TARGETS abacus DESTINATION lib)
install(TARGETS bc DESTINATION bin)
install(DIRECTORY include/abacus DESTINATION include FILES_MATCHING PATTERN "*.hh")
install(DIRECTORY docs DESTINATION "${CMAKE_INSTALL_PREFIX}")
