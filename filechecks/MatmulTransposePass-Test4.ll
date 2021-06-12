; ModuleID = '/tmp/a.ll'
source_filename = "matmul4/src/matmul4.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx11.0.0"

; Function Attrs: nounwind ssp uwtable
define dso_local void @copy_mask(i32 %dim, i32 %row, i32 %col, i64* %mat, i64* %mask) #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc17, %entry
  %i.0 = phi i8 [ 0, %entry ], [ %inc18, %for.inc17 ]
  %conv = zext i8 %i.0 to i32
  %cmp = icmp slt i32 %conv, 4
  br i1 %cmp, label %for.body, label %for.end19

for.body:                                         ; preds = %for.cond
  br label %for.cond2

for.cond2:                                        ; preds = %for.inc, %for.body
  %j.0 = phi i8 [ 0, %for.body ], [ %inc, %for.inc ]
  %conv3 = zext i8 %j.0 to i32
  %cmp4 = icmp slt i32 %conv3, 4
  br i1 %cmp4, label %for.body6, label %for.end

for.body6:                                        ; preds = %for.cond2
  %conv7 = zext i8 %i.0 to i32
  %add = add i32 %row, %conv7
  %mul = mul i32 %add, %dim
  %add8 = add i32 %mul, %col
  %conv9 = zext i8 %j.0 to i32
  %add10 = add i32 %add8, %conv9
  %idxprom = zext i32 %add10 to i64
  %arrayidx = getelementptr inbounds i64, i64* %mat, i64 %idxprom
  %0 = load i64, i64* %arrayidx, align 8
  %conv11 = zext i8 %i.0 to i32
  %mul12 = mul nsw i32 %conv11, 4
  %conv13 = zext i8 %j.0 to i32
  %add14 = add nsw i32 %mul12, %conv13
  %idxprom15 = sext i32 %add14 to i64
  %arrayidx16 = getelementptr inbounds i64, i64* %mask, i64 %idxprom15
  store i64 %0, i64* %arrayidx16, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body6
  %inc = add i8 %j.0, 1
  br label %for.cond2, !llvm.loop !4

for.end:                                          ; preds = %for.cond2
  br label %for.inc17

for.inc17:                                        ; preds = %for.end
  %inc18 = add i8 %i.0, 1
  br label %for.cond, !llvm.loop !7

for.end19:                                        ; preds = %for.cond
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind ssp uwtable
define dso_local void @add_mask(i32 %dim, i32 %row, i32 %col, i64* %mat, i64* %mask) #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc18, %entry
  %i.0 = phi i8 [ 0, %entry ], [ %inc19, %for.inc18 ]
  %conv = zext i8 %i.0 to i32
  %cmp = icmp slt i32 %conv, 4
  br i1 %cmp, label %for.body, label %for.end20

for.body:                                         ; preds = %for.cond
  br label %for.cond2

for.cond2:                                        ; preds = %for.inc, %for.body
  %j.0 = phi i8 [ 0, %for.body ], [ %inc, %for.inc ]
  %conv3 = zext i8 %j.0 to i32
  %cmp4 = icmp slt i32 %conv3, 4
  br i1 %cmp4, label %for.body6, label %for.end

for.body6:                                        ; preds = %for.cond2
  %conv7 = zext i8 %i.0 to i32
  %mul = mul nsw i32 %conv7, 4
  %conv8 = zext i8 %j.0 to i32
  %add = add nsw i32 %mul, %conv8
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds i64, i64* %mask, i64 %idxprom
  %0 = load i64, i64* %arrayidx, align 8
  %conv9 = zext i8 %i.0 to i32
  %add10 = add i32 %row, %conv9
  %mul11 = mul i32 %add10, %dim
  %add12 = add i32 %mul11, %col
  %conv13 = zext i8 %j.0 to i32
  %add14 = add i32 %add12, %conv13
  %idxprom15 = zext i32 %add14 to i64
  %arrayidx16 = getelementptr inbounds i64, i64* %mat, i64 %idxprom15
  %1 = load i64, i64* %arrayidx16, align 8
  %add17 = add i64 %1, %0
  store i64 %add17, i64* %arrayidx16, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body6
  %inc = add i8 %j.0, 1
  br label %for.cond2, !llvm.loop !8

for.end:                                          ; preds = %for.cond2
  br label %for.inc18

for.inc18:                                        ; preds = %for.end
  %inc19 = add i8 %i.0, 1
  br label %for.cond, !llvm.loop !9

for.end20:                                        ; preds = %for.cond
  ret void
}

