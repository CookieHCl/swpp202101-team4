; ModuleID = 'foo.c'
source_filename = "foo.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i64 @max(i64 %0, i64 %1) #0 {
  %3 = alloca i64, align 8
  %4 = alloca i64, align 8
  store i64 %0, i64* %3, align 8
  store i64 %1, i64* %4, align 8
  %5 = load i64, i64* %3, align 8
  %6 = load i64, i64* %4, align 8
  %7 = icmp sgt i64 %5, %6
  br i1 %7, label %8, label %10

8:                                                ; preds = %2
  %9 = load i64, i64* %3, align 8
  br label %12

10:                                               ; preds = %2
  %11 = load i64, i64* %4, align 8
  br label %12

12:                                               ; preds = %10, %8
  %13 = phi i64 [ %9, %8 ], [ %11, %10 ]
  ret i64 %13
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i64 @recfibo(i64 %0) #0 {
  %2 = alloca i64, align 8
  %3 = alloca i64, align 8
  store i64 %0, i64* %3, align 8
  %4 = load i64, i64* %3, align 8
  %5 = icmp sle i64 %4, 0
  br i1 %5, label %6, label %7

6:                                                ; preds = %1
  store i64 0, i64* %2, align 8
  br label %13

7:                                                ; preds = %1
  %8 = load i64, i64* %3, align 8
  %9 = load i64, i64* %3, align 8
  %10 = sub nsw i64 %9, 1
  %11 = call i64 @recfibo(i64 %10)
  %12 = mul nsw i64 %8, %11
  store i64 %12, i64* %2, align 8
  br label %13

13:                                               ; preds = %7, %6
  %14 = load i64, i64* %2, align 8
  ret i64 %14
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i64 @f1(i64 %0) #0 {
  %2 = alloca i64, align 8
  %3 = alloca i64, align 8
  %4 = alloca i64, align 8
  store i64 %0, i64* %3, align 8
  %5 = load i64, i64* %3, align 8
  %6 = icmp sle i64 %5, 0
  br i1 %6, label %7, label %8

7:                                                ; preds = %1
  store i64 0, i64* %2, align 8
  br label %16

8:                                                ; preds = %1
;   CHECK-NOT:  call f2
;   CHECK:  call max
  %9 = load i64, i64* %3, align 8
  %10 = sub nsw i64 %9, 1
  %11 = call i64 @f2(i64 %10)
  %12 = add nsw i64 %11, 1
  store i64 %12, i64* %4, align 8
  %13 = load i64, i64* %3, align 8
  %14 = load i64, i64* %4, align 8
  %15 = call i64 @max(i64 %13, i64 %14)
  store i64 %15, i64* %2, align 8
  br label %16

16:                                               ; preds = %8, %7
  %17 = load i64, i64* %2, align 8
  ret i64 %17
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i64 @f2(i64 %0) #0 {
  %2 = alloca i64, align 8
  %3 = alloca i64, align 8
  %4 = alloca i64, align 8
  store i64 %0, i64* %3, align 8
  %5 = load i64, i64* %3, align 8
  %6 = icmp sle i64 %5, 0
  br i1 %6, label %7, label %8

7:                                                ; preds = %1
  store i64 0, i64* %2, align 8
  br label %16

8:                                                ; preds = %1
;   CHECK:  call f1
;   CHECK:  call max
  %9 = load i64, i64* %3, align 8
  %10 = sdiv i64 %9, 2
  %11 = call i64 @f1(i64 %10)
  %12 = add nsw i64 %11, 1
  store i64 %12, i64* %4, align 8
  %13 = load i64, i64* %3, align 8
  %14 = load i64, i64* %4, align 8
  %15 = call i64 @max(i64 %13, i64 %14)
  store i64 %15, i64* %2, align 8
  br label %16

16:                                               ; preds = %8, %7
  %17 = load i64, i64* %2, align 8
  ret i64 %17
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
;   CHECK:      sp = sub sp 28 64
;   CHECK:      call recfibo
;   CHECK-NOT:	call max
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 10, i32* %2, align 4
  store i32 20, i32* %3, align 4
  %4 = load i32, i32* %2, align 4
  %5 = sext i32 %4 to i64
  %6 = call i64 @recfibo(i64 %5)
  %7 = load i32, i32* %3, align 4
  %8 = sext i32 %7 to i64
  %9 = call i64 @recfibo(i64 %8)
  %10 = call i64 @max(i64 %6, i64 %9)
  %11 = trunc i64 %10 to i32
  ret i32 %11
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.0-4ubuntu1 "}
