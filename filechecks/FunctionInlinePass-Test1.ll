; ModuleID = 'foo.c'
source_filename = "foo.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

define dso_local i64 @max(i64 %0, i64 %1) {
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

define dso_local i64 @min(i64 %0, i64 %1) {
  %3 = alloca i64, align 8
  %4 = alloca i64, align 8
  store i64 %0, i64* %3, align 8
  store i64 %1, i64* %4, align 8
  %5 = load i64, i64* %3, align 8
  %6 = load i64, i64* %4, align 8
  %7 = icmp sgt i64 %5, %6
  br i1 %7, label %8, label %10

8:                                                ; preds = %2
  %9 = load i64, i64* %4, align 8
  br label %12

10:                                               ; preds = %2
  %11 = load i64, i64* %3, align 8
  br label %12

12:                                               ; preds = %10, %8
  %13 = phi i64 [ %9, %8 ], [ %11, %10 ]
  ret i64 %13
}

define dso_local i64 @diff(i64 %0, i64 %1) {
  %3 = alloca i64, align 8
  %4 = alloca i64, align 8
  store i64 %0, i64* %3, align 8
  store i64 %1, i64* %4, align 8
  %5 = load i64, i64* %3, align 8
  %6 = load i64, i64* %4, align 8
  %7 = icmp sgt i64 %5, %6
  br i1 %7, label %8, label %12

8:                                                ; preds = %2
  %9 = load i64, i64* %3, align 8
  %10 = load i64, i64* %4, align 8
  %11 = sub nsw i64 %9, %10
  br label %16

12:                                               ; preds = %2
  %13 = load i64, i64* %4, align 8
  %14 = load i64, i64* %3, align 8
  %15 = sub nsw i64 %13, %14
  br label %16

16:                                               ; preds = %12, %8
  %17 = phi i64 [ %11, %8 ], [ %15, %12 ]
  ret i64 %17
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() {
;   CHECK-NOT:	call diff
;   CHECK-NOT:	call max
;   CHECK-NOT:	call min
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 10, i32* %2, align 4
  store i32 20, i32* %3, align 4
  %4 = load i32, i32* %2, align 4
  %5 = sext i32 %4 to i64
  %6 = load i32, i32* %3, align 4
  %7 = sext i32 %6 to i64
  %8 = call i64 @diff(i64 %5, i64 %7)
  %9 = load i32, i32* %3, align 4
  %10 = sext i32 %9 to i64
  %11 = call i64 @max(i64 %8, i64 %10)
  %12 = load i32, i32* %2, align 4
  %13 = sext i32 %12 to i64
  %14 = load i32, i32* %2, align 4
  %15 = sext i32 %14 to i64
  %16 = load i32, i32* %3, align 4
  %17 = sext i32 %16 to i64
  %18 = call i64 @diff(i64 %15, i64 %17)
  %19 = call i64 @min(i64 %13, i64 %18)
  %20 = call i64 @diff(i64 %11, i64 %19)
  %21 = trunc i64 %20 to i32
  ret i32 %21
}

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.0-4ubuntu1 "}
