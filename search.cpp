#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define BUFSIZE (13 * 1024 * 1024)
#define BKTSIZE 128
#define MASK (0xFFFFFE00)
char delimeter;

int backSearch(const char *query);

int makeOcc(char *occ_table, FILE *bwt_file, int *bkt);
int getOcc(int n, char c, FILE *occ, FILE *bwt);

int count(FILE *bwt, FILE *occ, const char *query, int *bkt);
int decode(FILE *bwt, FILE *occ, int index, int *bkt);

int main(int argc, char const *argv[])
{
    const char *deli_string = argv[1];
    if (strlen(deli_string) > 1)
    {
        delimeter = '\n';
    }
    else
    {
        delimeter = deli_string[0];
    }

    const char *path = argv[2];
    const char *folder = argv[3];
    printf("%s\n", folder);

    const char *opt = argv[4];
    const char *query_string = argv[5];
    printf("%s\n", query_string);

    char *occ_table = malloc(strlen(folder) + 10);
    strcpy(occ_table, folder);
    strcat(occ_table, "/occ");

    FILE *bwt_file = fopen(path, "r");

    FILE *occ = fopen(occ_table, "r");
    int *bkt = malloc(sizeof(int) * (BKTSIZE + 1));
    if (occ == NULL)
    {
        makeOcc(occ_table, bwt_file, bkt);
    }
    else{
        fseek(occ, -sizeof(int) * BKTSIZE, SEEK_END);
        fread(bkt, sizeof(int), BKTSIZE, occ);
    }

    int sum = 0;
    for(int i = 0;i < BKTSIZE + 1;i++){
        int tmp = sum;
        sum += bkt[i];
        bkt[i] = tmp;
    }

    occ = fopen(occ_table, "r");

    if (strcmp(opt, "-m") == 0)
    {
        int cnt = count(bwt_file, occ, query_string, bkt);
        printf("cnt is %d\n", cnt);
    }

    decode(bwt_file, occ, 2, bkt);


    return 0;
}

int makeOcc(char *occ_table, FILE *bwt_file, int *bkt)
{
    size_t file_len;
    fseek(bwt_file, 0, SEEK_END);
    file_len = ftell(bwt_file);
    fseek(bwt_file, 0, SEEK_SET);

    FILE *occ = fopen(occ_table, "w");

    char *readbuf = malloc(sizeof(char) * BUFSIZE);
    size_t readsize;
    while (true)
    {
        readsize = fread(readbuf, sizeof(char), BUFSIZE, bwt_file);
        for (int i = 0; i < readsize; i++)
        {
            char c = readbuf[i];
            if(c == delimeter){
                c = 0;
            }
            bkt[c]++;
            if (i % 1024 == 1024 - 1)
            {
                fwrite(bkt, sizeof(int), BKTSIZE, occ);
            }
        }

        if (readsize < BUFSIZE)
        {
            fwrite(bkt, sizeof(int), BKTSIZE, occ);
            break;
        }
    }
    fseek(bwt_file, 0, SEEK_SET);
    fclose(occ);
}

int getOcc(int n, char c, FILE *occ, FILE *bwt){
    int ret = 0;
    /* mask to get which entry */
    size_t offset = ((n & MASK) >> 10);
    /* calculate the real offset in occ file */
    offset = offset * sizeof(int) * BKTSIZE;
    fseek(occ, 0, SEEK_END);
    size_t len = ftell(occ);

    /* if offset is larger means we run into the last block */
    if(offset >= len){
        offset = len;
    }
    // printf("offset is %d\n", offset);
    int *bkt = malloc(sizeof(int) * BKTSIZE);
    memset(bkt, 0, sizeof(int) * BKTSIZE);
    if(offset != 0){
        int real_offset = offset - BKTSIZE * sizeof(int);
        fseek(occ, real_offset, SEEK_SET);
        fread(bkt, sizeof(int), BKTSIZE, occ);
    }
    ret = bkt[c];
    free(bkt);

    /* now read the real bwt to find the accurate occ */
    offset = offset / (sizeof(int) * BKTSIZE) * 1024;
    char *readbuf = malloc(sizeof(char) * 1024);
    fseek(bwt, offset, SEEK_SET);
    fread(readbuf, sizeof(char), 1024, bwt);

    for(int i = 0;i <= n - offset;i++){
        if(readbuf[i] == c){
            ret++;
        }
    }

    free(readbuf);
    return ret;
}

int count(FILE *bwt, FILE *occ, const char *query, int *bkt){
    int len = strlen(query);
    int start = bkt[query[len - 1]];
    int end = bkt[query[len - 1] + 1] - 1;
    for(int i = len - 2;i >= 0;i--){
        start = getOcc(start, query[i], occ, bwt) + bkt[query[i]] - 1;
        end = getOcc(end, query[i], occ, bwt) + bkt[query[i]] - 1;
        if(start > end){
            return 0;
        }
    }

    return end - start;

}

int decode(FILE *bwt, FILE *occ, int index, int *bkt){
    char *buf = malloc(sizeof(char) * 5000);
    char c;
    fseek(bwt, index, SEEK_SET);
    fread(&c, sizeof(char), 1, bwt);
    fseek(bwt, 0, SEEK_SET);
    buf[0] = c;
    int i = 1;
    while(c != delimeter){
        int before = getOcc(index, c, occ, bwt) - 1;
        index = before + bkt[c];
        fseek(bwt, index, SEEK_SET);
        fread(&c, sizeof(char), 1, bwt);
        fseek(bwt, 0, SEEK_SET);
        buf[i] = c;
        i++;
    }
    for(int j = i - 2;j >= 0;j--){
        printf("%c", buf[j]);
    }
    printf("\n");
}