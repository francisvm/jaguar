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
