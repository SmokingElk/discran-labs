#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>

#define EXTEND_SIZE 10
#define NOT_LEAF UINT32_MAX
#define PRINT_LINE() printf("LINE: %d\n", __LINE__)
#define PRINT_LOG(str) printf("%s, LINE: %d\n", str, __LINE__)

struct S3_Node;

typedef struct Pair {
    char key;
    struct S3_Node *value;
} Pair;

typedef struct Map {
    unsigned char size;
    unsigned char cap;
    Pair *items;
} Map;

void initMap(Map *map);
void setMap(Map *map, char key, struct S3_Node *value);
struct S3_Node* getMap(Map *map, char key);
void extendMap(Map *map);
void deleteMap(Map *map, void (*destructor)(struct S3_Node*));

typedef char* Pos;

typedef struct S3_Node {
    uint32_t leafNum;
    Pos start;

    // используем letter для внутренних вершин и ptr для листьев (глобальный указатель на конец)
    union {
        Pos letter;
        Pos *ptr;
    } end;

    Map children;
    struct S3_Node *suffixLink;
} S3_Node, *S3;

S3 createS3Node(Pos start, Pos end, uint32_t leafNum);
char getCharS3(S3 node, long index);
long lengthS3(S3 node);
S3 splitS3(S3 node, int index);
void deleteS3(S3 node);
void printS3(S3 node, long offset);

char* readText(long *length);
char* readPattern(char *buffer, long *length, long *capacity);

typedef struct SuffixTree {
    Pos end;
    S3 root;
} _SuffixTree, *SuffixTree;

SuffixTree buildSuffixTree(char *text, long m);
void deleteSuffixTree(SuffixTree tree);

uint32_t* leavesDFS(S3 root, uint32_t *matches, long *matchCount, long *matchCap);
void countingSort (uint32_t *arr, long n);
uint32_t* findMatches(long patternNum, SuffixTree tree, char *pattern, long m, uint32_t *matches, long *matchCap);

int main() {
    long n;
    char *text = readText(&n);
    
    SuffixTree tree = buildSuffixTree(text, n);

    long cap = 0;
    long m = 0;
    char *pattern = NULL;
    long patternNum = 0;

    long matchCap = 0;
    uint32_t *matches = NULL;

    while (1) {
        m = 0;
        pattern = readPattern(pattern, &m, &cap);
        if (feof(stdin) || m == 0) break;
        patternNum++;

        clock_t start = clock(); 
        matches = findMatches(patternNum, tree, pattern, m, matches, &matchCap);
        double timePassed = (double)(clock() - start) / CLOCKS_PER_SEC * 1000.0;
        printf("time: %fms\n", timePassed);
    }
    
    free(matches);
    free(pattern);
    deleteSuffixTree(tree);
    free(text);

    return 0;
}

void initMap(Map *map) {
    map->size = 0;
    map->cap = EXTEND_SIZE;
    map->items = (Pair*)malloc(sizeof(Pair) * EXTEND_SIZE);

    for (int i = 0; i < map->cap; i++) {
        map->items[i].key = '\0';
        map->items[i].value = NULL;
    }
} 

void setMap(Map *map, char key, struct S3_Node *value) {
    if (map->cap / 4 * 3 <= map->size) {
        extendMap(map);
    }
    
    unsigned char hash = (unsigned char)key % map->cap;

    while (map->items[hash].key != key) {
        if (map->items[hash].key == '\0') break;
        hash = (hash + 1) % map->cap;
    }
    
    if (map->items[hash].key == '\0') map->size++;
    map->items[hash].key = key;
    map->items[hash].value = value;
}

struct S3_Node* getMap(Map *map, char key) {
    unsigned char hash = (unsigned char)(key) % map->cap;

    int counter = 0;
    while (map->items[hash].key != key && counter < map->cap) {
        if (map->items[hash].key == '\0') return NULL;
        hash = (hash + 1) % map->cap;
        counter++;
    }

    if (map->items[hash].key != key) return NULL;
    return map->items[hash].value;
}

