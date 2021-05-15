; ModuleID = 'filechecks/malloc2.c'
source_filename = "filechecks/malloc2.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32* @new(i32 %x) #0 {
; CHECK: start new 1:
entry:
; CHECK-NOT: store 4 
; CHECK-NOT: store 8 r1 sp 4
; CHECK-NOT: store 8 r1 sp 12
  %x.addr = alloca i32, align 4
  %a1 = alloca i32*, align 8
  %to_remove1 = alloca i32*, align 8
  %to_remove2 = alloca i32*, align 8
  %to_remove3 = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  %call = call noalias i8* @malloc(i64 4) #2
  %0 = bitcast i8* %call to i32*
  store i32* %0, i32** %a1, align 8
  %call1 = call noalias i8* @malloc(i64 4) #2
  %1 = bitcast i8* %call1 to i32*
  store i32* %1, i32** %to_remove1, align 8
  store i32 5, i32* %to_remove3, align 4
  %2 = load i32*, i32** %a1, align 8
  ret i32* %2
}
; CHECK: end new

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) #1

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git f05b649610564b11c481a20598dbb3f532c4602a)"}
