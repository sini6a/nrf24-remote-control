# Makefile for nRF24LE1 Remote Firmware
# Requires: SDCC (sdcc.sourceforge.net) and packihx (comes with SDCC)
#
# Usage:
#   make          - build firmware
#   make clean    - remove generated files
#   make flash    - flash to device via Programmer.exe (Windows)

TARGET  = remote
SDK_DIR = ../nRF24LE1_SDK/include   # Adjust to your SDK path

CC      = sdcc
PACKIHX = packihx

CFLAGS  = -mmcs51 \
           --model-small \
           --std-sdcc99 \
           -I$(SDK_DIR) \
           --opt-code-size \
           --no-xinit-opt

SRCS    = main.c radio.c buttons.c
RELS    = $(SRCS:.c=.rel)

# ── Default target ───────────────────────────────────────────
all: $(TARGET).hex

# ── Compile each .c to .rel ──────────────────────────────────
%.rel: %.c
	$(CC) $(CFLAGS) -c $<

# ── Link ─────────────────────────────────────────────────────
$(TARGET).ihx: $(RELS)
	$(CC) $(CFLAGS) $(RELS) -o $(TARGET).ihx

# ── Convert to Intel HEX ─────────────────────────────────────
$(TARGET).hex: $(TARGET).ihx
	$(PACKIHX) $(TARGET).ihx > $(TARGET).hex
	@echo ""
	@echo "Build complete: $(TARGET).hex"
	@echo "Flash with: .\\Programmer.exe \\\\.\\COM3 w $(TARGET).hex"

# ── Flash (Windows, adjust COM port) ─────────────────────────
flash: $(TARGET).hex
	Programmer.exe \\.\COM3 w $(TARGET).hex

# ── Clean ────────────────────────────────────────────────────
clean:
	del /Q *.rel *.ihx *.hex *.map *.mem *.lst *.rst *.sym *.asm 2>NUL || \
	rm -f  *.rel *.ihx *.hex *.map *.mem *.lst *.rst *.sym *.asm
