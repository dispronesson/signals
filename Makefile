CC = gcc
CFLAGS_DEBUG = -g -ggdb -std=c17 -pedantic -W -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable
CFLAGS_RELEASE = -std=c17 -pedantic -W -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable

SRC_DIR = src
BUILD_DIR = build
DEBUG_DIR = $(BUILD_DIR)/debug
RELEASE_DIR = $(BUILD_DIR)/release

SRC_FILES_PARENT = $(SRC_DIR)/parent.c $(SRC_DIR)/parentutils.c
OBJ_FILES_DEBUG_PARENT = $(patsubst $(SRC_DIR)/%.c, $(DEBUG_DIR)/%.o, $(SRC_FILES_PARENT))
OBJ_FILES_RELEASE_PARENT = $(patsubst $(SRC_DIR)/%.c, $(RELEASE_DIR)/%.o, $(SRC_FILES_PARENT))
EXEC_NAME_PARENT = parent

SRC_FILES_CHILD = $(SRC_DIR)/child.c $(SRC_DIR)/childutils.c
OBJ_FILES_DEBUG_CHILD = $(patsubst $(SRC_DIR)/%.c, $(DEBUG_DIR)/%.o, $(SRC_FILES_CHILD))
OBJ_FILES_RELEASE_CHILD = $(patsubst $(SRC_DIR)/%.c, $(RELEASE_DIR)/%.o, $(SRC_FILES_CHILD))
EXEC_NAME_CHILD = child

all: debug

debug: $(DEBUG_DIR)/$(EXEC_NAME_PARENT) $(DEBUG_DIR)/$(EXEC_NAME_CHILD)

release: $(RELEASE_DIR)/$(EXEC_NAME_PARENT) $(RELEASE_DIR)/$(EXEC_NAME_CHILD)

$(DEBUG_DIR)/$(EXEC_NAME_PARENT): $(OBJ_FILES_DEBUG_PARENT) | $(DEBUG_DIR)
	$(CC) $^ -o $@

$(RELEASE_DIR)/$(EXEC_NAME_PARENT): $(OBJ_FILES_RELEASE_PARENT) | $(RELEASE_DIR)
	$(CC) $^ -o $@

$(DEBUG_DIR)/$(EXEC_NAME_CHILD): $(OBJ_FILES_DEBUG_CHILD) | $(DEBUG_DIR)
	$(CC) $^ -o $@

$(RELEASE_DIR)/$(EXEC_NAME_CHILD): $(OBJ_FILES_RELEASE_CHILD) | $(RELEASE_DIR)
	$(CC) $^ -o $@

$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.c | $(DEBUG_DIR)
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@

$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.c | $(RELEASE_DIR)
	$(CC) $(CFLAGS_RELEASE) -c $< -o $@

$(DEBUG_DIR) $(RELEASE_DIR):
	@mkdir -p $@

clean:
	rm -fr $(BUILD_DIR)

.PHONY: all debug release clean