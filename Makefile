CFLAGS=
LDFLAGS=

all: workerinstall workerupdate 
#simpledatabasehandler_unit_tests

workerinstall: workerinstall.o
	$(CC) workerinstall.o -o $(LDFLAGS) workerinstall

workerupdate: workerupdate.o
	$(CC) workerupdate.o -o $(LDFLAGS) workerupdate

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

#simpledatabasehandler_unit_tests:
#	gcc -g -c ./simpledatabasehandler/unit_tests.c -o ./simpledatabasehandler/unit_tests.o -I./simpledatabasehandler
#	gcc -g -c ./simpledatabasehandler/simpledatabasehandler.c -o ./simpledatabasehandler/simpledatabasehandler.o -I./simpledatabasehandler
#	gcc -g -o ./unit_tests ./simpledatabasehandler/unit_tests.o ./simpledatabasehandler/simpledatabasehandler.o

clean:
	rm -f *.o workerupdate workerinstall