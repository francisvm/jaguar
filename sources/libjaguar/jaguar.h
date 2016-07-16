#pragma once

#include <pthread.h>
#include <stdint.h>

/*
 * All tiger types are 32 bit types. In order to use a correct type, we use
 * a typedef here.
 */
typedef int32_t result_t;

/*
 * The type of function to be called from C code. This should be a Tiger
 * function.
 */
typedef int (__cdecl *function_t)(int, ...);

/*
 * The type of a Tiger task.
 * For now, it's only a handle to the pthread-based thread.
 */
typedef pthread_t task_t;

/*
 * Call a function asynchronously.
 * We want to force the `cdecl` calling convention here, in order to be sure
 * that the variadic arguments are correctly passed from LLVM to C code.
 */
task_t __cdecl tc_async_call(function_t f, ...);

/*
 * Wait for the task to be done, and return the value.
 */
void tc_async_return(task_t thread, result_t *result);

// FIXME: Add locks, detach, kill, any other pthread api is needed.
