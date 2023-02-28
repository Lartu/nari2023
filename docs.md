# Nari Documentation

Nari can only load one source file at a time, so all your code should be on a single file.
The suggested file extension is `.nari`, but you can use whatever you want.

A Nari source file is a sequence of **commands**. A command is a word surounded by whitespace.
Commands do stuff. For example, `1`, `10`, `+` and `.` are commands. `1 10 + .` is a valid
Nari code (which in fact prints `11`). Nari uses reverse Polish notation.

Available commands:
- `#` starts a comment. It consumes characters from the source file until another `#` is found. Since `#` is a command, remember that it must be separated from other commands by using whitespace.
- `"` starts a string. It consumes characters from the source file until another `"` is found. The string value is pushed to the stack.
- `{` starts a new command definition. Any non-existent word might be used after this command, and it will be the name of the new command.
- `+` pops two numeric values from the stack, from bottom to top, adds them and pushes the result back to the stack.
- `-` pops two numeric values from the stack, from bottom to top, subtracts them and pushes the result back to the stack.
- `*` pops two numeric values from the stack, from bottom to top, multiplies them and pushes the result back to the stack.
- `/` pops two numeric values from the stack, from bottom to top, divides them and pushes the result back to the stack.
- `%` pops two numeric values from the stack, from bottom to top, modulo-es them and pushes the result back to the stack.
- `.` pops and prints the whole stack, from bottom to top.
- `=` pops two values from the stack and then pushes 1 if they are equal in type and value, or 0 if they aren't.
- `!=` pops two values from the stack and then pushes 1 if they are not equal in type or value, or 0 if they are.
- `<` pops two numbers from the stack, from bottom to top, and pushes 1 if the bottom-most one is smaller than the top-most one, 0 otherwise.
- `>` pops two numbers from the stack, from bottom to top, and pushes 1 if the bottom-most one is greater than the top-most one, 0 otherwise.
- `<=` pops two numbers from the stack, from bottom to top, and pushes 1 if the bottom-most one is smaller than or equal to the top-most one, 0 otherwise.
- `>=` pops two numbers from the stack, from bottom to top, and pushes 1 if the bottom-most one is greater than or equal to the top-most one, 0 otherwise.
- `and` pops two numbers from the stack, then pushes 1 if both are not equal to 0. 0 otherwise.
- `or` pops two numbers from the stack, then pushes 1 if any of them is not equal to 0. 0 otherwise.
- `lf` pushes a line feed character to the stack.
- `sd` pushes a string delimiter (generally `"`) to the stack.
- `}` closes a new command definition.
- `[` (while) opens a while block.
- `:` pops a numeric value from the stack. If it's not 0, executes every command until a `]` is found matching the previous `[`. Otherwise it skips until the `]`.
- `]` (wend) if the `:` condition was not 0, jumps to the matching `[`.
- `(` (if) pops a numeric value from the stack. If it's not 0, executes every command until a matching `|` or `)` is found. Otherwise it skips until the matching `)` or `|`.
- `|` (else) if commands weren't being executed thanks to the matching `(`, commands start being executed. Otherwise, they stop being executed until a matching `)` is found.
- `)` (end if) closes an if block. If commands weren't being executed, they start being executed.
- `@` pops a string value from the stack and pushes the value of the variable with the name indicated by the string value.
- `$` pops a value and a string value from the stack, from bottom to top, and stores the first value in a variable named as the string value.
- `&` pops two string values from the stack, from bottom to top, and concatenates them.
- `s` pops a numeric value from the stack, turns it into a string, and pushes it back.
- `n` pops a string value from the stack, turns it into a number, and pushes it back. It must be a valid, integer number.
- **Any Integer Number** (for example `15` or `9` or `-145`) pushes said value to the stack.
- `@var` such as `@foo` or `@bar` pushes the value of the variable `foo` or `bar` or whatever to the stack.
- `$var` such as `$foo` or `$bar` pops a value and stores it to the variable called `var` or `foo` or `bar` or whatever you like.
- Any other word: if a command with that name has been declared, it is executed.

## Command definition examples

Command `sayHello` that prints `Hello!` when called.

```
{ sayHello " Hello!" lf . }
```

Command `duplicate` that pops a numeric value from the stack and duplicates it.

```
{ duplicate 2 * }
```

## While examples

Count from 1 to 100.

```
1 $i
[ @i 100 <= : @i lf . @i 1 + $i ]
```

Loop forever.

```
[ 1 : " Forever!" lf . ]
```

## If examples

Check if number is even or odd.

```
55 $number
@number 2 % 0 = ( " Is Even" lf . | " Is Odd" lf . )
```

Check if true. As you can see, the else is not mandatory.

```
1 ( " True" lf . )
```
