; ModuleID = 'chain10.c'
source_filename = "chain10.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i64 @type(i32 %n) #0 {
; CHECK: start type 1
; CHECK: add {{.*}} 1 64
; CHECK-NOT: add {{.*}} 4 32
; CHECK-NOT: add {{.*}} 4 64
; CHECK: end type
entry:
  %n.addr = alloca i32, align 4
  %a = alloca i64, align 8
  store i32 %n, i32* %n.addr, align 4
  %0 = load i32, i32* %n.addr, align 4
  %conv = sext i32 %0 to i64
  %add = add nsw i64 %conv, 1
  store i64 %add, i64* %a, align 8
  %1 = load i64, i64* %a, align 8
  %add1 = add nsw i64 %1, 1
  store i64 %add1, i64* %a, align 8
  %2 = load i64, i64* %a, align 8
  %add2 = add nsw i64 %2, 1
  %conv3 = trunc i64 %add2 to i32
  store i32 %conv3, i32* %n.addr, align 4
  %3 = load i32, i32* %n.addr, align 4
  %conv4 = sext i32 %3 to i64
  %add5 = add nsw i64 %conv4, 1
  %conv6 = trunc i64 %add5 to i32
  store i32 %conv6, i32* %n.addr, align 4
  %4 = load i32, i32* %n.addr, align 4
  %conv7 = sext i32 %4 to i64
  ret i64 %conv7
}

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git f05b649610564b11c481a20598dbb3f532c4602a)"}
