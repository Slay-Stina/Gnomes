cmake_minimum_required(VERSION 3.26)
project(Gnomes LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(LIB_NAME ${PROJECT_NAME}_game)
set(FMOD_INC /opt/fmodengine/api/core/inc)
set(FMOD_LIB_DIR /opt/fmodengine/api/core/lib/x86_64)

# Externa .so-filer som ska följa med i en release (riktig fil + soname-länkar)
file(GLOB DLL_FILES ${FMOD_LIB_DIR}/libfmod.so*)

set(DIR_ASSETS_ORIGIN ${CMAKE_SOURCE_DIR}/assets)
set(DIR_ASSETS_DESTINATION $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_ttf REQUIRED)

# Flag executable specific files
set(EXE_EXCLUSIVE ${CMAKE_SOURCE_DIR}/src/main.cpp)

# Flag files that both our executable and shared library need to know about
set(SHARED_SOURCES
        ${CMAKE_SOURCE_DIR}/src/arena.cpp
        ${CMAKE_SOURCE_DIR}/src/input.cpp
        ${CMAKE_SOURCE_DIR}/src/levels.cpp)


set(SHARED_LIB_NAME ${PROJECT_NAME}_common)
# SHARED STATIC LIBRARY
add_library(${SHARED_LIB_NAME} STATIC ${SHARED_SOURCES})
target_include_directories(${SHARED_LIB_NAME} PRIVATE include)
target_link_libraries(${SHARED_LIB_NAME} PRIVATE SDL3::SDL3 SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf)
set_target_properties(${SHARED_LIB_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Alla .cpp-filer i src/
file(GLOB_RECURSE LIB_EXCLUSIVE "src/*.cpp")
# Ta bort exe- och delade-filerna från biblioteksfilerna
list(REMOVE_ITEM LIB_EXCLUSIVE ${EXE_EXCLUSIVE} ${SHARED_SOURCES})

# EXEKVERBAR
add_executable(${PROJECT_NAME} ${EXE_EXCLUSIVE})
target_link_directories(${PROJECT_NAME} PRIVATE ${FMOD_LIB_DIR})
target_link_options(${PROJECT_NAME} PRIVATE "LINKER:--no-as-needed")
target_link_libraries(${PROJECT_NAME} PRIVATE fmod)
set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH ${FMOD_LIB_DIR}
        INSTALL_RPATH "$ORIGIN")
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_include_directories(${PROJECT_NAME} PRIVATE ${FMOD_INC})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 ${SHARED_LIB_NAME})

# DELAT BIBLIOTEK (.so)
add_library(${LIB_NAME} SHARED ${LIB_EXCLUSIVE})
target_include_directories(${LIB_NAME} PRIVATE ${FMOD_INC})
target_link_directories(${LIB_NAME} PRIVATE ${FMOD_LIB_DIR})
target_link_libraries(${LIB_NAME} PRIVATE fmod)
set_target_properties(${LIB_NAME} PROPERTIES
        BUILD_RPATH ${FMOD_LIB_DIR}
        INSTALL_RPATH "$ORIGIN")
target_include_directories(${LIB_NAME} PRIVATE include)
target_link_libraries(${LIB_NAME} PRIVATE ${SHARED_LIB_NAME} SDL3::SDL3 jsoncpp imgui)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${DIR_ASSETS_ORIGIN} ${DIR_ASSETS_DESTINATION}
        VERBATIM
)

install(TARGETS ${PROJECT_NAME} DESTINATION .)
install(TARGETS ${LIB_NAME} LIBRARY DESTINATION . RUNTIME DESTINATION .)
install(FILES ${DLL_FILES} DESTINATION .)
install(DIRECTORY ${CMAKE_BINARY_DIR}/assets DESTINATION .)
