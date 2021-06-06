; ModuleID = '/tmp/a.ll'
source_filename = "matmul2/src/matmul2.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx11.0.0"

; Function Attrs: nounwind ssp uwtable
define dso_local void @matmul(i32 %dim, i64* %c, i64* %a, i64* %b) #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc20, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc21, %for.inc20 ]
  %cmp = icmp ult i32 %i.0, %dim
  br i1 %cmp, label %for.body, label %for.end22

for.body:                                         ; preds = %for.cond
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc17, %for.body
  %j.0 = phi i32 [ 0, %for.body ], [ %inc18, %for.inc17 ]
  %cmp2 = icmp ult i32 %j.0, %dim
  br i1 %cmp2, label %for.body3, label %for.end19

for.body3:                                        ; preds = %for.cond1
  br label %for.cond4

for.cond4:                                        ; preds = %for.inc, %for.body3
  %k.0 = phi i32 [ 0, %for.body3 ], [ %inc, %for.inc ]
  %cmp5 = icmp ult i32 %k.0, %dim
  br i1 %cmp5, label %for.body6, label %for.end

for.body6:                                        ; preds = %for.cond4
  %mul = mul i32 %i.0, %dim
  %add = add i32 %mul, %k.0
  %idxprom = zext i32 %add to i64
  %arrayidx = getelementptr inbounds i64, i64* %a, i64 %idxprom
  %0 = load i64, i64* %arrayidx, align 8
  %mul7 = mul i32 %k.0, %dim
  %add8 = add i32 %mul7, %j.0
  %idxprom9 = zext i32 %add8 to i64
  %arrayidx10 = getelementptr inbounds i64, i64* %b, i64 %idxprom9
  %1 = load i64, i64* %arrayidx10, align 8
  %mul11 = mul i64 %0, %1
  %mul12 = mul i32 %i.0, %dim
  %add13 = add i32 %mul12, %j.0
  %idxprom14 = zext i32 %add13 to i64
  %arrayidx15 = getelementptr inbounds i64, i64* %c, i64 %idxprom14
  %2 = load i64, i64* %arrayidx15, align 8
  %add16 = add i64 %2, %mul11
  store i64 %add16, i64* %arrayidx15, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body6
  %inc = add i32 %k.0, 1
  br label %for.cond4, !llvm.loop !4

for.end:                                          ; preds = %for.cond4
  br label %for.inc17

for.inc17:                                        ; preds = %for.end
  %inc18 = add i32 %j.0, 1
  br label %for.cond1, !llvm.loop !7

for.end19:                                        ; preds = %for.cond1
  br label %for.inc20

for.inc20:                                        ; preds = %for.end19
  %inc21 = add i32 %i.0, 1
  br label %for.cond, !llvm.loop !8

for.end22:                                        ; preds = %for.cond
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind ssp uwtable
define dso_local void @read_mat(i32 %dim, i64* %mat) #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc4, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc5, %for.inc4 ]
  %cmp = icmp ult i32 %i.0, %dim
  br i1 %cmp, label %for.body, label %for.end6

for.body:                                         ; preds = %for.cond
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc, %for.body
  %j.0 = phi i32 [ 0, %for.body ], [ %inc, %for.inc ]
  %cmp2 = icmp ult i32 %j.0, %dim
  br i1 %cmp2, label %for.body3, label %for.end

for.body3:                                        ; preds = %for.cond1
  %call = call i64 (...) @read()
  %mul = mul i32 %i.0, %dim
  %add = add i32 %mul, %j.0
  %idxprom = zext i32 %add to i64
  %arrayidx = getelementptr inbounds i64, i64* %mat, i64 %idxprom
  store i64 %call, i64* %arrayidx, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %inc = add i32 %j.0, 1
  br label %for.cond1, !llvm.loop !9

for.end:                                          ; preds = %for.cond1
  br label %for.inc4

for.inc4:                                         ; preds = %for.end
  %inc5 = add i32 %i.0, 1
  br label %for.cond, !llvm.loop !10

for.end6:                                         ; preds = %for.cond
  ret void
}

declare i64 @read(...) #2

; Function Attrs: nounwind ssp uwtable
define dso_local void @print_mat(i32 %dim, i64* %mat) #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc4, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc5, %for.inc4 ]
  %cmp = icmp ult i32 %i.0, %dim
  br i1 %cmp, label %for.body, label %for.end6

for.body:                                         ; preds = %for.cond
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc, %for.body
  %j.0 = phi i32 [ 0, %for.body ], [ %inc, %for.inc ]
  %cmp2 = icmp ult i32 %j.0, %dim
  br i1 %cmp2, label %for.body3, label %for.end

for.body3:                                        ; preds = %for.cond1
  %mul = mul i32 %i.0, %dim
  %add = add i32 %mul, %j.0
  %idxprom = zext i32 %add to i64
  %arrayidx = getelementptr inbounds i64, i64* %mat, i64 %idxprom
  %0 = load i64, i64* %arrayidx, align 8
  call void @write(i64 %0)
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %inc = add i32 %j.0, 1
  br label %for.cond1, !llvm.loop !11

for.end:                                          ; preds = %for.cond1
  br label %for.inc4

for.inc4:                                         ; preds = %for.end
  %inc5 = add i32 %i.0, 1
  br label %for.cond, !llvm.loop !12

for.end6:                                         ; preds = %for.cond
  ret void
}

declare void @write(i64) #2

; Function Attrs: nounwind ssp uwtable
define dso_local i32 @main() #0 {
entry:
  %call = call i64 (...) @read()
  %conv = trunc i64 %call to i32
  %mul = mul i32 %conv, %conv
  %conv1 = zext i32 %mul to i64
  %mul2 = mul i64 %conv1, 8
  %call3 = call noalias i8* @malloc(i64 %mul2) #4
  %0 = bitcast i8* %call3 to i64*
  %mul4 = mul i32 %conv, %conv
  %conv5 = zext i32 %mul4 to i64
  %mul6 = mul i64 %conv5, 8
  %call7 = call noalias i8* @malloc(i64 %mul6) #4
  %1 = bitcast i8* %call7 to i64*
  %mul8 = mul i32 %conv, %conv
  %conv9 = zext i32 %mul8 to i64
  %mul10 = mul i64 %conv9, 8
  %call11 = call noalias i8* @malloc(i64 %mul10) #4
  %2 = bitcast i8* %call11 to i64*
  call void @read_mat(i32 %conv, i64* %0)
  call void @read_mat(i32 %conv, i64* %1)
  call void @matmul(i32 %conv, i64* %2, i64* %0, i64* %1)
  call void @print_mat(i32 %conv, i64* %2)
  ret i32 0
}

; Function Attrs: allocsize(0)
declare noalias i8* @malloc(i64) #3

attributes #0 = { nounwind ssp uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { allocsize(0) "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { allocsize(0) }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 11, i32 1]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git 4990141a4366eb00abdc8252d7cbb8adeacb9954)"}
!4 = distinct !{!4, !5, !6}
!5 = !{!"llvm.loop.mustprogress"}
!6 = !{!"llvm.loop.unroll.disable"}
!7 = distinct !{!7, !5, !6}
!8 = distinct !{!8, !5, !6}
!9 = distinct !{!9, !5, !6}
!10 = distinct !{!10, !5, !6}
!11 = distinct !{!11, !5, !6}
!12 = distinct !{!12, !5, !6}
