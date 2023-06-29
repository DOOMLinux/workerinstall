CFLAGS=-Wall $(shell pkg-config --cflags libxml-2.0 libcurl)
LDFLAGS= $(shell pkg-config --libs libxml-2.0 libcurl) -lm

all: bin/worker
#simpledatabasehandler_unit_tests

bin/workerinstall: workerinstall.o
	$(CC) $(LDFLAGS) -g workerinstall.o -o bin/workerinstall

bin/worker: worker.o
	$(CC) $(LDFLAGS) -g worker.o -o bin/worker

%.o:%.c
	$(CC) $(CFLAGS) -g -c $< -o $@

#simpledatabasehandler_unit_tests:
#	gcc -g -c ./simpledatabasehandler/unit_tests.c -o ./simpledatabasehandler/unit_tests.o -I./simpledatabasehandler
#	gcc -g -c ./simpledatabasehandler/simpledatabasehandler.c -o ./simpledatabasehandler/simpledatabasehandler.o -I./simpledatabasehandler
#	gcc -g -o ./unit_tests ./simpledatabasehandler/unit_tests.o ./simpledatabasehandler/simpledatabasehandler.o

clean:
	rm -f *.o bin/workerinstall bin/worker

test:
	./bin/workerinstall -v -P test-root/ workerinstall

install:
	@printf "Install not implemented\n"