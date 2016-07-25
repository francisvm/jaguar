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
* method

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
in                          /* This happens in parallel with the read.       */
  for i := 0 to 300000 do   /* ...                                           */
    ();                     /* ...                                           */
                            /* ...                                           */
  print_int(buf)            /* Here, we wait for the asynchronous call to be */
                            /* finished, and return the value.               */
end
```

#### Errors

Async calls done without being waited should be treated as synchronous. The
compiler should raise warnings as much as possible for this kind of behavior.

```tiger
let
in
  async read(130);
  print("This is executed after the read is done.")
end
```

//FIXME: Complex expressions: what should we do?

```tiger
var buf := ((); 10; 1320 + 30; (); async read(10))
```

## Implementation

### LLVM

The perfect usage of LLVM would be to use it as an external library, and
provide a non-intrusive usage for our front-end.

But, we can't use `LLVM IR` in order to lower the calls, since we need to do
some target-specific operations, like `push`, regarding the calling convention.

The goal here is to use `X86TargetLowering::LowerFormalArguments` in order to
pass the arguments to the function, but it looks that it's not that feasible.

First, we'll need a `MachineFunctionPass`, which allows us to use
`MachineInstrs` in order to modify the current function, so to implement the
intrinsics.

The problem is that we can't load a `MachineFunctionPass` as a dynamic pass,
as we can do with a `FunctionPass`, so, we need to modify the X86 backend.

The repository containing the modified `LLVM` implementation is
[here](https://github.com/thegameg/llvm/tree/jaguar).

In order to use it, the following command line must be used:

```
llc -jaguar file.ll
```

Besides, it only works on the `Triple::x86`.

## Threads

#### OS

One solution would be to spawn a `POSIX` thread using the `pthreads` API.

This implies that the OS scheduler has to take care of scheduling the threads.

##### Examples

Let's take an example of an use-case of the async call.

```tiger
let
  /* This function computes an useless result. */
  primitive compute(v : int) : int

  /* Var containing the asynchronous result of the computation. */
  var async_result := async compute(300)

  /* Var containing the synchronous result of the computation. */
  var result := compute(300)
in
  /* Sync. */
  print_int(result);

  /* Async. */
  print_int(async_result)
end
```

This should be translated to the equivalent C-program (of course, it's not
going to be translated to C, but LLVM).

```c
#include <jaguar.h>

/* This function computes an useless result. */
int compute(int v);

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

But, we're interested in an LLVM representation.

In order to achieve a correct LLVM representation, we'll call internal
functions (similar to intrinsics, but outside LLVM) This allows us to get
closer to the optimizer and take a generic approach, as opposed to a
library call.

```llvm
; ModuleID = 'basic.ll'
target triple = "i386-unknown-linux-gnu"

; Function Attrs: inlinehint nounwind
declare void @tc_print_int(i32) #0

; Function Attrs: nounwind
define void @tc_main() #1 {
entry__main:
  %result_19 = alloca i32
  %async_result_18 = alloca i32

  %async_result_thread = call i32 (i8* (...)*, i32, ...) @tc_async_call(i8* (...)* bitcast (i32 (i32)* @tc_compute to i8* (...)*), i32 1, i32 300)

  %call_compute = call i32 @tc_compute(i32 300)
  store i32 %call_compute, i32* %result_19
  %result_191 = load i32, i32* %result_19
  call void @tc_print_int(i32 %result_191)
  %0 = bitcast i32* %async_result_18 to i8**

  call void @tc_async_return(i32 %async_result_thread, i8** %0)

  %async_result_182 = load i32, i32* %async_result_18
  call void @tc_print_int(i32 %async_result_182)
  ret void
}

; Function Attrs: inlinehint nounwind
declare i32 @tc_compute(i32) #0

declare i32 @tc_async_call(i8* (...)*, i32, ...)

declare void @tc_async_return(i32, i8**)

attributes #0 = { inlinehint nounwind }
attributes #1 = { nounwind }
```

Here, the implementation of `tc_async_call` allocates memory on the heap
for the arguments to be copied, calls `pthread_create` with a wrapper function
called `tc_async_wrapper`, that unpacks the arguments, passes them to the
original callee function, and frees the memory.

It's supposed to return a handle to the launched task.

##### Problems

Let's take the following code as an example:

