CC = gcc
CFLAGS = -Wall -g -Isrc
DBG_CFLAGS = -Wall -g -DDEBUG_TRACE_EXECUTION -Isrc
RELEASE_CFLAGS = -Wall -O3 -Isrc

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c,obj/%.o,$(SRCS))
DBG_OBJS = $(patsubst src/%.c,obj/debug/%.o,$(SRCS))
RELEASE_OBJS = $(patsubst src/%.c,obj/release/%.o,$(SRCS))

TARGET = luac
DBG_TARGET = luac-debug
RELEASE_TARGET = luac-release

.PHONY: all clean test debug release

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

debug: $(DBG_TARGET)

$(DBG_TARGET): $(DBG_OBJS)
	$(CC) $(DBG_CFLAGS) -o $(DBG_TARGET) $(DBG_OBJS)

obj/debug/%.o: src/%.c
	@mkdir -p obj/debug
	$(CC) $(DBG_CFLAGS) -c $< -o $@

release: $(RELEASE_TARGET)

$(RELEASE_TARGET): $(RELEASE_OBJS)
	$(CC) $(RELEASE_CFLAGS) -o $(RELEASE_TARGET) $(RELEASE_OBJS)

obj/release/%.o: src/%.c
	@mkdir -p obj/release
	$(CC) $(RELEASE_CFLAGS) -c $< -o $@

test: all
	./run_tests.sh

test-debug: debug
	./run_tests.sh -d

clean:
	rm -rf $(TARGET) $(DBG_TARGET) $(RELEASE_TARGET) obj test/*.output
