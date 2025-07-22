#pragma once

#include "../variable/symbol_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

Value EvaluateExpression(VarType* result_type, char* tokens[], int start, int count) {
    Value result = {0};
    *result_type = TYPE_UNKNOWN;
    
    if (count == 0) {
        return result;
    }

    // Handle parentheses
    if (strcmp(tokens[start], "(") == 0 && strcmp(tokens[start+count-1], ")") == 0) {
        return EvaluateExpression(result_type, tokens, start+1, count-2);
    }

    // Handle unary operators
    if (count >= 2 && strcmp(tokens[start], "!") == 0) {
        VarType sub_type;
        Value sub_val = EvaluateExpression(&sub_type, tokens, start+1, count-1);
        if (sub_type != TYPE_BOOL) {
            printf("Type mismatch for '!' operator\n");
            return result;
        }
        result.boolValue = !sub_val.boolValue;
        *result_type = TYPE_BOOL;
        return result;
    }

    // Handle single token
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
    for (int i = start + 1; i < start + count - 1; i++) {
        char* op = tokens[i];
        
        // Logical OR (lowest precedence)
        if (strcmp(op, "||") == 0) {
            VarType type1, type2;
            Value val1 = EvaluateExpression(&type1, tokens, start, i - start);
            Value val2 = EvaluateExpression(&type2, tokens, i+1, start + count - i - 1);
            
            if (type1 != TYPE_BOOL || type2 != TYPE_BOOL) {
                printf("Type mismatch for '||' operator\n");
                return result;
            }
            
            result.boolValue = val1.boolValue || val2.boolValue;
            *result_type = TYPE_BOOL;
            return result;
        }
    }

    for (int i = start + 1; i < start + count - 1; i++) {
        char* op = tokens[i];
        
        // Logical AND
        if (strcmp(op, "&&") == 0) {
            VarType type1, type2;
            Value val1 = EvaluateExpression(&type1, tokens, start, i - start);
            Value val2 = EvaluateExpression(&type2, tokens, i+1, start + count - i - 1);
            
            if (type1 != TYPE_BOOL || type2 != TYPE_BOOL) {
                printf("Type mismatch for '&&' operator\n");
                return result;
            }
            
            result.boolValue = val1.boolValue && val2.boolValue;
            *result_type = TYPE_BOOL;
            return result;
        }
    }

    for (int i = start + 1; i < start + count - 1; i++) {
        char* op = tokens[i];
        
        // Comparison operators
        if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 || 
            strcmp(op, "<") == 0 || strcmp(op, ">") == 0 ||
            strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0) {
            
            VarType type1, type2;
            Value val1 = EvaluateExpression(&type1, tokens, start, i - start);
            Value val2 = EvaluateExpression(&type2, tokens, i+1, start + count - i - 1);
            
            // Type checking
            if (type1 != type2 || (type1 != TYPE_INT && type1 != TYPE_FLOAT)) {
                printf("Type mismatch for comparison operator\n");
                return result;
            }
            
            int cmp;
            if (type1 == TYPE_INT) {
                cmp = (val1.intValue > val2.intValue) - (val1.intValue < val2.intValue);
            } else {
                cmp = (val1.floatValue > val2.floatValue) - (val1.floatValue < val2.floatValue);
            }
            
            if (strcmp(op, "==") == 0) result.boolValue = (cmp == 0);
            else if (strcmp(op, "!=") == 0) result.boolValue = (cmp != 0);
            else if (strcmp(op, "<") == 0) result.boolValue = (cmp < 0);
            else if (strcmp(op, ">") == 0) result.boolValue = (cmp > 0);
            else if (strcmp(op, "<=") == 0) result.boolValue = (cmp <= 0);
            else if (strcmp(op, ">=") == 0) result.boolValue = (cmp >= 0);
            
            *result_type = TYPE_BOOL;
            return result;
        }
    }

    for (int i = start + 1; i < start + count - 1; i++) {
        char* op = tokens[i];
        
        // Arithmetic operators
        if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 || 
            strcmp(op, "*") == 0 || strcmp(op, "/") == 0) {
            
            VarType type1, type2;
            Value val1 = EvaluateExpression(&type1, tokens, start, i - start);
            Value val2 = EvaluateExpression(&type2, tokens, i+1, start + count - i - 1);
            
            // Type checking
            if ((type1 != TYPE_INT && type1 != TYPE_FLOAT) || 
                (type2 != TYPE_INT && type2 != TYPE_FLOAT)) {
                printf("Type mismatch for arithmetic operator\n");
                return result;
            }
            
            // Determine result type (promote to float if either is float)
            if (type1 == TYPE_FLOAT || type2 == TYPE_FLOAT) {
                float fval1 = (type1 == TYPE_FLOAT) ? val1.floatValue : (float)val1.intValue;
                float fval2 = (type2 == TYPE_FLOAT) ? val2.floatValue : (float)val2.intValue;
                
                if (strcmp(op, "+") == 0) result.floatValue = fval1 + fval2;
                else if (strcmp(op, "-") == 0) result.floatValue = fval1 - fval2;
                else if (strcmp(op, "*") == 0) result.floatValue = fval1 * fval2;
                else if (strcmp(op, "/") == 0) {
                    if (fval2 == 0.0f) {
                        printf("Division by zero\n");
                        return result;
                    }
                    result.floatValue = fval1 / fval2;
                }
                
                *result_type = TYPE_FLOAT;
            } else {
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
            return result;
        }
    }
    
    printf("Unsupported expression format\n");
    return result;
}