CC = gcc
AR = ar
FLAGS = -g 
RCS = rcs

RUDP = RUDP_API.a RUDP_Receiver RUDP_Sender 
.PHONY: all clean

all: rudp

rudp: $(RUDP)

RUDP_Receiver: RUDP_Receiver.o RUDP_API.a List.o
	$(CC) $(FLAGS) $^ -o $@

RUDP_Receiver.o: RUDP_Receiver.c RUDP_API.h
	$(CC) $(FLAGS) -c $<

RUDP_Sender: RUDP_Sender.o RUDP_API.a
	$(CC) $(FLAGS) $^ -o $@

RUDP_Sender.o: RUDP_Sender.c RUDP_API.h
	$(CC) $(FLAGS) -c $<

RUDP_API.a: RUDP_API.o
	$(AR) $(RCS) $@ $^

RUDP_API.o: RUDP_API.c RUDP_API.h
	$(CC) $(FLAGS) -c $<

List.o: List.c
	$(CC) $(FLAGS) -c List.c

clean:
	rm -f *.o *.a  RUDP_Receiver RUDP_Sender