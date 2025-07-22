# Mini-Interpreter

## Overview
This mini-interpreter is a lightweight scripting language implementation with C-like syntax. It supports variables, control structures, functions, and basic I/O operations. The interpreter processes scripts line by line, executing commands as they are parsed.

## Language Features

### 1. Variables and Data Types
Supported data types:
- `int`: Integer values (e.g., 42, -7)
- `float`: Floating-point values (e.g., 3.14, -0.5)
- `string`: Text values (e.g., "Hello")
- `bool`: Boolean values (true/false)

**Declaration and Assignment:**
```c
int x = 10
float pi = 3.14
string message = "Hello World"
bool flag = true
```

### 2. Input/Output Operations
**Print:**
```c
print "Hello World"    // String literal
print x                // Variable value
```

**Input:**
```c
input age             // Reads user input
print "You entered: "
print age
```

### 3. Control Structures
**If-Else:**
```c
if x > 5 {
    print "x is greater than 5"
} 
elseif x < 5 {
    print "x is less than 5"
}
else {
    print "x is 5"
}
endif
```

**While Loop:**
```c
int counter = 5
while counter > 0 {
    print counter
    counter = counter - 1
}
endwhile
```

**For Loop (C-style):**
```c
for (i = 0; i < 5; i = i + 1) {
    print i
}
endfor
```

### 4. Functions
**Definition:**
```c
function greet {
    print "Hello, world!"
}

function addNumbers(a, b) {
    return a + b
}
```

**Calling Functions:**
```c
greet()
int result = addNumbers(5, 3)
print result
```

### 5. Goto and Labels
```c
start:
print "Looping"
goto start
```

### 6. Expressions
Supported operators:
- Arithmetic: `+`, `-`, `*`, `/`
- Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Logical: `&&`, `||`, `!`

**Examples:**
```c
int result = (5 + 3) * 2
bool valid = (age >= 18) && (hasLicense == true)
```

## Compilation and Execution

### Building the Interpreter
1. Compile all header files together:
```bash
gcc -o mini-interpreter symbol_table.h print.h expression.h process_command.h
```

### Running Scripts
Create a script file (e.g., `test.txt`) with commands, then run:
```bash
./mini-interpreter test.txt
```

## Example Programs

### 1. Fibonacci Sequence
```c
function fibonacci {
    int n = 10
    int a = 0
    int b = 1
    
    print "Fibonacci sequence:"
    for (i = 0; i < n; i = i + 1) {
        print a
        int next = a + b
        a = b
        b = next
    }
    endfor
}

fibonacci()
```

### 2. User Authentication
```c
string password = "secret"
int attempts = 3

while attempts > 0 {
    input userInput
    if userInput == password {
        print "Access granted!"
        goto end
    } else {
        attempts = attempts - 1
        print "Attempts left: "
        print attempts
    }
}
endwhile
print "Access denied!"

end:
```

### 3. Temperature Converter
```c
function fahrenheitToCelsius(f) {
    float c = (f - 32) * 5 / 9
    return c
}

input tempF
float tempC = fahrenheitToCelsius(tempF)
print "Celsius: "
print tempC
```

## Limitations
1. Maximum 100 variables
2. Maximum 50 labels
3. Maximum 20 functions
4. Maximum 1000 loop iterations
5. No arrays or complex data structures
6. Limited error handling
7. No type checking in function parameters

## Error Messages
Common error messages:
- `Error: Too many variables`: Exceeded MAX_VARIABLES limit
- `Type mismatch`: Incompatible types in operation
- `Division by zero`: Attempted division by zero
- `Variable not found`: Accessing undefined variable
- `Missing endif/endwhile`: Unclosed control structure
- `Call stack overflow`: Excessive function recursion

## Conclusion
This mini-interpreter provides a simple yet powerful scripting environment for basic programming tasks. With its C-like syntax and support for essential programming constructs, it serves as both an educational tool and a lightweight automation solution.