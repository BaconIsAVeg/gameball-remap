# Program name
TARGET = gameball-remap

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -I/usr/include/libevdev-1.0
LDFLAGS = -levdev
INSTALL_DIR = /usr/local/bin

# Default target
all: $(TARGET)

# Build the program
$(TARGET): gameball-remap.c
	$(CC) $(CFLAGS) -o $(TARGET) gameball-remap.c $(LDFLAGS)
	@echo "Built $(TARGET)"

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
	@echo "Targets:"
	@echo "  make          - Build the program"
	@echo "  make install  - Install to $(INSTALL_DIR) (requires root)"
	@echo "  make clean    - Remove built files"
	@echo "  make uninstall- Remove installed program (requires root)"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  Requires libevdev package"
	@echo "  Run: sudo ./$(TARGET)"

.PHONY: all install clean uninstall help
