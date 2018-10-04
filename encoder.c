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

    FILE *SA_FILE = fopen("ext", "w+");
    FILE *T_FILE = fopen("ext2", "w+");

    setup_files(SA_FILE, T_FILE);

    /* modify the data to suit the algo */
    printf("%d is delimeter\n", delimeter);
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

	int bkt[129] = {0};

    int del_size = level0_main(T, bkt, tmp_len, delimeter);
    printf("%d is del_size\n", del_size);

    T = malloc(sizeof(char) * len);
    fseek(f, 0, SEEK_SET);
    fread(T, sizeof(char), sizeof(char) * len, f);
    fclose(f);

    int* SA = malloc(sizeof(int) * (tmp_len / 2 + 1));

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

    fseek(SA_FILE, 0, SEEK_SET);
    fwrite(SA, sizeof(int), tmp_len / 2, SA_FILE);

    // fseek(SA_FILE, 0, SEEK_SET);
    // for(int i = 0;i < tmp_len;i++){
    //     int tmp;
    //     fread(&tmp, sizeof(int), 1, SA_FILE);
    //     printf("%d ", tmp);
    // }
    // printf("\n");
    fseek(SA_FILE, 0, SEEK_SET);
    fread(SA, sizeof(int), tmp_len / 2, SA_FILE);

    /* With this proper SA, we can construct a index file in O(n) time 
     * and this file would only contain n / 4 * 4bytes infor, so it would
     * not be bigger than the original one
     */

    f = fopen(output, "w+");
    printf("traling is %d\n", trailing_del);
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
    printf("half %d\n", i);

    fread(SA, sizeof(int), tmp_len - tmp_len / 2, SA_FILE);

    for(;i < tmp_len;i++){
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
    printf("full %d\n", i);


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
    for(i = 0;i < tmp_len / 2;i++){
        if(BWT[i] == delimeter){
            int tmp = SA[i] - 1;
            AUX[pos] = tmp < 0 ? tmp_len - 1 : tmp;
            pos++;
        }
    }

    fread(SA, sizeof(int), tmp_len - tmp_len / 2, SA_FILE);
    for(;i < tmp_len;i++){
        if(BWT[i] == delimeter){
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
    int *del_pos = malloc(sizeof(int) * del_size);
    fseek(SA_FILE, 0, SEEK_SET);
    fread(del_pos, sizeof(int), del_size, SA_FILE);



    for(int i = del_size - 1;i >= 0;i--){
        if(del_pos[i] >= tmp_len / 2){
            break;
        }
        int tmp = del_pos[i];
        SA[tmp] = i;
    }

    fseek(SA_FILE, 0, SEEK_SET);
    fwrite(SA, sizeof(int), tmp_len / 2, SA_FILE);

    memset(SA, 0, (tmp_len / 2 + 1) * sizeof(int));

    for(int i = 0;i < del_size;i++){
        if(del_pos[i] < tmp_len / 2){
            break;
        }
        int tmp = del_pos[i];
        SA[tmp - tmp_len / 2] = i;
    }


    fwrite(SA, sizeof(int), tmp_len / 2 + 1, SA_FILE);

    fseek(SA_FILE, 0, SEEK_SET);
    // for(int i = 0;i < tmp_len;i++){
    //     int tmp;
    //     fread(&tmp, sizeof(int), 1, SA_FILE);
    //     printf("%d ", tmp);
    // }
    // printf("\n");
    
    memset(del_pos, 0, sizeof(int) * del_size);

    // for(int i = 0;i < del_size;i++){
    //     printf("%d ", AUX[i]);
    // }
    // printf("\n");

    for(int i = 0;i < del_size;i++){
        int index = AUX[i];
        if(index >= tmp_len / 2){
            int tmp = index - tmp_len / 2;
            // printf(" i = %d SA[%d] = %d\n",i, index, SA[tmp]);
            // del_pos[SA[tmp]] = i - 1 >= 0 ? i - 1 : del_size - 1;
            del_pos[i] = SA[tmp] - 1 < 0 ? del_size - 1 : SA[tmp] - 1;
        }
    }

    fseek(SA_FILE, 0, SEEK_SET);
    fread(SA, sizeof(int), tmp_len / 2, SA_FILE);

    for(int i = 0;i < del_size;i++){
        int index = AUX[i];
        if(index < tmp_len / 2){
            // printf(" i = %d SA[%d] = %d\n",i, index, SA[index]);
            // del_pos[SA[index]] = i - 1 >= 0 ? i - 1 : del_size - 1;
            del_pos[i] = SA[index] - 1 < 0 ? del_size - 1 : SA[index] - 1;
        }
    }

    // for(int i = 0;i < del_size;i++){
    //     printf("%d ", del_pos[i]);
    // }
    // printf("\n");

    fwrite(del_pos, sizeof(int), del_size, f);
    fclose(f);


    /* free SA and bkt */
    free(del_pos);
    free(SA);
    free(AUX);
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
    strcat(aux_file, ".aux");

    char* BWT = encode(encode_file, output_file, aux_file,delimeter);

    return 0;
}
