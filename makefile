CC = gcc
CFLAGS = -g -std=c99 `pkg-config --cflags libadwaita-1 gtk4` -I./include -I./xml.c/src
LDFLAGS = -L./xml.c/build -lxml -lzip `pkg-config --libs libadwaita-1 gtk4`
BUILD_DIR = ./build
VPATH = src include
SRC = main.c window.c parser.c
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

TARGET = $(BUILD_DIR)/epub

all: $(BUILD_DIR) xml-lib $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

xml-lib:
	$(MAKE) -C ./xml.c/build/

run:
	$(TARGET)

clean:
	rm -rf $(OBJS) $(TARGET)

.PHONY: all clean xml-lib
