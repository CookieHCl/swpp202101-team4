; ModuleID = 'vec.c'
source_filename = "vec.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @copy(i64* %A, i64* %B, i32 %n) #0 {
; CHECK : vload 4
; CHECK : vstore 4
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
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i64*, i64** %A.addr, align 8
  %3 = load i32, i32* %i, align 4
  %add = add nsw i32 %3, 0
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds i64, i64* %2, i64 %idxprom
  %4 = load i64, i64* %arrayidx, align 8
  %5 = load i64*, i64** %B.addr, align 8
  %6 = load i32, i32* %i, align 4
  %add1 = add nsw i32 %6, 0
  %idxprom2 = sext i32 %add1 to i64
  %arrayidx3 = getelementptr inbounds i64, i64* %5, i64 %idxprom2
  store i64 %4, i64* %arrayidx3, align 8
  %7 = load i64*, i64** %A.addr, align 8
  %8 = load i32, i32* %i, align 4
  %add4 = add nsw i32 %8, 1
  %idxprom5 = sext i32 %add4 to i64
  %arrayidx6 = getelementptr inbounds i64, i64* %7, i64 %idxprom5
  %9 = load i64, i64* %arrayidx6, align 8
  %10 = load i64*, i64** %B.addr, align 8
  %11 = load i32, i32* %i, align 4
  %add7 = add nsw i32 %11, 1
  %idxprom8 = sext i32 %add7 to i64
  %arrayidx9 = getelementptr inbounds i64, i64* %10, i64 %idxprom8
  store i64 %9, i64* %arrayidx9, align 8
  %12 = load i64*, i64** %A.addr, align 8
  %13 = load i32, i32* %i, align 4
  %add10 = add nsw i32 %13, 2
  %idxprom11 = sext i32 %add10 to i64
  %arrayidx12 = getelementptr inbounds i64, i64* %12, i64 %idxprom11
  %14 = load i64, i64* %arrayidx12, align 8
  %15 = load i64*, i64** %B.addr, align 8
  %16 = load i32, i32* %i, align 4
  %add13 = add nsw i32 %16, 2
  %idxprom14 = sext i32 %add13 to i64
  %arrayidx15 = getelementptr inbounds i64, i64* %15, i64 %idxprom14
  store i64 %14, i64* %arrayidx15, align 8
  %17 = load i64*, i64** %A.addr, align 8
  %18 = load i32, i32* %i, align 4
  %add16 = add nsw i32 %18, 3
  %idxprom17 = sext i32 %add16 to i64
  %arrayidx18 = getelementptr inbounds i64, i64* %17, i64 %idxprom17
  %19 = load i64, i64* %arrayidx18, align 8
  %20 = load i64*, i64** %B.addr, align 8
  %21 = load i32, i32* %i, align 4
  %add19 = add nsw i32 %21, 3
  %idxprom20 = sext i32 %add19 to i64
  %arrayidx21 = getelementptr inbounds i64, i64* %20, i64 %idxprom20
  store i64 %19, i64* %arrayidx21, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %22 = load i32, i32* %i, align 4
  %add22 = add nsw i32 %22, 4
  store i32 %add22, i32* %i, align 4
  br label %for.cond, !llvm.loop !2

for.end:                                          ; preds = %for.cond
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main(i32 %argc, i8** %argv) #0 {
; CHECK-NOT: vload4
; CHECK-NOT: vstore4
entry:
  %retval = alloca i32, align 4
  %argc.addr = alloca i32, align 4
  %argv.addr = alloca i8**, align 8
  %a = alloca i64*, align 8
  %b = alloca i64*, align 8
  %i = alloca i64, align 8
  store i32 0, i32* %retval, align 4
  store i32 %argc, i32* %argc.addr, align 4
  store i8** %argv, i8*** %argv.addr, align 8
  %call = call noalias i8* @malloc(i64 800) #2
  %0 = bitcast i8* %call to i64*
  store i64* %0, i64** %a, align 8
  %call1 = call noalias i8* @malloc(i64 800) #2
  %1 = bitcast i8* %call1 to i64*
  store i64* %1, i64** %b, align 8
  store i64 0, i64* %i, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %2 = load i64, i64* %i, align 8
  %cmp = icmp slt i64 %2, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %3 = load i64, i64* %i, align 8
  %4 = load i64*, i64** %a, align 8
  %5 = load i64, i64* %i, align 8
  %arrayidx = getelementptr inbounds i64, i64* %4, i64 %5
  store i64 %3, i64* %arrayidx, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %6 = load i64, i64* %i, align 8
  %inc = add nsw i64 %6, 1
  store i64 %inc, i64* %i, align 8
  br label %for.cond, !llvm.loop !4

for.end:                                          ; preds = %for.cond
  %7 = load i64*, i64** %a, align 8
  %8 = load i64*, i64** %b, align 8
  call void @copy(i64* %7, i64* %8, i32 100)
  %9 = load i32, i32* %retval, align 4
  ret i32 %9
}

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) #1

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git f05b649610564b11c481a20598dbb3f532c4602a)"}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.mustprogress"}
!4 = distinct !{!4, !3}
