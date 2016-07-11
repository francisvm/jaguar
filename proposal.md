# Async Tiger

## Grammar

```
# Function call.
| async id ( [ exp { , exp }] )
# Method call.
| async lvalue . id ( [ exp { , exp }] )

# Function declaration.
| async function id ( tyfields ) [ : type-id ] = exp
# Primitive declaration.
| async primitive id ( tyfields ) [ : type-id ]
# Method declaration.
| async method id ( tyfields ) [ : type-id ] = exp

```

## Bindings

The `AsyncFunctionDec` is handled the same way as the `FunctionDec` by the
`Binder`.

The `AsyncCallExp` is handled the same way as the `CallExp` by the
`Binder`.

## Typing

The `AsyncFunctionDec` is handled the same way as the `FunctionDec` by the
`TypeChecker`.

The `AsyncCallExp` is handled the same way as the `CallExp` by the
`TypeChecker`.

## Semantics

### Vocabulary

A `routine` is one of the following:

* function
* primitive
* method.

### Declarations

Tagging a routine declaration with the keyword `async` specifies to the
compiler that this routine is available to be called in an asynchronous context.

#### Example

```tiger
type char = int
type char_array = array of char
async function read(int size) : char_array =
(
  let
    var buf := char_array [size] of 0
  in
    for i := 0 to size do
      buf[i] = ord(getchar());

    buf
  end
)
```

### Calls

Calling a routine tagged with the keyword `async` should preserve the regular
semantics. This means that the `async` tag is meaningless.

#### Example

```tiger
var buf := read(10) /* Regular call, synchronous */
```

Tagging a routine call with the keyword `async` should asynchronously execute
the function in another thread, or at least, it should be executed outside
the main thread.

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
var buf := ((), 10, 1320 + 30, (), async read(10))
```

### LLVM

### Scheduling

#### OS

One solution would be to spawn a `POSIX` thread using the `pthreads` API.

This implies that the OS scheduler has to take care of scheduling the threads.

#### Thread pool

Another solution is to provide a thread pool in Tiger's runtime, available
using compiler intrinsics, translated during `TC-L`.

#### Resumable routines

The best solution is to handle the routines as resumable routines.

This means that the routines can be paused, the context saved, and replaced
with another routine, etc.

This implies that the routines should contain some kind of context, some kind
of state that allows the compiler to define where to resume.
