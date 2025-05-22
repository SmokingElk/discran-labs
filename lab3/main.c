// ===== RBTree.h =====
// #pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

// Константы для FNV-1a (32-битная версия)
#define FNV_OFFSET_BASIS 2166136261U
#define FNV_PRIME 16777619U

typedef enum Color { RED, BLACK } Color;
typedef char* Key;
typedef unsigned long int Value;
typedef enum FileError { 
    NO_ERRORS, 
    FE_CAN_NOT_WRITE, 
    FE_CAN_NOT_READ, 
    FE_PARSING_ERROR, 
    FE_OUT_OF_MEMORY 
} FileError;

typedef struct _Node {
    Key key;
    Value value;
    
    Color color;
    struct _Node *p;
    struct _Node *left;
    struct _Node *right;

    uint32_t keyHash;
} _Node, *RBTree;

RBTree createRBTree();
void deleteRBTree(RBTree tree);
bool getRBTree(RBTree tree, Key key, Value *res);
bool insertRBTree(RBTree *treeP, Key key, Value value);
bool removeRBTree(RBTree *treeP, Key key);
FileError saveRBTree(RBTree *treeP, char *path);
FileError loadRBTree(RBTree *treeP, char *path);
void printTree(RBTree tree, int offset);

// ===== RBTree.c =====
// #include "RBTree.h"

typedef enum _LoadError {
    LOAD_OK,
    PARSING_ERROR,
    OUT_OF_MEMORY
} _LoadError;

_Node _nilStruct = {NULL, 0, BLACK, NULL, NULL, NULL};
RBTree _NIL = &_nilStruct;

void _writeByte(FILE *file, unsigned char byte);
unsigned char _readByte(FILE *file);

RBTree _searchRBTree(RBTree tree, Key key);
void _insertFixupRBTree(RBTree *treeP, RBTree z);
void _deleteFixupRBTree(RBTree *treeP, RBTree x);
void _saveTraverse(RBTree tree, FILE *file);
_LoadError _loadTraverse(RBTree *treeP, FILE *file);
void _RotateLeft(RBTree *treeP, RBTree x);
void _RotateRight(RBTree *treeP, RBTree x);
void _Transplant(RBTree *treeP, RBTree u, RBTree v);

void printTree(RBTree tree, int offset);

RBTree createRBTree() {
    return _NIL;
}

void deleteRBTree(RBTree tree) {
    if (tree == _NIL) return;

    deleteRBTree(tree->left);
    deleteRBTree(tree->right);
    free(tree->key);
    free(tree);
}

bool getRBTree(RBTree tree, Key key, Value *res) {
    RBTree found = _searchRBTree(tree, key);

    if (found == _NIL) return false;
    *res = found->value;
    return true;
}

bool insertRBTree(RBTree *treeP, Key key, Value value) {
    RBTree y = _NIL;
    RBTree x = *treeP;

    while (x != _NIL) {
        y = x;

        int keyOrder = strcmp(key, x->key);

        if (keyOrder == 0) return false;
        if (keyOrder < 0) x = x->left;
        else x = x->right;
    }
    
    RBTree z = malloc(sizeof(_Node));

    z->key = key;
    z->value = value;

    z->color = RED;
    z->left = _NIL;
    z->right = _NIL;

    z->p = y;
    if (y == _NIL) {
        *treeP = z;
    } else if (strcmp(key, y->key) < 0) {
        y->left = z;
    } else {
        y->right = z;
    }

    _insertFixupRBTree(treeP, z);
    
    return true;
}

