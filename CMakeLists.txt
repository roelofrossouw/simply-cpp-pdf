cmake_minimum_required(VERSION 3.22)
set(SC_MODULE pdf)
project(simply-cpp-${SC_MODULE} HOMEPAGE_URL "https://github.com/roelofrossouw/simply-cpp-pdf" LANGUAGES CXX)
include(cmake/sc_bootstrap.cmake)

option(SC_PDF_BUILD_TESTS "Build simply-cpp-pdf tests" ${PROJECT_IS_TOP_LEVEL})

get_sc_version()
message(STATUS "SimplyCPP ${SC_MODULE} Version: ${SC_VERSION}")

find_or_install_package(sc-core simply-cpp-dev simply-cpp 1.20.0)
# sc::canvas, which pdf implements, and sc::drawing.
find_or_install_package(sc-graphics simply-cpp-graphics-dev simply-cpp-graphics 6.3.0)
# The PDF itself is written by sc-pdflite.
find_or_install_package(sc-pdflite simply-cpp-pdflite-dev simply-cpp-pdflite 4.2.0)

set(SOURCE_OBJECTS)
set(SOURCE_LIBRARIES)
set(SOURCE_LINK_LIBRARIES)

add_sc_object(pdf
        LINK_LIBRARIES sc::sc-core sc::sc-graphics sc::sc-pdflite
        PUBLIC_LINK_LIBRARIES sc::sc-core sc::sc-graphics sc::sc-pdflite)

add_sc_libraries(DESCRIPTION "simply-cpp-pdf")

install_sc_module(PATH_VARS CMAKE_INSTALL_INCLUDEDIR)
install(FILES cmake/SimplyCppFunctions.cmake cmake/sc_bootstrap.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sc-pdf
        COMPONENT development)

if (SC_PDF_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests sc-pdf-test)
endif ()

add_sc_example(sc-pdf-demo)

package_sc_module(
        DEV_DEPENDS "simply-cpp-dev (>= 1.20.0~)" "simply-cpp-graphics-dev (>= 6.3.0~)" "simply-cpp-pdflite-dev (>= 4.2.0~)")
