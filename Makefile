# Detect operating system
UNAME_S := $(shell uname -s)

# Program name
TARGET = gameball-remap

# Common compiler flags
CFLAGS = -Wall -Wextra -O2

# Platform-specific settings
ifeq ($(UNAME_S),Linux)
    CC = gcc
    CFLAGS += -I/usr/include/libevdev-1.0
    LDFLAGS = -levdev
    INSTALL_DIR = /usr/local/bin
endif

ifeq ($(UNAME_S),FreeBSD)
    CC = cc
    CFLAGS += -I/usr/local/include
    LDFLAGS = -L/usr/local/lib -levdev
    INSTALL_DIR = /usr/local/bin
endif

# Default target
all: $(TARGET)

# Build the program
$(TARGET): gameball-remap.c
	$(CC) $(CFLAGS) -o $(TARGET) gameball-remap.c $(LDFLAGS)
	@echo "Built $(TARGET) for $(UNAME_S)"

# Install the program (requires root)
install: $(TARGET)
	install -m 755 $(TARGET) $(INSTALL_DIR)/$(TARGET)
	@echo "Installed $(TARGET) to $(INSTALL_DIR)"

# Clean build artifacts
clean:
	rm -f $(TARGET)

# Uninstall the program (requires root)
uninstall:
	rm -f $(INSTALL_DIR)/$(TARGET)
	@echo "Uninstalled $(TARGET) from $(INSTALL_DIR)"

# Show help
help:
	@echo "Makefile for $(TARGET)"
	@echo ""
	@echo "Detected OS: $(UNAME_S)"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build the program"
	@echo "  make install  - Install to $(INSTALL_DIR) (requires root)"
	@echo "  make clean    - Remove built files"
	@echo "  make uninstall- Remove installed program (requires root)"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "Usage:"
ifeq ($(UNAME_S),Linux)
	@echo "  Linux: Requires libevdev package"
	@echo "  Run: sudo ./$(TARGET)"
endif
ifeq ($(UNAME_S),FreeBSD)
	@echo "  FreeBSD: Install libevdev (pkg install libevdev)"
	@echo "  FreeBSD: Load evdev module first: kldload evdev"
	@echo "  Run: sudo ./$(TARGET)"
endif

.PHONY: all install clean uninstall help
