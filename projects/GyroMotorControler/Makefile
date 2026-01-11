# Makefile for GyroMotorControler (moved to projects/GyroMotorControler)
# Defaults (can be overridden on make command line)
MCU ?= atmega328p
AVRDUDE_MCU ?= m328p
F_CPU ?= 16000000UL
TARGET ?= GyroMotorControler

CXX := avr-g++
OBJCOPY := avr-objcopy
SIZE := avr-size
AVRDUDE := avrdude

# avrdude defaults (override with environment or on command line)
PROGRAMMER ?= stk500v2
PORT ?= COM5
BAUD ?= 57600
AVRDUDEFLAGS ?= -c $(PROGRAMMER) -p $(AVRDUDE_MCU) -P $(PORT) -b $(BAUD)

CXXFLAGS := -mmcu=$(MCU) -DF_CPU=$(F_CPU) -std=gnu++98 -Os -Wall -Wextra
LDFLAGS := -mmcu=$(MCU)

SRCS := $(filter-out mainold.cpp,$(wildcard *.cpp))
OBJS := $(SRCS:.cpp=.o)

.PHONY: all clean flash size strip_bom
all: $(TARGET).hex

# .PHONY: strip_bom
strip_bom:
	@echo "Stripping UTF-8 BOMs from source files (if present)..."
	@powershell -NoProfile -Command "Get-ChildItem -Filter *.cpp | ForEach-Object { $s = Get-Content -Raw $_.FullName; if ($s.Length -gt 0 -and [char]0xFEFF -eq $s[0]) { $s = $s.Substring(1); Set-Content -Encoding UTF8 -Value $s $_.FullName } }" || (for f in $(SRCS); do sed -i '1s/^\xEF\xBB\xBF//' "$$f"; done)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	$(SIZE) $<

# Flash using avrdude. You can override programmer/port/baud/mcu when calling make,
# e.g. `make flash PORT=COM3 PROGRAMMER=usbasp`.
flash: $(TARGET).hex
	$(AVRDUDE) $(AVRDUDEFLAGS) -U flash:w:$(TARGET).hex:i

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).hex $(TARGET).eep $(TARGET).lss

# Convenience: build and then flash in one command
upload: all flash
	@echo "Uploaded $(TARGET).hex"
