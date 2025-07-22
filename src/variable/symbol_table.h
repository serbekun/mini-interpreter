#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_VARIABLES 100
#define MAX_LABELS 50
#define MAX_FUNCTIONS 20
#define MAX_SCOPES 10

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
    int scope;
} Variable;

typedef struct Label {
    char name[32];
    int line_number;
} Label;

typedef struct Function {
    char name[32];
    int start_line;
    int end_line;
} Function;

Variable symbol_table[MAX_VARIABLES];
Label labels[MAX_LABELS];
Function functions[MAX_FUNCTIONS];
int var_count = 0;
int label_count = 0;
int function_count = 0;
int current_scope = 0;

Variable* FindVariable(const char* name) 
{
    for (int i = var_count - 1; i >= 0; i--) {
        if (strcmp(symbol_table[i].name, name) == 0 && 
            symbol_table[i].scope <= current_scope) {
            return &symbol_table[i];
        }
    }
    return NULL;
}

Variable* AddVariable(const char* name, VarType type) 
{
    // Check if variable already exists in current scope
    for (int i = 0; i < var_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0 && 
            symbol_table[i].scope == current_scope) {
            return NULL;
        }
    }
    
    if (var_count >= MAX_VARIABLES) {
        printf("Error: Too many variables\n");
        return NULL;
    }
    
    Variable* var = &symbol_table[var_count++];
    strncpy(var->name, name, sizeof(var->name));
    var->type = type;
    var->scope = current_scope;
    
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

void RemoveVariablesInScope() {
    int new_count = 0;
    for (int i = 0; i < var_count; i++) {
        if (symbol_table[i].scope < current_scope) {
            if (new_count != i) {
                symbol_table[new_count] = symbol_table[i];
            }
            new_count++;
        } else {
            // Free string memory if needed
            if (symbol_table[i].type == TYPE_STRING) {
                free(symbol_table[i].value.stringValue);
            }
        }
    }
    var_count = new_count;
}

void AddLabel(const char* name, int line_number) {
    if (label_count >= MAX_LABELS) {
        printf("Error: Too many labels\n");
        return;
    }
    
    // Check for duplicate
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, name) == 0) {
            printf("Error: Duplicate label '%s'\n", name);
            return;
        }
    }
    
    strncpy(labels[label_count].name, name, sizeof(labels[label_count].name));
    labels[label_count].line_number = line_number;
    label_count++;
}

int FindLabel(const char* name) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, name) == 0) {
            return labels[i].line_number;
        }
    }
    return -1; // Not found
}

void AddFunction(const char* name, int start_line, int end_line) {
    if (function_count >= MAX_FUNCTIONS) {
        printf("Error: Too many functions\n");
        return;
    }
    
    // Check for duplicate
    for (int i = 0; i < function_count; i++) {
        if (strcmp(functions[i].name, name) == 0) {
            printf("Error: Duplicate function '%s'\n", name);
            return;
        }
    }
    
    strncpy(functions[function_count].name, name, sizeof(functions[function_count].name));
    functions[function_count].start_line = start_line;
    functions[function_count].end_line = end_line;
    function_count++;
}

Function* FindFunction(const char* name) {
    for (int i = 0; i < function_count; i++) {
        if (strcmp(functions[i].name, name) == 0) {
            return &functions[i];
        }
    }
    return NULL;
}
