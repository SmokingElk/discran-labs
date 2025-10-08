#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define EXTEND_SIZE 10
#define NOT_LEAF 4294967295
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
void insertMap(Map *map, char key, struct S3_Node *value);
struct S3_Node* getMap(Map *map, char key);
void extendMap(Map *map);
void forEachMap(Map *map, void (*callback)(struct S3_Node*));
void deleteMap(Map *map, void (*destructor)(struct S3_Node*));

typedef char* Pos;

typedef struct S3_Node {
    size_t leafNum;
    Pos start;
    Pos end; // actually stores Pos* (a.k.a. char**) if Node is leaf 
    Map children;
    struct S3_Node *suffixLink;
    struct S3_Node *parrent;
} S3_Node, *S3;

S3 createS3Node(Pos start, Pos end, size_t leafNum, S3 parent);
char getCharS3(S3 node, size_t index);
size_t lengthS3(S3 node);
S3 splitS3(S3 node, int index);
void deleteS3(S3 node);
void printS3(S3 node, size_t offset);

char* readPattern(size_t *length);

S3 buildS3(char *text, size_t m, Pos *end);

int main() {
    size_t m;
    char *pattern = readPattern(&m);
    Pos end = pattern;
    S3 tree = buildS3(pattern, m, &end);
    
    printS3(tree, 0);

    deleteS3(tree);
    free(pattern);

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

void insertMap(Map *map, char key, struct S3_Node *value) {
    if (map->cap / 4 * 3 <= map->size) {
        extendMap(map);
    }
    
    unsigned char hash = (unsigned char)key % map->cap;

    while (map->items[hash].key != '\0') {
        hash = (hash + 1) % map->cap;
    }
    
    map->items[hash].key = key;
    map->items[hash].value = value;
    map->size++;
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
    
    Pair *newItems = (Pair*)malloc(sizeof(Pair) * newCap);

    for (int i = 0; i < newCap; i++) {
        newItems[i].key = '\0';
        newItems[i].value = NULL;
    }

    for (int i = 0; i < map->size; i++) {
        Pair item = map->items[i];
        unsigned char hash = (unsigned char)(item.key) % newCap;

        while (map->items[hash].key != '\0') {
            hash = (hash + 1) % newCap;
        }
        
        newItems[hash] = item;
    }

    free(map->items);
    map->items = newItems;
    map->cap = newCap;
}

void forEachMap(Map *map, void (*callback)(struct S3_Node*)) {
    for (int i = 0; i < map->size; i++) {
        if (map->items[i].value != NULL) {
            callback(map->items[i].value);
        }
    }
}

void deleteMap(Map *map, void (*destructor)(struct S3_Node*)) {
    for (int i = 0; i < map->size; i++) {
        if (map->items[i].value != NULL) {
            destructor(map->items[i].value);
        }
    }

    free(map->items);
}

S3 createS3Node(Pos start, Pos end, size_t leafNum, S3 parrent) {
    S3 res = malloc(sizeof(S3_Node));

    res->start = start;
    res->end = end;
    res->leafNum = leafNum;
    res->suffixLink = NULL;
    res->parrent = parrent;

    initMap(&res->children);

    return res;
}

char getCharS3(S3 node, size_t index) {
    return *(node->start + index);
}

size_t lengthS3(S3 node) {
    if (node->leafNum == NOT_LEAF) {
        return node->end - node->start;
    }

    Pos end = *((Pos*)node->end);
    return (end - node->start);
}

S3 splitS3(S3 node, int index) {
    Pos mid = node->start + index;

    S3 res = createS3Node(mid, node->end, node->leafNum, node);
    
    node->end = mid;
    node->leafNum = NOT_LEAF;
    insertMap(&node->children, *mid, res);

    return res;
}

S3 insertS3(S3 node, int index, Pos start, Pos end, size_t leafNum) {
    if (index + 1 == lengthS3(node)) {
        if (getMap(node->children, *start) != NULL) return NULL;
        
        S3 res = createS3Node(start, end, leafNum, node);
        insertMap(node->children, *start, res);
        return res;
    } 

    if (*(node->start + index) == *start) return NULL;

    S3 res = createS3Node(start, end, leafNum, node);
    S3 inner = splitS3(node, index);
    insertMap(inner->children, *start, res);
    
    return res;
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
        printf("%c", getCharS3(node, i));
    }

    printf("\n");

    if (offset > 0 ) return;

    for (int i = 0; i < node->children.size; i++) {
        if (node->children.items[i].value != NULL) {
            printS3(node->children.items[i].value, offset + 1);
        }
    }
}

