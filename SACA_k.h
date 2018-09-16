#pragma once

#include <stdio.h>
#include <stdbool.h>

#define EMPTY (1 << 31)

#define BKTSIZE 128


enum DIRECTION {
    START,
    END
};


void print_SA(int* SA, int len);

/* LEVEL0 OPERATIONS */
int level0_main(char *T, int *SA, int *bkt, int len, char del);
int set_lms_0(char* T, int len);
void gen_bkt(char* T, int* bkt, int len, enum DIRECTION dir);
int place_lms_0(char* T, int* SA, int len, int* bkt);
int induceL_0(char* T, int* SA, int* bkt, int len, bool erase);
int induceS_0(char* T, int* SA, int* bkt, int len, bool erase);
int compactLMS_0(int* SA, char* T, int len);
int retrive0(char* T, int* SA, int* bkt, int len, int T1_len);
int renameLMS_0(char* T, int* SA, int T1_len, int T_len);

/* LEVEL1 OPERATIONS */
int level1_main(int *T, int *SA, int len);
int set_lms_1(int* T, int len);
int place_lms_1(int* T, int* SA, int len);
int induceL_1(int* T, int* SA, int len, bool erase);
int induceS_1(int* T, int* SA, int len, bool erase);
int compactLMS_1(int* SA, int* T, int len);
int renameLMS_1(int* T, int* SA, int T1_len, int T_len);
int retrive1(int* T, int* SA, int len, int T1_len);