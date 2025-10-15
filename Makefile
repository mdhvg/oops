CC := gcc
CFLAGS := -O2 -Wall -std=c99

TARGET := oops
SRC := main.c

PREFIX := /usr/local/bin

.PHONY: all install clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

install: $(TARGET)
	@echo "Installing $(TARGET) to $(PREFIX)"
	install -Dm755 $(TARGET) $(PREFIX)/$(TARGET)

uninstall:
	@echo "Removing $(TARGET) from $(PREFIX)"
	-rm -f $(PREFIX)/$(TARGET)

clean:
	rm -f $(TARGET)