char* readPattern(size_t *length) {
    char *buffer = (char*)malloc(sizeof(char) * EXTEND_SIZE);
    size_t size = 0;
    size_t cap = EXTEND_SIZE;

    while (1) {
        char letter = fgetc(stdin);

        if (letter == '\n' || letter == EOF) {
            letter = '\0';
        }

        if (size >= cap) {
            cap += EXTEND_SIZE;
            buffer = realloc(buffer, cap);
        }

        buffer[size] = letter;
        size++;

        if (letter == '\0') {
            break;
        }
    }
    
    *length = size;
    return buffer;
}

S3 searchS3(S3 root, char *suffix, size_t length, size_t *stopOut) {
    if (length == 0) return root;

    Pos it = suffix;
    size_t stop = 1;
    S3 vertex = getMap(root->children, suffix[0]);
    size_t vertexLen = lengthS3(vertex);

    for (Pos it = suffix; it < suffix + length; it++) {
        if (vertex->start + stop < vertexLen) {
            if (*it != *(vertex->start + stop)) return NULL;
            stop++;
            continue;
        }
        
        S3 vertex = getMap(root->children, *it);

        if (vertex == NULL) return NULL;

        vertexLen = lengthS3(vertex);
        stop = 1;
    }

    *stopOut = stop - 1;
    return vertex;
}

S3 jumpS3(S3 root, char *suffix, size_t length, size_t *stopOut) {
    if (length == 0) return root;
    
    Pos it = suffix;
    S3 vertex = getMap(root->children, suffix[0]);
    size_t vertexLen = lengthS3(vertex);

    while (length >= vertexLen) {
        it += vertexLen;
        S3 vertex = getMap(root->children, *it);
        if (vertex == NULL) return NULL;
        length -= vertexLen;
        vertexLen = lengthS3(vertex);
    }

    *stopOut = length - 1;
    return vertex;
}

S3 buildS3(char *text, size_t m, Pos *end) {
    // алгоритм Укконена

    size_t leavesCount = 0;

    S3 root = createS3Node(NULL, NULL, NOT_LEAF, NULL);

    // строим T(i = 0)
    (*end)++;
    // текущая вершина
    S3 currentNode = createS3Node(text, (Pos)(end), ++leavesCount);
    insertMap(&root->children, text[0], currentNode);

    // индекс буквы в вершине, НА которой остановились
    size_t stop = 0;

    for (int i = 1; i < m; i++) {
        // правило 1
        (*end)++;
        for (int j = leavesCount; j < i + 1; j++) {
            // [j:i+1] - текущий суффикс
            
            // вставляем букву
            char insertLetter = text[i];
            
            // gamma - строка, по которой откатились
            Pos gammaStart = currentNode->start;
            size_t gammaLength = stop + 1;

            S3 suffixEnd;

            // если мы стоим в вершине
            if (lengthS3(currentNode) == stop + 1) {
                // пытаемся перейти из нее
                suffixEnd = currentNode;
                gammaLength = 0;
            } else {
                // иначе откатываемся назад на 1
                suffixEnd = currentNode->parrent;
            }
            
            // если перешли в корень
            if (suffixEnd == root) {
                // наивный поиск (ищем текущий суффикс без последней буквы)
                size_t newStop;
                suffixEnd = searchS3(root, text + j, j - i, &newStop);

                // если остались в корне - добавить новую ветку
                if (suffixEnd == root) {
                    // правило 3
                    if (getMap(root->children, insertLetter) != NULL) {
                        break;
                    }

                    // правило 2
                    currentNode = createS3Node(text + i, (Pos)(end), ++leavesCount, root);
                    insertMap(root->children, text[i], currentNode);
                    stop = i - j;
                } else {
                    S3 inserted = insertS3(suffixEnd, newStop + 1, text + j, (Pos)(end), ++leavesCount); 

                    // правило 3
                    if (inserted == NULL) break;
                    
                    // правило 2
                    currentNode = inserted;
                    stop = i - j;
                }
            } else {
                // иначе - переходим по суффиксной ссылке
                suffixEnd = suffixEnd->suffixLink;

                // и прыжки по счетчику
                size_t newStop;
                suffixEnd = jumpS3(suffixEnd, gammaStart, gammaLength, &newStop);

                S3 inserted = insertS3(suffixEnd, newStop + 1, text + j, (Pos)(end), ++leavesCount); 

                // правило 3
                if (inserted == NULL) break;

                // создаем суффиксную ссылку
                currentNode->parrent->suffixLink = inserted->parrent;

                // правило 2
                currentNode = inserted;
                stop = i - j;
            }
        }
    }

    return root;
}
