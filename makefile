all: clean level0 level1
	gcc -g test.c l0.o l1.o
level0:
	gcc -g -o l0.o -c l0.c
level1:
	gcc -g -o l1.o -c l1.c
clean:
	rm -rf *.o