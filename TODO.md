Prio 1
======
* str function
* apply function
* macros
* real number constants (handle decimals)
* errors should result in breaking at the stack, with possibility to fix issue
* not allowing cons to take a non-list as second argument

Todo
====
* docstrings
* crashes when GC:ing in break mode
* use Obj* with STRING type to handle strings in most cases (instead of c str)
* memory pool to avoid unnecessary amounts of alloc / free
* eval function
* read function
* more math functions like mod, round, max, min, etc
* let expressions
* quasiquoting
* some kind of support for lists/arrays using [ and ]
* rest arg syntax
* better repl interaction, with keypresses, history, colors, etc
* run gc when "low" on memory

Done
====
* not doing string comparison for special form symbols
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
