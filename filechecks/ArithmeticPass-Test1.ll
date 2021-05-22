; ModuleID = 'Arithmeticpass-test1.c'
source_filename = "Arithmeticpass-test1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @myand(i32 %0, i32 %1) #0 {
; CHECK: start myand 2:
; CHECK: urem
; CHECK-NOT: and
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  store i32 %1, i32* %4, align 4
  %8 = load i32, i32* %3, align 4
  %9 = load i32, i32* %3, align 4
  %10 = add nsw i32 %8, %9
  store i32 %10, i32* %5, align 4
  %11 = load i32, i32* %4, align 4
  %12 = and i32 %11, 15
  store i32 %12, i32* %6, align 4
  %13 = load i32, i32* %5, align 4
  %14 = load i32, i32* %6, align 4
  %15 = add nsw i32 %13, %14
  store i32 %15, i32* %7, align 4
  %16 = load i32, i32* %7, align 4
  ret i32 %16
}
; CHECK: end myand

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
; CHECK: start main 0:
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 1, i32* %2, align 4
  store i32 2, i32* %3, align 4
  %5 = load i32, i32* %2, align 4
  %6 = load i32, i32* %3, align 4
  %7 = call i32 @myand(i32 %5, i32 %6)
  store i32 %7, i32* %4, align 4
  ret i32 0
}
; CHECK: end main

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git 4990141a4366eb00abdc8252d7cbb8adeacb9954)"}
