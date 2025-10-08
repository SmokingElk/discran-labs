#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

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
char getCharS3(S3 node, size_t index);
size_t lengthS3(S3 node);
S3 splitS3(S3 node, int index);
void deleteS3(S3 node);
void printS3(S3 node, size_t offset);

char* readText(size_t *length);
char* readPattern(char *buffer, size_t *length, size_t *capacity);

typedef struct SuffixTree {
    Pos end;
    S3 root;
} _SuffixTree, *SuffixTree;

SuffixTree buildSuffixTree(char *text, size_t m);
void deleteSuffixTree(SuffixTree tree);

uint32_t* leavesDFS(S3 root, uint32_t *matches, size_t *matchCount, size_t *matchCap);
void countingSort (uint32_t *arr, size_t n);
uint32_t* findMatches(size_t patternNum, SuffixTree tree, char *pattern, size_t m, uint32_t *matches, size_t *matchCap);

int main() {
    size_t n;
    char *text = readText(&n);
    
    SuffixTree tree = buildSuffixTree(text, n);

    printS3(tree->root, 0);

    size_t cap = 0;
    size_t m = 0;
    char *pattern = NULL;
    size_t patternNum = 0;

    size_t matchCap = 0;
    uint32_t *matches = NULL;

    while (1) {
        m = 0;
        pattern = readPattern(pattern, &m, &cap);
        if (feof(stdin) || m == 0) break;
        patternNum++;

        matches = findMatches(patternNum, tree, pattern, m, matches, &matchCap);
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

char getCharS3(S3 node, size_t index) {
    return *(node->start + index);
}

size_t lengthS3(S3 node) {
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

void printS3(S3 node, size_t offset) {
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

char* readText(size_t *length) {
    char *buffer = (char*)malloc(sizeof(char) * EXTEND_SIZE);
    size_t size = 0;
    size_t cap = EXTEND_SIZE;

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

SuffixTree buildSuffixTree(char *text, size_t m) {
    SuffixTree tree = malloc(sizeof(_SuffixTree));
    tree->root = createS3Node(NULL, NULL, NOT_LEAF);
    tree->end = text;
    
    struct {
        S3 node;
        char edge;
        Pos letter;
        size_t length;
    } active = {tree->root, '\0', text, 0};

    size_t remainder = 0;
    S3 prevNode = NULL;

    for (size_t i = 0; i < m; i++) {
        if (i > 0) {
            printS3(tree->root, 0);
            printf("\n");
        }
        // правило 1 - дописываем буквы в концы каждого листа
        tree->end = text + i + 1;
        char inserting = text[i];
        remainder++;
        prevNode = NULL;

        while (remainder > 0) {
            if (active.length == 0) {
                active.letter = text + i - remainder + 1;
                active.edge = *active.letter;
            }

            S3 nextNode = getMap(&active.node->children, active.edge);
            if (nextNode == NULL) {
                // вставка нового листа из (например, из корня)
                S3 newLeaf = createS3Node(text + (i - remainder + 1), (Pos)(&tree->end), 0);
                newLeaf->leafNum = (newLeaf->start - text); 
                setMap(&active.node->children, active.edge, newLeaf);

                if (prevNode != NULL) {
                    prevNode->suffixLink = active.node;
                    prevNode = NULL;
                }
            } else {
                size_t edgeLen = lengthS3(nextNode);
                if (active.length >= edgeLen) {
                    // прыжки по счетчику
                    active.node = nextNode;
                    active.length -= edgeLen;
                    active.letter += edgeLen;
                    active.edge = *active.letter;
                    continue;
                }

                if (getCharS3(nextNode, active.length) == inserting) {
                    // правило 3 - буква уже есть
                    active.length++;
                    if (prevNode != NULL) {
                        prevNode->suffixLink = active.node;
                    }
                    break;
                }

                // правило 2
                // разделяем ребро
                S3 midNode = createS3Node(nextNode->start, nextNode->start + active.length, NOT_LEAF);
                setMap(&active.node->children, active.edge, midNode);

                S3 newLeaf = createS3Node(text + (i - remainder + 1), (Pos)(&tree->end), 0);
                newLeaf->leafNum = (newLeaf->start - text); 
                setMap(&midNode->children, inserting, newLeaf);

                nextNode->start += active.length;
                setMap(&midNode->children, *nextNode->start, nextNode);

                // проводим суффиксную ссылку
                midNode->suffixLink = tree->root;
                if (prevNode != NULL) {
                    prevNode->suffixLink = midNode;
                }
                prevNode = midNode;
            }

            remainder--;

            // обновляем активную точку
            // если мы не в корне
            if (active.node != tree->root) {
                active.node = active.node->suffixLink ? active.node->suffixLink : tree->root;
                continue;
            }

            // если в корне
            // если можем уменьшить активную длину - уменьшаем
            if (active.length == 0) continue;
            
            active.length--;
            active.letter++;
            active.edge = *active.letter; 
        }   
    }
    
    return tree;
}

void deleteSuffixTree(SuffixTree tree) {
    deleteS3(tree->root);
    free(tree);
}

char* readPattern(char *buffer, size_t *length, size_t *capacity) {
    size_t size = *length;
    size_t cap = *capacity;

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

uint32_t* leavesDFS(S3 root, uint32_t *matches, size_t *matchCount, size_t *matchCap) {
    if (root->leafNum != NOT_LEAF) {
        if (*matchCount >= *matchCap) {
            *matchCap += EXTEND_SIZE;
            size_t newSize = (*matchCap) * sizeof(uint32_t);
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

void countingSort (uint32_t *arr, size_t n) {
    uint32_t min = arr[0], max = arr[0];

    for (size_t i = 1; i < n; i++) {
        if (arr[i] < min) min = arr[i];
        if (arr[i] > max) max = arr[i];
    }

    uint32_t range = max - min + 1;
    
    uint32_t *counts = malloc(sizeof(uint32_t) * range);
    for (size_t i = 0; i < range; i++) counts[i] = 0;
    
    for (size_t i = 0; i < n; i++) counts[arr[i] - min]++;

    size_t it = 0;
    
    for (uint32_t i = 0; i < range; i++) {
        for (uint32_t j = 0; j < counts[i]; j++) {
            arr[it++] = i + min;
        }
    }

    free(counts);
}

uint32_t* findMatches(size_t patternNum, SuffixTree tree, char *pattern, size_t m, uint32_t *matches, size_t *matchCap) {
    S3 currentNode = tree->root;
    size_t edgeLength = 0;
    size_t edgePos = 0;

    for (size_t i = 0; i < m; i++) {
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
    size_t matchCount = 0;
    matches = leavesDFS(currentNode, matches, &matchCount, matchCap);

    countingSort(matches, matchCount);

    printf("%ld: ", patternNum);
    for (size_t i = 0; i < matchCount - 1; i++) {
        printf("%d, ", matches[i] + 1);
    }
    printf("%d\n", matches[matchCount - 1] + 1);

    return matches;
}