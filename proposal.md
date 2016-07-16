# Jaguar Compiler

## Grammar

```
# Function call.
| async id ( [ exp { , exp }] )
# Method call.
| async lvalue . id ( [ exp { , exp }] )
```

## Bindings

The `AsyncCallExp` is handled the same way as the `CallExp` by the
`Binder`.

## Typing

The `AsyncCallExp` is handled the same way as the `CallExp` by the
`TypeChecker`.

## Semantics

### Vocabulary

A `routine` is one of the following:

* function
* primitive
* method.

### Calls

Tagging a routine call with the keyword `async` should asynchronously execute
the routine in another thread.

#### Example

```tiger
var buf := async read(10) /* Asynchronous call */
```

The asynchronous call returning a value should be waited by the main thread,
so the first use of the result of the call is the wait of the thread.

#### Example

```tiger
let
  var buf := async read(10) /* Asynchronous call */
  var tmp := ""             /* This happens at the same time with the read.  */
in                          /* ...                                           */
  for i := 0 to 300000 do   /* ...                                           */
    ();                     /* ...                                           */
                            /* ...                                           */
  print_int(size(buf))      /* Here, we wait for the asynchronous call to be */
                            /* finished, and return the value.               */
end
```

#### Errors

Async calls done without being waited should be treated as synchronous. The
compiler should raise warnings as much as possible for this kind of behavior.

```tiger
let
in
async read(130)
print("This is executed after the read's end")
```

## Implementation

### Use-binder

#### Use-list

We currently have a `def_` attribute on the Bindable<T> class allowing us
to retrieve the definition of a use.

We now need a list of the uses of a declaration, as LLVM does.

#### Result bind

We need to bind the result variable to the routine call. This means that:

```tiger
var buf := async read(10)
```

`buf` needs to be binded to `read` somehow.

//FIXME: Complex expressions:

```tiger
var buf := ((); 10; 1320 + 30; (); async read(10))
```

### LLVM

### Scheduling

#### OS

One solution would be to spawn a `POSIX` thread using the `pthreads` API.

This implies that the OS scheduler has to take care of scheduling the threads.

##### Examples

Let's take an example of an use-case of the async call.

```tiger
let
  /* This function computes an useless result. */
  function compute(v : int) : int =
  (
    let
      var result := 0
    in
      for i := 0 to v do
      (
        for j := 0 to v do
        (
          result = v + (result + (j + i) * 2 / (((i * j) + 1) * (result + j))) / 23;
        )
      );
      result
    end
  )

  /* Var containing the asynchronous result of the computation. */
  var async_result := async compute(300)

  /* Var containing the synchronous result of the computation. */
  var result := compute(300)
in
  /* Sync. */
  print_int(result);

  /* Async. */
  print_int(async_result);
end
```

This should be translated to the equivalent C-program (of course, it's not
going to be translated to C, but LLVM).

```c
#include <jaguar.h>

/* This function computes an useless result. */
int compute(int v)
{
  int result = 0;
  for (int i = 0; i < v; ++i)
    for (int j = 0; j < v; ++j)
      result = v + (result + (j + i) * 2 / (((i * j) + 1) * (result + j))) / 23;

  return result;
}

int main(void)
{
  // Var containing the asynchronous result of the computation.
  int async_result = 0;
  // Launch a thread running the asynchronous routine.
  task_t async_result_thread = tc_async_call((function_t)&compute, 300);

  // Var containing the synchronous result of the computation.
  int result = compute(300);
  // Sync.
  tc_print_int(result);

  // Jon the thread, wait for the routine to be done.
  tc_async_return(async_result_thread, &async_result);
  // Async.
  tc_print_int(async_result);

  return 0;
}
```

##### libjaguar

The library handling all the asynchronous calls from Tiger code to C code, is
called `libjaguar`.

![libjaguar.h](https://github.com/thegameg/async-tiger/blob/master/sources/libjaguar/jaguar.h)

is the main interface.

It will be implemented in C or C++, but it will always export a C API in order to
correctly work with Tiger functions.

#### Thread pool

Another solution is to provide a thread pool in Tiger's runtime, available
using compiler intrinsics, translated during `TC-L`.

#### Resumable routines

The best solution is to handle the routines as resumable routines.

This means that the routines can be paused, the context saved, and replaced
with another routine, etc.

This implies that the routines should contain some kind of context, some kind
of state that allows the compiler to define where to resume.
