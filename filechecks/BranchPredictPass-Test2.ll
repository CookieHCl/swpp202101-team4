; ModuleID = '/tmp/a.ll'
source_filename = "test2-nested-loops.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
; CHECK: start main
; CHECK: icmp sgt
; CHECK: br {{.*}} .for.cond.cleanup .for.body
; CHECK: icmp sgt
; CHECK: br {{.*}} .for.cond.cleanup3 .for.body4
; CHECK: end main
entry:
  %call = call i64 (...) @read()
  br label %for.cond

for.cond:                                         ; preds = %for.inc5, %entry
  %i.0 = phi i64 [ 1, %entry ], [ %inc6, %for.inc5 ]
  %cmp = icmp sle i64 %i.0, %call
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  br label %for.end7

for.body:                                         ; preds = %for.cond
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc, %for.body
  %j.0 = phi i64 [ 1, %for.body ], [ %inc, %for.inc ]
  %cmp2 = icmp sle i64 %j.0, %call
  br i1 %cmp2, label %for.body4, label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond1
  br label %for.end

for.body4:                                        ; preds = %for.cond1
  %mul = mul nsw i64 %i.0, %j.0
  call void @write(i64 %mul)
  br label %for.inc

for.inc:                                          ; preds = %for.body4
  %inc = add nsw i64 %j.0, 1
  br label %for.cond1, !llvm.loop !2

for.end:                                          ; preds = %for.cond.cleanup3
  br label %for.inc5

for.inc5:                                         ; preds = %for.end
  %inc6 = add nsw i64 %i.0, 1
  br label %for.cond, !llvm.loop !5

for.end7:                                         ; preds = %for.cond.cleanup
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

declare dso_local i64 @read(...) #2

declare dso_local void @write(i64) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.1 (https://github.com/llvm/llvm-project.git 4973ce53ca8abfc14233a3d8b3045673e0e8543c)"}
!2 = distinct !{!2, !3, !4}
!3 = !{!"llvm.loop.mustprogress"}
!4 = !{!"llvm.loop.unroll.disable"}
!5 = distinct !{!5, !3, !4}
