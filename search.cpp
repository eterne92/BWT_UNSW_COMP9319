#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unordered_set>
#include <set>

using namespace std;

#define BUFSIZE (13 * 1024 * 1024)
#define BKTSIZE 128
#define MASK (0xFFFFFE00)
char delimeter;
FILE *aux;
int aux_size;
FILE *tmp_file;

static int find_aux(int index){
    fseek(aux, sizeof(int) * index, SEEK_SET);
    int tmp;
    fread(&tmp, sizeof(int), 1, aux);
    return tmp;
}

int backSearch(const char* query);

int makeOcc(char* occ_table, FILE* bwt_file, int* bkt);
int getOcc(int n, char c, FILE* occ, FILE* bwt);

int count(FILE* bwt, FILE* occ, const char* query, int* bkt);
int decode(FILE* bwt, FILE* occ, int index, int* bkt, bool output, bool from_del);

int main(int argc, char const* argv[])
{
    const char* deli_string = argv[1];
    if (strlen(deli_string) > 1) {
        delimeter = '\n';
    } else {
        delimeter = deli_string[0];
    }

    printf("delimeter is %c\n", delimeter);

    const char* path = argv[2];

    char* aux_file = (char *)malloc(strlen(path) + 5);
    strcpy(aux_file, path);
    strcat(aux_file, ".aux");

    aux = fopen(aux_file, "r");
    fseek(aux, 0, SEEK_END);
    aux_size = ftell(aux) / sizeof(int);
    printf("aux size is %d\n", aux_size);

    const char* folder = argv[3];
    // printf("%s\n", folder);

    const char* opt = argv[4];
    const char* query_string = argv[5];
    // printf("%s\n", query_string);

    char* occ_table = (char *)malloc(strlen(folder) + 10);
    strcpy(occ_table, folder);
    strcat(occ_table, "/occ");

    FILE* bwt_file = fopen(path, "r");

    FILE* occ = fopen(occ_table, "r");
    int* bkt = (int *)malloc(sizeof(int) * (BKTSIZE + 1));
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
        if( i != 0 && bkt[i] > bkt[i - 1] )
            printf("bkt[%d] is %d\n", i, bkt[i]);
    }

    occ = fopen(occ_table, "r");

    if (strcmp(opt, "-m") == 0) {
        // int cnt = count(bwt_file, occ, query_string, bkt);
        // printf("cnt is %d\n", cnt);
    }

    tmp_file = fopen("output.txt", "w+");

    // for(int i = aux_size - 1;i >= aux_size - 10;i --){
        decode(bwt_file, occ, 0, bkt, true, false);
    // }
    return 0;
}

int makeOcc(char* occ_table, FILE* bwt_file, int* bkt)
{
    size_t file_len;
    fseek(bwt_file, 0, SEEK_END);
    file_len = ftell(bwt_file);
    fseek(bwt_file, 0, SEEK_SET);

    FILE* occ = fopen(occ_table, "w");

    char* readbuf = (char *)malloc(sizeof(char) * BUFSIZE);
    size_t readsize;
    while (true) {
        readsize = fread(readbuf, sizeof(char), BUFSIZE, bwt_file);
        for (int i = 0; i < readsize; i++) {
            char c = readbuf[i];
            if(c == 1){
                printf("%d index is 1\n", i);
            }
            if (c == delimeter) {
                c = 0;
            }
            bkt[c]++;
            if (i % 1024 == 1024 - 1) {
                fwrite(bkt, sizeof(int), BKTSIZE, occ);
            }
        }

        if (readsize < BUFSIZE) {
            fwrite(bkt, sizeof(int), BKTSIZE, occ);
            break;
        }
    }
    fseek(bwt_file, 0, SEEK_SET);
    fclose(occ);
}

int getOcc(int n, char c, FILE* occ, FILE* bwt)
{
    int ret = 0;
    /* mask to get which entry */
    size_t offset = ((n & MASK) >> 10);
    /* calculate the real offset in occ file */
    offset = offset * sizeof(int) * BKTSIZE;
    fseek(occ, 0, SEEK_END);
    size_t len = ftell(occ);

    /* if offset is larger means we run into the last block */
    if (offset >= len) {
        offset = len;
    }
    // printf("offset is %d\n", offset);
    int* bkt = (int *)malloc(sizeof(int) * BKTSIZE);
    memset(bkt, 0, sizeof(int) * BKTSIZE);
    if (offset != 0) {
        int real_offset = offset - BKTSIZE * sizeof(int);
        fseek(occ, real_offset, SEEK_SET);
        fread(bkt, sizeof(int), BKTSIZE, occ);
    }
    ret = bkt[c];
    free(bkt);

    /* now read the real bwt to find the accurate occ */
    offset = offset / (sizeof(int) * BKTSIZE) * 1024;
    char* readbuf = (char *)malloc(sizeof(char) * 1024);
    fseek(bwt, offset, SEEK_SET);
    fread(readbuf, sizeof(char), 1024, bwt);

    for (int i = 0; i <= n - offset; i++) {
        if (readbuf[i] == c || (c == 0 && readbuf[i] == delimeter)) {
            ret++;
        }
    }

    free(readbuf);
    return ret;
}

int count(FILE* bwt, FILE* occ, const char* query, int* bkt)
{
    int len = strlen(query);
    int start = bkt[query[len - 1]] - 1;
    int end = bkt[query[len - 1] + 1] - 1;

    for (int i = len - 2; i >= 0; i--) {
        start = getOcc(start, query[i], occ, bwt) + bkt[query[i]] - 1;
        end = getOcc(end, query[i], occ, bwt) + bkt[query[i]] - 1;
        if (start > end) {
            return 0;
        }
    }

    unordered_set<int> index;

    for (int i = start + 1; i <= end; i++) {
        int j = decode(bwt, occ, i, bkt, false, false);
        index.insert(j);
    }

    printf("done part 1\n");


    int rec = 0;
    set<int> recs;
    for(auto v:index){
        int tmp = find_aux(v);
        recs.insert(tmp);
        // break;
    }
    for(auto v:recs){
        printf("%d index\n", aux_size - v);
        decode(bwt, occ, v + 1, bkt, true, true);
    }
    printf("rec size is %u\n", recs.size());
    // decode(bwt, occ, 0, bkt, true, true);
    //     // printf("try to get decode\n");
    //     int before = getOcc(v, 0, occ, bwt);
    //     // break;
    //     printf("%d, %d\n", before, v);
    // }
    // printf("%ld\n", index.size());
    return end - start;
}

int decode(FILE* bwt, FILE* occ, int index, int* bkt, bool output, bool from_del)
{
    char* buf = (char *)malloc(sizeof(char) * 500000);
    char c;
    fseek(bwt, index, SEEK_SET);
    fread(&c, sizeof(char), 1, bwt);
    fseek(bwt, 0, SEEK_SET);
    buf[0] = c;
    int i = 1;

    while (c != delimeter) {
        int before = getOcc(index, c, occ, bwt) - 1;
        index = before + bkt[c];
        // if(c == '\n'){
            printf("c is %c and index %d, before %d, bkt %d ",c, index, before, bkt[c]);
        // }
        fseek(bwt, index, SEEK_SET);
        fread(&c, sizeof(char), 1, bwt);
        printf("got %c\n", c);
        buf[i] = c;
        i++;
    }

    char *write_buf = (char *) malloc(sizeof(char) * 500000);

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

    int ret = getOcc(index, 0, occ, bwt);

    free(buf);
    free(write_buf);
    return ret;
}