; Function Attrs: nounwind ssp uwtable
define dso_local void @mask_mul(i64* %c, i64* %a, i64* %b) #0 {
; CHECK-DAG: load
; CHECK-DAG: br
; CHECK-DAG: load
; CHECK-DAG: load
; CHECK-DAG: store

entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc31, %entry
  %i.0 = phi i8 [ 0, %entry ], [ %inc32, %for.inc31 ]
  %conv = zext i8 %i.0 to i32
  %cmp = icmp slt i32 %conv, 4
  br i1 %cmp, label %for.body, label %for.end33

for.body:                                         ; preds = %for.cond
  br label %for.cond2

for.cond2:                                        ; preds = %for.inc28, %for.body
  %j.0 = phi i8 [ 0, %for.body ], [ %inc29, %for.inc28 ]
  %conv3 = zext i8 %j.0 to i32
  %cmp4 = icmp slt i32 %conv3, 4
  br i1 %cmp4, label %for.body6, label %for.end30

for.body6:                                        ; preds = %for.cond2
  br label %for.cond7

for.cond7:                                        ; preds = %for.inc, %for.body6
  %k.0 = phi i8 [ 0, %for.body6 ], [ %inc, %for.inc ]
  %sum.0 = phi i64 [ 0, %for.body6 ], [ %add21, %for.inc ]
  %conv8 = zext i8 %k.0 to i32
  %cmp9 = icmp slt i32 %conv8, 4
  br i1 %cmp9, label %for.body11, label %for.end

for.body11:                                       ; preds = %for.cond7
  %conv12 = zext i8 %i.0 to i32
  %mul = mul nsw i32 %conv12, 4
  %conv13 = zext i8 %k.0 to i32
  %add = add nsw i32 %mul, %conv13
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds i64, i64* %a, i64 %idxprom
  %0 = load i64, i64* %arrayidx, align 8
  %conv14 = zext i8 %k.0 to i32
  %mul15 = mul nsw i32 %conv14, 4
  %conv16 = zext i8 %j.0 to i32
  %add17 = add nsw i32 %mul15, %conv16
  %idxprom18 = sext i32 %add17 to i64
  %arrayidx19 = getelementptr inbounds i64, i64* %b, i64 %idxprom18
  %1 = load i64, i64* %arrayidx19, align 8
  %mul20 = mul i64 %0, %1
  %add21 = add i64 %sum.0, %mul20
  br label %for.inc

for.inc:                                          ; preds = %for.body11
  %inc = add i8 %k.0, 1
  br label %for.cond7, !llvm.loop !10

for.end:                                          ; preds = %for.cond7
  %conv22 = zext i8 %i.0 to i32
  %mul23 = mul nsw i32 %conv22, 4
  %conv24 = zext i8 %j.0 to i32
  %add25 = add nsw i32 %mul23, %conv24
  %idxprom26 = sext i32 %add25 to i64
  %arrayidx27 = getelementptr inbounds i64, i64* %c, i64 %idxprom26
  store i64 %sum.0, i64* %arrayidx27, align 8
  br label %for.inc28

for.inc28:                                        ; preds = %for.end
  %inc29 = add i8 %j.0, 1
  br label %for.cond2, !llvm.loop !11

for.end30:                                        ; preds = %for.cond2
  br label %for.inc31

for.inc31:                                        ; preds = %for.end30
  %inc32 = add i8 %i.0, 1
  br label %for.cond, !llvm.loop !12

for.end33:                                        ; preds = %for.cond
  ret void
}

; Function Attrs: nounwind ssp uwtable
define dso_local void @matmul(i32 %dim, i64* %c, i64* %a, i64* %b) #0 {
entry:
  %c_mask = alloca [16 x i64], align 16
  %a_mask = alloca [16 x i64], align 16
  %b_mask = alloca [16 x i64], align 16
  br label %for.cond

for.cond:                                         ; preds = %for.inc15, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %add16, %for.inc15 ]
  %cmp = icmp ult i32 %i.0, %dim
  br i1 %cmp, label %for.body, label %for.end17

for.body:                                         ; preds = %for.cond
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc12, %for.body
  %j.0 = phi i32 [ 0, %for.body ], [ %add13, %for.inc12 ]
  %cmp2 = icmp ult i32 %j.0, %dim
  br i1 %cmp2, label %for.body3, label %for.end14

for.body3:                                        ; preds = %for.cond1
  br label %for.cond4

for.cond4:                                        ; preds = %for.inc, %for.body3
  %k.0 = phi i32 [ 0, %for.body3 ], [ %add, %for.inc ]
  %cmp5 = icmp ult i32 %k.0, %dim
  br i1 %cmp5, label %for.body6, label %for.end

