CC = gcc
CFLAGS = -g `pkg-config --cflags libadwaita-1 webkitgtk-6.0 gtk4` -I./include -I./xml.c/src -Wl,--export-dynamic
LDFLAGS = -L./xml.c/build -lxml  -lzip `pkg-config --libs webkitgtk-6.0 libadwaita-1 gtk4 gmodule-export-2.0`
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
	mkdir -p ./xml.c/$(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

xml-lib:
	$(MAKE) -C ./xml.c/build/

run:
	$(TARGET)

clean:
	rm -rf $(OBJS) $(TARGET)

.PHONY: all clean xml-lib
