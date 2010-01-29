CC=gcc
RM=rm
CFLAGS=-c -Wall -pedantic
LIBS=-lssh -lpthread
EXECUTABLE=sshbrute
INSTALL=install

all: $(EXECUTABLE)

$(EXECUTABLE): $(EXECUTABLE).o
	$(CC) $(LIBS) $(EXECUTABLE).o -o $(EXECUTABLE)

sshbrute.o: $(EXECUTABLE).c
	$(CC) $(CFLAGS) $(EXECUTABLE).c

install: all
	$(INSTALL) $(EXECUTABLE) /usr/bin/

clean:
	$(RM) -rf *.o $(EXECUTABLE)