```tiger
let
  var a := 10
  var b := 30
  var c := async foo(a, b)
in
  a := 30;
  b := 40
end
```

Normally, we would just pass `a, b` to `pthread_create`, in order to call the
routine.

But, `pthread_create`'s prototype is the following:
```
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine) (void *), void *arg);
```

So, there's only one argument possible for the callee.

###### Solution

In order to implement this, the following runtime functions have been added:

```c
typedef void *(*function_t)();

struct async_function
{
  function_t f;
  size_t nb_args;
  void *args[0];
};

// Function called in a separate thread.
void *tc_async_wrapper(void *arg)
{
  struct async *a = arg;
  for (ssize_t i = a->nb_args - 1; i >= 0; --i)
    __asm__ volatile ("push %%eax\n"
                     :
                     :"a"(a->args[i])
                     : "sp");

  void *res = (void *)a->f();

  __asm__ volatile ("add %0, %%esp\n"
                    :
                    :"r"(a->nb_args * sizeof (arg_t))
                    : "sp");
  free(a);

  return res;
}

/** \name Internal functions (calls generated by the compiler only). */
/** \{ */

/** \brief Call a function in a separate thread.
    \param f       The callee function.
    \param nb_args The number of arguments the function is taking.
    \param ...     The arguments to be passed to the function.

    An element size is always the size of a word on 32-bit systems.
*/
pthread_t tc_async_call(function_t f, int nb_args, ...)
{
  // Prepare the argument list.
  va_list ap;
  va_start(ap, nb_args);

  // Allocate heap space for the structure followed by the arguments.
  struct async_function *a = malloc(sizeof (struct async_function)
                                    + nb_args * sizeof (void *));
  a->nb_args = nb_args;
  a->f = f;

  // Fill the arguments.
  for (int i = 0; i < nb_args; ++i)
    a->args[i] = va_arg(ap, void *);

  // Cleanup the argument list.
  va_end(ap);

  // Create a thread with the intermediate function call.
  pthread_t thread;
  int res = pthread_create(&thread, NULL, &tc_async_wrapper, a);
  assert(!res && "Failed to create a thread.");
  return thread;
}
/** \} */

/** \name Internal functions (calls generated by the compiler only). */
/** \{ */

/** \brief Waits for the result of a task and sets the result.
    \param thread  The thread id that should be joined.
    \param result  The result memory slot.

    An element size is always the size of a word on 32-bit systems.
*/
void tc_async_return(pthread_t thread, void **result)
{
  pthread_join(thread, result);
}
/** \} */
```

###### Solution

But actually, `tc_async_wrapper` is not implemented with inline x86 asm, but
using LLVM's `MachineInstrs`.

```
%struct.async_function = type { i8* (...)*, i32, [0 x i8*] }

declare i8* @llvm.tc_async_call(i8* (...)* %f, i32 %nb_args, i8** %args) #1

; Function Attrs: nounwind
define i8* @tc_async_wrapper(i8* %arg) #0 {
  %fun = bitcast i8* %arg to %struct.async_function*
  %pf = getelementptr inbounds %struct.async_function,
             %struct.async_function* %fun, i32 0, i32 0
  %pnb_args = getelementptr inbounds %struct.async_function,
             %struct.async_function* %fun, i32 0, i32 1
  %pargs = getelementptr inbounds %struct.async_function,
             %struct.async_function* %fun, i32 0, i32 2
  %args = bitcast [0 x i8*]* %pargs to i8**
  %f = load i8* (...)*, i8* (...)** %pf, align 4
  %nb_args = load i32, i32* %pnb_args, align 4
  %result = call i8* @llvm.tc_async_call(i8* (...)* %f, i32 %nb_args,
                                          i8** %args)
  ret i8* %result
}

attributes #0 = { nounwind "disable-tail-calls"="false" "no-frame-pointer-elim"="true" }
attributes #1 = { nounwind }
```

#### Thread pool

Another solution is to provide a thread pool in Tiger's runtime, available
using compiler intrinsics, translated during `TC-L`.

#### Resumable routines

The best solution is to handle the routines as resumable routines.

This means that the routines can be paused, the context saved, and replaced
with another routine, etc.

This implies that the routines should contain some kind of context, some kind
of state that allows the compiler to define where to resume.
