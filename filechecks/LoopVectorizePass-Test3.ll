; ModuleID = 'vec4.c'
source_filename = "vec4.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @matmul(i32** %A, i32** %B, i32** %C) #0 {
entry:
; CHECK : vload
  %A.addr = alloca i32**, align 8
  %B.addr = alloca i32**, align 8
  %C.addr = alloca i32**, align 8
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  store i32** %A, i32*** %A.addr, align 8
  store i32** %B, i32*** %B.addr, align 8
  store i32** %C, i32*** %C.addr, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc20, %entry
  %0 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %0, 8
  br i1 %cmp, label %for.body, label %for.end22

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %j, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc17, %for.body
  %1 = load i32, i32* %j, align 4
  %cmp2 = icmp slt i32 %1, 8
  br i1 %cmp2, label %for.body3, label %for.end19

for.body3:                                        ; preds = %for.cond1
  store i32 0, i32* %k, align 4
  br label %for.cond4

for.cond4:                                        ; preds = %for.inc, %for.body3
  %2 = load i32, i32* %k, align 4
  %cmp5 = icmp slt i32 %2, 8
  br i1 %cmp5, label %for.body6, label %for.end

for.body6:                                        ; preds = %for.cond4
  %3 = load i32**, i32*** %A.addr, align 8
  %4 = load i32, i32* %i, align 4
  %idxprom = sext i32 %4 to i64
  %arrayidx = getelementptr inbounds i32*, i32** %3, i64 %idxprom
  %5 = load i32*, i32** %arrayidx, align 8
  %6 = load i32, i32* %k, align 4
  %idxprom7 = sext i32 %6 to i64
  %arrayidx8 = getelementptr inbounds i32, i32* %5, i64 %idxprom7
  %7 = load i32, i32* %arrayidx8, align 4
  %8 = load i32**, i32*** %B.addr, align 8
  %9 = load i32, i32* %k, align 4
  %idxprom9 = sext i32 %9 to i64
  %arrayidx10 = getelementptr inbounds i32*, i32** %8, i64 %idxprom9
  %10 = load i32*, i32** %arrayidx10, align 8
  %11 = load i32, i32* %j, align 4
  %idxprom11 = sext i32 %11 to i64
  %arrayidx12 = getelementptr inbounds i32, i32* %10, i64 %idxprom11
  %12 = load i32, i32* %arrayidx12, align 4
  %mul = mul nsw i32 %7, %12
  %13 = load i32**, i32*** %C.addr, align 8
  %14 = load i32, i32* %i, align 4
  %idxprom13 = sext i32 %14 to i64
  %arrayidx14 = getelementptr inbounds i32*, i32** %13, i64 %idxprom13
  %15 = load i32*, i32** %arrayidx14, align 8
  %16 = load i32, i32* %j, align 4
  %idxprom15 = sext i32 %16 to i64
  %arrayidx16 = getelementptr inbounds i32, i32* %15, i64 %idxprom15
  store i32 %mul, i32* %arrayidx16, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body6
  %17 = load i32, i32* %k, align 4
  %inc = add nsw i32 %17, 1
  store i32 %inc, i32* %k, align 4
  br label %for.cond4, !llvm.loop !2

for.end:                                          ; preds = %for.cond4
  br label %for.inc17

for.inc17:                                        ; preds = %for.end
  %18 = load i32, i32* %j, align 4
  %inc18 = add nsw i32 %18, 1
  store i32 %inc18, i32* %j, align 4
  br label %for.cond1, !llvm.loop !4

for.end19:                                        ; preds = %for.cond1
  br label %for.inc20

for.inc20:                                        ; preds = %for.end19
  %19 = load i32, i32* %i, align 4
  %inc21 = add nsw i32 %19, 1
  store i32 %inc21, i32* %i, align 4
  br label %for.cond, !llvm.loop !5

for.end22:                                        ; preds = %for.cond
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
