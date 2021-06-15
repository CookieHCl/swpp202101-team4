; ModuleID = 'chain2.c'
source_filename = "chain2.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @sub(i32 %a) #0 {
; CHECK: start sub 1
; CHECK: sub {{.*}} 1 32
; CHECK: sub 2 {{.*}} 32
; CHECK: sub 4294967294 {{.*}} 32
; CHECK: sub {{.*}} 4294967289 32
; CHECK: sub {{.*}} 4294967290 32
; CHECK: end sub
entry:
  %a.addr = alloca i32, align 4
  store i32 %a, i32* %a.addr, align 4
  %0 = load i32, i32* %a.addr, align 4
  %sub = sub nsw i32 %0, 1
  store i32 %sub, i32* %a.addr, align 4
  %1 = load i32, i32* %a.addr, align 4
  %sub1 = sub nsw i32 1, %1
  store i32 %sub1, i32* %a.addr, align 4
  %2 = load i32, i32* %a.addr, align 4
  %sub2 = sub nsw i32 %2, 4
  store i32 %sub2, i32* %a.addr, align 4
  %3 = load i32, i32* %a.addr, align 4
  %sub3 = sub nsw i32 5, %3
  store i32 %sub3, i32* %a.addr, align 4
  %4 = load i32, i32* %a.addr, align 4
  %sub4 = sub nsw i32 %4, 1
  store i32 %sub4, i32* %a.addr, align 4
  %5 = load i32, i32* %a.addr, align 4
  ret i32 %5
}

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git f05b649610564b11c481a20598dbb3f532c4602a)"}
