; ModuleID = 'vec3.c'
source_filename = "vec3.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @matmul(i32** %A, i32** %B, i32** %C, i32 %n) #0 {
entry:
; CHECK: vload
  %A.addr = alloca i32**, align 8
  %B.addr = alloca i32**, align 8
  %C.addr = alloca i32**, align 8
  %n.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  %k18 = alloca i32, align 4
  store i32** %A, i32*** %A.addr, align 8
  store i32** %B, i32*** %B.addr, align 8
  store i32** %C, i32*** %C.addr, align 8
  store i32 %n, i32* %n.addr, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc42, %entry
  %0 = load i32, i32* %i, align 4
  %1 = load i32, i32* %n.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end44

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %j, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc39, %for.body
  %2 = load i32, i32* %j, align 4
  %3 = load i32, i32* %n.addr, align 4
  %cmp2 = icmp slt i32 %2, %3
  br i1 %cmp2, label %for.body3, label %for.end41

for.body3:                                        ; preds = %for.cond1
  store i32 0, i32* %k, align 4
  br label %for.cond4

for.cond4:                                        ; preds = %for.inc, %for.body3
  %4 = load i32, i32* %k, align 4
  %5 = load i32, i32* %n.addr, align 4
  %div = sdiv i32 %5, 8
  %mul = mul nsw i32 %div, 8
  %cmp5 = icmp slt i32 %4, %mul
  br i1 %cmp5, label %for.body6, label %for.end

for.body6:                                        ; preds = %for.cond4
  %6 = load i32**, i32*** %A.addr, align 8
  %7 = load i32, i32* %i, align 4
  %idxprom = sext i32 %7 to i64
  %arrayidx = getelementptr inbounds i32*, i32** %6, i64 %idxprom
  %8 = load i32*, i32** %arrayidx, align 8
  %9 = load i32, i32* %k, align 4
  %idxprom7 = sext i32 %9 to i64
  %arrayidx8 = getelementptr inbounds i32, i32* %8, i64 %idxprom7
  %10 = load i32, i32* %arrayidx8, align 4
  %11 = load i32**, i32*** %B.addr, align 8
  %12 = load i32, i32* %k, align 4
  %idxprom9 = sext i32 %12 to i64
  %arrayidx10 = getelementptr inbounds i32*, i32** %11, i64 %idxprom9
  %13 = load i32*, i32** %arrayidx10, align 8
  %14 = load i32, i32* %j, align 4
  %idxprom11 = sext i32 %14 to i64
  %arrayidx12 = getelementptr inbounds i32, i32* %13, i64 %idxprom11
  %15 = load i32, i32* %arrayidx12, align 4
  %mul13 = mul nsw i32 %10, %15
  %16 = load i32**, i32*** %C.addr, align 8
  %17 = load i32, i32* %i, align 4
  %idxprom14 = sext i32 %17 to i64
  %arrayidx15 = getelementptr inbounds i32*, i32** %16, i64 %idxprom14
  %18 = load i32*, i32** %arrayidx15, align 8
  %19 = load i32, i32* %j, align 4
  %idxprom16 = sext i32 %19 to i64
  %arrayidx17 = getelementptr inbounds i32, i32* %18, i64 %idxprom16
  store i32 %mul13, i32* %arrayidx17, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body6
  %20 = load i32, i32* %k, align 4
  %add = add nsw i32 %20, 8
  store i32 %add, i32* %k, align 4
  br label %for.cond4, !llvm.loop !2

for.end:                                          ; preds = %for.cond4
  %21 = load i32, i32* %n.addr, align 4
  %div19 = sdiv i32 %21, 8
  %mul20 = mul nsw i32 %div19, 8
  store i32 %mul20, i32* %k18, align 4
  br label %for.cond21

for.cond21:                                       ; preds = %for.inc37, %for.end
  %22 = load i32, i32* %k18, align 4
  %23 = load i32, i32* %n.addr, align 4
  %cmp22 = icmp slt i32 %22, %23
  br i1 %cmp22, label %for.body23, label %for.end38

for.body23:                                       ; preds = %for.cond21
  %24 = load i32**, i32*** %A.addr, align 8
  %25 = load i32, i32* %i, align 4
  %idxprom24 = sext i32 %25 to i64
  %arrayidx25 = getelementptr inbounds i32*, i32** %24, i64 %idxprom24
  %26 = load i32*, i32** %arrayidx25, align 8
  %27 = load i32, i32* %k18, align 4
  %idxprom26 = sext i32 %27 to i64
  %arrayidx27 = getelementptr inbounds i32, i32* %26, i64 %idxprom26
  %28 = load i32, i32* %arrayidx27, align 4
  %29 = load i32**, i32*** %B.addr, align 8
  %30 = load i32, i32* %k18, align 4
  %idxprom28 = sext i32 %30 to i64
  %arrayidx29 = getelementptr inbounds i32*, i32** %29, i64 %idxprom28
  %31 = load i32*, i32** %arrayidx29, align 8
  %32 = load i32, i32* %j, align 4
  %idxprom30 = sext i32 %32 to i64
  %arrayidx31 = getelementptr inbounds i32, i32* %31, i64 %idxprom30
  %33 = load i32, i32* %arrayidx31, align 4
  %mul32 = mul nsw i32 %28, %33
  %34 = load i32**, i32*** %C.addr, align 8
  %35 = load i32, i32* %i, align 4
  %idxprom33 = sext i32 %35 to i64
  %arrayidx34 = getelementptr inbounds i32*, i32** %34, i64 %idxprom33
  %36 = load i32*, i32** %arrayidx34, align 8
  %37 = load i32, i32* %j, align 4
  %idxprom35 = sext i32 %37 to i64
  %arrayidx36 = getelementptr inbounds i32, i32* %36, i64 %idxprom35
  store i32 %mul32, i32* %arrayidx36, align 4
  br label %for.inc37

for.inc37:                                        ; preds = %for.body23
  %38 = load i32, i32* %k18, align 4
  %inc = add nsw i32 %38, 1
  store i32 %inc, i32* %k18, align 4
  br label %for.cond21, !llvm.loop !4

for.end38:                                        ; preds = %for.cond21
  br label %for.inc39

for.inc39:                                        ; preds = %for.end38
  %39 = load i32, i32* %j, align 4
  %inc40 = add nsw i32 %39, 1
  store i32 %inc40, i32* %j, align 4
  br label %for.cond1, !llvm.loop !5

for.end41:                                        ; preds = %for.cond1
  br label %for.inc42

for.inc42:                                        ; preds = %for.end41
  %40 = load i32, i32* %i, align 4
  %inc43 = add nsw i32 %40, 1
  store i32 %inc43, i32* %i, align 4
  br label %for.cond, !llvm.loop !6

for.end44:                                        ; preds = %for.cond
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
!6 = distinct !{!6, !3}
