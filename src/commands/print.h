#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void Print(const char* value) {
    printf("%s\n", value);
}

void PrintInt(int value) {
    printf("%d\n", value);
}

void PrintFloat(float value) {
    printf("%f\n", value);
}

void PrintBool(int value) {
    if (value) {
        printf("true\n");
    } else {
        printf("false\n");
    }
}