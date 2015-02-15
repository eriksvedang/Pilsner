Prio 1
======
* tail recursion optimization
* nicer stack trace in break mode
* str function
* apply function
* macros
* real number constants (handle decimals)
* errors should result in breaking at the stack, with possibility to fix issue
* not allowing cons to take a non-list as second argument
* not doing string comparison for special form symbols

Todo
====
* docstrings
* calling (break) on toplevel messes up value stack and is weird
* crashes when GC:ing in break mode
* use Obj* with STRING type to handle strings in most cases (instead of c str)
* memory pool to avoid unnecessary amounts of alloc / free
* compiled code
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
* proper freeing of strings and the gc
* loading of files from disk
* comments
* quoting
* lambdas should keep track of their arg count to avoid pulling to much from the stack
* do statement should not create bugs on the stack and should eval in correct order
