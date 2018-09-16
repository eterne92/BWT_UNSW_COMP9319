#include "SACA_k.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char const* argv[])
{
    time_t start = time(NULL);
    int len;
    int *SA, *bkt;
    char* T;
    FILE* f = fopen("newtest.txt", "r");
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    T = malloc(sizeof(char) * len);
    SA = malloc(sizeof(int) * len);
    bkt = malloc(sizeof(int) * BKTSIZE);

    fread(T, sizeof(char), len, f);
    time_t read_done = time(NULL);
    printf("read done, with time as %lf\n", difftime(read_done, start));
    char del = '$';
    level0_main(T, SA, bkt, len, del);
    time_t end = time(NULL);
    printf("SA construct done, with time as %lf\n", difftime(end, read_done));

    char* BWT = (char*)SA;

    for(int i = 0;i < len;i++){
        printf("%d ", SA[i]);
    }

    for (int i = 0; i < len; i++) {
        char c;
        if (SA[i] > 0) {
            int pos = SA[i] - 1;
            c = abs(T[pos]);
        } else {
            c = abs(T[len - 1]);
        }
        if (c == 1) {
            c = del;
        }
        BWT[i] = c;
    }

    memcpy(T, BWT, len * sizeof(char));

    FILE* f2 = fopen("output.txt", "w");
    fwrite(T, sizeof(char), len * sizeof(char), f2);
    free(SA);
    time_t write_done = time(NULL);
    printf("write done, with time as %lf\n", difftime(write_done, end));
}