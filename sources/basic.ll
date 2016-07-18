; ModuleID = 'tc'
target triple = "i386-unknown-linux-gnu"

; Function Attrs: inlinehint nounwind
declare void @tc_print_int(i32) #0

; Function Attrs: nounwind
declare i32 @compute_21(i32) #1

; TC-related LLVM intrinsics.
declare i32 @tc_async_call(i32 (i32, ...)*, ...) #0
declare void @tc_async_return(i32, i32*) #0

; Function Attrs: nounwind
define void @tc_main() #1 {
entry__main:
  ; Var containing the synchronous result of the computation.
  %result_23 = alloca i32
  ; Var containing the asynchronous result of the computation.
  %async_result_22 = alloca i32

  ; The thread handle.
  %async_result_thread = call i32 (i32 (i32, ...)*, ...) @tc_async_call(
                                i32 (i32, ...)* bitcast (i32 (i32)* @compute_21
                                                to       i32 (i32, ...)*),
                                i32 300)

  %call_compute_21 = call i32 @compute_21(i32 300)
  store i32 %call_compute_21, i32* %result_23

  ; Sync.
  %result_232 = load i32, i32* %result_23
  call void @tc_print_int(i32 %result_232)

  ; Join the thread, wait for the routine to be done.
  ; This should store in the alloca'd variable.
  call void @tc_async_return(i32 %async_result_thread, i32* %async_result_22)

  ; Async.
  %async_result_223 = load i32, i32* %async_result_22
  call void @tc_print_int(i32 %async_result_223)
  ret void
}

attributes #0 = { inlinehint nounwind }
attributes #1 = { nounwind }
