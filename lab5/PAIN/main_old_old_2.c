#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define EXTEND_SIZE 10
#define NOT_LEAF 4294967295
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
    size_t leafNum;
    Pos start;

    // используем letter для внутренних вершин и ptr для листьев (глобальный указатель на конец)
    union {
        Pos letter;
        Pos *ptr;
    } end;

    Map children;
    struct S3_Node *suffixLink;
} S3_Node, *S3;

S3 createS3Node(Pos start, Pos end, size_t leafNum);
char getCharS3(S3 node, size_t index);
size_t lengthS3(S3 node);
S3 splitS3(S3 node, int index);
void deleteS3(S3 node);
void printS3(S3 node, size_t offset);

char* readPattern(size_t *length);

typedef struct SuffixTree {
    Pos end;
    S3 root;
} _SuffixTree, *SuffixTree;

SuffixTree buildSuffixTree(char *text, size_t m);
void deleteSuffixTree(SuffixTree tree);

typedef struct RingBuffer {
    char *buffer;
    size_t m;
} RingBuffer;

RingBuffer createRingBuffer(size_t m);
void setRingBuffer(RingBuffer buffer, size_t index, char value);
char getRingBuffer(RingBuffer buffer, size_t index);
void deleteRingBuffer(RingBuffer buffer);

void findMatches(SuffixTree tree, size_t m);

