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
    fread(T, sizeof(char), sizeof(char) * len, f);

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

    int tmp_len = len;
    int trailing_del = 0;
    while(T[tmp_len - 1] == 2){
        tmp_len--;
        trailing_del++;
    }

    int* SA = malloc(sizeof(int) * len);
    memset(SA, 0, sizeof(int) * len);
    int* bkt = malloc(sizeof(int) * BKTSIZE);
    memset(bkt, 0, sizeof(int) * BKTSIZE);

    int del_size = level0_main(T, SA, bkt, tmp_len, delimeter);

    fseek(f, 0, SEEK_SET);
    fread(T, sizeof(char), sizeof(char) * len, f);
    fclose(f);

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


    /* With this proper SA, we can construct a index file in O(n) time 
     * and this file would only contain n / 4 * 4bytes infor, so it would
     * not be bigger than the original one
     */

    f = fopen(output, "w+");
    printf("traling is %d\n", trailing_del);
    fwrite(T + tmp_len, sizeof(char), trailing_del, f);
    

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
        BWT[i] = c;
    }


    /* free SA and bkt */
    free(bkt);
    fwrite(BWT, sizeof(char), tmp_len, f);
    fclose(f);
    free(SA);
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

    return 0;
}
