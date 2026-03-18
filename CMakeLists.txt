if (APPLE)
    cmake_minimum_required(VERSION 3.30)
else ()
    cmake_minimum_required(VERSION 3.10)
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(SC_VERSION 1.0.2)

## External Dependencies ##
# git remote add base64 https://github.com/tobiaslocker/base64.git
# git subtree add --prefix=third_party/base64 base64 master --squash
# (Update with:  git fetch base64 master; git stash; git subtree pull --prefix=third_party/base64 base64 master --squash; git stash pop)
set(BASE64_ENABLE_TESTING OFF)
add_subdirectory(third_party/base64 base64 EXCLUDE_FROM_ALL)

# git remote add lunasvg https://github.com/sammycage/lunasvg.git
# git subtree add --prefix=third_party/lunasvg lunasvg master --squash
# (Update with:   git fetch lunasvg master; git stash; git subtree pull --prefix=third_party/lunasvg lunasvg master --squash; git stash pop)
set(LUNASVG_BUILD_STATIC ON)
set(LUNASVG_BUILD_EXAMPLES OFF)
add_subdirectory(third_party/lunasvg lunasvg EXCLUDE_FROM_ALL)

## LIBRARIES ##
project(sc-base64)
add_library(${PROJECT_NAME} STATIC src/base64.cpp include/base64.h)
set_target_properties(${PROJECT_NAME} PROPERTIES EXCLUDE_FROM_ALL ON)
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(${PROJECT_NAME} base64)

project(sc-svg2png)
add_library(${PROJECT_NAME} STATIC src/svg2png.cpp include/svg2png.h)
set_target_properties(${PROJECT_NAME} PROPERTIES EXCLUDE_FROM_ALL ON)
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(${PROJECT_NAME} PUBLIC lunasvg)

project(sc-rest)
add_library(${PROJECT_NAME} STATIC src/rest.cpp include/rest.h)
set_target_properties(${PROJECT_NAME} PROPERTIES EXCLUDE_FROM_ALL ON)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(${PROJECT_NAME} PUBLIC curl)

project(sc-timer)
add_library(${PROJECT_NAME} STATIC src/timer.cpp include/timer.h)
set_target_properties(${PROJECT_NAME} PROPERTIES EXCLUDE_FROM_ALL ON)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/include")


set(SOURCE_OBJECTS
        src/sc.cpp sc.h
        $<TARGET_OBJECTS:sc-base64>
        $<TARGET_OBJECTS:sc-svg2png>
        $<TARGET_OBJECTS:sc-rest>
        $<TARGET_OBJECTS:sc-timer>
)

set(USED_LIBRARIES
        base64
        lunasvg
        curl
)

# Combined static libary for easy usage
project(sc VERSION ${SC_VERSION} DESCRIPTION "simply-cpp common static library")
add_library(${PROJECT_NAME} STATIC ${SOURCE_OBJECTS})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${USED_LIBRARIES})

# Combined shared libary for easy usage
project(sc-shared VERSION ${SC_VERSION} DESCRIPTION "simply-cpp common shared library")
add_library(${PROJECT_NAME} SHARED ${SOURCE_OBJECTS})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${USED_LIBRARIES})

## TESTS ##

project(test_base64)
add_executable(${PROJECT_NAME} test/base64.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE sc)

project(test_rest)
add_executable(${PROJECT_NAME} test/rest.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE sc)

project(test_svg2png)
add_executable(${PROJECT_NAME} test/svg2png.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE sc)

project(test_timer)
add_executable(${PROJECT_NAME} test/timer.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE sc)

