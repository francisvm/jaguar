![Jaguar Compiler](https://github.com/thegameg/async-tiger/blob/master/resources/logo.png)

# Jaguar Compiler (Async Tiger Compiler)

The Jaguar Compiler project aims at adding asynchronous support to the Tiger
language.

Take a look at the
[proposal.md](https://github.com/thegameg/async-tiger/blob/master/proposal.md)
in order to see more about the specifications of the project.

### Authors

* [Rémi Billon](https://github.com/HunterB06)
* [Arnaud Gaillard](https://github.com/Jambonino)
* [Francis Visoiu Mistrih](https://github.com/thegameg)

## Tiger Compiler

The Tiger Compiler, also called `TC`, is a project aiming at the implementation
of a Tiger language compiler, in C++.

The compiler implementation is based on the book "Modern Compiler
Implementation" by Andrew W. Appel.

TC compiles Tiger source to:

* x86
* ARM
* MIPS

or `LLVM`, which allows us to target more architectures.
