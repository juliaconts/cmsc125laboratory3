CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude -pthread

DEBUG_FLAGS = -fsanitize=thread -fno-omit-frame-pointer

# All source files to be compiled
SRCS = src/main.c src/bank.c src/transaction.c src/buffer.c src/deadlock.c src/timer.c src/parser.c src/utils.c
OBJS = $(SRCS:.c=.o)
EXEC = bankdb

# all: Compile the simulator 
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# debug build (thread sanitizer)
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(EXEC)

# run example
run: $(EXEC)
	./$(EXEC) --accounts=accounts.txt --trace=trace.txt --deadlock=prevention

# clean: Remove binaries and object files 
clean:
	rm -f $(OBJS) $(EXEC)

# full rebuild
rebuild: clean all