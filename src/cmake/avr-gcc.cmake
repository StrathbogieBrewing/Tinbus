find_program(AVR_CC avr-gcc REQUIRED)
find_program(AVR_CXX avr-g++ REQUIRED)
find_program(AVR_OBJCOPY avr-objcopy REQUIRED)
find_program(AVR_SIZE_TOOL avr-size REQUIRED)
find_program(AVR_OBJDUMP avr-objdump REQUIRED)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)
set(CMAKE_C_COMPILER ${AVR_CC})
set(CMAKE_CXX_COMPILER ${AVR_CXX})
set(CMAKE_ASM_COMPILER ${AVR_CC})

add_definitions("-fpack-struct")
add_definitions("-fshort-enums")
add_definitions("-Wall")
add_definitions("-Werror")
add_definitions("-pedantic")
add_definitions("-pedantic-errors")
add_definitions("-funsigned-char")
add_definitions("-funsigned-bitfields")
add_definitions("-ffunction-sections")
add_definitions("-c")
add_definitions("-std=gnu99")

function(add_avr_executable EXECUTABLE_NAME)

   if(NOT ARGN)
      message(FATAL_ERROR "No source files given for ${EXECUTABLE_NAME}")
   endif(NOT ARGN)

   set(elf_file ${EXECUTABLE_NAME}.elf)
   set(hex_file ${EXECUTABLE_NAME}.hex)
   set(lst_file ${EXECUTABLE_NAME}.lst)
   set(map_file ${EXECUTABLE_NAME}.map)
   set(eeprom_image ${EXECUTABLE_NAME}-eeprom.hex)

   add_executable(${elf_file} EXCLUDE_FROM_ALL ${ARGN})

   add_custom_command(
      OUTPUT ${hex_file}
      COMMAND
         ${AVR_OBJCOPY} -j .text -j .data -O ihex ${elf_file} ${hex_file}
      COMMAND
         ${AVR_SIZE_TOOL} ${AVR_SIZE_ARGS} ${elf_file}
      DEPENDS ${elf_file}
   )

   add_custom_command(
      OUTPUT ${lst_file}
      COMMAND
         ${AVR_OBJDUMP} -d ${elf_file} > ${lst_file}
      DEPENDS ${elf_file}
   )

   add_custom_target(
      ${EXECUTABLE_NAME}
      ALL
      DEPENDS ${hex_file} ${lst_file} ${eeprom_image}
   )

   set_target_properties(
      ${elf_file}
      PROPERTIES
         COMPILE_FLAGS "-mmcu=${AVR_MCU_TYPE} -DF_CPU=${AVR_MCU_SPEED}"
         LINK_FLAGS "-mmcu=${AVR_MCU_TYPE} -Wl,--gc-sections -mrelax -Wl,-Map,${map_file}"
   )

   get_directory_property(clean_files ADDITIONAL_MAKE_CLEAN_FILES)
   set_directory_properties(
      PROPERTIES
         ADDITIONAL_MAKE_CLEAN_FILES "${map_file}"
   )

   if(NOT AVR_UPLOADTOOL_PORT)
   set(
         AVR_UPLOADTOOL_PORT usb
         CACHE STRING "Set default upload tool port: usb"
   )
   endif(NOT AVR_UPLOADTOOL_PORT)

   set(AVR_UPLOADTOOL_BASE_OPTIONS -p ${AVR_MCU_TYPE} -c ${AVR_PROGRAMMER} -P ${AVR_UPLOADTOOL_PORT})

   if(AVR_UPLOADTOOL_BAUDRATE)
      set(AVR_UPLOADTOOL_BASE_OPTIONS ${AVR_UPLOADTOOL_BASE_OPTIONS} -b ${AVR_UPLOADTOOL_BAUDRATE})
   endif()

   if(AVR_UPLOADTOOL_BIT_RATE)
      set(AVR_UPLOADTOOL_BASE_OPTIONS ${AVR_UPLOADTOOL_BASE_OPTIONS} -B ${AVR_UPLOADTOOL_BIT_RATE})
   endif()

   set(AVR_SET_FUSE_OPTIONS -U lfuse:w:${AVR_L_FUSE}:m -U hfuse:w:${AVR_H_FUSE}:m)

   if(AVR_E_FUSE)
      set(AVR_SET_FUSE_OPTIONS ${AVR_SET_FUSE_OPTIONS} -U efuse:w:${AVR_E_FUSE}:m)
   endif()

   add_custom_command(
      OUTPUT ${eeprom_image}
      COMMAND
         ${AVR_OBJCOPY} -j .eeprom --set-section-flags=.eeprom=alloc,load
            --change-section-lma .eeprom=0 --no-change-warnings
            -O ihex ${elf_file} ${eeprom_image}
      DEPENDS ${elf_file}
   )

   add_custom_target(
      upload
      ${AVR_UPLOADTOOL} ${AVR_UPLOADTOOL_BASE_OPTIONS} ${AVR_UPLOADTOOL_OPTIONS}
         -U flash:w:${hex_file}
      DEPENDS ${hex_file}
      COMMENT "Uploading ${hex_file} to ${AVR_MCU_TYPE} using ${AVR_PROGRAMMER}"
   )

   add_custom_target(
      upload_eeprom
      ${AVR_UPLOADTOOL} ${AVR_UPLOADTOOL_BASE_OPTIONS} ${AVR_UPLOADTOOL_OPTIONS}
         -U eeprom:w:${eeprom_image}
      DEPENDS ${eeprom_image}
      COMMENT "Uploading ${eeprom_image} to ${AVR_MCU_TYPE} using ${AVR_PROGRAMMER}"
   )

   add_custom_target(
      get_addr
      ${AVR_UPLOADTOOL} ${AVR_UPLOADTOOL_BASE_OPTIONS} -n
         -U eeprom:r:ee.hex:h
      COMMENT "Get modbus address from ${AVR_MCU_TYPE}"
   )

   add_custom_target(
      get_fuses
      ${AVR_UPLOADTOOL} ${AVR_UPLOADTOOL_BASE_OPTIONS} -n
         -U lfuse:r:-:b
         -U hfuse:r:-:b
      COMMENT "Get fuses from ${AVR_MCU_TYPE}"
   )

   add_custom_target(
      set_fuses
      ${AVR_UPLOADTOOL} ${AVR_UPLOADTOOL_BASE_OPTIONS} 
         ${AVR_SET_FUSE_OPTIONS}
         COMMENT "Setup: High Fuse: ${AVR_H_FUSE} Low Fuse: ${AVR_L_FUSE}"
   )

   add_custom_target(
      get_status
      ${AVR_UPLOADTOOL} ${AVR_UPLOADTOOL_BASE_OPTIONS} -n -v
      COMMENT "Get status from ${AVR_MCU_TYPE}"
   )

endfunction(add_avr_executable)
