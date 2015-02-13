Todo
====
* proper freeing of strings and the gc
* run gc at some point 
* use Obj* with STRING type to handle strings in most cases (instead of c str)
* memory pool to avoid unnecessary amounts of alloc / free
* nicer stack trace in break mode
* real number constants (handle decimals)
* compiled code
* eval function
* read function
* apply function
* str function
* more math functions like mod, round, max, min, etc
* let expressions
* lambdas should keep track of their arg count to avoid pulling to much from the stack
* errors should result in breaking at the stack, with possibility to fix issue
* do statement should not create bugs on the stack and should eval in correct order
* macros
* quasiquoting
* some kind of support for lists/arrays using [ and ]
* rest arg syntax
* better repl interaction, with keypresses, history, colors, etc

Done
====
* loading of files from disk
* comments
* quoting
