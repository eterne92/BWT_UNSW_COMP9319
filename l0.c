#include "SACA_k.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/* at level 0
 * A extra bkt array is used to record the bktsize
 */


int del_size = 0;

FILE *SA_FILE;
FILE *T_FILE;

void setup_files(FILE *s, FILE *t){
    SA_FILE = s;
    T_FILE = t;
}

static void dump_file(int *SA, int len){
    fseek(SA_FILE, 0, SEEK_SET);
    fwrite(SA, sizeof(int), len, SA_FILE);
}

static int get_SA(int *SA, int index){
    if(index < MEM_MAX){
        return SA[index];
    }
    else{
        fseek(SA_FILE, index * sizeof(int), SEEK_SET);
        int tmp;
        fread(&tmp, sizeof(int), 1, SA_FILE);
        return tmp;
    }
}

static void set_SA(int *SA, int index, int value){
    if(index < MEM_MAX){
        SA[index] = value;
    }
    else{
        fseek(SA_FILE, index * sizeof(int), SEEK_SET);
        fwrite(&value, sizeof(int), 1, SA_FILE);
    }
}

void print_SA(int *SA, int len){
    for(int i = 0;i < len;i++){
        int tmp = get_SA(SA, i);
        if(tmp == EMPTY){
            printf("^ ");
        }
        else{
            printf("%d ", tmp);
        }
    }
    printf("\n");
}




/* We only use char less than 127, so the
 * highest bit of the input string char is
 * used to indicate the S/L type.
 */
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

/* gen_bkt based on input string. All delimeters
 * have been set at value 1.
 */
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

/* Place all lms at their position(back of the bkt, 
 * one by one). As the assumption made for normal
 * input, there is no back to back LMS(There is always
 * a L type in between). But that won't suit our spec,
 * we consider all delimeters are LMS. So the input
 * file should get modified.
 */
int place_lms_0(char* T, int* SA, int len, int* bkt)
{
    int cnt = 0;
    for (int i = 0; i < len - 2; i++) {
        if (CHAR_VAL(T[i + 1]) == 1) {
            /* it's a del */
            /* Delimeters are placed base on their
             * original position. The last char of
             * input string T(which is always a 
             * delimeter) is put at the front.
             * So at deeper level, the last element
             * of T is always 0.
             */
            int pos = bkt[1];
            // SA[pos] = i + 1;
            set_SA(SA, pos, i + 1);
            bkt[1]--;
            cnt++;
            del_size++;
            continue;
        }
        if (!IS_S(T[i]) && IS_S(T[i + 1])) {
            /* T[i + 1] is a LMS */
            char c = CHAR_VAL(T[i + 1]);
            int pos = bkt[c];
            // SA[pos] = i + 1;
            set_SA(SA, pos, i + 1);
            bkt[c]--;
            cnt++;
        }
    }
    // SA[0] = len - 1;
    set_SA(SA, 0, len - 1);
    del_size++;
}


int induceL_0(char* T, int* SA, int* bkt, int len, bool erase)
{
    for (int i = 0; i < len; i++) {
        int SA_i = get_SA(SA, i);
        // if (SA[i] != 0) {
        if(SA_i != 0){
            // int prev = SA[i];
            int prev = SA_i;
            // int j = SA[i] - 1;
            int j = SA_i - 1;
            if (T[j] <= 0) {

                continue;
            }
            /* now it's a L type */
            char c = CHAR_VAL(T[j]);
            int pos = bkt[c];
            // SA[pos] = j;
            set_SA(SA, pos, j);
            bkt[c]++;
            if (erase) {
                /* erase the previous one */
                if (CHAR_VAL(T[prev]) != 1) {
                    /* delimeters are all set at position
                     * don't move them.
                     */
                    // SA[i] = 0;
                    set_SA(SA, i, 0);
                }
            }
        }
    }
}

