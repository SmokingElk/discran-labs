// ===== RBTree.h =====
// #pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <map>
#include <time.h>
#include <string>

typedef char* Key;
typedef unsigned long int Value;

#define MAX_KEY_LENGTH 257
#define MAX_INPUT_LENGTH 280
#define MAX_PATH_LENGTH 257

#define RESULT_SUCCESS "OK"
#define RESULT_NOT_FOUND "NoSuchWord"
#define RESULT_ALREADY_EXISTS "Exist"
#define RESULT_ERROR "ERROR"

void UI(std::map<std::string, Value> map);

int main() {
    std::map<std::string, Value> map;
    
    clock_t start = clock();
    UI(map);
    double timePassed = (double)(clock() - start) / CLOCKS_PER_SEC * 1000.0;
    printf("time: %fms\n", timePassed);

    return 0;
}

void readKey (Key dst, char *src, int keyLength) {
    for (int i = 0; i < keyLength; i++) {
        char letter = src[i];
        if ('A' <= letter && letter <= 'Z') letter = letter - 'A' + 'a';
        dst[i] = letter;
    }

    dst[keyLength] = '\0';
}

void UI(std::map<std::string, Value> map) {
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

            readKey(keyS, command + 2, keyLength);

            Value value;
            sscanf(sep + 1, "%lu", &value);

            if (map.count(keyS) == 0) {
                map[keyS] = value;
                printf("%s\n", RESULT_SUCCESS);
            } else {
                printf("%s\n", RESULT_ALREADY_EXISTS);
            }

            break;

        case '-':
            keyLength = strlen(command + 2) - 1;

            readKey(keyS, command + 2, keyLength);

            if (map.count(keyS) > 0) {
                map.erase(keyS);
                printf("%s\n", RESULT_SUCCESS);
            } else {
                printf("%s\n", RESULT_NOT_FOUND);
            }

            break;

        default:
            keyLength = strlen(command) - 1;

            readKey(keyS, command, keyLength);

            Value foundedValue;

            if (map.count(keyS) > 0) {
                printf("%s: %lu\n", RESULT_SUCCESS, map[keyS]);
            } else {
                printf("%s\n", RESULT_NOT_FOUND);
            }

            break;
        }
    }
}