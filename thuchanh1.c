#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORD_LEN 64
#define MAX_LINE 1024

typedef struct {
    char word[MAX_WORD_LEN];
    char *lineNumbers;
    size_t lineCap;
} IndexEntry;

// Kiểm tra stop word
int isStopWord(char *word, char **stopWords, int stopCount) {
    for (int i = 0; i < stopCount; i++) {
        if (strcmp(word, stopWords[i]) == 0)
            return 1;
    }
    return 0;
}

// Kiểm tra danh từ riêng (FIXED)
int isProperNoun(const char *originalWord, char prev) {
    // Không viết hoa → không phải tên riêng
    if (!isupper(originalWord[0])) return 0;

    // Nếu đứng sau dấu kết thúc câu hoặc đầu dòng → KHÔNG phải tên riêng
    if (prev == '.' || prev == '?' || prev == '!' || prev == '\n')
        return 0;

    return 1;
}

// So sánh từ cho qsort
int compareFunc(const void *a, const void *b) {
    return strcmp(((IndexEntry*)a)->word, ((IndexEntry*)b)->word);
}

// Thêm số dòng, KHÔNG bị trùng
void addLineNumber(IndexEntry *entry, int lineNum) {
    char buffer[16];
    sprintf(buffer, "%d,", lineNum);

    int len = strlen(entry->lineNumbers);

    if (len > 0) {
        int last = len - 1;

        while (last > 0 && entry->lineNumbers[last] == ',') last--;

        while (last > 0 && entry->lineNumbers[last - 1] != ',') last--;

        int lastNum = atoi(&entry->lineNumbers[last]);
        if (lastNum == lineNum) return;
    }

    size_t needed = strlen(buffer) + 1;
    if (strlen(entry->lineNumbers) + needed >= entry->lineCap) {
        entry->lineCap *= 2;
        entry->lineNumbers = realloc(entry->lineNumbers, entry->lineCap);
        if (!entry->lineNumbers) {
            perror("realloc failed");
            exit(1);
        }
    }
    strcat(entry->lineNumbers, buffer);
}

int main() {
    char textFile[256], stopFile[256];

    printf("Nhap ten tep van ban: ");
    scanf("%255s", textFile);
    printf("Nhap ten tep stop words: ");
    scanf("%255s", stopFile);

    FILE *ftext = fopen(textFile, "r");
    FILE *fstop = fopen(stopFile, "r");
    if (!ftext || !fstop) {
        printf("Khong the mo tep.\n");
        return 1;
    }

    // Đọc stop words
    char **stopWords = NULL;
    int stopCount = 0;
    char tmp[MAX_WORD_LEN];
    while (fscanf(fstop, "%63s", tmp) == 1) {
        for (int i = 0; tmp[i]; i++) tmp[i] = tolower(tmp[i]);
        stopWords = realloc(stopWords, sizeof(char*) * (stopCount + 1));
        stopWords[stopCount] = strdup(tmp);
        stopCount++;
    }
    fclose(fstop);

    IndexEntry *index = NULL;
    int indexCount = 0;

    char line[MAX_LINE];
    int lineNum = 0;

    while (fgets(line, MAX_LINE, ftext)) {
        lineNum++;

        char *ptr = line;
        char prevChar = '\n'; // Đầu dòng = giống sau dấu câu

        while (*ptr) {

            // Bỏ qua ký tự không phải chữ
            while (*ptr && !isalpha(*ptr)) {
                prevChar = *ptr;     // FIX: cập nhật mọi ký tự
                ptr++;
            }

            if (!*ptr) break;

            char original[MAX_WORD_LEN];
            char word[MAX_WORD_LEN];
            int k = 0;

            // Lấy từ
            while (*ptr && isalpha(*ptr) && k < MAX_WORD_LEN - 1) {
                original[k] = *ptr;
                ptr++;
                k++;
            }
            original[k] = '\0';

            // Chuyển lowercase để so sánh
            for (int i = 0; i <= k; i++)
                word[i] = tolower(original[i]);

            // Skip stop word
            if (isStopWord(word, stopWords, stopCount)) continue;

            // Skip Proper Nouns (FIXED)
            if (isProperNoun(original, prevChar)) continue;

            // Tìm trong index
            int found = 0;
            for (int i = 0; i < indexCount; i++) {
                if (strcmp(index[i].word, word) == 0) {
                    addLineNumber(&index[i], lineNum);
                    found = 1;
                    break;
                }
            }

            if (!found) {
                index = realloc(index, sizeof(IndexEntry) * (indexCount + 1));
                strcpy(index[indexCount].word, word);

                index[indexCount].lineCap = 64;
                index[indexCount].lineNumbers = malloc(index[indexCount].lineCap);
                index[indexCount].lineNumbers[0] = '\0';

                addLineNumber(&index[indexCount], lineNum);
                indexCount++;
            }
        }
    }
    fclose(ftext);

    // Xoá dấu phẩy cuối
    for (int i = 0; i < indexCount; i++) {
        int len = strlen(index[i].lineNumbers);
        if (len > 0 && index[i].lineNumbers[len - 1] == ',')
            index[i].lineNumbers[len - 1] = '\0';
    }

    qsort(index, indexCount, sizeof(IndexEntry), compareFunc);

    FILE *fout = fopen("index.txt", "w");
    if (!fout) {
        perror("Khong the tao index.txt");
        return 1;
    }

    for (int i = 0; i < indexCount; i++) {
        fprintf(fout, "%-12s %s\n", index[i].word, index[i].lineNumbers);
    }
    fclose(fout);

    printf("Da tao index.txt thanh cong!\n");

    // Free memory
    for (int i = 0; i < indexCount; i++)
        free(index[i].lineNumbers);
    free(index);

    for (int i = 0; i < stopCount; i++)
        free(stopWords[i]);
    free(stopWords);

    return 0;
}
