; ModuleID = 'vec6.c'
source_filename = "vec6.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @reuse(i64* %A, i64 %n) #0 {
entry:
; CHECK-DAG: vstore 2
; CHECK-DAG: vload
; CHECK-DAG: store
; CHECK-DAG: store
; CHECK-DAG: load
; CHECK-DAG: load
  %A.addr = alloca i64*, align 8
  %n.addr = alloca i64, align 8
  %i = alloca i32, align 4
  store i64* %A, i64** %A.addr, align 8
  store i64 %n, i64* %n.addr, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %conv = sext i32 %0 to i64
  %1 = load i64, i64* %n.addr, align 8
  %mul = mul nsw i64 4, %1
  %cmp = icmp slt i64 %conv, %mul
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i64*, i64** %A.addr, align 8
  %3 = load i32, i32* %i, align 4
  %add = add nsw i32 %3, 1
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds i64, i64* %2, i64 %idxprom
  %4 = load i64, i64* %arrayidx, align 8
  %5 = load i64*, i64** %A.addr, align 8
  %6 = load i32, i32* %i, align 4
  %idxprom2 = sext i32 %6 to i64
  %arrayidx3 = getelementptr inbounds i64, i64* %5, i64 %idxprom2
  store i64 %4, i64* %arrayidx3, align 8
  %7 = load i64*, i64** %A.addr, align 8
  %8 = load i32, i32* %i, align 4
  %idxprom4 = sext i32 %8 to i64
  %arrayidx5 = getelementptr inbounds i64, i64* %7, i64 %idxprom4
  %9 = load i64, i64* %arrayidx5, align 8
  %10 = load i64*, i64** %A.addr, align 8
  %11 = load i32, i32* %i, align 4
  %add6 = add nsw i32 %11, 1
  %idxprom7 = sext i32 %add6 to i64
  %arrayidx8 = getelementptr inbounds i64, i64* %10, i64 %idxprom7
  store i64 %9, i64* %arrayidx8, align 8
  %12 = load i64*, i64** %A.addr, align 8
  %13 = load i32, i32* %i, align 4
  %add9 = add nsw i32 %13, 3
  %idxprom10 = sext i32 %add9 to i64
  %arrayidx11 = getelementptr inbounds i64, i64* %12, i64 %idxprom10
  %14 = load i64, i64* %arrayidx11, align 8
  %15 = load i64*, i64** %A.addr, align 8
  %16 = load i32, i32* %i, align 4
  %add12 = add nsw i32 %16, 2
  %idxprom13 = sext i32 %add12 to i64
  %arrayidx14 = getelementptr inbounds i64, i64* %15, i64 %idxprom13
  store i64 %14, i64* %arrayidx14, align 8
  %17 = load i64*, i64** %A.addr, align 8
  %18 = load i32, i32* %i, align 4
  %add15 = add nsw i32 %18, 1
  %idxprom16 = sext i32 %add15 to i64
  %arrayidx17 = getelementptr inbounds i64, i64* %17, i64 %idxprom16
  %19 = load i64, i64* %arrayidx17, align 8
  %20 = load i64*, i64** %A.addr, align 8
  %21 = load i32, i32* %i, align 4
  %add18 = add nsw i32 %21, 3
  %idxprom19 = sext i32 %add18 to i64
  %arrayidx20 = getelementptr inbounds i64, i64* %20, i64 %idxprom19
  store i64 %19, i64* %arrayidx20, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %22 = load i32, i32* %i, align 4
  %add21 = add nsw i32 %22, 4
  store i32 %add21, i32* %i, align 4
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
