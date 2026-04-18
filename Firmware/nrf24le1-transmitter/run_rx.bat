@echo off
setlocal

set "FLAGS=-mmcs51 --model-small --std-sdcc99 -DNRF24LE1_PACKAGE_32_PIN"
set "INC=-I.\nRF24LE1_SDK\include -I.\nRF24LE1_SDK\_target_sdcc_nrf24le1_32\include"
set "SDK_UART=.\nRF24LE1_SDK\src\uart\src"
set "SDK_TMR1=.\nRF24LE1_SDK\src\timer1\src"

sdcc %FLAGS% %INC% -c main_rx.c || goto :fail
sdcc %FLAGS% %INC% -c radio.c || goto :fail
sdcc %FLAGS% %INC% -c debug.c || goto :fail

sdcc %FLAGS% %INC% -c %SDK_UART%\uart_configure_manual_baud_calc.c || goto :fail
sdcc %FLAGS% %INC% -c %SDK_UART%\uart_send_wait_for_complete.c || goto :fail
sdcc %FLAGS% %INC% -c %SDK_TMR1%\timer1_configure.c || goto :fail

sdcc %FLAGS% ^
    main_rx.rel radio.rel debug.rel ^
    uart_configure_manual_baud_calc.rel ^
    uart_send_wait_for_complete.rel ^
    timer1_configure.rel ^
    -o output_rx.ihx || goto :fail

echo RX build completed successfully.
echo output_rx.ihx generated.
exit /b 0

:fail
echo RX build failed.
exit /b 1
