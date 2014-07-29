root = ..

CC = gcc-4.8
SOURCES = main.c $(wildcard test_*.c)

_OBJECTS = $(patsubst %.c,%.o,$(SOURCES))
_OBJECTS += $(wildcard $(root)/build/*.o)
OBJECTS = $(filter-out $(root)/build/main.o,$(_OBJECTS))

INCLUDE_FOLDERS = $(root)/include

CFLAGS += $(addprefix -I,$(INCLUDE_FOLDERS)) -std=c99 

CFLAGS += $(shell pkg-config --cflags cmocka)
LDFLAGS += $(shell pkg-config --libs cmocka)

.PHONY: test run clean start

test: start $(OBJECTS)
	@ echo "LD $@"
	@ $(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)
	@ echo "done"
	@ echo ""

start:
	@ echo "-- Building unit tests"

%.o : %.c
	@ echo "CC $@"
	@ $(CC) $(CFLAGS) -c $< -o $@

run: test
	@ echo "-- Running unit tests"
	@ ./test

clean:
	@ rm -f test
	@ rm -f *.o
