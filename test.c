#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "SACA_k.h"

int main(int argc, char const* argv[])
{
    int len;
    int *SA, *bkt;
    char* T;
    FILE* f = fopen("w3c2", "r");
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    T = malloc(sizeof(char) * len);
    SA = malloc(sizeof(int) * len);
    bkt = malloc(sizeof(int) * BKTSIZE);

    fread(T, sizeof(char), len, f);
    level0_main(T, SA, bkt, len, '$');
}