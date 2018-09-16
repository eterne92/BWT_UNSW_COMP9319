#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "SACA_k.h"

void encode(const char *filename, char delimeter){
    FILE *f = fopen(filename, "r");
    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    fseek(f, 0, SEEK_SET);
    /* touch memory very carefully */
    char *origin = malloc(sizeof(char) * len);
    char *T = malloc(sizeof(char) * len);
    fread(origin, sizeof(char), sizeof(char) * len, f);
    fclose(f);

    /* modify the data to suit the algo */
    int pos = 0;
    char prev = 0;
    for(int i = 0;i < len;i++){
        if(prev == delimeter && origin[i] == delimeter){
            continue;
        }
        else if(origin[i] == delimeter){
            prev = delimeter;
            T[pos] = 1;
            pos++;
        }
        else{
            T[pos] = origin[i];
            pos++;
            prev = origin[i];
        }
    }

    free(origin);
    int *SA = malloc(sizeof(int) * pos);
    memset(SA, 0, sizeof(int) * pos);
    int *bkt = malloc(sizeof(int) * BKTSIZE);
    memset(bkt, 0, sizeof(int) * BKTSIZE);

    int del_size = level0_main(T, SA, bkt, pos, delimeter);

    /* NOW SA IS SORTED */

    /* construct BWT based on SA and modified T */
    char* BWT = (char*)SA;

    for (int i = 0; i < pos; i++) {
        char c;
        if (SA[i] > 0) {
            int j = SA[i] - 1;
            c = abs(T[j]);
        } else {
            c = abs(T[pos - 1]);
        }
        if (c == 1) {
            c = delimeter;
        }
        BWT[i] = c;
    }

    memcpy(T, BWT, pos);
    int T_len = pos;
    /* free SA and bkt */
    free(SA);
    free(bkt);
    origin = malloc(sizeof(char) * len);
    /* now we only use 2n bytes */
    f = fopen(filename, "r");
    fread(origin, sizeof(char), sizeof(char) * len, f);
    fclose(f);

    BWT = malloc(sizeof(char) * len);
    memset(BWT, 0, len);
    pos = 0;
    for(int i = len - 2;i >= 0;i--){
        /* put the char before delimeter at position */
        if(origin[i + 1] == delimeter){
            BWT[pos] = origin[i];
            pos++;
        }
    }
    memcpy(BWT + pos, T + del_size, T_len - del_size);

    f = fopen("output", "w");
    fwrite(BWT, sizeof(char), len, f);
    fclose(f);
    free(BWT);
    free(origin);
    free(T);
}