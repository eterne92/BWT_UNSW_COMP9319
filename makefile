# all: clean encoder searcher
# searcher:
# 	gcc -std=c11 -g -o bwtsearch search.c
# encoder: level0 level1
# 	gcc -std=c11 -o3 -o bwtencode encoder.c l0.o l1.o
# test: clean level0 level1
# 	gcc -std=c11  -o3 -o test.o test.c l0.o l1.o
# level0:
# 	gcc -std=c11  -o3 -o l0.o -c l0.c
# level1:
# 	gcc -std=c11  -o3 -o l1.o -c l1.c
# clean:
# 	rm -rf *.o
# 	rm bwt*

all: clean encoder searcher
searcher:
	gcc -std=c11 -g -o bwtsearch search.c
encoder: level0 level1
	gcc -std=c11 -g -o bwtencode encoder.c l0.o l1.o
test: clean level0 level1
	gcc -std=c11  -o3 -o test.o test.c l0.o l1.o
level0:
	gcc -std=c11  -o3 -o l0.o -c l0.c
level1:
	gcc -std=c11  -o3 -o l1.o -c l1.c
clean:
	rm -rf *.o
	rm bwt*

