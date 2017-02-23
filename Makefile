CFLAGS += -pthread -std=c99

main: main.c

clean:
	$(RM) main

.PHONY: clean