int induceS_0(char* T, int* SA, int* bkt, int len, bool erase)
{
    for (int i = len - 1; i >= 0; i--) {
        int SA_i = get_SA(SA, i);
        // if (SA[i] != 0) {
        if(SA_i != 0){
            // int prev = SA[i];
            int prev = SA_i;
            // int j = SA[i] - 1;
            int j = SA_i - 1;
            if (!IS_S(T[j])) {
                continue;
            }
            /* now it's a S type */
            char c = CHAR_VAL(T[j]);
            /* still we don't care about the
             * delimeters. Since they are always
             * LMS
             */
            if (c != 1) {
                int pos = bkt[c];
                // SA[pos] = j;
                set_SA(SA, pos, j);
                bkt[c]--;
            }
            if (erase) {
                // SA[i] = 0;
                set_SA(SA, i, 0);
            }
        }
    }
}

/* Move all the LMS to the left end of SA
 * The rest space is reserved for T1
 */
int compactLMS_0(int* SA, char* T, int len)
{

    int pos = 0;
    for (int i = 0; i < len; i++) {
        int SA_i = get_SA(SA, i);
        // if (SA[i] != 0) {
        if(SA_i != 0){
            /* it's LMS */
            // SA[pos] = SA[i];
            set_SA(SA, pos, SA_i);
            pos++;
        }
    }

    for (int i = pos; i < len; i++) {
        // SA[i] = EMPTY;
        set_SA(SA, i, EMPTY);
    }

    return pos;
}



static bool cmp_lms_0(char* T, int prev_lms, int now_lms, int len)
{
    int i1 = prev_lms;
    int i2 = now_lms;
    while(i1 < len && i2 < len){
        if(T[i1] != T[i2]){
            return false;
        }
        i1++;
        i2++;
        if(i1 == len || i2 == len){
            return false;
        }
        if(IS_S(T[i1]) && !IS_S(T[i1 - 1])){
            /* i1 is LMS */
            return T[i1] == T[i2];
        }
        if(IS_S(T[i2]) && !IS_S(T[i2 - 1])){
            /* i2 is LMS */
            return T[i1] == T[i2];
        }
    }
    return false;
}

int renameLMS_0(char* T, int* SA, int T1_len, int T_len)
{
    int name;
    int namecnt = 0;
    // int* start = SA + T1_len;
    int start = T1_len;
    /* first every del got it's own name */
    /* DELS CANT BE BACK TO BACK, otherwise
     * start[pos / 2] will be the same. And
     * EVERY THING GOES TO HELL.
     */
    for (int i = 0; i < del_size; i++) {
        // int pos = SA[i];
        int pos = get_SA(SA, i);
        name = i;
        // start[pos / 2] = name;
        set_SA(SA, pos / 2 + start, name);
        // SA[i] = 1;
        set_SA(SA, i, 1);
        namecnt++;
    }

    int prev, now, prev_len;
    // prev = SA[del_size - 1];
    prev = get_SA(SA, del_size - 1);
    /* Now set up all the other LMS 
     * We only have at most len / 2 LMS
     * and LMS can't be back to back
     * So start[pos / 2] should be unique
     */
    for (int i = del_size; i < T1_len; i++) {
        bool same = false;
        // now = SA[i];
        now = get_SA(SA, i);
        same = cmp_lms_0(T, prev, now, T_len);
        if (same) {
            // int pos = SA[i];
            int pos = now;
            // SA[name]++;
            int tmp = get_SA(SA, name);
            set_SA(SA, name, tmp+1);
            // start[pos / 2] = name;
            set_SA(SA, pos / 2 + start, name);
        } else {
            // int pos = SA[i];
            int pos = now;
            name = i;
            // SA[name] = 1;
            set_SA(SA, name, 1);
            // start[pos / 2] = name;
            set_SA(SA, pos / 2 + start, name);
            namecnt++;
            prev = now;
        }
    }
    return namecnt;
}

void move_name(int *SA, int T_len, int T1_len){
    int* start = SA + T1_len;
    /* move all names to the back of SA */
    int pos = T_len - T1_len - 1;
    for (int i = T_len - T1_len - 1; i >= 0; i--) {
        if (start[i] != EMPTY) {
            int tmp = start[i];
            start[i] = EMPTY;
            start[pos] = tmp;
            pos--;
        }
    }

}

void rename_s(int *SA, int T_len, int T1_len){
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
}

