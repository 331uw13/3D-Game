CC = gcc
CFLAGS = -O2 -Wall -Wextra -Isrc -Isrc/platform # Base flags and include path for project headers
LDFLAGS = # Base linker flags
LIBS = # Base libraries

TARGET_NAME = game
SRCS := $(filter-out src/platform/platform_%.c,$(shell find src -name "*.c"))

# Platform detection and specific settings
ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET_NAME).exe
    PLATFORM_SRC = src/platform/platform_win.c
    CFLAGS += -DPLATFORM_DESKTOP
    
    LIBS += -lraylib -lopengl32 -lgdi32 -lwinmm -static
    RM = del /Q
    TARGET_PATH = $(subst /,\,$(TARGET))
    define CLEAN_CMD
	@rm -f $(TARGET_PATH) 2>/dev/null || true
	@find . -name "*.o" -type f -delete 2>/dev/null || true
	@echo "Clean complete."
    endef
else
    # Assume Unix-like (Linux, macOS)
    TARGET := $(TARGET_NAME)
    PLATFORM_SRC = src/platform/platform_unix.c
    CFLAGS += -DPLATFORM_DESKTOP

    LIBS += -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 # TODO: should link x11 in mac? 
    RM = rm -f
    TARGET_PATH = $(TARGET)
    define CLEAN_CMD
	@echo "Cleaning build files..."
	$(RM) $(OBJS) $(TARGET_PATH)
	@echo "Clean complete."
    endef
endif

SRCS += $(PLATFORM_SRC)
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linking $@..."
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LIBS)
	@echo "$@ built successfully!"

%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(CLEAN_CMD)

.PHONY: all clean
