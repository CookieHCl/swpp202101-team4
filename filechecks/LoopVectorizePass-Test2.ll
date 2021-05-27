; ModuleID = 'vec5.c'
source_filename = "vec5.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @cascade(i64* %A, i32 %n) #0 {
; CHECK-NOT: vload
; CHECK-NOT: vstore
entry:
  %A.addr = alloca i64*, align 8
  %n.addr = alloca i32, align 4
  %i = alloca i32, align 4
  store i64* %A, i64** %A.addr, align 8
  store i32 %n, i32* %n.addr, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %0, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, i32* %i, align 4
  %conv = sext i32 %1 to i64
  %2 = load i64*, i64** %A.addr, align 8
  %3 = load i32, i32* %i, align 4
  %add = add nsw i32 %3, 0
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds i64, i64* %2, i64 %idxprom
  store i64 %conv, i64* %arrayidx, align 8
  %4 = load i64*, i64** %A.addr, align 8
  %5 = load i32, i32* %i, align 4
  %idxprom1 = sext i32 %5 to i64
  %arrayidx2 = getelementptr inbounds i64, i64* %4, i64 %idxprom1
  %6 = load i64, i64* %arrayidx2, align 8
  %7 = load i64*, i64** %A.addr, align 8
  %8 = load i32, i32* %i, align 4
  %add3 = add nsw i32 %8, 1
  %idxprom4 = sext i32 %add3 to i64
  %arrayidx5 = getelementptr inbounds i64, i64* %7, i64 %idxprom4
  store i64 %6, i64* %arrayidx5, align 8
  %9 = load i64*, i64** %A.addr, align 8
  %10 = load i32, i32* %i, align 4
  %add6 = add nsw i32 %10, 1
  %idxprom7 = sext i32 %add6 to i64
  %arrayidx8 = getelementptr inbounds i64, i64* %9, i64 %idxprom7
  %11 = load i64, i64* %arrayidx8, align 8
  %12 = load i64*, i64** %A.addr, align 8
  %13 = load i32, i32* %i, align 4
  %add9 = add nsw i32 %13, 2
  %idxprom10 = sext i32 %add9 to i64
  %arrayidx11 = getelementptr inbounds i64, i64* %12, i64 %idxprom10
  store i64 %11, i64* %arrayidx11, align 8
  %14 = load i64*, i64** %A.addr, align 8
  %15 = load i32, i32* %i, align 4
  %add12 = add nsw i32 %15, 2
  %idxprom13 = sext i32 %add12 to i64
  %arrayidx14 = getelementptr inbounds i64, i64* %14, i64 %idxprom13
  %16 = load i64, i64* %arrayidx14, align 8
  %17 = load i64*, i64** %A.addr, align 8
  %18 = load i32, i32* %i, align 4
  %add15 = add nsw i32 %18, 3
  %idxprom16 = sext i32 %add15 to i64
  %arrayidx17 = getelementptr inbounds i64, i64* %17, i64 %idxprom16
  store i64 %16, i64* %arrayidx17, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %19 = load i32, i32* %i, align 4
  %add18 = add nsw i32 %19, 4
  store i32 %add18, i32* %i, align 4
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
