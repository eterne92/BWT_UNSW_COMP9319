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
    FILE* f = fopen("dummy.txt", "r");
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    T = malloc(sizeof(char) * len);
    SA = malloc(sizeof(int) * len);
    bkt = malloc(sizeof(int) * BKTSIZE);

    fread(T, sizeof(char), len, f);
    fclose(f);
    char del = '|';
    int prev = 0;
    for(int i = 0 ;i < len;i++){
        if(T[i] == del && prev == del){
            prev = del;
            T[i] = 2;
        }
        else if(T[i] == del){
            prev = del;
            T[i] = 1;
        }
        else{
            prev = T[i];
        }
        printf("%d ", T[i]);
    }
    time_t read_done = time(NULL);
    printf("read done, with time as %lf\n", difftime(read_done, start));
    level0_main(T, SA, bkt, len, del);
    time_t end = time(NULL);
    printf("SA construct done, with time as %lf\n", difftime(end, read_done));
    free(bkt);

    int pos = 0;
    for(int i = len - 1;i >= 0;i--){
        if(abs(T[i]) == 1 || abs(T[i]) == 2){
            printf("find del\n");
            SA[pos] = i;
            pos++;
        }
    }

    for(int i = 0 ;i < len;i++){
        char c = abs(T[SA[i]]);
        if(c == 1){
            c = del;
        }
        printf("%d ", SA[i]);
    }
    printf("\n");

    char* BWT = (char*)SA;

    for (int i = 0; i < len; i++) {
        char c;
        if (SA[i] > 0) {
            int pos = SA[i] - 1;
            c = abs(T[pos]);
        } else {
            c = abs(T[len - 1]);
        }
        if (c == 1 || c == 2) {
            c = del;
        }
        BWT[i] = c;
    }

    memcpy(T, BWT, len * sizeof(char));
    free(SA);

    FILE* f2 = fopen("output.txt", "w");
    fwrite(T, sizeof(char), len * sizeof(char), f2);
    free(T);
    fclose(f2);
    time_t write_done = time(NULL);
    printf("write done, with time as %lf\n", difftime(write_done, end));
}
