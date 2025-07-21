#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../variable/symbol_table.h"
#include "../operates/expression.h"
#include "../commands/print.h"

#define TOKENS_NUM 10
#define MAX_LINE_LENGTH 512

void ProcessCommand(const char *filename) {
    char cmd[256];
    char line[MAX_LINE_LENGTH];

    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file '%s'\n", filename);
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        strncpy(cmd, line, sizeof(cmd));
        cmd[sizeof(cmd)-1] = '\0';

        char *tokens[TOKENS_NUM] = {0};
        int argc = 0;
        
        char *token = strtok(cmd, " ");
        while (token != NULL && argc < TOKENS_NUM) {
            tokens[argc++] = token;
            token = strtok(NULL, " ");
        }
        
        if (argc == 0) continue;
        
        const char *command = tokens[0];
        
        // Exit command
        if (!strcmp(command, "exit")) {
            const char *arg1 = argc > 1 ? tokens[1] : "0";
            printf("Program ended with exit code '%s'\n", arg1);
            fclose(file);
            return;
        }
        // Print command
        else if (!strcmp(command, "print")) {
            if (argc < 2) {
                printf("Error: print requires an argument\n");
                continue;
            }
            // Reconstruct the argument (may contain spaces)
            char arg[MAX_LINE_LENGTH] = "";
            for (int i = 1; i < argc; i++) {
                strcat(arg, tokens[i]);
                if (i < argc-1) strcat(arg, " ");
            }
            
            // Check if variable exists
            Variable* var = FindVariable(arg);
            if (var != NULL) {
                switch (var->type) {
                    case TYPE_INT: PrintInt(var->value.intValue); break;
                    case TYPE_FLOAT: PrintFloat(var->value.floatValue); break;
                    case TYPE_STRING: Print(var->value.stringValue); break;
                    case TYPE_BOOL: PrintBool(var->value.boolValue); break;
                    default: printf("Unknown variable type\n");
                }
            } else {
                // Print as string literal
                Print(arg);
            }
        }
        // Variable declaration/assignment
        else if (!strcmp(command, "int") || !strcmp(command, "float") || 
                 !strcmp(command, "string") || !strcmp(command, "bool")) {
            if (argc < 4 || strcmp(tokens[2], "=") != 0) {
                printf("Syntax error\n");
                continue;
            }
            
            // Determine type
            VarType type = TYPE_UNKNOWN;
            if (!strcmp(command, "int")) type = TYPE_INT;
            else if (!strcmp(command, "float")) type = TYPE_FLOAT;
            else if (!strcmp(command, "string")) type = TYPE_STRING;
            else if (!strcmp(command, "bool")) type = TYPE_BOOL;
            
            // Create or find variable
            Variable* var = FindVariable(tokens[1]);
            if (var == NULL) {
                var = AddVariable(tokens[1], type);
                if (var == NULL) {
                    printf("Error creating variable\n");
                    continue;
                }
            }
            
            // Evaluate expression
            VarType expr_type;
            Value value = EvaluateExpression(&expr_type, tokens, 3, argc-3);
            
            // Type checking
            if (expr_type != type && !(type == TYPE_FLOAT && expr_type == TYPE_INT)) {
                printf("Type mismatch\n");
                continue;
            }
            
            // Handle type conversions
            if (type == TYPE_FLOAT && expr_type == TYPE_INT) {
                var->value.floatValue = (float)value.intValue;
            } else {
                switch (type) {
                    case TYPE_INT: var->value.intValue = value.intValue; break;
                    case TYPE_FLOAT: var->value.floatValue = value.floatValue; break;
                    case TYPE_BOOL: var->value.boolValue = value.boolValue; break;
                    case TYPE_STRING: 
                        free(var->value.stringValue);
                        var->value.stringValue = strdup(value.stringValue); 
                        break;
                    default: break;
                }
            }
        }
    }
    
    fclose(file);
}