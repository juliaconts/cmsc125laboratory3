CC = gcc

CFLAGS = -Wall -Wextra -g -Iinclude -pthread -std=gnu11
DEBUG_FLAGS = -fsanitize=thread -fno-omit-frame-pointer

EXEC = bankdb

SRCS = \
src/main.c \
src/bank.c \
src/transaction.c \
src/timer.c \
src/lock_mgr.c \
src/buffer_pool.c \
src/metrics.c \
src/utils.c

OBJS = $(SRCS:.c=.o)

# Default build
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ThreadSanitizer build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(EXEC)

# Run sample
run: $(EXEC)
	./$(EXEC) --accounts=accounts.txt --trace=trace.txt --deadlock=prevention

# Clean build files
clean:
	rm -f $(OBJS) $(EXEC)

# Full rebuild
rebuild: clean all