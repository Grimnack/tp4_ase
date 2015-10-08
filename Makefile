CC = gcc
CFLAGS= -W -Wall -m32 -g
EXEC= scheduler

all: $(EXEC)

scheduler: main.o hw.o scheduler.o
	$(CC) $(CFLAGS) -o $@ $^

test:
	./scheduler

.PHONY: clean mrproper

clean:
	rm -rf *.o ~* *.s core

mrproper: clean
	rm -rf $(EXEC)
