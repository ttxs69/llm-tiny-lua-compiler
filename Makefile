CC = gcc
CFLAGS = -Wall -g

SRCS = main.c lexer.c parser.c codegen.c bytecode.c vm.c
OBJS = $(SRCS:.c=.o)
TARGET = luac

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	./$(TARGET) test.lua > output.txt
	@echo "Generated output:"
	@cat output.txt
	@if grep -q "30.000000" output.txt; then \
		echo "Test passed!"; \
	else \
		echo "Test failed!"; \
		exit 1; \
	fi

clean:
	rm -f $(TARGET) $(OBJS) output.txt