bool removeRBTree(RBTree *treeP, Key key) {
    RBTree z = _searchRBTree(*treeP, key);

    if (z == _NIL) return false;

    RBTree y = z, x;
    Color yOriginalColor = y->color;

    if (z->left == _NIL) {
        x = z->right;
        _Transplant(treeP, z, z->right);
    } else if (z->right == _NIL) {
        x = z->left;
        _Transplant(treeP, z, z->left);
    } else {
        y = z->right;
        while (y->left != _NIL) y = y->left;
        yOriginalColor = y->color;

        x = y->right;

        if (y->p == z) {
            // if we don't do _Transplant in else
            // we need to ensure, that x->p is y (in case it is NIL)
            x->p = y;
        } else {
            // if y in "middle" of z->right
            // cut it from z->right
            _Transplant(treeP, y, y->right);
            y->right = z->right;
            y->right->p = y;
        }
        
        _Transplant(treeP, z, y);
        y->left = z->left;
        y->left->p = y;
        y->color = z->color;
    }

    free(z->key);
    free(z);

    if (yOriginalColor == BLACK) {
        _deleteFixupRBTree(treeP, x);
    }

    return true;
}

FileError saveRBTree(RBTree *treeP, char *path) {
    FILE *file = fopen(path, "wb");

    if (errno == EACCES) {
        return FE_CAN_NOT_WRITE;
    }

    _saveTraverse(*treeP, file);

    fclose(file);

    return NO_ERRORS;
}

FileError loadRBTree(RBTree *treeP, char *path) {
    FILE *file = fopen(path, "rb");

    if (errno == EACCES) {
        return FE_CAN_NOT_READ;
    }

    RBTree newTree = createRBTree();

    unsigned char exists = _readByte(file);

    if (exists == 0) {
        deleteRBTree(*treeP);
        *treeP = newTree;
        fclose(file);
        return NO_ERRORS;
    } else if (exists != 1) {
        fclose(file);
        return FE_PARSING_ERROR;
    } 

    _LoadError error = _loadTraverse(&newTree, file);

    if (error != LOAD_OK) {
        switch (error) {
        case OUT_OF_MEMORY:
            return FE_OUT_OF_MEMORY;

        case PARSING_ERROR:
            return FE_PARSING_ERROR;
        
        default:
            return FE_PARSING_ERROR;
        }
    }

    newTree->p = _NIL;

    deleteRBTree(*treeP);
    *treeP = newTree;

    fclose(file);

    return NO_ERRORS;
}

void printTree(RBTree tree, int offset) {
    if (tree == _NIL) return;

    printTree(tree->right, offset + 2);
    for (int i = 0; i < offset; i++) printf(" ");
    printf("%s { key: %s, value: %lu }\n", tree->color == RED ? "RED  " : "BLACK", tree->key, tree->value);
    printTree(tree->left, offset + 2);
}

RBTree _searchRBTree(RBTree tree, Key key) {
    while (tree != _NIL) {
        int keyOrder = strcmp(key, tree->key);
    
        if (keyOrder < 0) tree = tree->left;
        else if (keyOrder > 0) tree = tree->right;
        else break;
    }
    
    return tree;
}

void _insertFixupRBTree(RBTree *treeP, RBTree z) {
    while (z->p->color == RED) {
        if (z->p == z->p->p->left) {
            RBTree y = z->p->p->right;

            // Case 1 - "red uncle"
            if (y->color == RED) {
                z->p->color = BLACK;
                y->color = BLACK;
                z->p->p->color = RED;
                z = z->p->p;
            } else {
                // Case 2 - "right children z"
                if (z == z->p->right) {
                    z = z->p;
                    _RotateLeft(treeP, z);
                }

                // Case 3 - "black uncle"
                z->p->color = BLACK;
                z->p->p->color = RED;
                _RotateRight(treeP, z->p->p);
            }
        } else {
            RBTree y = z->p->p->left;

            // Case 1 - "red uncle"
            if (y->color == RED) {
                z->p->color = BLACK;
                y->color = BLACK;
                z->p->p->color = RED;
                z = z->p->p;
            } else {
                // Case 2 - "left children z"
                if (z == z->p->left) {
                    z = z->p;
                    _RotateRight(treeP, z);
                }

                // Case 3 - "black uncle"
                z->p->color = BLACK;
                z->p->p->color = RED;
                _RotateLeft(treeP, z->p->p);
            }
        }
    }
    
    (*treeP)->color = BLACK;
}

