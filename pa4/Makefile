# Makefile to compile Umix Programming Assignment 4 (pa4) [updated: 1/11/10]

LIBDIR = $(UMIXPUBDIR)/lib
# LIBDIR = $(UMIXROOTDIR)/sys

CC 	= cc 
FLAGS 	= -g -L$(LIBDIR) -lumix4

PA4 =	pa4a pa4b pa4c tc1 tc2 tc3 tc4 spawn order queue

pa4:	$(PA4)

pa4a:	pa4a.c aux.h umix.h
	$(CC) $(FLAGS) -o pa4a pa4a.c

pa4b:	pa4b.c aux.h umix.h mythreads.h mythreads.o
	$(CC) $(FLAGS) -o pa4b pa4b.c mythreads.o

pa4c:	pa4c.c aux.h umix.h mythreads.h mythreads.o
	$(CC) $(FLAGS) -o pa4c pa4c.c mythreads.o

tc1:	tc1.c aux.h umix.h mythreads.h mythreads.o
	$(CC) $(FLAGS) -o tc1 tc1.c mythreads.o

tc2:	tc2.c aux.h umix.h mythreads.h mythreads.o
	$(CC) $(FLAGS) -o tc2 tc2.c mythreads.o

tc3:	tc3.c aux.h umix.h mythreads.h mythreads.o
	$(CC) $(FLAGS) -o tc3 tc3.c mythreads.o

tc4:	tc4.c aux.h umix.h mythreads.h mythreads.o
	$(CC) $(FLAGS) -o tc4 tc4.c mythreads.o

spawn:	spawn.c aux.h umix.h mythreads.h mythreads.o
	$(CC) $(FLAGS) -o spawn spawn.c mythreads.o

order:	order.c aux.h umix.h mythreads.h mythreads.o
	$(CC) $(FLAGS) -o order order.c mythreads.o

queue:	queue.c aux.h umix.h mythreads.h mythreads.o
	$(CC) $(FLAGS) -o queue queue.c mythreads.o


mythreads.o:	mythreads.c aux.h umix.h mythreads.h
	$(CC) $(FLAGS) -c mythreads.c

clean:
	rm -f *.o $(PA4)
