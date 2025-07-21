#pragma once

#include "../variable/symbol_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Value EvaluateExpression(VarType* result_type, char* tokens[], int start, int count) {
    Value result = {0};
    *result_type = TYPE_UNKNOWN;
    
    if (count == 1) {
        char* token = tokens[start];
        char* endptr;
        
        // Check for variable
        Variable* var = FindVariable(token);
        if (var != NULL) {
            *result_type = var->type;
            return var->value;
        }
        
        // Check integer
        long intVal = strtol(token, &endptr, 10);
        if (*endptr == '\0') {
            result.intValue = (int)intVal;
            *result_type = TYPE_INT;
            return result;
        }
        
        // Check float
        float floatVal = strtof(token, &endptr);
        if (*endptr == '\0') {
            result.floatValue = floatVal;
            *result_type = TYPE_FLOAT;
            return result;
        }
        
        // Check boolean
        if (strcmp(token, "true") == 0) {
            result.boolValue = 1;
            *result_type = TYPE_BOOL;
            return result;
        }
        if (strcmp(token, "false") == 0) {
            result.boolValue = 0;
            *result_type = TYPE_BOOL;
            return result;
        }
        
        // Default to string
        result.stringValue = strdup(token);
        *result_type = TYPE_STRING;
        return result;
    }

    // Handle binary operations
    if (count == 3) {
        char* op = tokens[start+1];
        VarType type1, type2;
        Value val1 = EvaluateExpression(&type1, tokens, start, 1);
        Value val2 = EvaluateExpression(&type2, tokens, start+2, 1);
        
        // Type checking
        if (type1 != type2 || type1 == TYPE_STRING || type1 == TYPE_BOOL) {
            printf("Type mismatch or unsupported operation\n");
            return result;
        }
        
        if (type1 == TYPE_INT) {
            if (strcmp(op, "+") == 0) result.intValue = val1.intValue + val2.intValue;
            else if (strcmp(op, "-") == 0) result.intValue = val1.intValue - val2.intValue;
            else if (strcmp(op, "*") == 0) result.intValue = val1.intValue * val2.intValue;
            else if (strcmp(op, "/") == 0) {
                if (val2.intValue == 0) {
                    printf("Division by zero\n");
                    return result;
                }
                result.intValue = val1.intValue / val2.intValue;
            }
            *result_type = TYPE_INT;
        }
        else if (type1 == TYPE_FLOAT) {
            if (strcmp(op, "+") == 0) result.floatValue = val1.floatValue + val2.floatValue;
            else if (strcmp(op, "-") == 0) result.floatValue = val1.floatValue - val2.floatValue;
            else if (strcmp(op, "*") == 0) result.floatValue = val1.floatValue * val2.floatValue;
            else if (strcmp(op, "/") == 0) {
                if (val2.floatValue == 0.0f) {
                    printf("Division by zero\n");
                    return result;
                }
                result.floatValue = val1.floatValue / val2.floatValue;
            }
            *result_type = TYPE_FLOAT;
        }
        return result;
    }
    
    printf("Unsupported expression format\n");
    return result;
}