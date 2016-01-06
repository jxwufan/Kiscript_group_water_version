cmake_minimum_required(VERSION 3.3)

project(kiscript)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Werror")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(GLIB REQUIRED)
find_package(Pkgconfig)
pkg_check_modules(GC bdw-gc REQUIRED)
link_directories(${GC_LIBRARY_DIRS})
include_directories(${GC_INCLUDE_DIRS})

set(SOURCE_FILES utils.c glib_utils.c parser.c lexical_parser_utils.c
        lexical_parser.c syntactic_parser_utils.c expression_parser.c
        statement_parser.c function_parser.c program_parser.c syntactic_parser.c
        main.c activation_record.c activation_record.h variable.c variable.h interpreter.c interpreter.h module_tests.c module_tests.h)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)
add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(${CMAKE_PROJECT_NAME} ${GLIB_LIBRARIES} ${GC_LIBRARIES})
