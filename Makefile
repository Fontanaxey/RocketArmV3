# Versioning
VERSION_MAJOR  = 0
VERSION_MINOR  = 1
VERSION_PATCH  = 1
VERSION_SUFFIX = -alpha

VERSION_STR = "$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)$(VERSION_SUFFIX)"

CFLAGS += -DAPP_VERSION=$(VERSION_STR)
CC       := gcc
CFLAGS   := -Wall -Wextra -Werror -Iinclude -O2 -g -MMD -MP
LDLIBS   := -lncurses

SRC_DIR   := src
INC_DIR   := include
BUILD_DIR := build
BIN_DIR   := bin
DOC_DIR   := docs

TARGET    := $(BIN_DIR)/robot_arm
SRCS      := $(wildcard $(SRC_DIR)/*.c)
OBJS      := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
DEPS      := $(OBJS:.o=.d)

# Binari necessari
DOXYGEN   := $(shell command -v doxygen 2> /dev/null)

all: directories $(TARGET)

directories:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

$(TARGET): $(OBJS)
	@echo "Linking: $@"
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling: $<"
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

# Target Documentazione con controllo dipendenza
doc:
ifndef DOXYGEN
	$(error "Doxygen non è installato. Esegui: sudo apt install doxygen graphviz")
endif
	@echo "Generating documentation..."
	$(DOXYGEN) Doxyfile

clean:
	@echo "Cleaning up..."
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(DOC_DIR)

.PHONY: all clean directories doc