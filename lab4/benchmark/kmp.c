#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <inttypes.h>

#define EXTEND_SIZE 10

typedef struct Num {
    uint32_t num;
    size_t index;
    int64_t line;
} Num;

typedef struct TextReader {
    Num *buffer;
    size_t bufSize;
    size_t offset;

    size_t lineNum;
    size_t lineIndex;
} _TextReader, *TextReader;

TextReader createTextReader(size_t patternSize) {
    TextReader res = malloc(sizeof(_TextReader));
    
    res->offset = 0;
    res->bufSize = patternSize + 3;
    res->buffer = malloc(sizeof(Num) * res->bufSize);
    res->lineNum = 0;
    res->lineIndex = 0;

    return res;
}

void deleteTextReader(TextReader reader) {
    free(reader->buffer);
    free(reader);
}

Num nextNum(TextReader reader) {
    bool parseNum = false;
    uint32_t num = 0;

    while (1) {
        int c = fgetc(stdin);

        if (c == ' ' || c == '\n' || c == EOF) {
            Num res = {num, reader->lineIndex, reader->lineNum};

            if (!parseNum && c == EOF) {
                res.line = -1;
                return res;
            } 

            if (parseNum) {
                reader->lineIndex++;
            }

            if (c == '\n') {
                reader->lineNum++;
                reader->lineIndex = 0;
            }

            if (parseNum) {
                return res;
            }
        }

        char letter = c;
        if ('0' <= letter && letter <= '9') {
            num = num * 10 + (letter - '0');
            parseNum = true;
        }
    }
}

Num getNum(TextReader reader, size_t index) {
    while (reader->offset <= index ) {
        reader->buffer[reader->offset % reader->bufSize] = nextNum(reader); 
        if (reader->buffer[reader->offset % reader->bufSize].line == -1) {
            return reader->buffer[reader->offset % reader->bufSize];
        }
        
        reader->offset++;
    }
    
    return reader->buffer[index % reader->bufSize];
}

uint32_t *readSequence(size_t *size) {
    uint32_t *sequence = malloc(sizeof(uint32_t) * EXTEND_SIZE);
    size_t sequenceCap = EXTEND_SIZE;
    size_t sequenceSize = 0;
    uint32_t num = 0;
    bool parseNum = false;
    
    while (1) {
        int symbol = fgetc(stdin);

        if (symbol == '\n' || symbol == EOF) {
            if (!parseNum) break;

            if (sequenceSize >= sequenceCap) {
                sequence = realloc(sequence, sizeof(uint32_t) * (sequenceCap + EXTEND_SIZE));
                sequenceCap += EXTEND_SIZE;
            }

            sequence[sequenceSize] = num;
            sequenceSize++;
            break;
        }

        char c = symbol;
        
        if (c == ' ') {
            if (!parseNum) continue;
            parseNum = false;
            
            if (sequenceSize >= sequenceCap) {
                sequence = realloc(sequence, sizeof(uint32_t) * (sequenceCap + EXTEND_SIZE));
                sequenceCap += EXTEND_SIZE;
            }

            sequence[sequenceSize] = num;
            sequenceSize++;
            num = 0;
        }

        if ('0' <= c && c <= '9') {
            parseNum = true;
            num *= 10;
            num += c - '0';
        }
    }

    *size = sequenceSize;
    return sequence;
}

uint16_t *calcZ(uint32_t *s, size_t m) {
    uint16_t *Z = malloc(sizeof(uint16_t) * m);

    size_t l = 0;
    size_t r = 0;
    
    Z[0] = 0;
    
    for (size_t i = 1; i < m; i++) {
        if (i < r) {
            // если мы в пределах текущего z блока, переходим к решению задачи в начале строки
            // если значение длина строки B не превосходит Z функции в соответствующем индексе,
            // строка B целиком совпадает, а дальше заканчивается Z блок, поэтому там точно нет совпадения
            // в противном случае, совпадает по меньшей мере количество букв, равное Z функции
            Z[i] = r - i < Z[i - l] ? r - i : Z[i - l];
        } else {
            // если нет - такова се ля ви, считаем по определению 
            Z[i] = 0;
        }

        // расширяем общий префикс насколько это возможно
        while (i + Z[i] < m && s[Z[i]] == s[i + Z[i]]) {
            Z[i]++;
        }
        
        // если нашли Z блок с большей правой границей - обновляем
        if (i + Z[i] > r) {
            l = i;
            r = i + Z[i];
        }
    }

    return Z;
}

uint16_t *preprocess(uint32_t *p, size_t m) {
    uint16_t *Z = calcZ(p, m);

    uint16_t *SP = malloc(sizeof(uint16_t) * m);
    
    for (size_t i = 0; i < m; i++) {
        SP[i] = 0;
    }

    for (size_t j = m; j >= 2; j--) {
        size_t i = j + Z[j - 1] - 1;
        SP[i - 1] = Z[j - 1];
    }

    free(Z);

    return SP;
}

void search(uint32_t *P, size_t m, uint16_t *SP) {
    if (m == 0) return;

    size_t c = 1;
    size_t p = 1;

    TextReader reader = createTextReader(m);

    while (!feof(stdin)) {
        while (p <= m && P[p - 1] == getNum(reader, c - 1).num && 
            getNum(reader, c - 1).line != -1) {
                
            p++;
            c++;
        }
        
        if (p == m + 1) {
            Num num = getNum(reader, c - m - 1);
            printf("%ld, %lu\n", num.line + 1, num.index + 1);
        } 

        if (p == 1) {
            c++;
        } else {
            p = SP[p - 2] + 1;
        }
    }

    deleteTextReader(reader);
}

int main() {
    size_t patternSize;
    uint32_t *pattern = readSequence(&patternSize);
    clock_t start = clock();    
    uint16_t *SP = preprocess(pattern, patternSize);

    search(pattern, patternSize, SP);
    double timePassed = (double)(clock() - start) / CLOCKS_PER_SEC * 1000.0;
    printf("time: %fms\n", timePassed);

    free(SP);
    free(pattern);
    return 0;
}
