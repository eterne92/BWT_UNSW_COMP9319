#include "SACA_k.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* encode(const char* filename, const char* output, const char *aux_file, char delimeter)
{
    FILE* f = fopen(filename, "r");
    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    fseek(f, 0, SEEK_SET);
    /* touch memory very carefully */
    char* T = malloc(sizeof(char) * len);
    fread(T, sizeof(char), len, f);
    fclose(f);

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

    int* SA = malloc(sizeof(int) * len);
    memset(SA, 0, sizeof(int) * len);
    int* bkt = malloc(sizeof(int) * BKTSIZE);
    memset(bkt, 0, sizeof(int) * BKTSIZE);

    int del_size = level0_main(T, SA, bkt, len, delimeter);

    /* NOW SA IS SORTED */
    /* since all real records delimeters are in bkt 1 and 
     * empty records in bkt 2(which we don't care). And all
     * delimeters in bkt 1 is sorted decrease by their position.
     * it's not hard to find that if we just put all the delimeters
     * from tail to head into the SA, the new SA will keep the
     * same property as the old one.
     */
    int pos = 0;
    for (int i = len - 1; i >= 0; i--) {
        if (abs(T[i]) == 1 || abs(T[i]) == 2) {
            SA[pos] = i;
            pos++;
        }
    }

    
    /* construct BWT based on SA and modified T */
    char* BWT = (char*)SA;

    for (int i = 0; i < len; i++) {
        char c;
        if (SA[i] > 0) {
            int j = SA[i] - 1;
            c = abs(T[j]);
        } else {
            c = abs(T[len - 1]);
        }
        if (c == 1 || c == 2) {
            c = delimeter;
        }
        BWT[i] = c;
    }


    memcpy(T, BWT, len);
    /* free SA and bkt */
    free(bkt);
    free(SA);
    free(BWT);

    f = fopen(output, "w");
    fwrite(T, sizeof(char), len, f);
    fclose(f);
    free(T);
}

int main(int argc, char const* argv[])
{
    if (argc != 5) {
        return 0;
    }

    char delimeter;
    const char* del_string = argv[1];
    if (strlen(del_string) > 1) {
        // assert(strcmp(del_string, "\\n") == 0);
        delimeter = '\n';
    } else {
        delimeter = del_string[0];
    }

    const char* encode_file = argv[3];
    const char* output_file = argv[4];

    char* aux_file = malloc(strlen(output_file) + 5);
    strcpy(aux_file, output_file);
    snprintf(aux_file, 5, ".aux");
    char* BWT = encode(encode_file, output_file, aux_file,delimeter);
    free(aux_file);

    return 0;
}
