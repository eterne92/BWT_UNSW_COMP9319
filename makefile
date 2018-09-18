all: encoder
encoder: clean level0 level1
	gcc -o3 -o bwtencode encoder.c l0.o l1.o
test: clean level0 level1
	gcc -o3 -o test.o test.c l0.o l1.o
level0:
	gcc -o3 -o l0.o -c l0.c
level1:
	gcc -o3 -o l1.o -c l1.c
clean:
	rm -rf *.o
# all: encoder
# 	gcc -g -o bwt.o bwt.c encoder.o l0.o l1.o
# encoder: clean level0 level1
# 	gcc -g -o encoder.o -c encoder.c
# test: clean level0 level1
# 	gcc -g -o test.o test.c l0.o l1.o
# level0:
# 	gcc -g -o l0.o -c l0.c
# level1:
# 	gcc -g -o l1.o -c l1.c
# clean:
# 	rm -rf *.o
