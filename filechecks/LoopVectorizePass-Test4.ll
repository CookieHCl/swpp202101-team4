; ModuleID = 'vec7.c'
source_filename = "vec7.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @add(i64* %A, i64* %B, i32 %n) #0 {
; CHECK: start add 3
; CHECK: vload 8
; CHECK-DAG: vstore 8
; CHECK-DAG: vload 8
; CHECK-DAG: vstore 8
; CHECK: end add
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
  %mul = mul nsw i32 16, %1
  %cmp = icmp slt i32 %0, %mul
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i64*, i64** %B.addr, align 8
  %3 = load i32, i32* %i, align 4
  %add = add nsw i32 %3, 0
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds i64, i64* %2, i64 %idxprom
  %4 = load i64, i64* %arrayidx, align 8
  %5 = load i64*, i64** %A.addr, align 8
  %6 = load i32, i32* %i, align 4
  %add1 = add nsw i32 %6, 0
  %idxprom2 = sext i32 %add1 to i64
  %arrayidx3 = getelementptr inbounds i64, i64* %5, i64 %idxprom2
  store i64 %4, i64* %arrayidx3, align 8
  %7 = load i64*, i64** %B.addr, align 8
  %8 = load i32, i32* %i, align 4
  %add4 = add nsw i32 %8, 1
  %idxprom5 = sext i32 %add4 to i64
  %arrayidx6 = getelementptr inbounds i64, i64* %7, i64 %idxprom5
  %9 = load i64, i64* %arrayidx6, align 8
  %10 = load i64*, i64** %A.addr, align 8
  %11 = load i32, i32* %i, align 4
  %add7 = add nsw i32 %11, 1
  %idxprom8 = sext i32 %add7 to i64
  %arrayidx9 = getelementptr inbounds i64, i64* %10, i64 %idxprom8
  store i64 %9, i64* %arrayidx9, align 8
  %12 = load i64*, i64** %B.addr, align 8
  %13 = load i32, i32* %i, align 4
  %add10 = add nsw i32 %13, 2
  %idxprom11 = sext i32 %add10 to i64
  %arrayidx12 = getelementptr inbounds i64, i64* %12, i64 %idxprom11
  %14 = load i64, i64* %arrayidx12, align 8
  %15 = load i64*, i64** %A.addr, align 8
  %16 = load i32, i32* %i, align 4
  %add13 = add nsw i32 %16, 2
  %idxprom14 = sext i32 %add13 to i64
  %arrayidx15 = getelementptr inbounds i64, i64* %15, i64 %idxprom14
  store i64 %14, i64* %arrayidx15, align 8
  %17 = load i64*, i64** %B.addr, align 8
  %18 = load i32, i32* %i, align 4
  %add16 = add nsw i32 %18, 3
  %idxprom17 = sext i32 %add16 to i64
  %arrayidx18 = getelementptr inbounds i64, i64* %17, i64 %idxprom17
  %19 = load i64, i64* %arrayidx18, align 8
  %20 = load i64*, i64** %A.addr, align 8
  %21 = load i32, i32* %i, align 4
  %add19 = add nsw i32 %21, 3
  %idxprom20 = sext i32 %add19 to i64
  %arrayidx21 = getelementptr inbounds i64, i64* %20, i64 %idxprom20
  store i64 %19, i64* %arrayidx21, align 8
  %22 = load i64*, i64** %B.addr, align 8
  %23 = load i32, i32* %i, align 4
  %add22 = add nsw i32 %23, 4
  %idxprom23 = sext i32 %add22 to i64
  %arrayidx24 = getelementptr inbounds i64, i64* %22, i64 %idxprom23
  %24 = load i64, i64* %arrayidx24, align 8
  %25 = load i64*, i64** %A.addr, align 8
  %26 = load i32, i32* %i, align 4
  %add25 = add nsw i32 %26, 4
  %idxprom26 = sext i32 %add25 to i64
  %arrayidx27 = getelementptr inbounds i64, i64* %25, i64 %idxprom26
  store i64 %24, i64* %arrayidx27, align 8
  %27 = load i64*, i64** %B.addr, align 8
  %28 = load i32, i32* %i, align 4
  %add28 = add nsw i32 %28, 5
  %idxprom29 = sext i32 %add28 to i64
  %arrayidx30 = getelementptr inbounds i64, i64* %27, i64 %idxprom29
  %29 = load i64, i64* %arrayidx30, align 8
  %30 = load i64*, i64** %A.addr, align 8
  %31 = load i32, i32* %i, align 4
  %add31 = add nsw i32 %31, 5
  %idxprom32 = sext i32 %add31 to i64
  %arrayidx33 = getelementptr inbounds i64, i64* %30, i64 %idxprom32
  store i64 %29, i64* %arrayidx33, align 8
  %32 = load i64*, i64** %B.addr, align 8
  %33 = load i32, i32* %i, align 4
  %add34 = add nsw i32 %33, 6
  %idxprom35 = sext i32 %add34 to i64
  %arrayidx36 = getelementptr inbounds i64, i64* %32, i64 %idxprom35
  %34 = load i64, i64* %arrayidx36, align 8
  %35 = load i64*, i64** %A.addr, align 8
  %36 = load i32, i32* %i, align 4
  %add37 = add nsw i32 %36, 6
  %idxprom38 = sext i32 %add37 to i64
  %arrayidx39 = getelementptr inbounds i64, i64* %35, i64 %idxprom38
  store i64 %34, i64* %arrayidx39, align 8
  %37 = load i64*, i64** %B.addr, align 8
  %38 = load i32, i32* %i, align 4
  %add40 = add nsw i32 %38, 7
  %idxprom41 = sext i32 %add40 to i64
  %arrayidx42 = getelementptr inbounds i64, i64* %37, i64 %idxprom41
  %39 = load i64, i64* %arrayidx42, align 8
  %40 = load i64*, i64** %A.addr, align 8
  %41 = load i32, i32* %i, align 4
  %add43 = add nsw i32 %41, 7
  %idxprom44 = sext i32 %add43 to i64
  %arrayidx45 = getelementptr inbounds i64, i64* %40, i64 %idxprom44
  store i64 %39, i64* %arrayidx45, align 8
  %42 = load i64*, i64** %B.addr, align 8
  %43 = load i32, i32* %i, align 4
  %add46 = add nsw i32 %43, 8
  %idxprom47 = sext i32 %add46 to i64
  %arrayidx48 = getelementptr inbounds i64, i64* %42, i64 %idxprom47
  %44 = load i64, i64* %arrayidx48, align 8
  %45 = load i64*, i64** %A.addr, align 8
  %46 = load i32, i32* %i, align 4
  %add49 = add nsw i32 %46, 8
  %idxprom50 = sext i32 %add49 to i64
  %arrayidx51 = getelementptr inbounds i64, i64* %45, i64 %idxprom50
  store i64 %44, i64* %arrayidx51, align 8
  %47 = load i64*, i64** %B.addr, align 8
  %48 = load i32, i32* %i, align 4
  %add52 = add nsw i32 %48, 9
  %idxprom53 = sext i32 %add52 to i64
  %arrayidx54 = getelementptr inbounds i64, i64* %47, i64 %idxprom53
  %49 = load i64, i64* %arrayidx54, align 8
  %50 = load i64*, i64** %A.addr, align 8
  %51 = load i32, i32* %i, align 4
  %add55 = add nsw i32 %51, 9
  %idxprom56 = sext i32 %add55 to i64
  %arrayidx57 = getelementptr inbounds i64, i64* %50, i64 %idxprom56
  store i64 %49, i64* %arrayidx57, align 8
  %52 = load i64*, i64** %B.addr, align 8
  %53 = load i32, i32* %i, align 4
  %add58 = add nsw i32 %53, 10
  %idxprom59 = sext i32 %add58 to i64
  %arrayidx60 = getelementptr inbounds i64, i64* %52, i64 %idxprom59
  %54 = load i64, i64* %arrayidx60, align 8
  %55 = load i64*, i64** %A.addr, align 8
  %56 = load i32, i32* %i, align 4
  %add61 = add nsw i32 %56, 10
  %idxprom62 = sext i32 %add61 to i64
  %arrayidx63 = getelementptr inbounds i64, i64* %55, i64 %idxprom62
  store i64 %54, i64* %arrayidx63, align 8
  %57 = load i64*, i64** %B.addr, align 8
  %58 = load i32, i32* %i, align 4
  %add64 = add nsw i32 %58, 11
  %idxprom65 = sext i32 %add64 to i64
  %arrayidx66 = getelementptr inbounds i64, i64* %57, i64 %idxprom65
  %59 = load i64, i64* %arrayidx66, align 8
  %60 = load i64*, i64** %A.addr, align 8
  %61 = load i32, i32* %i, align 4
  %add67 = add nsw i32 %61, 11
  %idxprom68 = sext i32 %add67 to i64
  %arrayidx69 = getelementptr inbounds i64, i64* %60, i64 %idxprom68
  store i64 %59, i64* %arrayidx69, align 8
  %62 = load i64*, i64** %B.addr, align 8
  %63 = load i32, i32* %i, align 4
  %add70 = add nsw i32 %63, 12
  %idxprom71 = sext i32 %add70 to i64
  %arrayidx72 = getelementptr inbounds i64, i64* %62, i64 %idxprom71
  %64 = load i64, i64* %arrayidx72, align 8
  %65 = load i64*, i64** %A.addr, align 8
  %66 = load i32, i32* %i, align 4
  %add73 = add nsw i32 %66, 12
  %idxprom74 = sext i32 %add73 to i64
  %arrayidx75 = getelementptr inbounds i64, i64* %65, i64 %idxprom74
  store i64 %64, i64* %arrayidx75, align 8
  %67 = load i64*, i64** %B.addr, align 8
  %68 = load i32, i32* %i, align 4
  %add76 = add nsw i32 %68, 13
  %idxprom77 = sext i32 %add76 to i64
  %arrayidx78 = getelementptr inbounds i64, i64* %67, i64 %idxprom77
  %69 = load i64, i64* %arrayidx78, align 8
  %70 = load i64*, i64** %A.addr, align 8
  %71 = load i32, i32* %i, align 4
  %add79 = add nsw i32 %71, 13
  %idxprom80 = sext i32 %add79 to i64
  %arrayidx81 = getelementptr inbounds i64, i64* %70, i64 %idxprom80
  store i64 %69, i64* %arrayidx81, align 8
  %72 = load i64*, i64** %B.addr, align 8
  %73 = load i32, i32* %i, align 4
  %add82 = add nsw i32 %73, 14
  %idxprom83 = sext i32 %add82 to i64
  %arrayidx84 = getelementptr inbounds i64, i64* %72, i64 %idxprom83
  %74 = load i64, i64* %arrayidx84, align 8
  %75 = load i64*, i64** %A.addr, align 8
  %76 = load i32, i32* %i, align 4
  %add85 = add nsw i32 %76, 14
  %idxprom86 = sext i32 %add85 to i64
  %arrayidx87 = getelementptr inbounds i64, i64* %75, i64 %idxprom86
  store i64 %74, i64* %arrayidx87, align 8
  %77 = load i64*, i64** %B.addr, align 8
  %78 = load i32, i32* %i, align 4
  %add88 = add nsw i32 %78, 15
  %idxprom89 = sext i32 %add88 to i64
  %arrayidx90 = getelementptr inbounds i64, i64* %77, i64 %idxprom89
  %79 = load i64, i64* %arrayidx90, align 8
  %80 = load i64*, i64** %A.addr, align 8
  %81 = load i32, i32* %i, align 4
  %add91 = add nsw i32 %81, 15
  %idxprom92 = sext i32 %add91 to i64
  %arrayidx93 = getelementptr inbounds i64, i64* %80, i64 %idxprom92
  store i64 %79, i64* %arrayidx93, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %82 = load i32, i32* %i, align 4
  %add94 = add nsw i32 %82, 16
  store i32 %add94, i32* %i, align 4
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
