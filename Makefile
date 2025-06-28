CC = gcc
CFLAGS = -Wall -g -Isrc
DBG_CFLAGS = -Wall -g -DDEBUG_TRACE_EXECUTION -Isrc
RELEASE_CFLAGS = -Wall -O3 -Isrc

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c,obj/%.o,$(SRCS))
TARGET = luac

.PHONY: all clean test debug

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	./run_tests.sh

debug: CFLAGS = $(DBG_CFLAGS)
debug: clean all

release: CFLAGS = $(RELEASE_CFLAGS)
release: clean all

clean:
	rm -rf $(TARGET) obj output.txt test/*.output
