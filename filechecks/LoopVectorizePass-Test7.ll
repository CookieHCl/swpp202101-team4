; ModuleID = 'vec9.c'
source_filename = "vec9.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @add(i64* %A, i64* %B, i32 %n, i32 %m) #0 {
entry:
; CHECK: vload 4
; CHECK: vload 4
; CHECK: vstore 4
  %A.addr = alloca i64*, align 8
  %B.addr = alloca i64*, align 8
  %n.addr = alloca i32, align 4
  %m.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %j43 = alloca i32, align 4
  store i64* %A, i64** %A.addr, align 8
  store i64* %B, i64** %B.addr, align 8
  store i32 %n, i32* %n.addr, align 4
  store i32 %m, i32* %m.addr, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc59, %entry
  %0 = load i32, i32* %i, align 4
  %1 = load i32, i32* %n.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end61

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %j, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc, %for.body
  %2 = load i32, i32* %j, align 4
  %3 = load i32, i32* %m.addr, align 4
  %cmp2 = icmp slt i32 %2, %3
  br i1 %cmp2, label %for.body3, label %for.end

for.body3:                                        ; preds = %for.cond1
  %4 = load i64*, i64** %B.addr, align 8
  %5 = load i32, i32* %m.addr, align 4
  %6 = load i32, i32* %i, align 4
  %mul = mul nsw i32 %5, %6
  %7 = load i32, i32* %j, align 4
  %add = add nsw i32 %mul, %7
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds i64, i64* %4, i64 %idxprom
  %8 = load i64, i64* %arrayidx, align 8
  %9 = load i64*, i64** %A.addr, align 8
  %10 = load i32, i32* %m.addr, align 4
  %11 = load i32, i32* %i, align 4
  %mul4 = mul nsw i32 %10, %11
  %12 = load i32, i32* %j, align 4
  %add5 = add nsw i32 %mul4, %12
  %idxprom6 = sext i32 %add5 to i64
  %arrayidx7 = getelementptr inbounds i64, i64* %9, i64 %idxprom6
  %13 = load i64, i64* %arrayidx7, align 8
  %add8 = add nsw i64 %13, %8
  store i64 %add8, i64* %arrayidx7, align 8
  %14 = load i64*, i64** %B.addr, align 8
  %15 = load i32, i32* %m.addr, align 4
  %16 = load i32, i32* %i, align 4
  %mul9 = mul nsw i32 %15, %16
  %17 = load i32, i32* %j, align 4
  %add10 = add nsw i32 %mul9, %17
  %add11 = add nsw i32 %add10, 1
  %idxprom12 = sext i32 %add11 to i64
  %arrayidx13 = getelementptr inbounds i64, i64* %14, i64 %idxprom12
  %18 = load i64, i64* %arrayidx13, align 8
  %19 = load i64*, i64** %A.addr, align 8
  %20 = load i32, i32* %m.addr, align 4
  %21 = load i32, i32* %i, align 4
  %mul14 = mul nsw i32 %20, %21
  %22 = load i32, i32* %j, align 4
  %add15 = add nsw i32 %mul14, %22
  %add16 = add nsw i32 %add15, 1
  %idxprom17 = sext i32 %add16 to i64
  %arrayidx18 = getelementptr inbounds i64, i64* %19, i64 %idxprom17
  %23 = load i64, i64* %arrayidx18, align 8
  %add19 = add nsw i64 %23, %18
  store i64 %add19, i64* %arrayidx18, align 8
  %24 = load i64*, i64** %B.addr, align 8
  %25 = load i32, i32* %m.addr, align 4
  %26 = load i32, i32* %i, align 4
  %mul20 = mul nsw i32 %25, %26
  %27 = load i32, i32* %j, align 4
  %add21 = add nsw i32 %mul20, %27
  %add22 = add nsw i32 %add21, 2
  %idxprom23 = sext i32 %add22 to i64
  %arrayidx24 = getelementptr inbounds i64, i64* %24, i64 %idxprom23
  %28 = load i64, i64* %arrayidx24, align 8
  %29 = load i64*, i64** %A.addr, align 8
  %30 = load i32, i32* %m.addr, align 4
  %31 = load i32, i32* %i, align 4
  %mul25 = mul nsw i32 %30, %31
  %32 = load i32, i32* %j, align 4
  %add26 = add nsw i32 %mul25, %32
  %add27 = add nsw i32 %add26, 2
  %idxprom28 = sext i32 %add27 to i64
  %arrayidx29 = getelementptr inbounds i64, i64* %29, i64 %idxprom28
  %33 = load i64, i64* %arrayidx29, align 8
  %add30 = add nsw i64 %33, %28
  store i64 %add30, i64* %arrayidx29, align 8
  %34 = load i64*, i64** %B.addr, align 8
  %35 = load i32, i32* %m.addr, align 4
  %36 = load i32, i32* %i, align 4
  %mul31 = mul nsw i32 %35, %36
  %37 = load i32, i32* %j, align 4
  %add32 = add nsw i32 %mul31, %37
  %add33 = add nsw i32 %add32, 3
  %idxprom34 = sext i32 %add33 to i64
  %arrayidx35 = getelementptr inbounds i64, i64* %34, i64 %idxprom34
  %38 = load i64, i64* %arrayidx35, align 8
  %39 = load i64*, i64** %A.addr, align 8
  %40 = load i32, i32* %m.addr, align 4
  %41 = load i32, i32* %i, align 4
  %mul36 = mul nsw i32 %40, %41
  %42 = load i32, i32* %j, align 4
  %add37 = add nsw i32 %mul36, %42
  %add38 = add nsw i32 %add37, 3
  %idxprom39 = sext i32 %add38 to i64
  %arrayidx40 = getelementptr inbounds i64, i64* %39, i64 %idxprom39
  %43 = load i64, i64* %arrayidx40, align 8
  %add41 = add nsw i64 %43, %38
  store i64 %add41, i64* %arrayidx40, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %44 = load i32, i32* %j, align 4
  %add42 = add nsw i32 %44, 4
  store i32 %add42, i32* %j, align 4
  br label %for.cond1, !llvm.loop !2