int main() {
    size_t m;
    char *pattern = readPattern(&m);
    
    SuffixTree tree = buildSuffixTree(pattern, m);

    printS3(tree->root, 0);

    findMatches(tree, m);

    deleteSuffixTree(tree);
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
    printf("extend\n");
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

void deleteMap(Map *map, void (*destructor)(struct S3_Node*)) {
    for (int i = 0; i < map->size; i++) {
        if (map->items[i].value != NULL) {
            destructor(map->items[i].value);
        }
    }

    free(map->items);
}

S3 createS3Node(Pos start, Pos end, size_t leafNum) {
    S3 res = malloc(sizeof(S3_Node));

    res->start = start;
    res->end.letter = end;
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

    printf("\n");

    for (int i = 0; i < node->children.cap; i++) {
        if (node->children.items[i].value != NULL) {
            printf("%c: ", node->children.items[i].key != '\n' ? node->children.items[i].key : '$');
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
    size_t leavesCount = 0;

    for (size_t i = 0; i < m; i++) {
        tree->end = text + i + 1;

        char inserting = text[i];

        remainder++;
        
        if (remainder == 1) {
            active.letter = text + i;
            active.edge = inserting;

            if ((long)active.length < 0) {
                active.length = 0;
            }
        }

        S3 prevNode = NULL;

        while (remainder > 0) {
            if (getMap(&active.node->children, active.edge) == NULL) {
                S3 newLeaf = createS3Node(active.letter, (Pos)(&tree->end), leavesCount++);
                setMap(&active.node->children, active.edge, newLeaf);

                // правило 2
                if (prevNode != NULL) {
                    prevNode->suffixLink = active.node;
                }
                prevNode = newLeaf;
                
                remainder--;

                // правило 1
                if (active.node == tree->root) {
                    active.letter++;
                    if (active.letter - text < m) {
                        active.edge = *active.letter;
                    }

                    active.length--;
                    
                    // правило 3
                } else {
                    active.node = active.node->suffixLink;
                }

                continue;
            }

            S3 splitNode = getMap(&active.node->children, active.edge);
            size_t splitLen = lengthS3(splitNode);

            if (active.length >= splitLen) {
                // прыжки по счетчику
                active.node = splitNode;
                active.length -= splitLen;
                active.letter += splitLen;
                active.edge = *active.letter;
                continue;
            }

            if (getCharS3(splitNode, active.length) == inserting) {
                // если буква уже есть - просто запоминаем в остатке и заканчиваем фазу
                active.length++;

                // правило 2
                if (prevNode != NULL) {
                    prevNode->suffixLink = splitNode;
                }

                break;
            }

            // разделение ребра
            S3 midNode = createS3Node(splitNode->start, splitNode->start + active.length, NOT_LEAF);
            setMap(&active.node->children, active.edge, midNode);

            S3 leafNode = createS3Node(text + i, (Pos)(&tree->end), leavesCount++);
            setMap(&midNode->children, inserting, leafNode);
            
            splitNode->start += active.length;
            setMap(&midNode->children, *splitNode->start, splitNode);

            remainder--;

            // правило 2
            if (prevNode != NULL) {
                prevNode->suffixLink = midNode;
            }

            prevNode = midNode;

            // правило 1
            if (active.node == tree->root) {
                active.length--;
                active.letter++;

                if (active.letter - text < m) {
                    active.edge = *active.letter;
                }

                // правило 3
            } else {
                active.node = active.node->suffixLink;
            }
        }   
    }
    
    return tree;
}

void deleteSuffixTree(SuffixTree tree) {
    deleteS3(tree->root);
    free(tree);
}

RingBuffer createRingBuffer(size_t m) {
    RingBuffer res;
    res.m = m;
    res.buffer = (char*)malloc(sizeof(char) * m);

    return res;
}

void setRingBuffer(RingBuffer buffer, size_t index, char value) {
    buffer.buffer[index % buffer.m] = value;
}

char getRingBuffer(RingBuffer buffer, size_t index) {
    return buffer.buffer[index % buffer.m];
}

void deleteRingBuffer(RingBuffer buffer) {
    free(buffer.buffer);
}

void findMatches(SuffixTree tree, size_t m) {
    S3 nodeBefore = NULL;
    S3 currentNode = tree->root;
    size_t currentLength = 0;
    size_t letter = 0;

    size_t calcPos = 0;
    size_t matchesNum = 0;

    bool readNext = true;
    char textLetter;

    while (1) {
        if (matchesNum == m - 1) {
            printf("%ld\n", calcPos + 1);
        }

        printf("\n");

        if (readNext) {
            textLetter = fgetc(stdin);
            if (textLetter == '\0' || textLetter == '\n') break;
            printf("text letter: %c\n", textLetter);
        } 

        readNext = true;


        if (currentNode == tree->root) {
            printf("--root\n");
        } else {
            printf("--");
            for (size_t i = 0; i < lengthS3(currentNode); i++) {
                char sgn = getCharS3(currentNode, i);

                if (i == letter) printf("[");
                printf("%c", sgn != '\n' ? sgn : '$');
                if (i == letter) printf("]");
            }

            if (letter == lengthS3(currentNode)) {
                printf("[_]");
            }

            printf("\n");
        }

        // если мы стоим в вершине
        if (currentNode == tree->root || letter == currentLength) {
            printf("in vertex\n");
            S3 nextNode = getMap(&currentNode->children, textLetter);

            if (nextNode != NULL) {
                // совпадение
                
                nodeBefore = currentNode;
                currentNode = nextNode;
                currentLength = lengthS3(nextNode);
                
                letter = 1;
                matchesNum++;
                
                printf("match vertex %ld\n", currentLength);
                continue;
            } 
            
            // несовпадение
            printf("missmatch vertex %ld\n", currentLength);

            calcPos++;

            S3 endNode;
            size_t bethaLength;

            // уже есть суффиксная ссылка в текущей вершине
            if (currentNode->suffixLink) {
                endNode = currentNode->suffixLink;
                bethaLength = 0;

            // мы стоим в корне
            } else if (currentNode == tree->root) {
                endNode = tree->root;
                bethaLength = 0;
            
            // откатываемся в предыдущую вершину и у нее есть суффиксная ссылка
            } else if (nodeBefore->suffixLink) {
                endNode = nodeBefore->suffixLink;
                bethaLength = lengthS3(currentNode);

            // если у предыдущей вершины нет суффиксной ссылки - это корень, переходим в него
            } else {
                endNode = nodeBefore;
                bethaLength = lengthS3(currentNode) - 1;
            }

            size_t endLength = lengthS3(endNode);

            Pos pos = currentNode->start;

            while (bethaLength > endLength) {
                bethaLength -= endLength;
                pos += endLength;
                endNode = getMap(&endNode->children, *pos);
                endLength = lengthS3(endNode);
            }
            
            letter = bethaLength;
            currentNode = endNode;
            currentLength = endLength;

            if (matchesNum < 2) {
                matchesNum = 0;
            } else {
                readNext = false;
                matchesNum--;
            }

            continue;
        }

        printf("on edge\n");
        
        if (getCharS3(currentNode, letter) == textLetter) {
            // совпадение
            printf("match edge %ld\n", letter);

            matchesNum++;
            letter++;
            continue;
        } 

        // несовпадение
        printf("missmatch edge\n");
        
        calcPos++;

        S3 endNode;
        size_t bethaLength;

        if (nodeBefore->suffixLink) {
            printf("node before has suffix link\n");
            endNode = nodeBefore->suffixLink;
            bethaLength = letter - 1;

            size_t endLength = lengthS3(endNode);

            Pos pos = currentNode->start;

            while (bethaLength > endLength) {
                bethaLength -= endLength;
                pos += endLength;
                endNode = getMap(&endNode->children, *pos);
                endLength = lengthS3(endNode);
            }
            
            letter = bethaLength;
            currentNode = endNode;
            currentLength = endLength;

        // если у предыдущей вершины нет суффиксной ссылки - это корень, переходим в него
        } else {
            currentNode = getMap(&nodeBefore->children, *(currentNode->start + 1));
            if (currentNode == NULL) {
                currentNode = tree->root;
            }
            currentLength = lengthS3(currentNode);
            letter -= 1;
        }

        if (matchesNum < 2) {
            matchesNum = 0;
        } else {
            readNext = false;
            matchesNum--;
        }

        printf("matched %ld\n", matchesNum);
    }
}