cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(RocketArmV3 
        VERSION 0.1.1 
        DESCRIPTION "Robotic Arm Control System"
        LANGUAGES C)

set(VERSION_SUFFIX "-alpha")
set(FULL_VERSION "${PROJECT_VERSION}${VERSION_SUFFIX}")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set(CURSES_NEED_NCURSES TRUE)
set(CURSES_NEED_WIDE TRUE)
find_package(Curses REQUIRED)

set(SOURCES
    src/main.c
    src/ui_ncurses.c
    src/serial_linux.c
    src/errhandler.c
)

add_executable(robot_arm ${SOURCES})

target_compile_options(robot_arm PRIVATE 
    -Wall 
    -Wextra 
    -Werror 
    -Wpedantic
    -Wshadow
    -Wformat=2
    -O2
    -g
    -fstack-protector-strong
    -D_FORTIFY_SOURCE=2
)

target_include_directories(robot_arm PRIVATE 
    "${CMAKE_SOURCE_DIR}/include"
    ${CURSES_INCLUDE_DIRS}
)

target_link_libraries(robot_arm PRIVATE 
    ${CURSES_LIBRARIES}
)

target_compile_definitions(robot_arm PRIVATE 
    APP_NAME="${PROJECT_NAME}"
    APP_VERSION="${FULL_VERSION}"
    _DEFAULT_SOURCE
)

find_package(Doxygen)
if(DOXYGEN_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/Doxyfile")
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_SOURCE_DIR}/Doxyfile"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Generating documentation with Doxygen..."
        VERBATIM
    )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(robot_arm PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(robot_arm PRIVATE -fsanitize=address)
endif()