for.body6:                                        ; preds = %for.cond4
  %arraydecay = getelementptr inbounds [16 x i64], [16 x i64]* %a_mask, i64 0, i64 0
  call void @copy_mask(i32 %dim, i32 %i.0, i32 %k.0, i64* %a, i64* %arraydecay)
  %arraydecay7 = getelementptr inbounds [16 x i64], [16 x i64]* %b_mask, i64 0, i64 0
  call void @copy_mask(i32 %dim, i32 %k.0, i32 %j.0, i64* %b, i64* %arraydecay7)
  %arraydecay8 = getelementptr inbounds [16 x i64], [16 x i64]* %c_mask, i64 0, i64 0
  %arraydecay9 = getelementptr inbounds [16 x i64], [16 x i64]* %a_mask, i64 0, i64 0
  %arraydecay10 = getelementptr inbounds [16 x i64], [16 x i64]* %b_mask, i64 0, i64 0
  call void @mask_mul(i64* %arraydecay8, i64* %arraydecay9, i64* %arraydecay10)
  %arraydecay11 = getelementptr inbounds [16 x i64], [16 x i64]* %c_mask, i64 0, i64 0
  call void @add_mask(i32 %dim, i32 %i.0, i32 %j.0, i64* %c, i64* %arraydecay11)
  br label %for.inc

for.inc:                                          ; preds = %for.body6
  %add = add i32 %k.0, 4
  br label %for.cond4, !llvm.loop !13

for.end:                                          ; preds = %for.cond4
  br label %for.inc12

for.inc12:                                        ; preds = %for.end
  %add13 = add i32 %j.0, 4
  br label %for.cond1, !llvm.loop !14

for.end14:                                        ; preds = %for.cond1
  br label %for.inc15

for.inc15:                                        ; preds = %for.end14
  %add16 = add i32 %i.0, 4
  br label %for.cond, !llvm.loop !15

for.end17:                                        ; preds = %for.cond
  ret void
}

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
  br label %for.cond1, !llvm.loop !16

for.end:                                          ; preds = %for.cond1
  br label %for.inc4

for.inc4:                                         ; preds = %for.end
  %inc5 = add i32 %i.0, 1
  br label %for.cond, !llvm.loop !17

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
  br label %for.cond1, !llvm.loop !18

for.end:                                          ; preds = %for.cond1
  br label %for.inc4

for.inc4:                                         ; preds = %for.end
  %inc5 = add i32 %i.0, 1
  br label %for.cond, !llvm.loop !19

for.end6:                                         ; preds = %for.cond
  ret void
}

declare void @write(i64) #2

; Function Attrs: nounwind ssp uwtable
define dso_local i32 @main() #0 {
entry:
  %call = call i64 (...) @read()
  %conv = trunc i64 %call to i32
  %rem = urem i32 %conv, 4
  %cmp = icmp ne i32 %rem, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %cleanup

if.end:                                           ; preds = %entry
  %mul = mul i32 %conv, %conv
  %conv2 = zext i32 %mul to i64
  %mul3 = mul i64 %conv2, 8
  %call4 = call noalias i8* @malloc(i64 %mul3) #4
  %0 = bitcast i8* %call4 to i64*
  %mul5 = mul i32 %conv, %conv
  %conv6 = zext i32 %mul5 to i64
  %mul7 = mul i64 %conv6, 8
  %call8 = call noalias i8* @malloc(i64 %mul7) #4
  %1 = bitcast i8* %call8 to i64*
  %mul9 = mul i32 %conv, %conv
  %conv10 = zext i32 %mul9 to i64
  %mul11 = mul i64 %conv10, 8
  %call12 = call noalias i8* @malloc(i64 %mul11) #4
  %2 = bitcast i8* %call12 to i64*
  call void @read_mat(i32 %conv, i64* %0)
  call void @read_mat(i32 %conv, i64* %1)
  call void @matmul(i32 %conv, i64* %2, i64* %0, i64* %1)
  call void @print_mat(i32 %conv, i64* %2)
  br label %cleanup

cleanup:                                          ; preds = %if.end, %if.then
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
!13 = distinct !{!13, !5, !6}
!14 = distinct !{!14, !5, !6}
!15 = distinct !{!15, !5, !6}
!16 = distinct !{!16, !5, !6}
!17 = distinct !{!17, !5, !6}
!18 = distinct !{!18, !5, !6}
!19 = distinct !{!19, !5, !6}