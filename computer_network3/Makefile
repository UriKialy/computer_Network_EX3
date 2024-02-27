CC=gcc

FLAGS=-Wall -g -ggdb3

SENDER= TCP_Sender.o   
RECIVER= TCP_Reciver.o 

SENDER_FILES = TCP_Sender.c
RECIVER_FILES= TCP_Reciver.c List.c List.h

all: TCP_Reciver TCP_Sender

TCP_Reciver: $(RECIVER)
	$(CC) $(FLAGS) -o $@ $^
TCP_Sender: $(SENDER)
	$(CC) $(FLAGS) -o $@ $^
TCP_Sender.o: $(SENDER_FILES)
	$(CC) $(FLAGS) -c TCP_Sender.c -o $@
TCP_Reciver.o: $(RECIVER_FILES)
	$(CC) $(FLAGS) -c TCP_Reciver.c -o $@	

.PHONY: clean all

clean:
	rm -f *.o *.a *.so *.gch  TCP_Reciver TCP_Sender