for.end:                                          ; preds = %for.cond1
  %45 = load i32, i32* %m.addr, align 4
  %div = sdiv i32 %45, 4
  %mul44 = mul nsw i32 %div, 4
  store i32 %mul44, i32* %j43, align 4
  br label %for.cond45

for.cond45:                                       ; preds = %for.inc57, %for.end
  %46 = load i32, i32* %j43, align 4
  %47 = load i32, i32* %m.addr, align 4
  %cmp46 = icmp slt i32 %46, %47
  br i1 %cmp46, label %for.body47, label %for.end58

for.body47:                                       ; preds = %for.cond45
  %48 = load i64*, i64** %B.addr, align 8
  %49 = load i32, i32* %m.addr, align 4
  %50 = load i32, i32* %i, align 4
  %mul48 = mul nsw i32 %49, %50
  %51 = load i32, i32* %j43, align 4
  %add49 = add nsw i32 %mul48, %51
  %idxprom50 = sext i32 %add49 to i64
  %arrayidx51 = getelementptr inbounds i64, i64* %48, i64 %idxprom50
  %52 = load i64, i64* %arrayidx51, align 8
  %53 = load i64*, i64** %A.addr, align 8
  %54 = load i32, i32* %m.addr, align 4
  %55 = load i32, i32* %i, align 4
  %mul52 = mul nsw i32 %54, %55
  %56 = load i32, i32* %j43, align 4
  %add53 = add nsw i32 %mul52, %56
  %idxprom54 = sext i32 %add53 to i64
  %arrayidx55 = getelementptr inbounds i64, i64* %53, i64 %idxprom54
  %57 = load i64, i64* %arrayidx55, align 8
  %add56 = add nsw i64 %57, %52
  store i64 %add56, i64* %arrayidx55, align 8
  br label %for.inc57

for.inc57:                                        ; preds = %for.body47
  %58 = load i32, i32* %j43, align 4
  %inc = add nsw i32 %58, 1
  store i32 %inc, i32* %j43, align 4
  br label %for.cond45, !llvm.loop !4

for.end58:                                        ; preds = %for.cond45
  br label %for.inc59

for.inc59:                                        ; preds = %for.end58
  %59 = load i32, i32* %i, align 4
  %inc60 = add nsw i32 %59, 1
  store i32 %inc60, i32* %i, align 4
  br label %for.cond, !llvm.loop !5

for.end61:                                        ; preds = %for.cond
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
!5 = distinct !{!5, !3}
