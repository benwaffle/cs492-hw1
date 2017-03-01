CFLAGS += -pthread -std=c99

main: main.c queue.c

clean:
	$(RM) main

.PHONY: clean
