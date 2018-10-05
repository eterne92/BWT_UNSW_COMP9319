#include "SACA_k.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void encode(const char *filename, const char *output, const char *aux_file,
             const char *sa_file, const char *t_file, char delimeter) {
    FILE *f = fopen(filename, "r");
    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    fseek(f, 0, SEEK_SET);
    /* touch memory very carefully */
    char *T = malloc(sizeof(char) * len);
    fread(T, sizeof(char), sizeof(char) * len, f);

    /* SA_FILE and T_FILE used to dump Suffix Array and T */
    FILE *SA_FILE = fopen(sa_file, "w+");
    FILE *T_FILE = fopen(t_file, "w+");

    setup_files(SA_FILE, T_FILE);

    /* modify the data to suit the algo */
    char prev = 0;
    for (int i = 0; i < len; i++) {
        /* all delimeters that indicates the EMPTY record is changed to 2
         * so they would never be a LMS at SACA operations.
         */
        if (T[i] == delimeter && prev == delimeter) {
            T[i] = 2;
            prev = delimeter;
        } else if (T[i] == delimeter) {
            /* These are real records */
            T[i] = 1;
            prev = delimeter;
        } else {
            prev = T[i];
        }
    }

    /* count the trailing delimeters */
    int tmp_len = len;
    int trailing_del = 0;
    while (T[tmp_len - 1] == 2) {
        tmp_len--;
        trailing_del++;
    }

    /* we assume the bkt size is 128 */
    int bkt[BKTSIZE + 1] = {0};

    /* now construct the SA for modified T */
    level0_main(T, bkt, tmp_len, delimeter);

    /* at this moment, T is freed, reload it with origin file */
    T = malloc(sizeof(char) * len);
    fseek(f, 0, SEEK_SET);
    fread(T, sizeof(char), sizeof(char) * len, f);
    fclose(f);

    /* from now on, never touch the whole 4n SA memory */
    int *SA = malloc(sizeof(int) * (tmp_len / 2 + 1));

    fseek(SA_FILE, 0, SEEK_SET);
    fread(SA, sizeof(int), tmp_len / 2, SA_FILE);
    /* NOW SA IS SORTED */
    /* since all real records delimeters are in bkt 1 and
     * empty records in bkt 2(which we don't care). And all
     * delimeters in bkt 1 is sorted decrease by their position.
     * it's not hard to find that if we just put all the delimeters
     * from tail to head into the SA, the new SA will keep the
     * same property as the old one.
     */
    int pos = 0;
    for (int i = tmp_len - 1; i >= 0; i--) {
        if (T[i] == delimeter) {
            SA[pos] = i;
            pos++;
        }
    }

    int del_size = pos;

    fseek(SA_FILE, 0, SEEK_SET);
    fwrite(SA, sizeof(int), tmp_len / 2, SA_FILE);


    fseek(SA_FILE, 0, SEEK_SET);
    fread(SA, sizeof(int), tmp_len / 2, SA_FILE);

    /* With this proper SA, we can construct a index file in O(n) time
     * and this file would only contain n / 4 * 4bytes infor, so it would
     * not be bigger than the original one
     */

    f = fopen(output, "w+");
    fwrite(T + tmp_len, sizeof(char), trailing_del, f);

    /* construct BWT based on SA and modified T */
    char *BWT = malloc(sizeof(char) * len);

    int i = 0;
    for (i = 0; i < tmp_len / 2; i++) {
        char c;
        if (SA[i] > 0) {
            int j = SA[i] - 1;
            c = T[j];
        } else {
            c = T[tmp_len - 1];
        }
        BWT[i] = c;
    }

    fread(SA, sizeof(int), tmp_len - tmp_len / 2, SA_FILE);

    for (; i < tmp_len; i++) {
        char c;
        int index = i - tmp_len / 2;
        if (SA[index] > 0) {
            int j = SA[index] - 1;
            c = T[j];
        } else {
            c = T[tmp_len - 1];
        }
        BWT[i] = c;
    }

    /* now BWT is constructed, we don't need T anymore */
    free(T);

    fwrite(BWT, sizeof(char), tmp_len, f);
    fclose(f);

    /* make aux file */
    f = fopen(aux_file, "w+");
    fseek(f, trailing_del * sizeof(int), SEEK_SET);
    int *AUX = malloc(sizeof(int) * del_size);

    fseek(SA_FILE, 0, SEEK_SET);
    fread(SA, sizeof(int), tmp_len / 2, SA_FILE);

    pos = 0;
    for (i = 0; i < tmp_len / 2; i++) {
        /* for each delimeter in BWT, it's position in SA is the position of
         * its successor, so the delimeter's position is just before it.
         */
        if (BWT[i] == delimeter) {
            int tmp = SA[i] - 1;
            AUX[pos] = tmp < 0 ? tmp_len - 1 : tmp;
            pos++;
        }
    }

    fread(SA, sizeof(int), tmp_len - tmp_len / 2, SA_FILE);
    for (; i < tmp_len; i++) {
        if (BWT[i] == delimeter) {
            int tmp = SA[i - tmp_len / 2] - 1;
            AUX[pos] = tmp < 0 ? tmp_len - 1 : tmp;
            pos++;
        }
    }

    /* now AUX contain all the delimeter position, no need for SA and BWT */
    free(BWT);
    /* now there is at most 3n memory in use
     * SA 2n, AUX n.
     */
    memset(SA, 0, (tmp_len / 2) * sizeof(int));
    /* get the delimeter positions from back to front */
    /* and set them into the SA_FILE, so we get a checktable on the file */
    int *del_pos = malloc(sizeof(int) * del_size);
    fseek(SA_FILE, 0, SEEK_SET);
    fread(del_pos, sizeof(int), del_size, SA_FILE);

    for (int i = del_size - 1; i >= 0; i--) {
        if (del_pos[i] >= tmp_len / 2) {
            break;
        }
        int tmp = del_pos[i];
        SA[tmp] = i;
    }

    fseek(SA_FILE, 0, SEEK_SET);
    fwrite(SA, sizeof(int), tmp_len / 2, SA_FILE);

    memset(SA, 0, (tmp_len / 2 + 1) * sizeof(int));

    for (int i = 0; i < del_size; i++) {
        if (del_pos[i] < tmp_len / 2) {
            break;
        }
        int tmp = del_pos[i];
        SA[tmp - tmp_len / 2] = i;
    }

    fwrite(SA, sizeof(int), tmp_len / 2 + 1, SA_FILE);
    /* now check table is done, for each position set their actual position
     * by just checking the check table
     */
    fseek(SA_FILE, 0, SEEK_SET);

    memset(del_pos, 0, sizeof(int) * del_size);

    for (int i = 0; i < del_size; i++) {
        int index = AUX[i];
        if (index >= tmp_len / 2) {
            int tmp = index - tmp_len / 2;
            del_pos[i] = SA[tmp] - 1 < 0 ? del_size - 1 : SA[tmp] - 1;
        }
    }

    fseek(SA_FILE, 0, SEEK_SET);
    fread(SA, sizeof(int), tmp_len / 2, SA_FILE);

    for (int i = 0; i < del_size; i++) {
        int index = AUX[i];
        if (index < tmp_len / 2) {
            del_pos[i] = SA[index] - 1 < 0 ? del_size - 1 : SA[index] - 1;
        }
    }

    fwrite(del_pos, sizeof(int), del_size, f);
    fclose(f);

    /* free SA and bkt */
    free(del_pos);
    free(SA);
    free(AUX);

    /* remove all the tmp files */
    remove(sa_file);
    remove(t_file);
}

int main(int argc, char const *argv[]) {
    if (argc != 5) {
        return 0;
    }

    char delimeter;
    const char *del_string = argv[1];
    if (strlen(del_string) > 1) {
        // assert(strcmp(del_string, "\\n") == 0);
        delimeter = '\n';
    } else {
        delimeter = del_string[0];
    }

    const char *folder = argv[2];

    const char *encode_file = argv[3];
    const char *output_file = argv[4];

    char *aux_file = malloc(strlen(output_file) + 5);
    strcpy(aux_file, output_file);
    strcat(aux_file, ".aux");

    char *sa_file = malloc(strlen(folder) + 10);
    strcpy(sa_file, folder);
    strcat(sa_file, "/sa_file");
    char *t_file = malloc(strlen(folder) + 10);
    strcpy(t_file, folder);
    strcat(t_file, "/t_file");

    encode(encode_file, output_file, aux_file, sa_file, t_file, delimeter);

    free(aux_file);
    free(sa_file);
    free(t_file);

    return 0;
}
