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
