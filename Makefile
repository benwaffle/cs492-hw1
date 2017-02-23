CFLAGS += -pthread

main: main.c

clean:
	$(RM) main

.PHONY: clean
