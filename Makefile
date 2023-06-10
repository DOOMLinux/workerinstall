CFLAGS=-Wall $(shell pkg-config --cflags libxml-2.0)
LDFLAGS= $(shell pkg-config --libs libxml-2.0)

all: bin/workerinstall bin/workerupdate 
#simpledatabasehandler_unit_tests

bin/workerinstall: workerinstall.o
	$(CC) $(LDFLAGS) workerinstall.o -o bin/workerinstall

bin/workerupdate: workerupdate.o
	$(CC) $(LDFLAGS) workerupdate.o -o bin/workerupdate

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

#simpledatabasehandler_unit_tests:
#	gcc -g -c ./simpledatabasehandler/unit_tests.c -o ./simpledatabasehandler/unit_tests.o -I./simpledatabasehandler
#	gcc -g -c ./simpledatabasehandler/simpledatabasehandler.c -o ./simpledatabasehandler/simpledatabasehandler.o -I./simpledatabasehandler
#	gcc -g -o ./unit_tests ./simpledatabasehandler/unit_tests.o ./simpledatabasehandler/simpledatabasehandler.o

clean:
	rm -f *.o bin/workerupdate bin/workerinstall