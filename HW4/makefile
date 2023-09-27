CC=gcc
CFLAGS=-I. -g -pthread
name=waitpid1
OBJS=$(name).o csapp.o

$(name): $(OBJS)
	$(CC) -o $(name) $(OBJS) $(CFLAGS)

clean:
	rm $(name) *.o