int retrive0(char* T, int* SA, int* bkt, int len, int T1_len)
{
    // int* last = SA + len - 1;
    int last = len - 1;
    /* we assume the last one is always del */
    // *last = len - 1;
    // last--;
    set_SA(SA, last, len - 1);
    last--;

    /* we already got the last del, i+1 start from the one
     * before the last del
     */
    for (int i = len - 3; i >= 0; i--) {
        if (!IS_S(T[i]) && IS_S(T[i + 1])) {
            /* T[i + 1] is a LMS */
            // *last = i + 1;
            set_SA(SA, last, i + 1);
            last--;
        }
    }
    last++;

    // print_SA(SA, len);

    /* Now T1 got it's real name back 
     * set SA to retrive the name, each
     * SA element point to the position
     * of T1
     */
    for (int i = 0; i < T1_len; i++) {
        // int pos = SA[i];
        int pos = get_SA(SA, i);
        // SA[i] = last[pos];
        int last_pos = get_SA(SA, last + pos);
        set_SA(SA, i, last_pos);
    }
    for (int i = T1_len; i < len; i++) {
        // SA[i] = 0;
        set_SA(SA, i, 0);
    }
    // print_SA(SA, len);

    /* Now SA at its sorted compact version
     * put all LMS at it's correct position
     * base on the bkt.
     */
    for (int i = T1_len - 1; i >= del_size; i--) {
        int SA_i = get_SA(SA, i);
        char c = CHAR_VAL(T[SA_i]);
        int tmp = SA_i;
        // SA[i] = 0;
        set_SA(SA, i, 0);
        int pos = bkt[c];
        bkt[c]--;
        // SA[pos] = tmp;
        set_SA(SA, pos, tmp);
    }
}


int level0_main(char *T, int *bkt, int len, char del){
    int *SA = malloc(sizeof(int) * MEM_MAX);
    memset(SA, 0, sizeof(int) * MEM_MAX);

    set_lms_0(T, len);

    gen_bkt(T, bkt, len, END);
    place_lms_0(T, SA, len, bkt);
    // print_SA(SA, len);

    gen_bkt(T, bkt, len, START);
    induceL_0(T, SA, bkt, len, true);

    gen_bkt(T, bkt, len, END);
    induceS_0(T, SA, bkt, len, true);

    // print_SA(SA, len);

    /* ONLY LMS LEFT IN SA NOW */
    int T1_len = compactLMS_0(SA, T, len);
    // print_SA(SA, len);

    int name_size = renameLMS_0(T, SA, T1_len, len);
    // print_SA(SA, len);

    /* dump T to file */
    fwrite(T, sizeof(char), len, T_FILE);
    free(T);

    if(len >= MEM_MAX){
        /* dump sa to file */
        dump_file(SA, MEM_MAX);
        free(SA);

        SA = malloc(sizeof(int) * len);
        fseek(SA_FILE, 0, SEEK_SET);
        fread(SA, sizeof(int), len, SA_FILE);
    }

    move_name(SA, len, T1_len);

    int* T1 = SA + len - T1_len;
    if (name_size < T1_len) {
        rename_s(SA, len, T1_len);
        /* need recursive */
        level1_main(T1, SA, T1_len);
    } else {
        /* name is unique */
        for (int i = 0; i < T1_len; i++) {
            int pos = T1[i];
            SA[pos] = i;
        }
    }

    /* dump SA to file */
    if(len >= MEM_MAX){
        dump_file(SA, len);
        free(SA);
        SA = malloc(sizeof(int) * MEM_MAX);
        fseek(SA_FILE, 0, SEEK_SET);
        fread(SA, sizeof(int), MEM_MAX, SA_FILE);
    }

    fseek(T_FILE, 0, SEEK_SET);
    T = malloc(sizeof(char) * len);
    fread(T, sizeof(char), len, T_FILE);


    /* NOW LMS IS SORTED */
    /* SET LMS AT IT'S POSITION */
    gen_bkt(T, bkt, len, END);
    retrive0(T, SA, bkt, len, T1_len);


    gen_bkt(T, bkt, len, START);
    induceL_0(T, SA, bkt, len, false);

    gen_bkt(T, bkt, len, END);
    induceS_0(T, SA, bkt, len, false);

    if(len >= MEM_MAX){
        dump_file(SA, MEM_MAX);
    }
    else{
        dump_file(SA, len);
    }
    free(SA);
    free(T);
    return del_size;
}
