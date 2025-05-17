CC = gcc
CFLAGS = -g `pkg-config --cflags --libs libadwaita-1 gtk4` -I./include -I./xml.c/src/
BUILD_DIR = ./build
VPATH = src include
SRC = main.c window.c parser.c
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

TARGET = $(BUILD_DIR)/epub

all: $(BUILD_DIR) $(TARGET)

$(TARGET): $(OBJS) -lxml
	$(CC) $(OBJS) \
	-I./xml.c/src -L./xml.c/build -lxml -lzip `pkg-config --cflags --libs libadwaita-1 gtk4` \
	-o $(TARGET)

$(BUILD_DIR):
	mkdir -p ./build

$(BUILD_DIR)/%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

-lxml:
	$(MAKE) -C ./xml.c/build/

clean:
	rm -rf $(OBJS) $(TARGET)

.PHONY: all clean
