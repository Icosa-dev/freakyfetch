CC=gcc
OUTPUT_DIR=freakyfetch
PRIMARY_TARGETS=$(OUTPUT_DIR)/main $(OUTPUT_DIR)/$(FREAKY_FILE)
SOURCE_DIR=src
FREAKY_FILE=freaky.png

all: $(PRIMARY_TARGETS)

install:
	cp $(OUTPUT_DIR) /usr/local/bin

$(OUTPUT_DIR)/main: $(SOURCE_DIR)/main.c 
	$(CC) $(SOURCE_DIR)/main.c -o $(OUTPUT_DIR)/main

$(OUTPUT_DIR)/$(FREAKY_FILE): $(SOURCE_DIR)/$(FREAKY_FILE)
	cp $(SOURCE_DIR)/$(FREAKY_FILE) $(OUTPUT_DIR)
