# CMakeLists.txt — nRF52820 USB HID Receiver
#
# Toolchain : ARM GCC (arm-none-eabi)
# SDK       : nRF5 SDK 17.1.0  (place at ./nrf5_sdk)
# Build     : cmake .. -G "MinGW Makefiles" && mingw32-make
# Flash     : mingw32-make flash

cmake_minimum_required(VERSION 3.16)

# ── Toolchain (must be set before project()) ──────────────────────
set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER       arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER     arm-none-eabi-gcc)
set(CMAKE_OBJCOPY          arm-none-eabi-objcopy)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(hid_receiver C ASM)

option(BUILD_SENDER "Build nRF52820 ESB sender firmware" OFF)

# Keep objcopy/toolchain aligned even when arm-none-eabi-objcopy is not on PATH.
get_filename_component(TOOLCHAIN_BIN_DIR "${CMAKE_C_COMPILER}" DIRECTORY)
set(CMAKE_OBJCOPY "${TOOLCHAIN_BIN_DIR}/arm-none-eabi-objcopy" CACHE FILEPATH "objcopy tool" FORCE)

# ── SDK root ──────────────────────────────────────────────────────
set(SDK "${CMAKE_SOURCE_DIR}/nrf5_sdk" CACHE PATH "nRF5 SDK root")

# ── CPU / compile flags ───────────────────────────────────────────
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mabi=aapcs -mfloat-abi=soft")
set(CMAKE_C_FLAGS
    "${CPU_FLAGS} -Wall -O2 -g -ffunction-sections -fdata-sections --short-enums")
set(CMAKE_ASM_FLAGS "${CPU_FLAGS}")

# ── Linker flags ──────────────────────────────────────────────────
# -L points the linker at the directory containing nrf_common.ld,
# which is INCLUDEd by nrf52820_xxaa.ld.
set(CMAKE_EXE_LINKER_FLAGS
    "${CPU_FLAGS} \
    -Wl,--gc-sections \
    -Wl,-Map=${CMAKE_BINARY_DIR}/hid_receiver.map \
    -L${SDK}/modules/nrfx/mdk \
    -T${SDK}/modules/nrfx/mdk/nrf52820_xxaa.ld")

# ── Include paths ─────────────────────────────────────────────────
include_directories(
    # Application headers
    src

    # ESB (Enhanced ShockBurst, built-in radio)
    ${SDK}/components/proprietary_rf/esb

    # USB stack
    ${SDK}/components/libraries/usbd
    ${SDK}/components/libraries/usbd/class/hid
    ${SDK}/components/libraries/usbd/class/hid/generic

    # Utility libraries
    ${SDK}/components/libraries/util
    ${SDK}/components/libraries/strerror
    ${SDK}/components/libraries/experimental_section_vars
    ${SDK}/components/libraries/delay
    ${SDK}/components/libraries/atomic
    ${SDK}/components/libraries/atomic_fifo
    ${SDK}/components/libraries/queue
    ${SDK}/components/libraries/log
    ${SDK}/components/libraries/log/src
    ${SDK}/components/libraries/usbd/class/cdc
    ${SDK}/components/libraries/usbd/class/cdc/acm

    # UTF converter (used by app_usbd_string_desc.c)
    ${SDK}/external/utf_converter

    # No-SoftDevice stubs (nrf_error.h, sd_nvic_*, sd_softdevice_*)
    ${SDK}/components/drivers_nrf/nrf_soc_nosd

    # CMSIS + device headers
    ${SDK}/components/toolchain/cmsis/include
    ${SDK}/components/device

    # nrfx HAL + drivers
    ${SDK}/modules/nrfx
    ${SDK}/modules/nrfx/soc
    ${SDK}/modules/nrfx/hal
    ${SDK}/modules/nrfx/drivers/include
    ${SDK}/modules/nrfx/mdk

    # nrfx integration / legacy wrappers
    ${SDK}/integration/nrfx
    ${SDK}/integration/nrfx/legacy

    # Per-device sdk_config search path (overridden by our own sdk_config.h in src/)
    ${SDK}/config/nrf52820/config
)

