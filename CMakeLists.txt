cmake_minimum_required(VERSION 3.26)
project(Gnomes LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(LIB_NAME ${PROJECT_NAME}_game)

set(DIR_ASSETS_ORIGIN ${CMAKE_SOURCE_DIR}/assets)
set(DIR_ASSETS_DESTINATION $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)

# Flag executable specific files
set(EXE_EXCLUSIVE ${CMAKE_SOURCE_DIR}/src/main.cpp
        src/main.cpp
)

# Flag files that both our executable and shared library need to know about
set(SHARED_SOURCES
        ${CMAKE_SOURCE_DIR}/src/arena.cpp
        ${CMAKE_SOURCE_DIR}/src/input.cpp
        ${CMAKE_SOURCE_DIR}/src/levels.cpp)


set(SHARED_LIB_NAME ${PROJECT_NAME}_common)
# SHARED STATIC LIBRARY
add_library(${SHARED_LIB_NAME} STATIC ${SHARED_SOURCES})
target_include_directories(${SHARED_LIB_NAME} PRIVATE include)
target_link_libraries(${SHARED_LIB_NAME} PRIVATE SDL3::SDL3 SDL3_image::SDL3_image)
set_target_properties(${SHARED_LIB_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Alla .cpp-filer i src/
file(GLOB_RECURSE LIB_EXCLUSIVE "src/*.cpp")
# Ta bort exe- och delade-filerna från biblioteksfilerna
list(REMOVE_ITEM LIB_EXCLUSIVE ${EXE_EXCLUSIVE} ${SHARED_SOURCES})

# EXEKVERBAR
add_executable(${PROJECT_NAME} ${EXE_EXCLUSIVE})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 ${SHARED_LIB_NAME})

# DELAT BIBLIOTEK (.so)
add_library(${LIB_NAME} SHARED ${LIB_EXCLUSIVE})
target_include_directories(${LIB_NAME} PRIVATE include)
target_link_libraries(${LIB_NAME} PRIVATE SDL3::SDL3 ${SHARED_LIB_NAME} jsoncpp imgui)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${DIR_ASSETS_ORIGIN} ${DIR_ASSETS_DESTINATION}
        VERBATIM
)
