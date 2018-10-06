CFLAG = -std=c11 -g -Wall
all: clean encoder searcher
searcher:
	gcc $(CFLAG) -o bwtsearch search.c
encoder: level0 level1
	gcc $(CFLAG) -o bwtencode encode.c l0.o l1.o
level0:
	gcc $(CFLAG) -o l0.o -c l0.c
level1:
	gcc $(CFLAG) -o l1.o -c l1.c
.PHONY: clean
clean:
	rm -rf *.o