# ── Compile definitions ───────────────────────────────────────────
add_definitions(
    -DNRF52820_XXAA
    -DCONFIG_GPIO_AS_PINRESET
    -DFLOAT_ABI_SOFT
)

# ── Source files ──────────────────────────────────────────────────
set(APP_MAIN_FILE src/main.c)
set(APP_TARGET_NAME hid_receiver)

if(BUILD_SENDER)
    set(APP_MAIN_FILE src/main_sender.c)
    set(APP_TARGET_NAME hid_sender)
endif()

set(SRC_FILES
    ${APP_MAIN_FILE}

    # ESB library
    ${SDK}/components/proprietary_rf/esb/nrf_esb.c

    # USB stack
    ${SDK}/components/libraries/usbd/app_usbd.c
    ${SDK}/components/libraries/usbd/app_usbd_core.c
    ${SDK}/components/libraries/usbd/app_usbd_string_desc.c
    ${SDK}/components/libraries/usbd/app_usbd_serial_num.c
    ${SDK}/components/libraries/usbd/class/hid/app_usbd_hid.c
    ${SDK}/components/libraries/usbd/class/hid/generic/app_usbd_hid_generic.c

    # nrfx hardware drivers
    ${SDK}/modules/nrfx/drivers/src/nrfx_usbd.c
    ${SDK}/modules/nrfx/drivers/src/nrfx_power.c
    ${SDK}/modules/nrfx/drivers/src/nrfx_clock.c
    ${SDK}/modules/nrfx/soc/nrfx_atomic.c

    # Legacy nrf_drv wrappers (used by the USB stack)
    ${SDK}/integration/nrfx/legacy/nrf_drv_clock.c
    ${SDK}/integration/nrfx/legacy/nrf_drv_power.c

    # Utility libraries
    ${SDK}/components/libraries/util/app_error.c
    ${SDK}/components/libraries/util/app_error_handler_gcc.c
    ${SDK}/components/libraries/util/app_error_weak.c
    ${SDK}/components/libraries/util/app_util_platform.c
    ${SDK}/components/libraries/strerror/nrf_strerror.c
    ${SDK}/components/libraries/experimental_section_vars/nrf_section_iter.c
    ${SDK}/components/libraries/atomic/nrf_atomic.c
    ${SDK}/components/libraries/atomic_fifo/nrf_atfifo.c
    ${SDK}/components/libraries/queue/nrf_queue.c

    # UTF converter (string descriptor encoding)
    ${SDK}/external/utf_converter/utf.c

    # CDC ACM — virtual serial port for debug output
    ${SDK}/components/libraries/usbd/class/cdc/acm/app_usbd_cdc_acm.c

    # No-SoftDevice stubs
    ${SDK}/components/drivers_nrf/nrf_soc_nosd/nrf_nvic.c
    ${SDK}/components/drivers_nrf/nrf_soc_nosd/nrf_soc.c

    # Startup and system init
    ${SDK}/modules/nrfx/mdk/system_nrf52820.c
    ${SDK}/modules/nrfx/mdk/gcc_startup_nrf52820.S
)

# ── Target ────────────────────────────────────────────────────────
add_executable(${APP_TARGET_NAME} ${SRC_FILES})

# ── Post-build: produce .hex ──────────────────────────────────────
add_custom_command(TARGET ${APP_TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${APP_TARGET_NAME} ${APP_TARGET_NAME}.hex
    COMMENT "Generating ${APP_TARGET_NAME}.hex"
)

# ── Flash target (DAPLink + nrfutil) ─────────────────────────────
add_custom_target(flash
    COMMAND nrfutil device program --firmware ${APP_TARGET_NAME}.hex --core Application
    COMMAND nrfutil device reset
    DEPENDS ${APP_TARGET_NAME}
    COMMENT "Flashing via DAPLink"
)
