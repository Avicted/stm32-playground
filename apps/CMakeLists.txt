# Apps are FreeRTOS tasks built against the firmware's platform API (bsp.h, uart.h).
target_sources(firmware PRIVATE
    blinky/blinky.c
    shell/shell.c
)
