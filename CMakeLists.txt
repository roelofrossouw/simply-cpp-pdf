cmake_minimum_required(VERSION 3.22)
set(SC_MODULE pdf)
project(simply-cpp-${SC_MODULE} LANGUAGES CXX)
include(cmake/sc_bootstrap.cmake)

option(SC_PDF_BUILD_TESTS "Build simply-cpp-pdf tests" ${PROJECT_IS_TOP_LEVEL})

get_sc_version()
message(STATUS "SimplyCPP ${SC_MODULE} Version: ${SC_VERSION}")

find_or_install_package(sc-core simply-cpp-dev simply-cpp 1.6.0)
# sc::canvas, which pdf implements, and sc::svg.
find_or_install_package(sc-graphics simply-cpp-graphics-dev simply-cpp-graphics 1.0.0)
# PDFHummus isn't in Ubuntu or Homebrew: sc-pdfhummus packages it for both.
find_or_install_package(PDFHummus simply-cpp-pdfhummus simply-cpp-pdfhummus)
# Through pkg-config, linked by path as sc-redis does hiredis, so the exported targets don't need
# pkg-config.
config_or_install_package(fontconfig libfontconfig-dev fontconfig)
foreach (library fontconfig)
    string(TOUPPER "SC_PDF_${library}_LIBRARY" cached)
    if (${cached} AND NOT EXISTS "${${cached}}")
        message(STATUS "${library}: the cached library location is gone, looking again")
        unset(${cached} CACHE)
    endif ()
    find_library(${cached} NAMES ${library} HINTS ${${library}_LIBRARY_DIRS} REQUIRED)
endforeach ()

set(SOURCE_OBJECTS)
set(SOURCE_LIBRARIES)
set(SOURCE_LINK_LIBRARIES)

# Everything PDFHummus or fontconfig is in src/, out of the installed headers.
add_sc_object(pdf
        SOURCES src/font_collection.cpp src/font_collection.h
        INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
        LINK_LIBRARIES sc::sc-core sc::sc-graphics PDFHummus::PDFWriter PkgConfig::fontconfig
        PUBLIC_LINK_LIBRARIES sc::sc-core sc::sc-graphics PDFHummus::PDFWriter ${SC_PDF_FONTCONFIG_LIBRARY})

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

# simply-cpp-pdfhummus is installed by hand, not found by dpkg-shlibdeps; PDFHummus's CMake package
# looks for zlib, FreeType, libjpeg, libtiff, libpng and OpenSSL when a consumer configures.
package_sc_module(
        DEPENDS simply-cpp-pdfhummus
        DEV_DEPENDS "simply-cpp-dev (>= 1.6.0~)" "simply-cpp-graphics-dev (>= 1.0.0~)" simply-cpp-pdfhummus libfontconfig-dev
        zlib1g-dev libfreetype-dev libjpeg-dev libtiff-dev libpng-dev libssl-dev)