void extendMap(Map *map) {
    unsigned char newCap = map->cap + EXTEND_SIZE;
    if (256 - map->cap <= EXTEND_SIZE) newCap = 255;
    
    Pair *newItems = (Pair*)malloc(sizeof(Pair) * newCap);

    for (int i = 0; i < newCap; i++) {
        newItems[i].key = '\0';
        newItems[i].value = NULL;
    }

    for (int i = 0; i < map->cap; i++) {
        Pair item = map->items[i];
        if (item.key == '\0') continue;
        unsigned char hash = (unsigned char)(item.key) % newCap;

        while (newItems[hash].key != '\0') {
            hash = (hash + 1) % newCap;
        }
        
        newItems[hash] = item;
    }

    free(map->items);
    map->items = newItems;
    map->cap = newCap;
}

void deleteMap(Map *map, void (*destructor)(struct S3_Node*)) {
    for (int i = 0; i < map->cap; i++) {
        if (map->items[i].value != NULL) {
            destructor(map->items[i].value);
        }
    }

    free(map->items);
}

S3 createS3Node(Pos start, Pos end, uint32_t leafNum) {
    S3 res = malloc(sizeof(S3_Node));

    res->start = start;
    if (leafNum == NOT_LEAF) {
        res->end.letter = end;          
    } else {
        res->end.ptr = (Pos*) end;       
    }
    res->leafNum = leafNum;
    res->suffixLink = NULL;

    initMap(&res->children);

    return res;
}

char getCharS3(S3 node, long index) {
    return *(node->start + index);
}

long lengthS3(S3 node) {
    if (node->leafNum == NOT_LEAF) {
        return node->end.letter - node->start;
    }

    Pos end = *node->end.ptr;
    return end - node->start;
}

void deleteS3(S3 node) {
    deleteMap(&node->children, deleteS3);
    free(node);
}

void printS3(S3 node, long offset) {
    if (node->start == NULL) {
        printf("ROOT");
    }

    for (int i = 0; i < offset; i++) {
        printf("--");
    }

    for (int i = 0; i < lengthS3(node); i++) {
        char letter = getCharS3(node, i);
        printf("%c", letter != '\n' ? letter : '$');
    }

    if (node->leafNum != NOT_LEAF) {
        printf(" [%d]", node->leafNum);
    }

    printf("\n");

    for (int i = 0; i < node->children.cap; i++) {
        if (node->children.items[i].value != NULL) {
            printS3(node->children.items[i].value, offset + 1);
        }
    }
}

char* readText(long *length) {
    char *buffer = (char*)malloc(sizeof(char) * EXTEND_SIZE);
    long size = 0;
    long cap = EXTEND_SIZE;

    while (1) {
        char letter = fgetc(stdin);
        if (letter == '\r') continue;

        if (letter == '\n' || letter == EOF) {
            letter = '\n';
        }

        if (size >= cap) {
            cap += EXTEND_SIZE;
            buffer = realloc(buffer, cap);
        }

        buffer[size] = letter;
        size++;

        if (letter == '\n') {
            break;
        }
    }
    
    *length = size;
    return buffer;
}

SuffixTree buildSuffixTree(char *text, long m) {
    SuffixTree tree = (SuffixTree)malloc(sizeof(_SuffixTree));
    tree->root = createS3Node(NULL, NULL, NOT_LEAF);
    tree->root->suffixLink = tree->root;
    tree->end = NULL;

    struct {
        S3 node;
        Pos edge;
        long length;
    } active = {tree->root, text, 0};

    long remainder = 0;
    S3 lastCreatedNode = NULL;

    for (long i = 0; i < m; i++) {
        Pos pos = text + i;
        // правило 1
        tree->end = pos + 1;

        lastCreatedNode = NULL;
        remainder++;

        while (remainder > 0) {
            if (active.length == 0) {
                active.edge = pos;
            }

            char currentChar = *active.edge;
            S3 it = getMap(&active.node->children, currentChar);

            if (it == NULL) {
                S3 leaf = createS3Node(pos, (Pos)(&tree->end), i - remainder + 1);
                setMap(&active.node->children, currentChar, leaf);

                // проводим суффиксную ссылку
                if (lastCreatedNode != NULL) {
                    lastCreatedNode->suffixLink = active.node;
                }
                lastCreatedNode = active.node;
            } else {
                // прыжки по счетчику
                if (active.length >= lengthS3(it)) {
                    active.edge += lengthS3(it);
                    active.length -= lengthS3(it);
                    active.node = it;
                    continue;
                }

                // правило 3
                if (*(it->start + active.length) == *pos) {
                    active.length++;
                    
                    // проводим суффиксную ссылку
                    if (lastCreatedNode != NULL) {
                        lastCreatedNode->suffixLink = active.node;
                    }
                    lastCreatedNode = active.node;

                    break;
                }

                // правило 2
                Pos splitEnd = it->start + active.length;
                S3 splitNode = createS3Node(it->start, splitEnd, NOT_LEAF);

                it->start += active.length;
                S3 leaf = createS3Node(pos, (Pos)(&tree->end), i - remainder + 1);

                setMap(&active.node->children, currentChar, splitNode);
                setMap(&splitNode->children, *it->start, it);
                setMap(&splitNode->children, *pos, leaf);
                
                // проводим суффиксную ссылку
                if (lastCreatedNode != NULL) {
                    lastCreatedNode->suffixLink = splitNode;
                }
                lastCreatedNode = splitNode;
            }

            remainder--;

            if (active.node != tree->root) {
                active.node = (active.node->suffixLink != NULL) ? active.node->suffixLink : tree->root;
                continue;
            }

            if (active.length <= 0) continue;

            active.length--;
            active.edge = pos - remainder + 1;
        }
    }

    return tree;
}

