; ModuleID = 'unroll3.c'
source_filename = "unroll3.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @matadd(i64* %A, i64* %B, i64* %C, i32 %n) #0 {
; CHECK: start matadd 4
; CHECK-COUNT-9: store
; CHECK-DAG: epil
; CHECK: store
; CHECK: end matadd
entry:
  %A.addr = alloca i64*, align 8
  %B.addr = alloca i64*, align 8
  %C.addr = alloca i64*, align 8
  %n.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %j30 = alloca i32, align 4
  store i64* %A, i64** %A.addr, align 8
  store i64* %B, i64** %B.addr, align 8
  store i64* %C, i64** %C.addr, align 8
  store i32 %n, i32* %n.addr, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc50, %entry
  %0 = load i32, i32* %i, align 4
  %1 = load i32, i32* %n.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end52

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
  %18 = load i64*, i64** %A.addr, align 8
  %19 = load i32, i32* %n.addr, align 4
  %20 = load i32, i32* %i, align 4
  %mul13 = mul nsw i32 %19, %20
  %21 = load i32, i32* %j, align 4
  %add14 = add nsw i32 %mul13, %21
  %add15 = add nsw i32 %add14, 1
  %idxprom16 = sext i32 %add15 to i64
  %arrayidx17 = getelementptr inbounds i64, i64* %18, i64 %idxprom16
  %22 = load i64, i64* %arrayidx17, align 8
  %23 = load i64*, i64** %B.addr, align 8
  %24 = load i32, i32* %n.addr, align 4
  %25 = load i32, i32* %i, align 4
  %mul18 = mul nsw i32 %24, %25
  %26 = load i32, i32* %j, align 4
  %add19 = add nsw i32 %mul18, %26
  %add20 = add nsw i32 %add19, 1
  %idxprom21 = sext i32 %add20 to i64
  %arrayidx22 = getelementptr inbounds i64, i64* %23, i64 %idxprom21
  %27 = load i64, i64* %arrayidx22, align 8
  %add23 = add nsw i64 %22, %27
  %28 = load i64*, i64** %C.addr, align 8
  %29 = load i32, i32* %n.addr, align 4
  %30 = load i32, i32* %i, align 4
  %mul24 = mul nsw i32 %29, %30
  %31 = load i32, i32* %j, align 4
  %add25 = add nsw i32 %mul24, %31
  %add26 = add nsw i32 %add25, 1
  %idxprom27 = sext i32 %add26 to i64
  %arrayidx28 = getelementptr inbounds i64, i64* %28, i64 %idxprom27
  store i64 %add23, i64* %arrayidx28, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %32 = load i32, i32* %j, align 4
  %add29 = add nsw i32 %32, 2
  store i32 %add29, i32* %j, align 4
  br label %for.cond1, !llvm.loop !2

for.end:                                          ; preds = %for.cond1
  %33 = load i32, i32* %n.addr, align 4
  %div = sdiv i32 %33, 2
  %mul31 = mul nsw i32 %div, 2
  store i32 %mul31, i32* %j30, align 4
  br label %for.cond32

for.cond32:                                       ; preds = %for.inc48, %for.end
  %34 = load i32, i32* %j30, align 4
  %35 = load i32, i32* %n.addr, align 4
  %cmp33 = icmp slt i32 %34, %35
  br i1 %cmp33, label %for.body34, label %for.end49

for.body34:                                       ; preds = %for.cond32
  %36 = load i64*, i64** %A.addr, align 8
  %37 = load i32, i32* %n.addr, align 4
  %38 = load i32, i32* %i, align 4
  %mul35 = mul nsw i32 %37, %38
  %39 = load i32, i32* %j30, align 4
  %add36 = add nsw i32 %mul35, %39
  %idxprom37 = sext i32 %add36 to i64
  %arrayidx38 = getelementptr inbounds i64, i64* %36, i64 %idxprom37
  %40 = load i64, i64* %arrayidx38, align 8
  %41 = load i64*, i64** %B.addr, align 8
  %42 = load i32, i32* %n.addr, align 4
  %43 = load i32, i32* %i, align 4
  %mul39 = mul nsw i32 %42, %43
  %44 = load i32, i32* %j30, align 4
  %add40 = add nsw i32 %mul39, %44
  %idxprom41 = sext i32 %add40 to i64
  %arrayidx42 = getelementptr inbounds i64, i64* %41, i64 %idxprom41
  %45 = load i64, i64* %arrayidx42, align 8
  %add43 = add nsw i64 %40, %45
  %46 = load i64*, i64** %C.addr, align 8
  %47 = load i32, i32* %n.addr, align 4
  %48 = load i32, i32* %i, align 4
  %mul44 = mul nsw i32 %47, %48
  %49 = load i32, i32* %j30, align 4
  %add45 = add nsw i32 %mul44, %49
  %idxprom46 = sext i32 %add45 to i64
  %arrayidx47 = getelementptr inbounds i64, i64* %46, i64 %idxprom46
  store i64 %add43, i64* %arrayidx47, align 8
  br label %for.inc48

for.inc48:                                        ; preds = %for.body34
  %50 = load i32, i32* %j30, align 4
  %inc = add nsw i32 %50, 1
  store i32 %inc, i32* %j30, align 4
  br label %for.cond32, !llvm.loop !4

for.end49:                                        ; preds = %for.cond32
  br label %for.inc50

for.inc50:                                        ; preds = %for.end49
  %51 = load i32, i32* %i, align 4
  %inc51 = add nsw i32 %51, 1
  store i32 %inc51, i32* %i, align 4
  br label %for.cond, !llvm.loop !5

for.end52:                                        ; preds = %for.cond
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
