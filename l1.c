#include "SACA_k.h"
#include <stdbool.h>
#include <string.h>

/* LEVEL 1 IS MUCH MORE TRICKY 
 * EACH NAME ALWAYS POINT TO THE
 * START/END OF EACH BUCKET BASE
 * ON THE TYPE.
 */

/* the original paper used some very very tricky
 * math technique to determain the type of a element
 * But since there is a easier way to do that. I
 * slightly modified it. This could slow the performance
 * down a little bit, since we need to scan T1 one more
 * time to set the S/L type. But it's still linear.
 */

enum SHIFT {
    LEFT,
    RIGHT
};

enum LS {
    LARGE,
    SMALL
};


/* Shift a bkt if it is ocupied by other element */
static int shiftBkt(int* SA, int pos, int i, enum SHIFT sft)
{
    int tmp, tmp2;
    int t, dir;
    if (sft == LEFT) {
        dir = -1;
    } else {
        dir = 1;
    }

    tmp = SA[pos];
    for (t = pos + dir; SA[t] >= 0 || SA[t] == EMPTY; t += dir) {
        tmp2 = SA[t];
        SA[t] = tmp;
        tmp = tmp2;
    }
    SA[t] = tmp;

    /* mesure the step if we shift elements over the current position */
    if (t < i && sft == LEFT) {
        return 0;
    } else if (t > i && sft == RIGHT) {
        return 0;
    }
    return 1;
}


/* storebkt after the borrowed slot is now empty
 * or there is no borrowed slot at all.
 */
static int storeBktS(int* SA, int pos, int j, int i, int len)
{
    int step = 1;
    
    if (SA[pos] == EMPTY) {
        /* it's empty */
        if (SA[pos - 1] != EMPTY) {
            SA[pos] = j;
        } else {
            SA[pos - 1] = j;
            SA[pos] = -1;
        }
    } else {
        /* it's a counter */
        int d = SA[pos];
        int realpos = pos + d - 1;
        if (SA[realpos] != EMPTY) {
            /* bkt full */
            /* move right */
            for (int t = 0; t < -d; t++) {
                SA[pos - t] = SA[pos - t - 1];
            }
            SA[realpos + 1] = j;
            if (pos > i)
                step = 0;
        } else if (SA[realpos] == EMPTY) {
            SA[realpos] = j;
            SA[pos]--;
        }
    }
    return step;
}

static int storeBktL(int* SA, int pos, int j, int i, int len)
{
    int step = 1;
    if (SA[pos] == EMPTY) {
        /* it's empty */
        if (pos + 1 >= len || SA[pos + 1] != EMPTY) {
            SA[pos] = j;
        } else {
            SA[pos + 1] = j;
            SA[pos] = -1;
        }
    } else {
        /* it's a counter */
        int d = SA[pos];
        int realpos = pos - d + 1;
        if (realpos >= len || SA[realpos] != EMPTY) {
            /* bkt full */
            /* move left */
            for (int t = 0; t < -d; t++) {
                SA[pos + t] = SA[pos + t + 1];
            }
            SA[realpos - 1] = j;
            if (pos < i)
                step = 0;
        } else if (SA[realpos] == EMPTY) {
            SA[realpos] = j;
            SA[pos]--;
        }
    }
    return step;
}

int level1_main(int* T, int* SA, int len)
{
    place_lms_1(T, SA, len);
    induceL_1(T, SA, len, true);
    induceS_1(T, SA, len, true);

    /* now all lms are in position, compact them */
    int T1_len = compactLMS_1(SA, T, len);
    int name_size = renameLMS_1(T, SA, T1_len, len);

    int *T1 = SA + len - T1_len;

    if(name_size < T1_len){
        /* need recursive */
        // printf("need recursive name size is %d\n", name_size);
        level1_main(T1, SA, T1_len);
    }
    else{
        for (int i = 0; i < T1_len; i++) {
            int pos = T1[i];
            SA[pos] = i;
        }
    }

    /* LMS are sorted */
    retrive1(T, SA, len, T1_len);
    induceL_1(T, SA, len, false);
    induceS_1(T, SA, len, false);
    return 0;
}

/* same technique as level0, but we get rid of delimeters
 */
int set_lms_1(int* T, int len)
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

int place_lms_1(int* T, int* SA, int len)
{
    for (int i = 0; i < len; i++) {
        SA[i] = EMPTY;
    }

    for (int i = len - 1; i > 0; i--) {
        if (!IS_S(T[i - 1]) && IS_S(T[i])) {
            /* i is LMS */
            int pos = CHAR_VAL(T[i]);
            /* pos point to the end of its bkt */
            if (SA[pos] >= 0) {
                /* bkt borrowed by right */
                shiftBkt(SA, pos, i, RIGHT);
                SA[pos] = EMPTY;
            }

            storeBktS(SA, pos, i, i, len);
        }
    }

    for (int i = 1; i < len; i++) {
        if (SA[i] != EMPTY) {
            if (SA[i] < 0) {
                /* a counter */
                int d = -SA[i];
                for (int j = 0; j < d; j++) {
                    SA[i - j] = SA[i - j - 1];
                }
                SA[i - d] = EMPTY;
            }
        }
    }

    SA[0] = len - 1;
    return 0;
}

