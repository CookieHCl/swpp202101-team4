; ModuleID = 'arg.c'
source_filename = "arg.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @argument(i32 %c) #0 {
entry:
; CHECK: start argument 1
; CHECK: store 4 4 r{{[0-9]+}} 0
; CHECK: end argument
  %c.addr = alloca i32, align 4
  %ptr1 = alloca i32*, align 8
  %ptr2 = alloca i32**, align 8
  store i32 %c, i32* %c.addr, align 4
  store i32* %c.addr, i32** %ptr1, align 8
  store i32** %ptr1, i32*** %ptr2, align 8
  %0 = load i32**, i32*** %ptr2, align 8
  %1 = load i32*, i32** %0, align 8
  store i32 4, i32* %1, align 4
  ret void
}

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git f05b649610564b11c481a20598dbb3f532c4602a)"}
