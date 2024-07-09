CC=gcc
OUTPUT_DIR=freakyfetch
PRIMARY_TARGETS=$(OUTPUT_DIR)/main $(OUTPUT_DIR)/$(FREAKY_FILE)
SOURCE_DIR=src
FREAKY_FILE=freaky.png

all: $(OUTPUT_DIR) $(PRIMARY_TARGETS)

install: $(OUTPUT_DIR)/main
	cp $(OUTPUT_DIR)/main /usr/local/bin

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

$(OUTPUT_DIR)/main: $(SOURCE_DIR)/main.c | $(OUTPUT_DIR)
	$(CC) $(SOURCE_DIR)/main.c -o $(OUTPUT_DIR)/main

$(OUTPUT_DIR)/$(FREAKY_FILE): $(SOURCE_DIR)/$(FREAKY_FILE) | $(OUTPUT_DIR)
	cp $(SOURCE_DIR)/$(FREAKY_FILE) $(OUTPUT_DIR)

