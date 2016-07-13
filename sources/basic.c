/* This function computes an useless result. */
int compute(int v)
{
	int result = 0;
	for (int i = 0; i < v; ++i)
          result = v + (result + (j + i) * 2 / (((i * j) + 1)
			* (result + j))) / 23;

	return result;
}

int main(int argc, char const *argv[])
{
	// Var containing the asynchronous result of the computation.
	int async_result = 0;
	// Launch a thread running the asynchronous routine.
	pthread_t async_result_thread = tc_async_call(&compute, 300);

	// Var containing the synchronous result of the computation.
	int result = compute(300);
	// Sync.
	tc_print_int(result);

	// Jon the thread, wait for the routine to be done.
	tc_async_join(async_result_thread, &async_result);
	// Async.
	tc_print_int(async_result);

	return 0;
}
