#include "headers.h"

int parseInput(char* input, char** args) {
    int i = 0, argsSize = 0;
    char *token = strtok(input, " ");
    while (token != NULL) {
        args[i] = token;
        token = strtok(NULL, " ");
        i++;
        argsSize++;
    }
    return argsSize;
}

void readPs(char* input, char** result) {
    int i = 0;
    char *token = strtok(input, "\n");
    while (token != NULL) {
        result[i] = token;
        token = strtok(NULL, "\n");
        i++;
    }
}

string sliceString(char *toSlice) {
    string converted = toSlice;
    string result = converted.substr(1);

    return result;
}