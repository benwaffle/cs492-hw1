CFLAGS += -pthread -std=c99 -g -Wall -Wextra

main: main.c queue.c

clean:
	$(RM) main

.PHONY: clean
