target triple = "i386-unknown-linux-gnu"

; TC-related LLVM intrinsics.
define internal i32 @tc_call_in_thread(i32* arguments, i32 nb_args,
                                       i32 (i32, ...)* f) #0 {
  ; FIXME: unpack the arguments
  ; FIXME: free arguments
  ; FIXME: call f
  ; FIXME: return thread handle
  ret 0
}

; Function Attrs: inlinehint nounwind
define i32 @tc_async_call(i32 (i32, ...)* f, i32 nb_args ...) #0 {
  ; FIXME: malloc(nb_args * sizeof (i32))
  ; FIXME: for each argument: copy it inside the mallocd zone
  ; FIXME: call pthread_create
  ret 0
}

; Function Attrs: inlinehint nounwind
define void @tc_async_return(i32 thread, i32* result) #0 {
  ret void
}

attributes #0 = { inlinehint nounwind }
