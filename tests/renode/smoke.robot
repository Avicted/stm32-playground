*** Settings ***
Test Setup        Create Machine
Test Teardown     Test Teardown
Resource          ${RENODEKEYWORDS}

*** Variables ***
${ELF}            ${CURDIR}/../../build/f407/firmware/firmware.elf
${PROMPT}         >${SPACE}

*** Keywords ***
Create Machine
    Execute Command           $repl=@${CURDIR}/f407-disco.repl
    Execute Command           $elf=@${ELF}
    Execute Command           include @${CURDIR}/f407.resc
    Create Terminal Tester    sysbus.usart2    timeout=5

Boot To Prompt
    Start Emulation
    Wait For Line On Uart     stm32-playground: FreeRTOS
    Wait For Prompt On Uart   ${PROMPT}

*** Test Cases ***
Should Boot To Shell Prompt
    Boot To Prompt

Should Blink Green LED At 1 Hz
    Create LED Tester         sysbus.gpioPortD.UserLED
    Start Emulation
    Assert LED Is Blinking    testDuration=4    onDuration=0.5    offDuration=0.5    tolerance=0.05

Should Control LEDs From Shell
    Create LED Tester         sysbus.gpioPortD.RedLED
    Boot To Prompt
    Assert LED State          false
    Write Line To Uart        led red on
    Wait For Line On Uart     red led on
    Assert LED State          true
    Write Line To Uart        led red toggle
    Wait For Line On Uart     red led off
    Assert LED State          false

Should Report Button State
    Boot To Prompt
    Write Line To Uart        button
    Wait For Line On Uart     button released
    Execute Command           sysbus.gpioPortA.UserButton Press
    Write Line To Uart        button
    Wait For Line On Uart     button pressed

Should List Tasks
    Boot To Prompt
    Write Line To Uart        tasks
    Wait For Line On Uart     blinky
    Wait For Prompt On Uart   ${PROMPT}

Should Reject Unknown Command
    Boot To Prompt
    Write Line To Uart        reboot
    Wait For Line On Uart     unknown command
