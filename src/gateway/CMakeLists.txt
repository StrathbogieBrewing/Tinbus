cmake_minimum_required(VERSION 3.18)

set(AVR_UPLOADTOOL avrdude)
set(AVR_UPLOADTOOL_BAUDRATE 19200)
set(AVR_UPLOADTOOL_PORT /dev/ttySTK500V2)
set(AVR_PROGRAMMER stk500v2)

set(AVR_MCU_TYPE atmega324p)
set(AVR_MCU_SPEED "3686400UL")

set(AVR_L_FUSE 0xEC)
set(AVR_H_FUSE 0xD9)
set(AVR_E_FUSE 0xFE)

add_definitions("-Os")

include(../cmake/avr-gcc.cmake)

project(gateway C)

include(../cmake/check-git.cmake)
CheckGitSetup()

include_directories(../library/timer)
include_directories(../library/uart)
# include_directories(../library/utils)
include_directories(../library/modbus)
include_directories(../cobsm)

add_avr_executable(${PROJECT_NAME} 
    main.c
    ${CMAKE_BINARY_DIR}/generated/git_version.c
    ../library/uart/uart.c
    # ../library/uart/uart0.c
    ../library/uart/uart1.c
    ../library/timer/timer.c
    ../library/modbus/mb_crc.c
    # ../library/utils/hex_asc.c
    ../cobsm/cobsm.c
)

