CFLAGS=-Wall $(shell pkg-config --cflags libxml-2.0)
LDFLAGS= $(shell pkg-config --libs libxml-2.0)

all: workerinstall workerupdate 
#simpledatabasehandler_unit_tests

workerinstall: workerinstall.o
	$(CC) $(LDFLAGS) workerinstall.o -o workerinstall

workerupdate: workerupdate.o
	$(CC) $(LDFLAGS) workerupdate.o -o workerupdate

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

#simpledatabasehandler_unit_tests:
#	gcc -g -c ./simpledatabasehandler/unit_tests.c -o ./simpledatabasehandler/unit_tests.o -I./simpledatabasehandler
#	gcc -g -c ./simpledatabasehandler/simpledatabasehandler.c -o ./simpledatabasehandler/simpledatabasehandler.o -I./simpledatabasehandler
#	gcc -g -o ./unit_tests ./simpledatabasehandler/unit_tests.o ./simpledatabasehandler/simpledatabasehandler.o

clean:
	rm -f *.o workerupdate workerinstall