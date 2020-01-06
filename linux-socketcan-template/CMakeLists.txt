
# ==============================================================================
# cmake setup
# ==============================================================================

cmake_minimum_required(VERSION 3.0)
project(rubi_linux_template)

include(CTest)

enable_testing()

# ==============================================================================
# compiler setup
# ==============================================================================

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_compile_options(-fno-limit-debug-info)
endif()
add_compile_options(-Wall)

include_directories(inc)
include_directories(../rubi/inc)
include_directories(../rubi/src_linux_socketcan)

# ==============================================================================
# external dependencies
# ==============================================================================

find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)
find_package(Threads)

# ==============================================================================
# resources
# ==============================================================================

# ==============================================================================
# workarounds
# ==============================================================================

# ==============================================================================
# build config
# ==============================================================================

set(SRCS_NOMAIN 
  ../rubi/src/rubi_autodefs.c
  ../rubi/src/rubi.c 
  ../rubi/src/rubi_capabilities_enumerator.c 
  ../rubi/src/rubi_protocol.c 

  ../rubi/src_linux_socketcan/rubi_can.cpp
  ../rubi/src_linux_socketcan/socketcan.cpp
  ../rubi/src_linux_socketcan/rubi_auxiliary.cpp

  ../rubi/inc/rubi_can.h
  ../rubi/inc/rubi.h
  ../rubi/inc/rubi_auxiliary.h
  ../rubi/inc/rubi_capabilities_enumerator.h
  ../rubi/inc/rubi_protocol.h

  ../rubi/src_linux_socketcan/socketcan.h
)

add_library (${PROJECT_NAME} STATIC ${SRCS_NOMAIN})

target_link_libraries(${PROJECT_NAME}
  ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} 
)
target_link_libraries(${PROJECT_NAME} ${BOOST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )

add_executable(board src/main.c )
target_link_libraries(board ${PROJECT_NAME})
# ==============================================================================
# tests
# ==============================================================================
file(GLOB TEST_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} tests/*.cpp)

foreach(testSrc ${TEST_SRCS})
  get_filename_component(testName ${testSrc} NAME_WE)
  set(testName "test_${testName}")
  add_executable(${testName} "${testSrc}")
  target_link_libraries(${testName} ${PROJECT_NAME})

  set_target_properties(${testName} PROPERTIES 
      RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/tests)

  add_test(NAME ${testName} 
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMAND ${CMAKE_CURRENT_BINARY_DIR}/tests/${testName} )
endforeach(testSrc)