void deleteSuffixTree(SuffixTree tree) {
    deleteS3(tree->root);
    free(tree);
}

char* readPattern(char *buffer, long *length, long *capacity) {
    long size = *length;
    long cap = *capacity;

    while (1) {
        char letter = fgetc(stdin);
        if (letter == '\r') continue;

        if (letter == '\n' || letter == EOF) {
            letter = '\0';
        }

        if (size >= cap) {
            cap += EXTEND_SIZE;
            buffer = realloc(buffer, cap);
        }

        buffer[size] = letter;

        if (letter == '\0') break;
        size++;
    }
    
    *length = size;
    *capacity = cap;
    return buffer;
}

uint32_t* leavesDFS(S3 root, uint32_t *matches, long *matchCount, long *matchCap) {
    if (root->leafNum != NOT_LEAF) {
        if (*matchCount >= *matchCap) {
            *matchCap += EXTEND_SIZE;
            long newSize = (*matchCap) * sizeof(uint32_t);
            matches = (uint32_t*)realloc(matches, newSize);
        }

        matches[*matchCount] = root->leafNum;
        (*matchCount)++;
        return matches;
    }

    for (uint8_t i = 0; i < root->children.cap; i++) {
        if (root->children.items[i].key != '\0') {
            matches = leavesDFS(root->children.items[i].value, matches, matchCount, matchCap);
        }
    }

    return matches;
}

void countingSort (uint32_t *arr, long n) {
    uint32_t min = arr[0], max = arr[0];

    for (long i = 1; i < n; i++) {
        if (arr[i] < min) min = arr[i];
        if (arr[i] > max) max = arr[i];
    }

    uint32_t range = max - min + 1;
    
    uint32_t *counts = malloc(sizeof(uint32_t) * range);
    for (long i = 0; i < range; i++) counts[i] = 0;
    
    for (long i = 0; i < n; i++) counts[arr[i] - min]++;

    long it = 0;
    
    for (uint32_t i = 0; i < range; i++) {
        for (uint32_t j = 0; j < counts[i]; j++) {
            arr[it++] = i + min;
        }
    }

    free(counts);
}

uint32_t* findMatches(long patternNum, SuffixTree tree, char *pattern, long m, uint32_t *matches, long *matchCap) {
    S3 currentNode = tree->root;
    long edgeLength = 0;
    long edgePos = 0;

    for (long i = 0; i < m; i++) {
        char patternLetter = pattern[i];

        if (edgePos >= edgeLength) {
            // мы в вершине, пытаемся перейти на следующее ребро
            currentNode = getMap(&currentNode->children, patternLetter);

            // не смогли перейти => паттерна нет в тексте
            if (currentNode == NULL) return matches;

            edgePos = 1;
            edgeLength = lengthS3(currentNode);
        } else {
            // мы на ребре, пытаемся продвинуться по нему
            // не смогли пройти => паттерна нет в тексте
            if (patternLetter != getCharS3(currentNode, edgePos)) return matches;

            edgePos++;
        }
    }

    // если дошли до сюда - есть вхождение
    long matchCount = 0;
    matches = leavesDFS(currentNode, matches, &matchCount, matchCap);

    countingSort(matches, matchCount);

    // printf("%ld: ", patternNum);
    // for (long i = 0; i < matchCount - 1; i++) {
    //     printf("%d, ", matches[i] + 1);
    // }
    // printf("%d\n", matches[matchCount - 1] + 1);

    return matches;
}