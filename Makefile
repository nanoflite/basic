TARGET = basic

BUILD_FOLDER = build/basic

SOURCE_FOLDERS += ./arch/osx ./src

CC = gcc-10
CFLAGS = -g -Wall -Werror -std=c99 -lreadline -lm -DARCH_OSX=1 -DARCH_XMEGA=2 -DARCH=1

VPATH = $(SOURCE_FOLDERS)

.PHONY: start end all clean run

all: start $(BUILD_FOLDER) $(TARGET) end

SOURCES = $(foreach folder,$(SOURCE_FOLDERS),$(wildcard $(folder)/*.c))

ifneq "$(strip $(SOURCES))" "" 
	_OBJECTS = $(patsubst %.c,%.o,$(notdir $(SOURCES)))
endif

OBJECTS = $(patsubst %,$(BUILD_FOLDER)/%,$(_OBJECTS))

INCLUDE_FLAGS += \
    $(addprefix -I,$(INCLUDE_FOLDERS)) $(addprefix -I,$(SOURCE_FOLDERS)) -Iinclude

define run-cc
	@ echo "CC $<"
	@ $(CC) -c $(CFLAGS) $(INCLUDE_FLAGS) $< -o $(BUILD_FOLDER)/$%
endef

start:
	@ echo "-- Creating $(TARGET)..."

end:
	@ echo "done"
	@ echo ""

clean:
	@ echo $@
	@ rm -rf $(BUILD_FOLDER)
	@ make -C ./t clean	

$(BUILD_FOLDER):
	@ mkdir -p $@

(%.o): %.c
	$(run-cc)

$(TARGET): $(BUILD_FOLDER)/($(_OBJECTS))
	@ echo "LD $@"
	@ $(CC) $(OBJECTS) $(CFLAGS) $(LDFLAGS) -L$(BUILD_FOLDER) -o $(BUILD_FOLDER)/$@

run: all
	@ echo "-- Running $(TARGET)"
	@ BASIC_PATH=./examples $(BUILD_FOLDER)/$(TARGET)

test: all
	@ make -C ./t run

testdebug: all
	@ make -C ./t debug

debug: all
	@ echo "-- Debugging $(TARGET)"
	@ echo "run" | gdb $(BUILD_FOLDER)/$(TARGET)
