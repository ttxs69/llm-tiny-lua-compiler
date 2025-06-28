CC = gcc
CFLAGS = -Wall -g -Isrc
RELEASE_CFLAGS = -Wall -O3 -Isrc

ifeq ($(DEBUG_TRACE_EXECUTION), 1)
	CFLAGS += -DDEBUG_TRACE_EXECUTION
endif

ifeq ($(DEBUG_TRACE_PARSER), 1)
	CFLAGS += -DDEBUG_TRACE_PARSER
endif

ifeq ($(DEBUG_TRACE_CODEGEN), 1)
	CFLAGS += -DDEBUG_TRACE_CODEGEN
endif

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c,obj/%.o,$(SRCS))
RELEASE_OBJS = $(patsubst src/%.c,obj/release/%.o,$(SRCS))

TARGET = luac
RELEASE_TARGET = luac-release

.PHONY: all clean release test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

release: $(RELEASE_TARGET)

$(RELEASE_TARGET): $(RELEASE_OBJS)
	$(CC) $(RELEASE_CFLAGS) -o $(RELEASE_TARGET) $(RELEASE_OBJS)

obj/release/%.o: src/%.c
	@mkdir -p obj/release
	$(CC) $(RELEASE_CFLAGS) -c $< -o $@

clean:
	rm -rf $(TARGET) $(RELEASE_TARGET) obj test/*.output test/*.log

test:
	./run_tests.sh $(ARGS)
