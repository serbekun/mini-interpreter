#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_VARIABLES 100

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_UNKNOWN
} VarType;

typedef union {
    int intValue;
    float floatValue;
    char* stringValue;
    int boolValue;
} Value;

typedef struct Variable {
    char name[32];
    VarType type;
    Value value;
} Variable;

Variable symbol_table[MAX_VARIABLES];
int var_count = 0;

Variable* FindVariable(const char* name) 
{
    for (int i = 0; i < var_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            return &symbol_table[i];
        }
    }
    return NULL;
}

Variable* AddVariable(const char* name, VarType type) 
{
    if (FindVariable(name) != NULL) return NULL;
    
    if (var_count >= MAX_VARIABLES) {
        printf("Error: Too many variables\n");
        return NULL;
    }
    
    Variable* var = &symbol_table[var_count++];
    strncpy(var->name, name, sizeof(var->name));
    var->type = type;
    
    // Initialize with default values
    switch (type) {
        case TYPE_INT: var->value.intValue = 0; break;
        case TYPE_FLOAT: var->value.floatValue = 0.0f; break;
        case TYPE_STRING: var->value.stringValue = strdup(""); break;
        case TYPE_BOOL: var->value.boolValue = 0; break;
        default: break;
    }
    return var;
}