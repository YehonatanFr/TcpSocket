# Makefile for TCP project

all: receiver sender

sender: sender.c
	gcc -o sender sender.c

receiver: receiver.c
	gcc -o receiver receiver.c

clean:
	rm -f *.o sender receiver

runs:
	./sender

runc:
	./receiver

runs-strace:
	strace -f ./sneder

runc-strace:
	strace -f ./receiver

new: clean all runs