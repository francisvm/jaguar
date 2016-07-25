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

## Project

The project has been done for the PRPA class, (parallel programming). The code
we want to achieve at the end is something similar to this:

```tiger
let
  var buf := async read(10) /* Asynchronous call */
in                          /* This happens in parallel with the read.       */
  for i := 0 to 300000 do   /* ...                                           */
    ();                     /* ...                                           */
                            /* ...                                           */
  print_int(buf)            /* Here, we wait for the asynchronous call to be */
                            /* finished, and return the value.               */
end
```

* [Jaguar proposal](https://github.com/thegameg/async-tiger/blob/master/proposal.md)

* [LLVM-jaguar](https://github.com/thegameg/llvm/tree/jaguar)

* [Tiger Compiler assignments](https://www.lrde.epita.fr/~tiger/assignments.html)

The sources of the `Tiger Compiler` itself cannnot be open-sourced, since it
remains an assignment to the EPITA students.

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
