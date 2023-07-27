CC=gcc
CFLAGS = -std=c11 \
		-Wall -Wextra -Wpedantic \
		-Wformat=2 -Wno-unused-parameter -Wshadow \
		-Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
		-Wredundant-decls -Wnested-externs -Wmissing-include-dirs

# GCC warnings that Clang doesn't provide:
ifeq ($(CC),gcc)
    CFLAGS += -Wjump-misses-init -Wlogical-op
endif

snake: snake.o timer.o deque.o -lncurses -lm
	$(CC) -o $@ $^ $(CFLAGS)

snake.o: timer.h deque.h

timer.o: timer.c timer.h

deque.o: deque.c deque.h

.PHONY: clean
clean:
	rm *.o
