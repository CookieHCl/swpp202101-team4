; ModuleID = 'vec15.c'
source_filename = "vec15.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i64 @rearrange(i64* %A, i64* %re, i32 %n) #0 {
; CHECK: start rearrange 3
; CHECK: vload 4
; CHECK-NOT: vload
; CHECK: end rearrange
entry:
  %A.addr = alloca i64*, align 8
  %re.addr = alloca i64*, align 8
  %n.addr = alloca i32, align 4
  %sum = alloca i64, align 8
  %i = alloca i32, align 4
  store i64* %A, i64** %A.addr, align 8
  store i64* %re, i64** %re.addr, align 8
  store i32 %n, i32* %n.addr, align 4
  store i64 0, i64* %sum, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %1 = load i32, i32* %n.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i64*, i64** %A.addr, align 8
  %3 = load i64*, i64** %re.addr, align 8
  %4 = load i32, i32* %i, align 4
  %idxprom = sext i32 %4 to i64
  %arrayidx = getelementptr inbounds i64, i64* %3, i64 %idxprom
  %5 = load i64, i64* %arrayidx, align 8
  %arrayidx1 = getelementptr inbounds i64, i64* %2, i64 %5
  %6 = load i64, i64* %arrayidx1, align 8
  %7 = load i64, i64* %sum, align 8
  %add = add nsw i64 %7, %6
  store i64 %add, i64* %sum, align 8
  %8 = load i64*, i64** %A.addr, align 8
  %9 = load i64*, i64** %re.addr, align 8
  %10 = load i32, i32* %i, align 4
  %add2 = add nsw i32 %10, 1
  %idxprom3 = sext i32 %add2 to i64
  %arrayidx4 = getelementptr inbounds i64, i64* %9, i64 %idxprom3
  %11 = load i64, i64* %arrayidx4, align 8
  %arrayidx5 = getelementptr inbounds i64, i64* %8, i64 %11
  %12 = load i64, i64* %arrayidx5, align 8
  %13 = load i64, i64* %sum, align 8
  %add6 = add nsw i64 %13, %12
  store i64 %add6, i64* %sum, align 8
  %14 = load i64*, i64** %A.addr, align 8
  %15 = load i64*, i64** %re.addr, align 8
  %16 = load i32, i32* %i, align 4
  %add7 = add nsw i32 %16, 2
  %idxprom8 = sext i32 %add7 to i64
  %arrayidx9 = getelementptr inbounds i64, i64* %15, i64 %idxprom8
  %17 = load i64, i64* %arrayidx9, align 8
  %arrayidx10 = getelementptr inbounds i64, i64* %14, i64 %17
  %18 = load i64, i64* %arrayidx10, align 8
  %19 = load i64, i64* %sum, align 8
  %add11 = add nsw i64 %19, %18
  store i64 %add11, i64* %sum, align 8
  %20 = load i64*, i64** %A.addr, align 8
  %21 = load i64*, i64** %re.addr, align 8
  %22 = load i32, i32* %i, align 4
  %add12 = add nsw i32 %22, 3
  %idxprom13 = sext i32 %add12 to i64
  %arrayidx14 = getelementptr inbounds i64, i64* %21, i64 %idxprom13
  %23 = load i64, i64* %arrayidx14, align 8
  %arrayidx15 = getelementptr inbounds i64, i64* %20, i64 %23
  %24 = load i64, i64* %arrayidx15, align 8
  %25 = load i64, i64* %sum, align 8
  %add16 = add nsw i64 %25, %24
  store i64 %add16, i64* %sum, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %26 = load i32, i32* %i, align 4
  %inc = add nsw i32 %26, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond, !llvm.loop !2

for.end:                                          ; preds = %for.cond
  %27 = load i64, i64* %sum, align 8
  ret i64 %27
}

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git f05b649610564b11c481a20598dbb3f532c4602a)"}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.mustprogress"}
