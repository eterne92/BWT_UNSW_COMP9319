#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE (2 * 1024 * 1024)
#define BKTSIZE 128
#define MASK (0xFFFFFE00)
char delimeter;
FILE *aux;
int aux_size;
int bwt_size;

static int find_aux(int index) {
    fseek(aux, sizeof(int) * index, SEEK_SET);
    int tmp;
    fread(&tmp, sizeof(int), 1, aux);
    return tmp;
}

int backSearch(const char *query);

void makeOcc(char *occ_table, FILE *bwt_file, int *bkt);
int getOcc(int n, char c, FILE *occ, FILE *bwt);

int count(FILE *bwt, FILE *occ, const char *query, int *bkt, bool distinct,
          bool output);
int decode(FILE *bwt, FILE *occ, int index, int *bkt, bool output);
int backward_search(FILE *bwt, FILE *occ, int index, int *bkt, int start,
                    int end, char target_char);

int main(int argc, char const *argv[]) {
    const char *deli_string = argv[1];
    if (strlen(deli_string) > 1) {
        delimeter = '\n';
    } else {
        delimeter = deli_string[0];
    }

    const char *path = argv[2];

    char *aux_file = (char *)malloc(strlen(path) + 5);
    strcpy(aux_file, path);
    strcat(aux_file, ".aux");

    aux = fopen(aux_file, "r");
    fseek(aux, 0, SEEK_END);
    aux_size = ftell(aux) / sizeof(int);

    const char *folder = argv[3];
    // printf("%s\n", folder);

    const char *opt = argv[4];
    const char *query_string = argv[5];
    // printf("%s\n", query_string);

    char *occ_table = (char *)malloc(strlen(folder) + 10);
    strcpy(occ_table, folder);
    strcat(occ_table, "/occ");

    FILE *bwt_file = fopen(path, "r");
    fseek(bwt_file, 0, SEEK_SET);
    bwt_size = ftell(bwt_file);

    FILE *occ = fopen(occ_table, "r");
    int bkt[BKTSIZE + 1] = {0};
    if (occ == NULL) {
        makeOcc(occ_table, bwt_file, bkt);
    } else {
        fseek(occ, -sizeof(int) * BKTSIZE, SEEK_END);
        fread(bkt, sizeof(int), BKTSIZE, occ);
    }

    int sum = 0;
    for (int i = 0; i < BKTSIZE + 1; i++) {
        int tmp = sum;
        sum += bkt[i];
        bkt[i] = tmp;
    }

    occ = fopen(occ_table, "r");

    if (strcmp(opt, "-m") == 0) {
        int cnt = count(bwt_file, occ, query_string, bkt, false, false);
        printf("%d\n", cnt);
    } else if (strcmp(opt, "-n") == 0) {
        int cnt = count(bwt_file, occ, query_string, bkt, true, false);
        printf("%d\n", cnt);
    } else if (strcmp(opt, "-a") == 0) {
        int cnt = count(bwt_file, occ, query_string, bkt, true, true);
    }
    else if(strcmp(opt, "-i") == 0){
        int start, end;
        sscanf(query_string, "%d%d", &start, &end);
        for(int i = start; i <= end; i++){
            decode(bwt_file, occ, aux_size - i, bkt, true);
        }
    }

    return 0;
}

void makeOcc(char *occ_table, FILE *bwt_file, int *bkt) {
    size_t file_len;
    fseek(bwt_file, 0, SEEK_END);
    file_len = ftell(bwt_file);
    fseek(bwt_file, 0, SEEK_SET);
    FILE *occ;

    /* only generate occ when file is "large" */
    if(file_len >= 1024){
        occ = fopen(occ_table, "w+");
    }

    char readbuf[BUFSIZE];
    size_t readsize;
    while (true) {
        readsize = fread(readbuf, sizeof(char), BUFSIZE, bwt_file);
        for (int i = 0; i < readsize; i++) {
            char c = readbuf[i];
            if (c == delimeter) {
                c = 0;
            }
            bkt[c]++;
            if (i % 1024 == 1024 - 1) {
                fwrite(bkt, sizeof(int), BKTSIZE, occ);
            }
        }

        if (file_len < 1024) {
            /* don't write to file */
            return;
        }

        if (readsize < BUFSIZE) {
            fwrite(bkt, sizeof(int), BKTSIZE, occ);
            break;
        }
    }

    fclose(occ);
}

