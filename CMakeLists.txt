cmake_minimum_required(VERSION 3.5)

project(emu VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0048 NEW)

find_package(PkgConfig)
pkg_search_module(GTEST REQUIRED gtest_main)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Test REQUIRED)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Svg REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Svg REQUIRED)
include_directories("src/include")

add_library(emucomponents STATIC
        src/cpu/alu.cpp src/include/alu.h
        src/cpu/backplane.cpp src/include/backplane.h
        src/include/clock.h src/cpu/clock.cpp
        src/include/component.h src/cpu/component.cpp
        src/include/controller.h src/cpu/controller.cpp
        src/include/systembus.h src/cpu/systembus.cpp
        src/include/harness.h
        src/include/addressregister.h src/cpu/addressregister.cpp
        src/include/memory.h src/cpu/memory.cpp
        src/include/register.h src/cpu/register.cpp
        src/cpu/microcode.inc src/include/registers.h
        src/cpu/iochannel.cpp src/include/iochannel.h
        src/include/opcodes.h)

add_executable(emu
        src/gui/main.cpp

        src/gui/mainwindow.cpp src/gui/mainwindow.h
        src/gui/commands.cpp src/gui/commands.h
        src/gui/cputhread.cpp src/gui/cputhread.h
        src/gui/componentview.cpp src/gui/componentview.h
        src/gui/systembusview.cpp src/gui/systembusview.h
        src/gui/memdump.cpp src/gui/memdump.h
        src/gui/terminal.cpp src/gui/terminal.h

        src/gui/qled.cpp src/gui/qled.h
        src/gui/blockcursorstyle.cpp src/gui/blockcursorstyle.h)

target_link_libraries(
        emu
        PRIVATE
        emucomponents
        Qt${QT_VERSION_MAJOR}::Svg
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Core
)

add_executable(emu_cmdline
        src/cpu/emu.cpp
)

target_link_libraries(emu_cmdline emucomponents)

add_executable(dump_instructions
        src/dump_instructions.cpp src/include/alu.h src/include/opcodes.h
)

target_link_libraries(dump_instructions emucomponents)

add_executable(
        emu_test src/include/opcodes.h
        src/test/clock.cpp src/include/clock.h
        src/test/register.cpp src/include/register.h
        src/test/addressregister.cpp src/include/addressregister.h
        src/test/memory.cpp src/include/memory.h
        src/test/alu.cpp src/include/alu.h
        src/test/controller.cpp src/include/controller.h src/test/controllertest.h
        src/test/stack.cpp
        src/test/arithmetic.cpp
        src/test/jump.cpp
        src/test/io.cpp
        src/test/inout.cpp
        src/test/pushfl.cpp
        src/test/swap.cpp)

target_link_libraries(emu_test emucomponents ${GTEST_LDFLAGS})
target_compile_options(emu_test PUBLIC ${GTEST_CFLAGS})

include(CTest)
add_test(emu_tests emu_test)


