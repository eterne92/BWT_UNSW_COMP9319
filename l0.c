#include "SACA_k.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHAR_VAL(x) abs(x)
#define SET_S(x) (x = -x)
#define IS_S(x) (x < 0)

static char del;
int del_size = 0;

void print_T(char* T, int len)
{
    for (int i = 0; i < len; i++) {
        if (T[i] < 0) {
            printf("-");
        }
        if (CHAR_VAL(T[i]) == 1) {
            printf("%c ", del);
        } else {
            printf("%c ", CHAR_VAL(T[i]));
        }
    }
    printf("\n");
}

int set_lms_0(char* T, int len)
{
    bool next_s = true;
    bool current_s;
    SET_S(T[len - 1]);
    for (int i = len - 2; i >= 0; i--) {
        next_s = current_s;
        current_s = CHAR_VAL(T[i]) < CHAR_VAL(T[i + 1]) || (CHAR_VAL(T[i]) == CHAR_VAL(T[i + 1]) && next_s);
        if (current_s) {
            SET_S(T[i]);
        }
    }

    return 0;
}

void gen_bkt(char* T, int* bkt, int len, enum DIRECTION dir)
{
    memset(bkt, 0, BKTSIZE * sizeof(int));
    for (int i = 0; i < len; i++) {
        char c = CHAR_VAL(T[i]);
        bkt[c]++;
    }

    int sum = 0;
    if (dir == START) {
        /* calculate the start position */
        for (int i = 0; i < BKTSIZE; i++) {
            if (bkt[i] != 0) {
                int tmp = sum;
                sum += bkt[i];
                bkt[i] = tmp;
            }
        }
    } else {
        /* calculate the end position */
        for (int i = 0; i < BKTSIZE; i++) {
            if (bkt[i] != 0) {
                int tmp = sum;
                sum += bkt[i];
                bkt[i] = tmp + bkt[i] - 1;
            }
        }
    }
}

int place_lms_0(char* T, int* SA, int len, int* bkt)
{
    int cnt = 0;
    for (int i = 0; i < len - 2; i++) {
        if (CHAR_VAL(T[i + 1]) == 1) {
            /* it's a del */
            int pos = bkt[1];
            SA[pos] = i + 1;
            bkt[1]--;
            cnt++;
            del_size++;
            continue;
        }
        if (!IS_S(T[i]) && IS_S(T[i + 1])) {
            /* T[i + 1] is a LMS */
            char c = CHAR_VAL(T[i + 1]);
            int pos = bkt[c];
            SA[pos] = i + 1;
            bkt[c]--;

            cnt++;
        }
    }
    SA[0] = len - 1;
    del_size++;
}

int induceL_0(char* T, int* SA, int* bkt, int len, bool erase)
{
    for (int i = 0; i < len; i++) {
        if (SA[i] != 0) {
            int prev = SA[i];
            int j = SA[i] - 1;
            if (T[j] <= 0) {
                continue;
            }
            /* now it's a L type */
            char c = CHAR_VAL(T[j]);
            int pos = bkt[c];
            SA[pos] = j;
            bkt[c]++;
            if (erase) {
                /* erase the previous one */
                if (CHAR_VAL(T[prev]) != 1) {
                    SA[i] = 0;
                }
            }
        }
    }
}

int induceS_0(char* T, int* SA, int* bkt, int len, bool erase)
{
    for (int i = len - 1; i >= 0; i--) {
        if (SA[i] != 0) {
            int prev = SA[i];
            int j = SA[i] - 1;
            if (!IS_S(T[j])) {
                continue;
            }
            /* now it's a S type */
            char c = CHAR_VAL(T[j]);
            if (c != 1) {
                int pos = bkt[c];
                SA[pos] = j;
                bkt[c]--;
            }
            if (erase) {
                SA[i] = 0;
            }
        }
    }
}

int compactLMS_0(int* SA, char* T, int len)
{

    int pos = 0;
    for (int i = 0; i < len; i++) {
        if (SA[i] != 0) {
            /* it's LMS */
            SA[pos] = SA[i];
            pos++;
        }
    }

    for (int i = pos; i < len; i++) {
        SA[i] = EMPTY;
    }

    return pos;
}

static int lms_len_0(char* T, int lms, int len)
{
    if (lms == len - 1) {
        /* we are the last char */
        return 1;
    }
    /* got over all the s_type */
    int i = lms;
    while (CHAR_VAL(T[i]) <= CHAR_VAL(T[i + 1])) {
        i++;
    }
    /* now i is L_type */
    while (i < len && CHAR_VAL(T[i]) > CHAR_VAL(T[i + 1])) {
        i++;
    }
    return i - lms;
}

