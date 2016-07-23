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
pass the arguments to the function, but it looks that it's not that easy.

First, we'll need a `MachineFunctionPass`, which allows us to use
`MachineInstrs` in order to modify the current function, so to implement the
intrinsics.

The problem is that we can't load a `MachineFunctionPass` as a dynamic pass,
as we can do with a `FunctionPass`, so, we need to modify the X86 backend.

The repository containing the modified `LLVM` implementation is
[here](https://github.com/thegameg/llvm/tree/jaguar).

## Threads

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
  print_int(async_result)
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

But, we're interested in an LLVM representation.

In order to achieve a correct LLVM representation, we'll call internal
functions (similar to intrinsics, but outside LLVM) This allows us to get
closer to the optimizer and take a generic approach, as opposed to a
library call.

```llvm
; ModuleID = 'tc'
target triple = "i386-unknown-linux-gnu"

; Function Attrs: inlinehint nounwind
declare void @tc_print_int(i32) #0

; Function Attrs: nounwind
declare i32 @compute_21(i32) #1

; TC-related LLVM intrinsics.
declare i32 @llvm.tc_async_call(i32 (i32, ...)*, i32, ...) #2
declare void @llvm.tc_async_return(i32, i32*) #3

; Function Attrs: nounwind
define void @tc_main() #1 {
entry__main:
  ; Var containing the synchronous result of the computation.
  %result_23 = alloca i32
  ; Var containing the asynchronous result of the computation.
  %async_result_22 = alloca i32

  ; The thread handle.
  %async_result_thread = call i32 (i32 (i32, ...)*, ...) @llvm.tc_async_call(
                                i32 (i32, ...)* bitcast (i32 (i32)* @compute_21
                                                to       i32 (i32, ...)*),
                                i32 1, i32 300)

  %call_compute_21 = call i32 @compute_21(i32 300)
  store i32 %call_compute_21, i32* %result_23

  ; Sync.
  %result_232 = load i32, i32* %result_23
  call void @tc_print_int(i32 %result_232)

  ; Join the thread, wait for the routine to be done.
  ; This should store in the alloca'd variable.
  call void @llvm.tc_async_return(i32 %async_result_thread,
                                  i32* %async_result_22)

  ; Async.
  %async_result_223 = load i32, i32* %async_result_22
  call void @tc_print_int(i32 %async_result_223)
  ret void
}

attributes #0 = { inlinehint nounwind }
attributes #1 = { nounwind }
attributes #2 = { nounwind "jaguar-call"}
attributes #3 = { nounwind "jaguar-return"}
```

Here, the implementation of `tc_async_return` allocates memory on the heap
for the arguments to be copied, calls `pthread_create` with a wrapper function
called `in_thread_wrapper` here, that unpacks the arguments, passes them to the
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

In order to implement this, a C-based draft code has been created:

```c
typedef int(*function_t)(void);
typedef int arg_t;

struct async
{
  function_t f;
  size_t nb_args;
  arg_t args[0];
};

void *in_thread_wrapper(void *arg)
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

pthread_t async_call(function_t f, int nb_args, ...)
{
  va_list ap;
  va_start(ap, nb_args);

  struct async *a = malloc(sizeof (struct async) + nb_args * sizeof (arg_t));

  for (int i = 0; i < nb_args; ++i)
    a->args[i] = va_arg(ap, int);
  va_end(ap);

  a->nb_args = nb_args;
  a->f = f;

  pthread_t thread;
  int res = pthread_create(&thread, NULL, &in_thread_wrapper, a);
  assert(!res);
  return thread;
}
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