void _deleteFixupRBTree(RBTree *treeP, RBTree x) {
    while (x != *treeP && x->color == BLACK) {
        if (x == x->p->left) {
            RBTree w = x->p->right;

            // Case 1 - "red brother"
            if (w->color == RED) {
                w->color = BLACK;
                x->p->color = RED;
                _RotateLeft(treeP, x->p);
                w = x->p->right;
            }

            // Case 2 - "black both brother's children"
            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED;
                x = x->p;
            } else {
                // Case 3 - "black right brother's child,
                // red left"
                if (w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = RED;
                    _RotateRight(treeP, w);
                    w = x->p->right;
                }

                // Case 4 - "red right brother's child"
                w->color = x->p->color;
                x->p->color = BLACK;
                w->right->color = BLACK;
                _RotateLeft(treeP, x->p);
                x = *treeP;
            }
        } else {
            RBTree w = x->p->left;

            // Case 1 - "red brother"
            if (w->color == RED) {
                w->color = BLACK;
                x->p->color = RED;
                _RotateRight(treeP, x->p);
                w = x->p->left;
            }

            // Case 2 - "black both brother's children"
            if (w->right->color == BLACK && w->left->color == BLACK) {
                w->color = RED;
                x = x->p;
            } else {
                // Case 3 - "black left brother's child,
                // red right"
                if (w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = RED;
                    _RotateLeft(treeP, w);
                    w = x->p->left;
                }

                // Case 4 - "red left brother's child"
                w->color = x->p->color;
                x->p->color = BLACK;
                w->left->color = BLACK;
                _RotateRight(treeP, x->p);
                x = *treeP;
            }
        }
    }

    x->color = BLACK;
}

void _saveTraverse(RBTree tree, FILE *file) {
    if (tree == _NIL) {
        _writeByte(file, 0);
        return;
    }

    _writeByte(file, 1);

    short keyLen = strlen(tree->key);
    fwrite(&keyLen, sizeof(short), 1, file);
    fwrite(tree->key, sizeof(char), keyLen, file);
    fwrite(&tree->value, sizeof(Value), 1, file);
    
    _saveTraverse(tree->left, file);
    _saveTraverse(tree->right, file);
}

_LoadError _loadTraverse(RBTree *treeP, FILE *file) {
    RBTree node = malloc(sizeof(_Node));

    if (node == NULL) return OUT_OF_MEMORY;

    short keyLen;
    int readLen = fread(&keyLen, sizeof(short), 1, file);

    if (readLen != 1) {
        free(node);
        return PARSING_ERROR;
    }

    node->key = malloc(sizeof(char) * (keyLen + 1));
    int readKey = fread(node->key, sizeof(char), keyLen, file);
    node->key[keyLen] = '\0';

    if (readKey != keyLen) {
        free(node->key);
        free(node);
        return PARSING_ERROR;
    }

    int readValue = fread(&node->value, sizeof(Value), 1, file);

    if (readValue != 1) {
        free(node->key);
        free(node);
        return PARSING_ERROR;
    }

    node->left = _NIL;
    node->right = _NIL;

    unsigned char hasLeft = _readByte(file);
    if (hasLeft == 1) {
        _LoadError err = _loadTraverse(&node->left, file);
        if (err != LOAD_OK) {
            free(node->key);
            free(node);
            return err;
        }

        node->left->p = node;
    } else if (hasLeft != 0) {
        free(node->key);
        free(node);
        return PARSING_ERROR;
    }

    unsigned char hasRight = _readByte(file);
    if (hasRight == 1) {
        _LoadError err = _loadTraverse(&node->right, file);
        if (err != LOAD_OK) {
            free(node->key);
            free(node);
            return err;
        }

        node->right->p = node;
    } else if (hasRight != 0) {
        free(node->key);
        free(node);
        return PARSING_ERROR;
    }
    
    *treeP = node;

    return LOAD_OK;
}

void _Transplant(RBTree *treeP, RBTree u, RBTree v) {
    if (u->p == _NIL) {
        *treeP = v;
    } else if (u == u->p->left) {
        u->p->left = v;
    } else {
        u->p->right = v;
    }
    v->p = u->p;
}