int induceL_1(int* T, int* SA, int len, bool erase)
{
    int step = 1;
    for (int i = 0; i < len; i += step) {
        step = 1;
        if (SA[i] == EMPTY) {
            continue;
        }

        int prev = SA[i];
        int j = prev - 1;
        /* we only induce L type */
        if (IS_S(T[j])) {
            continue;
        }
        /* pos point to the start of bkt*/
        int pos = CHAR_VAL(T[j]);

        if (SA[pos] >= 0) {
            /* bkt borrowed by left */
            if (shiftBkt(SA, pos, i, LEFT) == 0) {
                step = 0;
            }
            SA[pos] = EMPTY;
        }

        if (storeBktL(SA, pos, j, i, len) == 0) {
            step = 0;
        }

        if (i > 0 && (erase || IS_S(T[prev]))) {
            if (step == 1) {
                SA[i] = EMPTY;
            } else {
                SA[i - 1] = EMPTY;
            }
        }
    }

    /* get rid of counters */
    for (int i = 1; i < len; i++) {
        if (SA[i] != EMPTY) {
            if (SA[i] < 0) {
                /* a counter */
                int d = -SA[i];
                for (int j = 0; j < d; j++) {
                    SA[i + j] = SA[i + j + 1];
                }
                SA[i + d] = EMPTY;
            }
        }
    }
    return 0;
}

int induceS_1(int* T, int* SA, int len, bool erase)
{
    int step = 1;
    for (int i = len - 1; i >= 0; i-=step) {
        step = 1;
        if (SA[i] == EMPTY) {
            continue;
        }

        int j = SA[i] - 1;
        if(j < 0){
            continue;
        }

        /* we only induce S types */
        if (!IS_S(T[j])) {
            continue;
        }
        int pos = CHAR_VAL(T[j]);
        /* pos point to the end of bkt */
        if (SA[pos] >= 0) {
            /* borrowed by right */
            if (shiftBkt(SA, pos, i, RIGHT) == 0) {
                step = 0;
            }
            SA[pos] = EMPTY;
        }

        if (storeBktS(SA, pos, j, i, len) == 0) {
            step = 0;
        }

        if (erase) {
            if (step == 1) {
                SA[i] = EMPTY;
            } else {
                SA[i + 1] = EMPTY;
            }
        }
    }

    /* get rid of counters */
    for (int i = 0; i < len; i++) {
        if (SA[i] != EMPTY) {
            if (SA[i] < 0) {
                /* a counter */
                int d = -SA[i];
                for (int j = 0; j < d; j++) {
                    SA[i - j] = SA[i - j - 1];
                }
                SA[i - d] = EMPTY;
            }
        }
    }
    return 0;
}

int compactLMS_1(int* SA, int* T, int len)
{

    int pos = 0;
    for (int i = 0; i < len; i++) {
        if (SA[i] > 0) {
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

static int lms_len_0(int* T, int lms, int len)
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

static bool cmp_lms_0(int* T, int prev_lms, int now_lms, int lms_len)
{
    for (int i = 0; i < lms_len; i++) {
        /* we don't need to convert val cos they should have the same type */
        if (T[prev_lms + i] != T[now_lms + i]) {
            return false;
        }
    }
    return true;
}

int renameLMS_1(int* T, int* SA, int T1_len, int T_len)
{
    int name;
    int namecnt = 1;
    int* start = SA + T1_len;

    int prev, now, prev_len;
    prev = SA[0];
    prev_len = lms_len_0(T, prev, T_len);
    name = 0;
    start[SA[0] / 2] = name;
    SA[name] = 1;
    
    for (int i = 1; i < T1_len; i++) {
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
    int cnt = 0;
    int pos = T_len - T1_len - 1;
    for (int i = T_len - T1_len - 1; i >= 0; i--) {
        if (start[i] != EMPTY) {
            start[pos] = start[i];
            start[i] = EMPTY;
            pos--;
            cnt++;
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

int retrive1(int* T, int* SA, int len, int T1_len)
{
    int* last = SA + len - 1;
    /* we assume the last one is always 0 */
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
        SA[i] = EMPTY;
    }

    /* now place the lms */
    int cur, pre, pos;
    pre = -1;
    for (int i = T1_len - 1; i >= 0; i--) {
        int j;
        j = SA[i];
        SA[i] = EMPTY;
        cur = CHAR_VAL(T[j]);
        if (pre != cur) {
            pre = cur;
            pos = cur;
        }
        SA[pos] = j;
        pos--;
    }
    return 0;
}