; ModuleID = 'unroll2.c'
source_filename = "unroll2.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @matadd(i64* %A, i64* %B, i64* %C, i32 %n) #0 {
; CHECK-COUNT-8: store
; CHECK-DAG: epil
; CHECK: store
entry:
  %A.addr = alloca i64*, align 8
  %B.addr = alloca i64*, align 8
  %C.addr = alloca i64*, align 8
  %n.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i64* %A, i64** %A.addr, align 8
  store i64* %B, i64** %B.addr, align 8
  store i64* %C, i64** %C.addr, align 8
  store i32 %n, i32* %n.addr, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc13, %entry
  %0 = load i32, i32* %i, align 4
  %1 = load i32, i32* %n.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end15

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %j, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc, %for.body
  %2 = load i32, i32* %j, align 4
  %3 = load i32, i32* %n.addr, align 4
  %cmp2 = icmp slt i32 %2, %3
  br i1 %cmp2, label %for.body3, label %for.end

for.body3:                                        ; preds = %for.cond1
  %4 = load i64*, i64** %A.addr, align 8
  %5 = load i32, i32* %n.addr, align 4
  %6 = load i32, i32* %i, align 4
  %mul = mul nsw i32 %5, %6
  %7 = load i32, i32* %j, align 4
  %add = add nsw i32 %mul, %7
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds i64, i64* %4, i64 %idxprom
  %8 = load i64, i64* %arrayidx, align 8
  %9 = load i64*, i64** %B.addr, align 8
  %10 = load i32, i32* %n.addr, align 4
  %11 = load i32, i32* %i, align 4
  %mul4 = mul nsw i32 %10, %11
  %12 = load i32, i32* %j, align 4
  %add5 = add nsw i32 %mul4, %12
  %idxprom6 = sext i32 %add5 to i64
  %arrayidx7 = getelementptr inbounds i64, i64* %9, i64 %idxprom6
  %13 = load i64, i64* %arrayidx7, align 8
  %add8 = add nsw i64 %8, %13
  %14 = load i64*, i64** %C.addr, align 8
  %15 = load i32, i32* %n.addr, align 4
  %16 = load i32, i32* %i, align 4
  %mul9 = mul nsw i32 %15, %16
  %17 = load i32, i32* %j, align 4
  %add10 = add nsw i32 %mul9, %17
  %idxprom11 = sext i32 %add10 to i64
  %arrayidx12 = getelementptr inbounds i64, i64* %14, i64 %idxprom11
  store i64 %add8, i64* %arrayidx12, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %18 = load i32, i32* %j, align 4
  %inc = add nsw i32 %18, 1
  store i32 %inc, i32* %j, align 4
  br label %for.cond1, !llvm.loop !2

for.end:                                          ; preds = %for.cond1
  br label %for.inc13

for.inc13:                                        ; preds = %for.end
  %19 = load i32, i32* %i, align 4
  %inc14 = add nsw i32 %19, 1
  store i32 %inc14, i32* %i, align 4
  br label %for.cond, !llvm.loop !4

for.end15:                                        ; preds = %for.cond
  ret void
}

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git f05b649610564b11c481a20598dbb3f532c4602a)"}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.mustprogress"}
!4 = distinct !{!4, !3}
