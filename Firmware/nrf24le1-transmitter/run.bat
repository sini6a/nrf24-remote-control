@echo off
setlocal

set "FLAGS=-mmcs51 --model-small --std-sdcc99 -DNRF24LE1_PACKAGE_32_PIN"
set "INC=-I.\nRF24LE1_SDK\include -I.\nRF24LE1_SDK\_target_sdcc_nrf24le1_32\include"
set "SDK_UART=.\nRF24LE1_SDK\src\uart\src"
set "SDK_TMR1=.\nRF24LE1_SDK\src\timer1\src"

REM Compile application sources
sdcc %FLAGS% %INC% -c main.c || goto :fail
sdcc %FLAGS% %INC% -c radio.c || goto :fail
sdcc %FLAGS% %INC% -c buttons.c || goto :fail
sdcc %FLAGS% %INC% -c debug.c || goto :fail

REM Compile SDK UART + Timer1 sources
sdcc %FLAGS% %INC% -c %SDK_UART%\uart_configure_manual_baud_calc.c || goto :fail
sdcc %FLAGS% %INC% -c %SDK_UART%\uart_send_wait_for_complete.c || goto :fail
sdcc %FLAGS% %INC% -c %SDK_TMR1%\timer1_configure.c || goto :fail

REM Link final image
sdcc %FLAGS% ^
    main.rel radio.rel buttons.rel debug.rel ^
    uart_configure_manual_baud_calc.rel ^
    uart_send_wait_for_complete.rel ^
    timer1_configure.rel ^
    -o output.ihx || goto :fail

echo Build completed successfully.
exit /b 0

:fail
echo Build failed.
exit /b 1
