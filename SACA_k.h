/* 
 * This is a implementation of SACA_K algorithm developed by GE NONG
 * A copy of the paper can be found at 
 * https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/ge-nong/saca-k-tois.pdf
 * slightly modified to suit this assignment.
 * This is a algorithm which use pure induce sort to construct Suffix Array.
 * With O(1) extra working space.
 * 
 */
#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/* We assume the original file is always less than 50M. 
 * so 31bit is enough to record all the indexes, 
 * the highest bit can be used to record whether this
 * position is EMPTY or used as a bkt/element
 */
#define EMPTY (1 << 31)

#define CHAR_VAL(x) abs(x)
#define SET_S(x) (x = -x)
#define IS_S(x) (x < 0)

/* The bucket size is limited at 128, we only care about
 * a subset of ascii char. We could compact that a little
 * bit more. But after hours working on this algo. I would
 * just leave it here. as several bytes extra memory is 
 * pretty nothing compare to the 250M limit.
 */
#define BKTSIZE 128

#define MEM_MAX (48 * 1024 * 1024)
// #define MEM_MAX (19 * 1024 * 1024)


enum DIRECTION {
    START,
    END
};


/* DEBUG USE FUNCTION */
void print_SA(int* SA, int len);

/* MOST OF THESE FUNCTIONS SHOULD NOT BEEN USED SEPERATELY
 * since each of them depend on others to be finished.
 * Further infomation of these functions is described at
 * their implementation file.
 * 
 * T: the input string
 * SA: the output Suffix Array
 * bkt: the bucket array
 * len: size of the input string(without trailing '\0')
 * del: delimeter char
 * 
 * All space should have been allocated. No extra heap
 * or stack array used.
 */

/* LEVEL0 OPERATIONS */
/* this is the real entry to the constructor. We always
 * assume there won't be two delimeters together, so the
 * input char *T should be slightly transfered. And after
 * the construction, the suffix array should also be slightly
 * modified to suit the real case.
 */
int level0_main(char *T, int *bkt, int len, char del);

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