; ModuleID = 'add.c'
source_filename = "add.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @add(i64* %A, i64* %B, i32 %n) #0 {
; CHECK: vload 4
; CHECK-DAG: vload 4
; CHECK-DAG: add
; CHECK-DAG: add
; CHECK-DAG: add
; CHECK-DAG: add
; CHECK-DAG: vstore 4
entry:
  %A.addr = alloca i64*, align 8
  %B.addr = alloca i64*, align 8
  %n.addr = alloca i32, align 4
  %i = alloca i32, align 4
  store i64* %A, i64** %A.addr, align 8
  store i64* %B, i64** %B.addr, align 8
  store i32 %n, i32* %n.addr, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %1 = load i32, i32* %n.addr, align 4
  %mul = mul nsw i32 4, %1
  %cmp = icmp slt i32 %0, %mul
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i64*, i64** %B.addr, align 8
  %3 = load i32, i32* %i, align 4
  %idxprom = sext i32 %3 to i64
  %arrayidx = getelementptr inbounds i64, i64* %2, i64 %idxprom
  %4 = load i64, i64* %arrayidx, align 8
  %5 = load i64*, i64** %A.addr, align 8
  %6 = load i32, i32* %i, align 4
  %idxprom1 = sext i32 %6 to i64
  %arrayidx2 = getelementptr inbounds i64, i64* %5, i64 %idxprom1
  %7 = load i64, i64* %arrayidx2, align 8
  %add = add nsw i64 %7, %4
  store i64 %add, i64* %arrayidx2, align 8
  %8 = load i64*, i64** %B.addr, align 8
  %9 = load i32, i32* %i, align 4
  %add3 = add nsw i32 %9, 1
  %idxprom4 = sext i32 %add3 to i64
  %arrayidx5 = getelementptr inbounds i64, i64* %8, i64 %idxprom4
  %10 = load i64, i64* %arrayidx5, align 8
  %11 = load i64*, i64** %A.addr, align 8
  %12 = load i32, i32* %i, align 4
  %add6 = add nsw i32 %12, 1
  %idxprom7 = sext i32 %add6 to i64
  %arrayidx8 = getelementptr inbounds i64, i64* %11, i64 %idxprom7
  %13 = load i64, i64* %arrayidx8, align 8
  %add9 = add nsw i64 %13, %10
  store i64 %add9, i64* %arrayidx8, align 8
  %14 = load i64*, i64** %B.addr, align 8
  %15 = load i32, i32* %i, align 4
  %add10 = add nsw i32 %15, 2
  %idxprom11 = sext i32 %add10 to i64
  %arrayidx12 = getelementptr inbounds i64, i64* %14, i64 %idxprom11
  %16 = load i64, i64* %arrayidx12, align 8
  %17 = load i64*, i64** %A.addr, align 8
  %18 = load i32, i32* %i, align 4
  %add13 = add nsw i32 %18, 2
  %idxprom14 = sext i32 %add13 to i64
  %arrayidx15 = getelementptr inbounds i64, i64* %17, i64 %idxprom14
  %19 = load i64, i64* %arrayidx15, align 8
  %add16 = add nsw i64 %19, %16
  store i64 %add16, i64* %arrayidx15, align 8
  %20 = load i64*, i64** %B.addr, align 8
  %21 = load i32, i32* %i, align 4
  %add17 = add nsw i32 %21, 3
  %idxprom18 = sext i32 %add17 to i64
  %arrayidx19 = getelementptr inbounds i64, i64* %20, i64 %idxprom18
  %22 = load i64, i64* %arrayidx19, align 8
  %23 = load i64*, i64** %A.addr, align 8
  %24 = load i32, i32* %i, align 4
  %add20 = add nsw i32 %24, 3
  %idxprom21 = sext i32 %add20 to i64
  %arrayidx22 = getelementptr inbounds i64, i64* %23, i64 %idxprom21
  %25 = load i64, i64* %arrayidx22, align 8
  %add23 = add nsw i64 %25, %22
  store i64 %add23, i64* %arrayidx22, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %26 = load i32, i32* %i, align 4
  %add24 = add nsw i32 %26, 4
  store i32 %add24, i32* %i, align 4
  br label %for.cond, !llvm.loop !2

for.end:                                          ; preds = %for.cond
  ret void
}

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git f05b649610564b11c481a20598dbb3f532c4602a)"}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.mustprogress"}
