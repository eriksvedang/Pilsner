Prio 1
======
* macros
* let expressions
* hashmaps/dictionaries
* print numbers more nicely
* str should print objects (lists etc) correctly
* some kind of mutable atom type

Todo
====
* crashes on load if recursive function can't find itself
* remove enums and use actual bytes (chars) for bytecode
* docstrings
* use Obj* with STRING type to handle strings in most cases (instead of c str)
* more math functions like round, get-line
* quasiquoting
* some kind of support for lists/arrays using [ and ]
* rest arg syntax
* better repl interaction, with keypresses, history, colors, etc
* run gc when "low" on memory
* memory pool to avoid unnecessary amounts of alloc / free
* being able to break to repl with a keyboard shortcut when in an infinite loop or similar
* convert variadic uses of + etc compile into a series of ADD ops
* direct lookup should look directly at cdr and not even keep a pointer to the pair?
* user proper typedef over function pointer instead of void *

Done
====
* crashes when calling (gc)
* real number constants (handle decimals)
* popping the global scope off the stack should be prohibited
* eval function
* read function
* apply function
* or should return first true value
* compiler errors should propagate to runtime (when compiling lambdas), if-statments with missing branch collapses for example
* Låt nil finnas i GC:n så att alla listavslutare är samma nil och det inte finns en enda alloc av onödiga nils någonstans.
* str function
* not doing string comparison for special form symbols
* errors should result in breaking at the stack, with possibility to fix issue
* not allowing cons to take a non-list as second argument
* make break-mode work again, without bugs
* compiled code
* calling (break) on toplevel messes up value stack and is weird
* write some C-macros for common access patterns
* handle if:s with missing else branch
* tail recursion optimization
* Code:s for common math operators
* store calling args in C-array to avoid creating excessive CONS-cells
* proper freeing of strings and the gc
* loading of files from disk
* comments
* quoting
* lambdas should keep track of their arg count to avoid pulling to much from the stack
* do statement should not create bugs on the stack and should eval in correct order
* pool of Obj:s instead of malloc/free
