cmake_minimum_required(VERSION 4.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-psabi)
endif()

find_package(PkgConfig REQUIRED)

pkg_check_modules(FONTCONFIG REQUIRED fontconfig)
include_directories(${FONTCONFIG_INCLUDE_DIRS})
link_directories(${FONTCONFIG_LIBRARY_DIRS})

pkg_check_modules(FREETYPE2 REQUIRED freetype2)
include_directories(${FREETYPE2_INCLUDE_DIRS})
link_directories(${FREETYPE2_LIBRARY_DIRS})

pkg_check_modules(LIBJPEG REQUIRED libjpeg)
include_directories(${LIBJPEG_INCLUDE_DIRS})
link_directories(${LIBJPEG_LIBRARY_DIRS})

# git remote add pdf-writer https://github.com/galkahana/PDF-Writer.git
# git stash; git subtree add --prefix=third_party/pdf-writer pdf-writer main --squash; git stash pop
# (Update with:   git fetch pdf-writer main; git stash; git subtree pull --prefix=third_party/pdf-writer pdf-writer main --squash; git stash pop)
set(USE_BUNDLED FALSE)
set(USE_UNBUNDLED_FALLBACK_BUNDLED TRUE)
add_subdirectory(third_party/pdf-writer pdf-writer EXCLUDE_FROM_ALL)

# git remote add sc https://github.com/roelofrossouw/simply-cpp.git
# git stash; git subtree add --prefix=third_party/sc sc main --squash; git stash pop
add_subdirectory(third_party/sc sc EXCLUDE_FROM_ALL)

# git remote add tinyxml2 https://github.com/leethomason/tinyxml2.git
# git stash; git subtree add --prefix=third_party/tinyxml2 tinyxml2 master --squash; git stash pop
add_subdirectory(third_party/tinyxml2 EXCLUDE_FROM_ALL)

#if(NOT TARGET sc)
#    add_subdirectory(third_party/sc sc EXCLUDE_FROM_ALL)
#endif()
#
#project(sc-svg-parser)
#add_library(${PROJECT_NAME} STATIC  )
#target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include")
#target_link_libraries(${PROJECT_NAME} PUBLIC sc )
#

project(sc-pdf)
add_library(${PROJECT_NAME} SHARED
        src/pdf.cpp
        src/pdf_rect.cpp
        src/font_spec.cpp
        src/font_collection.cpp
        src/svg_parser.cpp src/svg2pdf.cpp src/svg_element.cpp
        include/pdf.h
        include/pdf_rect.h
        include/font_spec.h
        include/font_collection.h
        include/svg_parser.h include/svg2pdf.h include/svg_element.h)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(${PROJECT_NAME} PUBLIC sc-shared tinyxml2::tinyxml2 PDFHummus::PDFWriter fontconfig)

project(test_pdf)
add_executable(${PROJECT_NAME} test/pdf.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE sc-pdf)