static bool cmp_lms_0(char* T, int prev_lms, int now_lms, int lms_len)
{
    for (int i = 0; i < lms_len; i++) {
        /* we don't need to convert val cos they should have the same type */
        if (T[prev_lms + i] != T[now_lms + i]) {
            return false;
        }
    }
    return true;
}

int renameLMS_0(char* T, int* SA, int T1_len, int T_len)
{
    int name;
    int namecnt = 0;
    int* start = SA + T1_len;
    /* first every del got it's own name */
    for (int i = 0; i < del_size; i++) {
        int pos = SA[i];
        name = i;
        start[pos / 2] = name;
        SA[i] = 1;
        namecnt++;
    }

    int prev, now, prev_len;
    prev = SA[del_size - 1];
    prev_len = lms_len_0(T, prev, T_len);
    for (int i = del_size; i < T1_len; i++) {
        bool same = false;
        now = SA[i];
        int now_len = lms_len_0(T, now, T_len);
        if (now_len == prev_len) {
            same = cmp_lms_0(T, prev, now, now_len);
        }

        if (same) {
            int pos = SA[i];
            SA[name]++;
            start[pos / 2] = name;
        } else {
            int pos = SA[i];
            name = i;
            SA[name] = 1;
            start[pos / 2] = name;
            namecnt++;
            prev = now;
            prev_len = now_len;
        }
    }

    /* move all names to the back of SA */
    int pos = T_len - T1_len - 1;
    for (int i = T_len - T1_len - 1; i >= 0; i--) {
        if (start[i] != EMPTY) {
            start[pos] = start[i];
            start[i] = EMPTY;
            pos--;
        }
    }


    if (namecnt == T1_len) {
        /* all unique names */
        return namecnt;
    }

    /* set LMS for next level */
    int* T1 = SA + T_len - T1_len;
    set_lms_1(T1, T1_len);

    /* rename all Stype of T1 as the back of their bkt */
    for(int i = 0 ;i < T1_len;i++){
        if(IS_S(T1[i])){
            int pos = CHAR_VAL(T1[i]);
            int bktsz = SA[pos];
            T1[i] = CHAR_VAL(T1[i]) + bktsz - 1;
            SET_S(T1[i]);
        }
    }

    return namecnt;
}

int retrive0(char* T, int* SA, int* bkt, int len, int T1_len)
{
    int* last = SA + len - 1;
    /* we assume the last one is always del */
    *last = len - 1;
    last--;

    /* we already got the last del, i+1 start from the one
     * before the last del
     */
    for (int i = len - 3; i >= 0; i--) {
        if (!IS_S(T[i]) && IS_S(T[i + 1])) {
            /* T[i + 1] is a LMS */
            *last = i + 1;
            last--;
        }
    }
    last++;

    for (int i = 0; i < T1_len; i++) {
        int pos = SA[i];
        SA[i] = last[pos];
    }
    for (int i = T1_len; i < len; i++) {
        SA[i] = 0;
    }

    for (int i = T1_len - 1; i >= del_size; i--) {
        char c = CHAR_VAL(T[SA[i]]);
        int tmp = SA[i];
        SA[i] = 0;
        int pos = bkt[c];
        bkt[c]--;
        SA[pos] = tmp;
    }
}


int level0_main(char *T, int *SA, int *bkt, int len, char del){
    del = del;

    for (int i = 0; i < len; i++) {
        if (T[i] == del) {
            T[i] = 1;
        }
    }

    set_lms_0(T, len);
    // print_T(T, len);

    gen_bkt(T, bkt, len, END);
    place_lms_0(T, SA, len, bkt);

    gen_bkt(T, bkt, len, START);
    induceL_0(T, SA, bkt, len, true);

    gen_bkt(T, bkt, len, END);
    induceS_0(T, SA, bkt, len, true);

    int T1_len = compactLMS_0(SA, T, len);

    int name_size = renameLMS_0(T, SA, T1_len, len);
    int* T1 = SA + len - T1_len;
    if (name_size < T1_len) {
        /* need recursive */
        printf("need recursive name size is %d\n", name_size);
        level1_main(T1, SA, T1_len);
    } else {
        for (int i = 0; i < T1_len; i++) {
            int pos = T1[i];
            SA[pos] = i;
        }
    }
    gen_bkt(T, bkt, len, END);
    retrive0(T, SA, bkt, len, T1_len);

    gen_bkt(T, bkt, len, START);
    induceL_0(T, SA, bkt, len, false);

    gen_bkt(T, bkt, len, END);
    induceS_0(T, SA, bkt, len, false);
    return 0;
}
