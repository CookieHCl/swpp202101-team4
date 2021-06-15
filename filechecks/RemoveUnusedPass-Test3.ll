; ModuleID = 'filechecks/unused.c'
source_filename = "filechecks/unused.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @norm(i32 %x, i32 %y) #0 {
entry:
; CHECK: start norm 2:
; CHECK-NOT: store 4 r{{[0-9]+}} sp 16
; CHECK-NOT: store 4 r{{[0-9]+}} sp 20
  %x.addr = alloca i32, align 4
  %y.addr = alloca i32, align 4
  %sx = alloca i32, align 4
  %sy = alloca i32, align 4
  %to_remove1 = alloca i32, align 4
  %to_remove2 = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  store i32 %y, i32* %y.addr, align 4
  %0 = load i32, i32* %x.addr, align 4
  %1 = load i32, i32* %x.addr, align 4
  %mul = mul nsw i32 %0, %1
  store i32 %mul, i32* %sx, align 4
  %2 = load i32, i32* %y.addr, align 4
  %3 = load i32, i32* %y.addr, align 4
  %mul1 = mul nsw i32 %2, %3
  store i32 %mul1, i32* %sy, align 4
  %4 = load i32, i32* %sx, align 4
  %5 = load i32, i32* %sy, align 4
  %add = add nsw i32 %4, %5
  store i32 %add, i32* %to_remove1, align 4
  %6 = load i32, i32* %x.addr, align 4
  %7 = load i32, i32* %y.addr, align 4
  %add2 = add nsw i32 %6, %7
  store i32 %add2, i32* %to_remove2, align 4
  %8 = load i32, i32* %sx, align 4
  %9 = load i32, i32* %sy, align 4
  %add3 = add nsw i32 %8, %9
  ret i32 %add3
}
; CHECK: end norm

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git f05b649610564b11c481a20598dbb3f532c4602a)"}
