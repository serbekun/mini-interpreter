#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../variable/symbol_table.h"
#include "../operates/expression.h"
#include "../commands/print.h"

#define MAX_LINE_LENGTH 512
#define MAX_LINES 100
#define MAX_TOKENS 50
#define MAX_STACK_DEPTH 20

typedef enum {
    CMD_NORMAL,
    CMD_IF,
    CMD_ELSE,
    CMD_WHILE,
    CMD_FOR
} CommandType;

typedef struct {
    int line_index;
    int condition_met;
    int in_else;
    CommandType type;
    int start_line;
    int condition_line;
    int end_line;
    int loop_counter;
    char init_cmd[MAX_LINE_LENGTH];
    char cond_expr[MAX_LINE_LENGTH];
    char inc_cmd[MAX_LINE_LENGTH];
} ControlFrame;

char* program_lines[MAX_LINES];
int line_count = 0;
ControlFrame control_stack[MAX_STACK_DEPTH];
int control_stack_top = 0;
int call_stack[MAX_STACK_DEPTH];
int call_stack_top = 0;
int current_line_index = 0;
int skip_until_endif = 0;

void TokenizeLine(char* line, char* tokens[], int* token_count) {
    *token_count = 0;
    char* token = strtok(line, " ");
    while (token != NULL && *token_count < MAX_TOKENS) {
        tokens[(*token_count)++] = token;
        token = strtok(NULL, " ");
    }
}

int FindMatchingEnd(int start_index, const char* start_delim, const char* end_delim) {
    int depth = 1;
    for (int i = start_index + 1; i < line_count; i++) {
        if (strncmp(program_lines[i], start_delim, strlen(start_delim)) == 0) {
            depth++;
        } else if (strncmp(program_lines[i], end_delim, strlen(end_delim)) == 0) {
            depth--;
            if (depth == 0) {
                return i;
            }
        }
    }
    return -1; // Not found
}