int getOcc(int n, char c, FILE *occ, FILE *bwt) {
    int ret = 0;
    /* mask to get which entry */
    size_t offset = ((n & MASK) >> 10);
    /* calculate the real offset in occ file */
    offset = offset * sizeof(int) * BKTSIZE;
    size_t len;
    if(occ){
        fseek(occ, 0, SEEK_END);
        len = ftell(occ);
    }
    else{
        len = 0;
    }

    /* if offset is larger means we run into the last block */
    if (offset >= len) {
        offset = len;
    }
    // printf("offset is %d\n", offset);
    int bkt[BKTSIZE] = {0};
    if (offset != 0) {
        int real_offset = offset - BKTSIZE * sizeof(int);
        fseek(occ, real_offset, SEEK_SET);
        fread(bkt, sizeof(int), BKTSIZE, occ);
    }
    ret = bkt[c];

    /* now read the real bwt to find the accurate occ */
    offset = offset / (sizeof(int) * BKTSIZE) * 1024;
    char readbuf[1024];
    fseek(bwt, offset, SEEK_SET);
    fread(readbuf, sizeof(char), 1024, bwt);

    for (int i = 0; i < n - offset; i++) {
        if (readbuf[i] == c || (c == 0 && readbuf[i] == delimeter)) {
            ret++;
        }
    }

    return ret;
}

static int cmp(const void *a, const void *b) {
    return (*((int *)b) - *((int *)a));
}

int count(FILE *bwt, FILE *occ, const char *query, int *bkt, bool distinct,
          bool output) {
    int len = strlen(query);
    int start = bkt[query[len - 1]];
    int end = bkt[query[len - 1] + 1] - 1;

    for (int i = len - 2; i >= 0; i--) {
        start = getOcc(start, query[i], occ, bwt) + bkt[query[i]];
        end = getOcc(end + 1, query[i], occ, bwt) + bkt[query[i]] - 1;
        if (start > end) {
            return 0;
        }
    }

    if (!distinct) {
        return end - start + 1;
    }

    int index[50000] = {0};
    int pos = 0;

    for (int i = start; i <= end; i++) {
        int j = backward_search(bwt, occ, i, bkt, start, end, query[0]);
        if (j != -1) {
            index[pos] = j;
            pos++;
        }
    }

    if (!output) {
        return pos;
    }

    for (int i = 0; i < pos; i++) {
        int tmp = find_aux(index[i]);
        index[i] = tmp;
    }

    qsort(index, pos, sizeof(int), cmp);

    for (int i = 0; i < pos; i++) {
        printf("%d\n", aux_size - index[i]);
    }

    return pos;
}

int decode(FILE *bwt, FILE *occ, int index, int *bkt, bool output) {
    char *buf = (char *)malloc(sizeof(char) * 6000);
    char c;
    fseek(bwt, index, SEEK_SET);
    fread(&c, sizeof(char), 1, bwt);
    buf[0] = c;

    int i = 1;

    while (c != delimeter) {
        int before = getOcc(index, c, occ, bwt);
        index = before + bkt[c];
        fseek(bwt, index, SEEK_SET);
        fread(&c, sizeof(char), 1, bwt);
        buf[i] = c;
        i++;
    }

    char *write_buf = (char *)malloc(sizeof(char) * 6000);

    int pos = 0;
    for (int j = i - 2; j >= 0; j--) {
        write_buf[pos] = buf[j];
        pos++;
    }
    write_buf[pos] = '\n';
    write_buf[pos + 1] = 0;

    if (output == true) {
        printf("%s", write_buf);
    }

    // int ret = getOcc(index, 0, occ, bwt);

    free(buf);
    free(write_buf);
    return 0;
}

int backward_search(FILE *bwt, FILE *occ, int index, int *bkt, int start,
                    int end, char target_char) {
    char c;
    fseek(bwt, index, SEEK_SET);
    fread(&c, sizeof(char), 1, bwt);

    while (c != delimeter) {
        int before = getOcc(index, c, occ, bwt);
        index = before + bkt[c];
        if (c == target_char) {
            if (index >= start && index <= end) {
                return -1;
            }
        }
        fseek(bwt, index, SEEK_SET);
        fread(&c, sizeof(char), 1, bwt);
    }

    int ret = getOcc(index, 0, occ, bwt);
    return ret;
}
