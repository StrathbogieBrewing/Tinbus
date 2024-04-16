cmake_minimum_required(VERSION 3.18)

set(AVR_UPLOADTOOL avrdude)
set(AVR_UPLOADTOOL_BAUDRATE 19200)
set(AVR_UPLOADTOOL_PORT /dev/ttySTK500V2)
set(AVR_PROGRAMMER stk500v2)

set(AVR_MCU_TYPE attiny2313)
# set(AVR_MCU_TYPE attiny4313)
set(AVR_MCU_SPEED "921600UL")

set(AVR_L_FUSE 0xDD)
set(AVR_H_FUSE 0xDB)
set(AVR_E_FUSE 0xFF)

add_definitions("-Os")

include(../cmake/avr-gcc.cmake)

project(jsa C ASM)

include(../cmake/check-git.cmake)
CheckGitSetup()

include_directories(../library/utils)
include_directories(../library/modbus)

add_avr_executable(${PROJECT_NAME} 
    main.c
    asm.S
    ${CMAKE_BINARY_DIR}/generated/git_version.c
    ../library/utils/format.c
    ../library/modbus/mb_crc.c
)
