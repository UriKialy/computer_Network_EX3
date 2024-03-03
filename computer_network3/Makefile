CC=gcc

FLAGS=-Wall -g -ggdb3

SENDER= TCP_Sender.o   
RECEIVER= TCP_Receiver.o 

SENDER_FILES = TCP_Sender.c
RECEIVER_FILES= TCP_Receiver.c List.c List.h

all: TCP_Receiver TCP_Sender

TCP_Reciver: $(RECEIVER)
	$(CC) $(FLAGS) -o $@ $^
TCP_Sender: $(SENDER)
	$(CC) $(FLAGS) -o $@ $^
TCP_Sender.o: $(SENDER_FILES)
	$(CC) $(FLAGS) -c TCP_Sender.c -o $@
TCP_Reciver.o: $(RECEIVER_FILES)
	$(CC) $(FLAGS) -c TCP_Receiver.c -o $@	

.PHONY: clean all

clean:
	rm -f *.o *.a *.so *.gch  TCP_Receiver TCP_Sender
