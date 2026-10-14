set(TP       ${PROJECT_SOURCE_DIR}/third_party)
set(FREERTOS ${TP}/FreeRTOS-Kernel)
set(DEVICE   ${TP}/cmsis_device_f4)

add_executable(firmware
    main.c
    syscalls.c
    bsp/bsp.c
    drivers/gpio.c
    drivers/uart.c
    ${DEVICE}/Source/Templates/gcc/startup_stm32f407xx.s
    ${DEVICE}/Source/Templates/system_stm32f4xx.c
    ${FREERTOS}/tasks.c
    ${FREERTOS}/queue.c
    ${FREERTOS}/list.c
    ${FREERTOS}/timers.c
    ${FREERTOS}/portable/GCC/ARM_CM4F/port.c
    ${FREERTOS}/portable/MemMang/heap_4.c
)
set_target_properties(firmware PROPERTIES SUFFIX .elf)

target_compile_definitions(firmware PRIVATE STM32F407xx USER_VECT_TAB_ADDRESS)
target_include_directories(firmware PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers
    ${PROJECT_SOURCE_DIR}/apps
    ${FREERTOS}/include
    ${FREERTOS}/portable/GCC/ARM_CM4F
    ${DEVICE}/Include
    ${TP}/CMSIS_5/CMSIS/Core/Include
)
target_compile_options(firmware PRIVATE -Wall -Wextra)

set(LDSCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/STM32F407VGTx.ld)
target_link_options(firmware PRIVATE -T${LDSCRIPT} -Wl,-Map=firmware.map -Wl,--print-memory-usage)
set_target_properties(firmware PROPERTIES LINK_DEPENDS ${LDSCRIPT})
target_link_libraries(firmware PRIVATE corelib)

add_custom_command(TARGET firmware POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:firmware> firmware.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:firmware>
    BYPRODUCTS firmware.bin
)
