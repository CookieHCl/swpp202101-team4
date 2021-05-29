; ModuleID = '/tmp/a.ll'
source_filename = "test4-not.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
; CHECK: start main
; CHECK: [[NOT:r.*]] = select {{.*}} 0 1
; CHECK: br [[NOT]] .while.end .while.body
; CHECK: end main
entry:
  br label %while.cond

while.cond:                                       ; preds = %if.end19, %entry
  %j.0 = phi i64 [ 0, %entry ], [ %j.3, %if.end19 ]
  %i.0 = phi i64 [ 0, %entry ], [ %i.3, %if.end19 ]
  %cmp = icmp slt i64 %i.0, 10
  %cmp1 = icmp slt i64 %j.0, 10
  %0 = or i1 %cmp, %cmp1
  br i1 %0, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %cmp2 = icmp sge i64 %i.0, 10
  br i1 %cmp2, label %if.then, label %if.else

if.then:                                          ; preds = %while.body
  %inc = add nsw i64 %j.0, 1
  %mul = mul nsw i64 2, %j.0
  %add = add nsw i64 %mul, 1
  call void @write(i64 %add)
  br label %if.end19

if.else:                                          ; preds = %while.body
  %cmp3 = icmp sge i64 %j.0, 10
  br i1 %cmp3, label %if.then4, label %if.else8

if.then4:                                         ; preds = %if.else
  %inc5 = add nsw i64 %i.0, 1
  %mul6 = mul nsw i64 2, %i.0
  %add7 = add nsw i64 %mul6, 1
  call void @write(i64 %add7)
  br label %if.end18

if.else8:                                         ; preds = %if.else
  %mul9 = mul nsw i64 2, %i.0
  %add10 = add nsw i64 %mul9, 1
  %mul11 = mul nsw i64 2, %j.0
  %add12 = add nsw i64 %mul11, 1
  %cmp13 = icmp slt i64 %add10, %add12
  br i1 %cmp13, label %if.then14, label %if.else16

if.then14:                                        ; preds = %if.else8
  call void @write(i64 %add10)
  %inc15 = add nsw i64 %i.0, 1
  br label %if.end

if.else16:                                        ; preds = %if.else8
  call void @write(i64 %add12)
  %inc17 = add nsw i64 %j.0, 1
  br label %if.end

if.end:                                           ; preds = %if.else16, %if.then14
  %j.1 = phi i64 [ %j.0, %if.then14 ], [ %inc17, %if.else16 ]
  %i.1 = phi i64 [ %inc15, %if.then14 ], [ %i.0, %if.else16 ]
  br label %if.end18

if.end18:                                         ; preds = %if.end, %if.then4
  %j.2 = phi i64 [ %j.0, %if.then4 ], [ %j.1, %if.end ]
  %i.2 = phi i64 [ %inc5, %if.then4 ], [ %i.1, %if.end ]
  br label %if.end19

if.end19:                                         ; preds = %if.end18, %if.then
  %j.3 = phi i64 [ %inc, %if.then ], [ %j.2, %if.end18 ]
  %i.3 = phi i64 [ %i.0, %if.then ], [ %i.2, %if.end18 ]
  br label %while.cond, !llvm.loop !2

while.end:                                        ; preds = %while.cond
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

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