void ProcessCommand(const char *filename) {
    // Read entire program into memory
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file '%s'\n", filename);
        return;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        program_lines[line_count] = strdup(line);
        line_count++;
        if (line_count >= MAX_LINES) break;
    }
    fclose(file);

    // Preprocess to find labels and functions
    for (int i = 0; i < line_count; i++) {
        char* line = program_lines[i];
        // Check for label (ends with colon)
        char* colon = strchr(line, ':');
        if (colon && colon[1] == '\0') {
            char label_name[32];
            strncpy(label_name, line, colon - line);
            label_name[colon - line] = '\0';
            AddLabel(label_name, i);
        }
        // Check for function definition
        else if (strncmp(line, "function ", 9) == 0) {
            char func_name[32];
            if (sscanf(line, "function %31s", func_name) == 1) {
                int end_line = FindMatchingEnd(i, "{", "}");
                if (end_line != -1) {
                    AddFunction(func_name, i, end_line);
                    i = end_line; // Skip to end of function
                }
            }
        }
    }

    // Main execution loop
    while (current_line_index < line_count) {
        char* line = program_lines[current_line_index];
        char tokens[MAX_TOKENS][MAX_LINE_LENGTH];
        char* token_ptrs[MAX_TOKENS];
        int token_count = 0;

        // Skip empty lines
        if (line[0] == '\0') {
            current_line_index++;
            continue;
        }

        // Skip labels
        char* colon = strchr(line, ':');
        if (colon && colon[1] == '\0') {
            current_line_index++;
            continue;
        }

        // Tokenize the line
        char line_copy[MAX_LINE_LENGTH];
        strcpy(line_copy, line);
        char* token = strtok(line_copy, " ");
        while (token != NULL && token_count < MAX_TOKENS) {
            strcpy(tokens[token_count], token);
            token_ptrs[token_count] = tokens[token_count];
            token_count++;
            token = strtok(NULL, " ");
        }

        if (token_count == 0) {
            current_line_index++;
            continue;
        }

        const char* command = token_ptrs[0];
        
        // Skip execution if inside false conditional block
        if (skip_until_endif > 0) {
            if (strcmp(command, "if") == 0 || 
                strcmp(command, "else") == 0 || 
                strcmp(command, "elseif") == 0) {
                // Nested control structures - just skip
            } 
            else if (strcmp(command, "endif") == 0) {
                skip_until_endif--;
            }
            current_line_index++;
            continue;
        }
        
        // Exit command
        if (!strcmp(command, "exit")) {
            const char *arg1 = token_count > 1 ? token_ptrs[1] : "0";
            printf("Program ended with exit code '%s'\n", arg1);
            break;
        }
        // Print command
        else if (!strcmp(command, "print")) {
            if (token_count < 2) {
                printf("Error: print requires an argument\n");
                current_line_index++;
                continue;
            }
            // Reconstruct the argument
            char arg[MAX_LINE_LENGTH] = "";
            for (int i = 1; i < token_count; i++) {
                strcat(arg, token_ptrs[i]);
                if (i < token_count-1) strcat(arg, " ");
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
        // Input command
        else if (!strcmp(command, "input")) {
            if (token_count < 2) {
                printf("Error: input requires a variable name\n");
                current_line_index++;
                continue;
            }
            
            char* var_name = token_ptrs[1];
            Variable* var = FindVariable(var_name);
            if (var == NULL) {
                printf("Error: variable '%s' not found\n", var_name);
                current_line_index++;
                continue;
            }
            
            char input_buffer[MAX_LINE_LENGTH];
            printf("Enter value for %s: ", var_name);
            if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
                printf("Error reading input\n");
                current_line_index++;
                continue;
            }
            input_buffer[strcspn(input_buffer, "\n")] = '\0';
            
            switch (var->type) {
                case TYPE_INT:
                    var->value.intValue = atoi(input_buffer);
                    break;
                case TYPE_FLOAT:
                    var->value.floatValue = atof(input_buffer);
                    break;
                case TYPE_STRING:
                    free(var->value.stringValue);
                    var->value.stringValue = strdup(input_buffer);
                    break;
                case TYPE_BOOL:
                    if (strcmp(input_buffer, "true") == 0) var->value.boolValue = 1;
                    else if (strcmp(input_buffer, "false") == 0) var->value.boolValue = 0;
                    else printf("Invalid boolean value\n");
                    break;
                default:
                    printf("Unsupported type\n");
            }
        }
        // Goto command
        else if (!strcmp(command, "goto")) {
            if (token_count < 2) {
                printf("Error: goto requires a label name\n");
                current_line_index++;
                continue;
            }
            
            char* label_name = token_ptrs[1];
            int target_line = FindLabel(label_name);
            if (target_line == -1) {
                printf("Error: label '%s' not found\n", label_name);
                current_line_index++;
            } else {
                current_line_index = target_line;
            }
            continue;
        }
        // Function call
        else if (token_count >= 1 && token_ptrs[token_count-1][0] == '(' && 
                 token_ptrs[token_count-1][1] == ')') {
            char func_name[32];
            strncpy(func_name, command, sizeof(func_name));
            Function* func = FindFunction(func_name);
            if (func == NULL) {
                printf("Error: function '%s' not defined\n", func_name);
                current_line_index++;
                continue;
            }
            
            if (call_stack_top >= MAX_STACK_DEPTH) {
                printf("Error: call stack overflow\n");
                current_line_index++;
                continue;
            }
            
            call_stack[call_stack_top++] = current_line_index + 1;
            current_scope++;
            current_line_index = func->start_line + 1;
            continue;
        }
        // Return from function
        else if (!strcmp(command, "return")) {
            if (call_stack_top == 0) {
                printf("Error: return outside function\n");
                current_line_index++;
                continue;
            }
            
            current_scope--;
            RemoveVariablesInScope();
            current_line_index = call_stack[--call_stack_top];
            continue;
        }
        // If statement
        else if (!strcmp(command, "if")) {
            if (token_count < 2) {
                printf("Syntax error: if requires condition\n");
                current_line_index++;
                continue;
            }
            
            // Extract condition tokens
            char cond_tokens[MAX_TOKENS][MAX_LINE_LENGTH];
            char* cond_ptrs[MAX_TOKENS];
            int cond_count = token_count - 1;
            for (int i = 0; i < cond_count; i++) {
                strcpy(cond_tokens[i], token_ptrs[i+1]);
                cond_ptrs[i] = cond_tokens[i];
            }
            
            // Evaluate condition
            VarType cond_type;
            Value cond_val = EvaluateExpression(&cond_type, cond_ptrs, 0, cond_count);
            if (cond_type != TYPE_BOOL) {
                printf("Condition must be boolean\n");
                current_line_index++;
                continue;
            }
            
            // Find endif line
            int endif_line = FindMatchingEnd(current_line_index, "if", "endif");
            if (endif_line == -1) {
                printf("Missing endif for if statement\n");
                current_line_index++;
                continue;
            }
            
            if (cond_val.boolValue) {
                // Condition true - execute block
                ControlFrame frame;
                frame.type = CMD_IF;
                frame.condition_met = 1;
                frame.start_line = current_line_index;
                frame.end_line = endif_line;
                control_stack[control_stack_top++] = frame;
                current_line_index++;
            } else {
                // Condition false - skip to else/elseif/endif
                skip_until_endif = 1;
                current_line_index = endif_line;
            }
            continue;
        }
        // Else if statement
        else if (!strcmp(command, "elseif")) {
            if (control_stack_top == 0 || control_stack[control_stack_top-1].type != CMD_IF) {
                printf("elseif without matching if\n");
                current_line_index++;
                continue;
            }
            
            ControlFrame* frame = &control_stack[control_stack_top-1];
            if (frame->condition_met) {
                // Previous condition was true - skip to endif
                current_line_index = frame->end_line;
                continue;
            }
            
            if (token_count < 2) {
                printf("Syntax error: elseif requires condition\n");
                current_line_index++;
                continue;
            }
            
            // Extract condition tokens
            char cond_tokens[MAX_TOKENS][MAX_LINE_LENGTH];
            char* cond_ptrs[MAX_TOKENS];
            int cond_count = token_count - 1;
            for (int i = 0; i < cond_count; i++) {
                strcpy(cond_tokens[i], token_ptrs[i+1]);
                cond_ptrs[i] = cond_tokens[i];
            }
            
            // Evaluate condition
            VarType cond_type;
            Value cond_val = EvaluateExpression(&cond_type, cond_ptrs, 0, cond_count);
            if (cond_type != TYPE_BOOL) {
                printf("Condition must be boolean\n");
                current_line_index++;
                continue;
            }
            
            if (cond_val.boolValue) {
                frame->condition_met = 1;
                current_line_index++;
            } else {
                // Skip to next elseif/else/endif
                current_line_index++;
            }
            continue;
        }
        // Else statement
        else if (!strcmp(command, "else")) {
            if (control_stack_top == 0 || control_stack[control_stack_top-1].type != CMD_IF) {
                printf("else without matching if\n");
                current_line_index++;
                continue;
            }
            
            ControlFrame* frame = &control_stack[control_stack_top-1];
            if (frame->condition_met) {
                // Previous condition was true - skip to endif
                current_line_index = frame->end_line;
            } else {
                // Execute else block
                frame->condition_met = 1;
                current_line_index++;
            }
            continue;
        }
        // End of if block
        else if (!strcmp(command, "endif")) {
            if (control_stack_top == 0 || control_stack[control_stack_top-1].type != CMD_IF) {
                printf("endif without matching if\n");
                current_line_index++;
                continue;
            }
            
            control_stack_top--;
            current_line_index++;
            continue;
        }
        // While loop
        else if (!strcmp(command, "while")) {
            if (token_count < 2) {
                printf("Syntax error: while requires condition\n");
                current_line_index++;
                continue;
            }
            
            // Extract condition tokens
            char cond_tokens[MAX_TOKENS][MAX_LINE_LENGTH];
            char* cond_ptrs[MAX_TOKENS];
            int cond_count = token_count - 1;
            for (int i = 0; i < cond_count; i++) {
                strcpy(cond_tokens[i], token_ptrs[i+1]);
                cond_ptrs[i] = cond_tokens[i];
            }
            
            // Evaluate condition
            VarType cond_type;
            Value cond_val = EvaluateExpression(&cond_type, cond_ptrs, 0, cond_count);
            if (cond_type != TYPE_BOOL) {
                printf("Condition must be boolean\n");
                current_line_index++;
                continue;
            }
            
            if (!cond_val.boolValue) {
                // Condition false - skip to endwhile
                int endwhile_line = FindMatchingEnd(current_line_index, "while", "endwhile");
                if (endwhile_line == -1) {
                    printf("Missing endwhile for while loop\n");
                    current_line_index++;
                    continue;
                }
                current_line_index = endwhile_line + 1;
                continue;
            }
            
            // Find endwhile
            int endwhile_line = FindMatchingEnd(current_line_index, "while", "endwhile");
            if (endwhile_line == -1) {
                printf("Missing endwhile for while loop\n");
                current_line_index++;
                continue;
            }
            
            // Setup control frame
            ControlFrame frame;
            frame.type = CMD_WHILE;
            frame.start_line = current_line_index;
            frame.end_line = endwhile_line;
            frame.condition_line = current_line_index;
            memcpy(frame.cond_expr, line + 6, sizeof(frame.cond_expr)); // Save condition expression
            control_stack[control_stack_top++] = frame;
            current_line_index++;
            continue;
        }
        // End of while loop
        else if (!strcmp(command, "endwhile")) {
            if (control_stack_top == 0 || control_stack[control_stack_top-1].type != CMD_WHILE) {
                printf("endwhile without matching while\n");
                current_line_index++;
                continue;
            }
            
            ControlFrame* frame = &control_stack[control_stack_top-1];
            
            // Re-evaluate condition
            char cond_tokens[MAX_TOKENS][MAX_LINE_LENGTH];
            char* cond_ptrs[MAX_TOKENS];
            int cond_count = 0;
            
            char cond_copy[MAX_LINE_LENGTH];
            strcpy(cond_copy, frame->cond_expr);
            char* token = strtok(cond_copy, " ");
            while (token != NULL && cond_count < MAX_TOKENS) {
                strcpy(cond_tokens[cond_count], token);
                cond_ptrs[cond_count] = cond_tokens[cond_count];
                cond_count++;
                token = strtok(NULL, " ");
            }
            
            VarType cond_type;
            Value cond_val = EvaluateExpression(&cond_type, cond_ptrs, 0, cond_count);
            if (cond_type != TYPE_BOOL) {
                printf("Condition must be boolean\n");
                control_stack_top--;
                current_line_index++;
                continue;
            }
            
            if (cond_val.boolValue) {
                // Loop again
                current_line_index = frame->start_line + 1;
            } else {
                // Exit loop
                control_stack_top--;
                current_line_index++;
            }
            continue;
        }
        // For loop
        else if (!strcmp(command, "for")) {
            if (token_count < 6 || strcmp(token_ptrs[1], "(") != 0 || 
                strcmp(token_ptrs[3], ";") != 0 || strcmp(token_ptrs[5], ";") != 0) {
                printf("Syntax error: for(init; condition; increment)\n");
                current_line_index++;
                continue;
            }
            
            // Find endfor
            int endfor_line = FindMatchingEnd(current_line_index, "for", "endfor");
            if (endfor_line == -1) {
                printf("Missing endfor for for loop\n");
                current_line_index++;
                continue;
            }
            
            // Execute initialization
            char init_cmd[MAX_LINE_LENGTH];
            snprintf(init_cmd, sizeof(init_cmd), "%s", token_ptrs[2]);
            char* init_tokens[MAX_TOKENS];
            int init_count = 0;
            char init_copy[MAX_LINE_LENGTH];
            strcpy(init_copy, init_cmd);
            char* token = strtok(init_copy, " ");
            while (token != NULL && init_count < MAX_TOKENS) {
                init_tokens[init_count++] = token;
                token = strtok(NULL, " ");
            }
            
            // Process initialization command
            char saved_line[MAX_LINE_LENGTH];
            strcpy(saved_line, program_lines[current_line_index]);
            strcpy(program_lines[current_line_index], init_cmd);
            ProcessCommand(filename);
            strcpy(program_lines[current_line_index], saved_line);
            
            // Setup control frame
            ControlFrame frame;
            frame.type = CMD_FOR;
            frame.start_line = current_line_index;
            frame.end_line = endfor_line;
            frame.loop_counter = 0;
            strncpy(frame.init_cmd, init_cmd, sizeof(frame.init_cmd));
            strncpy(frame.cond_expr, token_ptrs[4], sizeof(frame.cond_expr));
            strncpy(frame.inc_cmd, token_ptrs[6], sizeof(frame.inc_cmd));
            control_stack[control_stack_top++] = frame;
            current_line_index++;
            continue;
        }
        // End of for loop
        else if (!strcmp(command, "endfor")) {
            if (control_stack_top == 0 || control_stack[control_stack_top-1].type != CMD_FOR) {
                printf("endfor without matching for\n");
                current_line_index++;
                continue;
            }
            
            ControlFrame* frame = &control_stack[control_stack_top-1];
            
            // Execute increment
            char inc_cmd[MAX_LINE_LENGTH];
            snprintf(inc_cmd, sizeof(inc_cmd), "%s", frame->inc_cmd);
            char* inc_tokens[MAX_TOKENS];
            int inc_count = 0;
            char inc_copy[MAX_LINE_LENGTH];
            strcpy(inc_copy, inc_cmd);
            char* token = strtok(inc_copy, " ");
            while (token != NULL && inc_count < MAX_TOKENS) {
                inc_tokens[inc_count++] = token;
                token = strtok(NULL, " ");
            }
            
            // Process increment command
            char saved_line[MAX_LINE_LENGTH];
            strcpy(saved_line, program_lines[current_line_index]);
            strcpy(program_lines[current_line_index], inc_cmd);
            ProcessCommand(filename);
            strcpy(program_lines[current_line_index], saved_line);
            
            // Check condition
            char cond_tokens[MAX_TOKENS][MAX_LINE_LENGTH];
            char* cond_ptrs[MAX_TOKENS];
            int cond_count = 0;
            
            char cond_copy[MAX_LINE_LENGTH];
            strcpy(cond_copy, frame->cond_expr);
            token = strtok(cond_copy, " ");
            while (token != NULL && cond_count < MAX_TOKENS) {
                strcpy(cond_tokens[cond_count], token);
                cond_ptrs[cond_count] = cond_tokens[cond_count];
                cond_count++;
                token = strtok(NULL, " ");
            }
            
            VarType cond_type;
            Value cond_val = EvaluateExpression(&cond_type, cond_ptrs, 0, cond_count);
            if (cond_type != TYPE_BOOL) {
                printf("Condition must be boolean\n");
                control_stack_top--;
                current_line_index++;
                continue;
            }
            
            if (cond_val.boolValue && frame->loop_counter < 1000) {
                // Loop again
                frame->loop_counter++;
                current_line_index = frame->start_line + 1;
            } else {
                // Exit loop
                control_stack_top--;
                current_line_index++;
            }
            continue;
        }
        // Variable declaration/assignment
        else if (!strcmp(command, "int") || !strcmp(command, "float") || 
                 !strcmp(command, "string") || !strcmp(command, "bool")) {
            if (token_count < 4 || strcmp(token_ptrs[2], "=") != 0) {
                printf("Syntax error\n");
                current_line_index++;
                continue;
            }
            
            // Determine type
            VarType type = TYPE_UNKNOWN;
            if (!strcmp(command, "int")) type = TYPE_INT;
            else if (!strcmp(command, "float")) type = TYPE_FLOAT;
            else if (!strcmp(command, "string")) type = TYPE_STRING;
            else if (!strcmp(command, "bool")) type = TYPE_BOOL;
            
            // Create or find variable
            Variable* var = FindVariable(token_ptrs[1]);
            if (var == NULL) {
                var = AddVariable(token_ptrs[1], type);
                if (var == NULL) {
                    printf("Error creating variable\n");
                    current_line_index++;
                    continue;
                }
            }
            
            // Evaluate expression
            VarType expr_type;
            Value value = EvaluateExpression(&expr_type, token_ptrs, 3, token_count-3);
            
            // Type checking
            if (expr_type != type && !(type == TYPE_FLOAT && expr_type == TYPE_INT)) {
                printf("Type mismatch\n");
                current_line_index++;
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
        // Function definition
        else if (!strcmp(command, "function")) {
            // Already processed in pre-scan, skip to end
            int end_line = FindMatchingEnd(current_line_index, "{", "}");
            if (end_line != -1) {
                current_line_index = end_line + 1;
                continue;
            }
        }
        
        current_line_index++;
    }

    // Cleanup
    for (int i = 0; i < line_count; i++) {
        free(program_lines[i]);
    }
}