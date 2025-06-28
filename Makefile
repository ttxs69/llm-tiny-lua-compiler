CC = gcc
CFLAGS = -Wall -g
DBG_CFLAGS = -Wall -g -DDEBUG_TRACE_EXECUTION

SRCS = main.c lexer.c parser.c codegen.c bytecode.c vm.c table.c
OBJS = $(SRCS:.c=.o)
TARGET = luac

.PHONY: all clean test debug

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	./run_tests.sh

debug: CFLAGS = $(DBG_CFLAGS)
debug: all

clean:
	rm -f $(TARGET) $(OBJS) output.txt test/*.output
