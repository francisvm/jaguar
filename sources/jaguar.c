#include <assert.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

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

static int sum_of(int a, int b, int c, int d, int e, int f)
{
  printf("%d, %d, %d, %d, %d, %d\n", a, b, c, d, e, f);
  return a + b + c + d + e + f;
}

int main(int argc, char const *argv[])
{
  pthread_t t = async_call((function_t)sum_of, 6, 1, 2, 3, 4, 5, 6);
  int result = 0;
  pthread_join(t, (void *)&result);
  printf("result: %d\n", result);
  return 0;
}
