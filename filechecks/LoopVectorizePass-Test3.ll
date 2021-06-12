; ModuleID = 'margin.c'
source_filename = "margin.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @MarginError(i64* %A, i64* %B, i32 %n) #0 {
; CHECK: vload
; CHECK: vstore
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
  %2 = load i64*, i64** %A.addr, align 8
  %3 = load i32, i32* %i, align 4
  %idxprom = sext i32 %3 to i64
  %arrayidx = getelementptr inbounds i64, i64* %2, i64 %idxprom
  %4 = load i64, i64* %arrayidx, align 8
  %5 = load i64*, i64** %B.addr, align 8
  %6 = load i32, i32* %i, align 4
  %idxprom1 = sext i32 %6 to i64
  %arrayidx2 = getelementptr inbounds i64, i64* %5, i64 %idxprom1
  store i64 %4, i64* %arrayidx2, align 8
  %7 = load i64*, i64** %A.addr, align 8
  %8 = load i32, i32* %i, align 4
  %add = add nsw i32 %8, 1
  %idxprom3 = sext i32 %add to i64
  %arrayidx4 = getelementptr inbounds i64, i64* %7, i64 %idxprom3
  %9 = load i64, i64* %arrayidx4, align 8
  %10 = load i64*, i64** %B.addr, align 8
  %11 = load i32, i32* %i, align 4
  %add5 = add nsw i32 %11, 1
  %idxprom6 = sext i32 %add5 to i64
  %arrayidx7 = getelementptr inbounds i64, i64* %10, i64 %idxprom6
  store i64 %9, i64* %arrayidx7, align 8
  %12 = load i64*, i64** %A.addr, align 8
  %13 = load i32, i32* %i, align 4
  %add8 = add nsw i32 %13, 2
  %idxprom9 = sext i32 %add8 to i64
  %arrayidx10 = getelementptr inbounds i64, i64* %12, i64 %idxprom9
  %14 = load i64, i64* %arrayidx10, align 8
  %15 = load i64*, i64** %B.addr, align 8
  %16 = load i32, i32* %i, align 4
  %add11 = add nsw i32 %16, 2
  %idxprom12 = sext i32 %add11 to i64
  %arrayidx13 = getelementptr inbounds i64, i64* %15, i64 %idxprom12
  store i64 %14, i64* %arrayidx13, align 8
  %17 = load i64*, i64** %A.addr, align 8
  %18 = load i32, i32* %i, align 4
  %add14 = add nsw i32 %18, 3
  %idxprom15 = sext i32 %add14 to i64
  %arrayidx16 = getelementptr inbounds i64, i64* %17, i64 %idxprom15
  %19 = load i64, i64* %arrayidx16, align 8
  %20 = load i64*, i64** %B.addr, align 8
  %21 = load i32, i32* %i, align 4
  %add17 = add nsw i32 %21, 3
  %idxprom18 = sext i32 %add17 to i64
  %arrayidx19 = getelementptr inbounds i64, i64* %20, i64 %idxprom18
  store i64 %19, i64* %arrayidx19, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %22 = load i32, i32* %i, align 4
  %add20 = add nsw i32 %22, 3
  store i32 %add20, i32* %i, align 4
  br label %for.cond, !llvm.loop !2

for.end:                                          ; preds = %for.cond
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @MarginOK(i64* %A, i64* %B, i32 %n) #0 {
; CHECK: vload 4
; CHECK: vstore 4
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
  %2 = load i64*, i64** %A.addr, align 8
  %3 = load i32, i32* %i, align 4
  %idxprom = sext i32 %3 to i64
  %arrayidx = getelementptr inbounds i64, i64* %2, i64 %idxprom
  %4 = load i64, i64* %arrayidx, align 8
  %5 = load i64*, i64** %B.addr, align 8
  %6 = load i32, i32* %i, align 4
  %idxprom1 = sext i32 %6 to i64
  %arrayidx2 = getelementptr inbounds i64, i64* %5, i64 %idxprom1
  store i64 %4, i64* %arrayidx2, align 8
  %7 = load i64*, i64** %A.addr, align 8
  %8 = load i32, i32* %i, align 4
  %add = add nsw i32 %8, 1
  %idxprom3 = sext i32 %add to i64
  %arrayidx4 = getelementptr inbounds i64, i64* %7, i64 %idxprom3
  %9 = load i64, i64* %arrayidx4, align 8
  %10 = load i64*, i64** %B.addr, align 8
  %11 = load i32, i32* %i, align 4
  %add5 = add nsw i32 %11, 1
  %idxprom6 = sext i32 %add5 to i64
  %arrayidx7 = getelementptr inbounds i64, i64* %10, i64 %idxprom6
  store i64 %9, i64* %arrayidx7, align 8
  %12 = load i64*, i64** %A.addr, align 8
  %13 = load i32, i32* %i, align 4
  %add8 = add nsw i32 %13, 2
  %idxprom9 = sext i32 %add8 to i64
  %arrayidx10 = getelementptr inbounds i64, i64* %12, i64 %idxprom9
  %14 = load i64, i64* %arrayidx10, align 8
  %15 = load i64*, i64** %B.addr, align 8
  %16 = load i32, i32* %i, align 4
  %add11 = add nsw i32 %16, 2
  %idxprom12 = sext i32 %add11 to i64
  %arrayidx13 = getelementptr inbounds i64, i64* %15, i64 %idxprom12
  store i64 %14, i64* %arrayidx13, align 8
  %17 = load i64*, i64** %A.addr, align 8
  %18 = load i32, i32* %i, align 4
  %add14 = add nsw i32 %18, 3
  %idxprom15 = sext i32 %add14 to i64
  %arrayidx16 = getelementptr inbounds i64, i64* %17, i64 %idxprom15
  %19 = load i64, i64* %arrayidx16, align 8
  %20 = load i64*, i64** %B.addr, align 8
  %21 = load i32, i32* %i, align 4
  %add17 = add nsw i32 %21, 3
  %idxprom18 = sext i32 %add17 to i64
  %arrayidx19 = getelementptr inbounds i64, i64* %20, i64 %idxprom18
  store i64 %19, i64* %arrayidx19, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %22 = load i32, i32* %i, align 4
  %add20 = add nsw i32 %22, 4
  store i32 %add20, i32* %i, align 4
  br label %for.cond, !llvm.loop !4

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
!4 = distinct !{!4, !3}
