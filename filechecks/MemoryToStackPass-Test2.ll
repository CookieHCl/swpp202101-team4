; ModuleID = '/tmp/a.ll'
source_filename = "b.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
; CHECK: start main 0:
; CHECK-NOT: sp
; CHECK: call ____malloc
; CHECK-NOT: sp
; CHECK: end main
entry:
  %count = alloca [10 x i64], align 16
  %call = call i64 (...) @read()
  %call1 = call noalias i8* @malloc(i64 %call) #4
  %0 = bitcast i8* %call1 to i64*
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i64 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp ult i64 %i.0, %call
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  br label %for.end

for.body:                                         ; preds = %for.cond
  %call2 = call i64 (...) @read()
  %arrayidx = getelementptr inbounds i64, i64* %0, i64 %i.0
  store i64 %call2, i64* %arrayidx, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add i64 %i.0, 1
  br label %for.cond, !llvm.loop !2

for.end:                                          ; preds = %for.cond.cleanup
  br label %for.cond4

for.cond4:                                        ; preds = %for.inc9, %for.end
  %i3.0 = phi i64 [ 0, %for.end ], [ %inc10, %for.inc9 ]
  %cmp5 = icmp ult i64 %i3.0, 10
  br i1 %cmp5, label %for.body7, label %for.cond.cleanup6

for.cond.cleanup6:                                ; preds = %for.cond4
  br label %for.end11

for.body7:                                        ; preds = %for.cond4
  %arrayidx8 = getelementptr inbounds [10 x i64], [10 x i64]* %count, i64 0, i64 %i3.0
  store i64 0, i64* %arrayidx8, align 8
  br label %for.inc9

for.inc9:                                         ; preds = %for.body7
  %inc10 = add i64 %i3.0, 1
  br label %for.cond4, !llvm.loop !5

for.end11:                                        ; preds = %for.cond.cleanup6
  br label %for.cond13

for.cond13:                                       ; preds = %for.inc20, %for.end11
  %i12.0 = phi i64 [ 0, %for.end11 ], [ %inc21, %for.inc20 ]
  %cmp14 = icmp ult i64 %i12.0, %call
  br i1 %cmp14, label %for.body16, label %for.cond.cleanup15

for.cond.cleanup15:                               ; preds = %for.cond13
  br label %for.end22

for.body16:                                       ; preds = %for.cond13
  %arrayidx17 = getelementptr inbounds i64, i64* %0, i64 %i12.0
  %1 = load i64, i64* %arrayidx17, align 8
  %rem = urem i64 %1, 10
  %arrayidx18 = getelementptr inbounds [10 x i64], [10 x i64]* %count, i64 0, i64 %rem
  %2 = load i64, i64* %arrayidx18, align 8
  %inc19 = add i64 %2, 1
  store i64 %inc19, i64* %arrayidx18, align 8
  br label %for.inc20

for.inc20:                                        ; preds = %for.body16
  %inc21 = add i64 %i12.0, 1
  br label %for.cond13, !llvm.loop !6

for.end22:                                        ; preds = %for.cond.cleanup15
  br label %for.cond24

for.cond24:                                       ; preds = %for.inc29, %for.end22
  %i23.0 = phi i64 [ 0, %for.end22 ], [ %inc30, %for.inc29 ]
  %cmp25 = icmp ult i64 %i23.0, 10
  br i1 %cmp25, label %for.body27, label %for.cond.cleanup26

for.cond.cleanup26:                               ; preds = %for.cond24
  br label %for.end31

for.body27:                                       ; preds = %for.cond24
  %arrayidx28 = getelementptr inbounds [10 x i64], [10 x i64]* %count, i64 0, i64 %i23.0
  %3 = load i64, i64* %arrayidx28, align 8
  call void @write(i64 %3)
  br label %for.inc29

for.inc29:                                        ; preds = %for.body27
  %inc30 = add i64 %i23.0, 1
  br label %for.cond24, !llvm.loop !7

for.end31:                                        ; preds = %for.cond.cleanup26
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

declare dso_local i64 @read(...) #2

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) #3

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

declare dso_local void @write(i64) #2

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.1 (https://github.com/llvm/llvm-project.git 4973ce53ca8abfc14233a3d8b3045673e0e8543c)"}
!2 = distinct !{!2, !3, !4}
!3 = !{!"llvm.loop.mustprogress"}
!4 = !{!"llvm.loop.unroll.disable"}
!5 = distinct !{!5, !3, !4}
!6 = distinct !{!6, !3, !4}
!7 = distinct !{!7, !3, !4}