void _RotateLeft (RBTree *treeP, RBTree x) {
    RBTree y = x->right;

    x->right = y->left;
    if (y->left != _NIL) {
        y->left->p = x;
    }

    y->p = x->p;

    if (x->p == _NIL) {
        *treeP = y;
    } else if (x == x->p->left) {
        x->p->left = y;
    } else {
        x->p->right = y;
    }

    y->left = x;
    x->p = y;
}

void _RotateRight (RBTree *treeP, RBTree x) {
    RBTree y = x->left;

    x->left = y->right;
    if (y->right != _NIL) {
        y->right->p = x;
    }

    y->p = x->p;

    if (x->p == _NIL) {
        *treeP = y;
    } else if (x == x->p->right) {
        x->p->right = y;
    } else {
        x->p->left = y;
    }
    
    y->right = x;
    x->p = y;
}

void _writeByte(FILE *file, unsigned char byte) {
    fwrite(&byte, sizeof(unsigned char), 1, file);
}

unsigned char _readByte(FILE *file) {
    unsigned char res;
    fread(&res, sizeof(unsigned char), 1, file);
    return res;
}

// ===== main.c =====
// #include <stdio.h>
// #include "RBTree.h"

#define MAX_KEY_LENGTH 257
#define MAX_INPUT_LENGTH 280
#define MAX_PATH_LENGTH 257

#define RESULT_SUCCESS "OK"
#define RESULT_NOT_FOUND "NoSuchWord"
#define RESULT_ALREADY_EXISTS "Exist"
#define RESULT_ERROR "ERROR"

void readKey (Key dst, char *src, int keyLength);
void UI(RBTree *treeP);

int main() {
    RBTree tree = createRBTree();
    
    UI(&tree);

    deleteRBTree(tree);

    return 0;
}

void readKey (Key restrict dst, char *restrict src, int keyLength) {
    const char *end = src + keyLength;

    while (src < end) {
        char letter = *src++;
        *dst = (letter >= 'A' && letter <= 'Z') ? (letter | 0x20) : letter;
        dst++;
    }
    
    *dst = '\0';
}

void processKey (char *restrict src, int keyLength) {
    const char *end = src + keyLength;

    while (src < end) {
        char letter = *src;
        *src = (letter >= 'A' && letter <= 'Z') ? (letter | 0x20) : letter;
        src++;
    }
    
    *src = '\0';
}

void UI(RBTree *treeP) {
    char command[MAX_INPUT_LENGTH];
    char _key[MAX_KEY_LENGTH];
    Key keyS = _key;
    char path[MAX_PATH_LENGTH];
    int keyLength;
    char *sep;
    
    while (fgets(command, MAX_INPUT_LENGTH, stdin)) {
        if (command[0] == '\n' || command[0] == '\0') continue;

        switch (command[0]) {
        case '+':
            sep = strchr(command + 2, ' ');
            keyLength = sep - command - 2;

            Key key = malloc(sizeof(char) * (keyLength + 1));

            readKey(key, command + 2, keyLength);

            Value value;
            sscanf(sep + 1, "%lu", &value);

            if (insertRBTree(treeP, key, value)) {
                printf("%s\n", RESULT_SUCCESS);
            } else {
                printf("%s\n", RESULT_ALREADY_EXISTS);
                free(key);
            }

            break;

        case '-':
            keyLength = strlen(command + 2) - 1;

            readKey(keyS, command + 2, keyLength);

            if (removeRBTree(treeP, keyS)) {
                printf("%s\n", RESULT_SUCCESS);
            } else {
                printf("%s\n", RESULT_NOT_FOUND);
            }

            break;
        
        default:
            keyLength = strlen(command) - 1;

            readKey(keyS, command, keyLength);

            Value foundedValue;

            if (getRBTree(*treeP, keyS, &foundedValue)) {
                printf("%s: %lu\n", RESULT_SUCCESS, foundedValue);
            } else {
                printf("%s\n", RESULT_NOT_FOUND);
            }

            break;
        }
    }
}