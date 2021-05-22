start malloc_upto_8 1:
.entry:
  r1 = add arg1 7 64 
  r1 = udiv r1 8 64 
  r1 = mul r1 8 64 
  r1 = malloc r1 
  ret r1 
end malloc_upto_8

start min 2:
.entry:
  r1 = icmp slt arg1 arg2 32 
  br r1 .cond.true .cond.false 
.cond.true:
  r1 = mul arg1 1 32 
  br .cond.end 
.cond.false:
  r1 = mul arg2 1 32 
  br .cond.end 
.cond.end:
  ret r1 
end min

start count_leading_zeros 1:
.entry:
  r4 = mul 0 1 32 
  r3 = mul 31 1 32 
  br .for.cond 
.for.cond:
  r1 = icmp sge r3 0 32 
  br r1 .for.body .for.cond.cleanup 
.for.cond.cleanup:
  br .cleanup 
.for.body:
  r1 = shl 1 r3 32 
  r1 = and arg1 r1 32 
  r1 = icmp ne r1 0 32 
  br r1 .if.then .if.end 
.if.then:
  br .cleanup 
.if.end:
  r2 = add r4 1 32 
  br .for.inc 
.for.inc:
  r1 = add r3 4294967295 32 
  r3 = mul r1 1 32 
  r4 = mul r2 1 32 
  br .for.cond 
.cleanup:
  br .for.end 
.for.end:
  ret r4 
end count_leading_zeros

start floorlog2 1:
.entry:
  r4 = mul 0 1 32 
  r3 = mul 31 1 32 
  br .for.cond.i 
.for.cond.i:
  r1 = icmp sge r3 0 32 
  br r1 .for.body.i .for.cond.cleanup.i 
.for.cond.cleanup.i:
  br .count_leading_zeros.exit 
.for.body.i:
  r1 = shl 1 r3 32 
  r1 = and arg1 r1 32 
  r1 = icmp ne r1 0 32 
  br r1 .if.then.i .if.end.i 
.if.then.i:
  br .count_leading_zeros.exit 
.if.end.i:
  r1 = add r4 1 32 
  r2 = add r3 4294967295 32 
  r3 = mul r2 1 32 
  r4 = mul r1 1 32 
  br .for.cond.i 
.count_leading_zeros.exit:
  r1 = sub 32 r4 32 
  r1 = sub r1 1 32 
  ret r1 
end floorlog2

start input 2:
.entry:
  r2 = load 8 204800 0 
  r1 = load 4 204800 8 
  r1 = mul arg1 r1 32 
  r1 = add r1 arg2 32 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r1 = mul r1 4 64 
  r1 = add r2 r1 64 
  ret r1 
end input

start width 1:
.entry:
  r2 = load 4 204800 8 
  r1 = shl 1 arg1 32 
  r1 = sub r2 r1 32 
  r1 = add r1 1 32 
  ret r1 
end width

start height 1:
.entry:
  r2 = load 4 204800 16 
  r1 = shl 1 arg1 32 
  r1 = sub r2 r1 32 
  r1 = add r1 1 32 
  ret r1 
end height

start A 2:
.entry:
  r2 = load 8 204800 24 
  r1 = load 4 204800 32 
  r1 = add r1 1 32 
  r1 = mul arg1 r1 32 
  r1 = add r1 arg2 32 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r1 = mul r1 8 64 
  r1 = add r2 r1 64 
  ret r1 
end A

start P 4:
.entry:
  r2 = load 8 204800 24 
  r1 = load 4 204800 32 
  r1 = add r1 1 32 
  r1 = mul arg1 r1 32 
  r1 = add r1 arg2 32 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r1 = mul r1 8 64 
  r1 = add r2 r1 64 
  r3 = load 8 r1 0 
  r2 = load 4 204800 8 
  r1 = shl 1 arg2 32 
  r1 = sub r2 r1 32 
  r1 = add r1 1 32 
  r1 = mul arg3 r1 32 
  r1 = add r1 arg4 32 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r2 = mul r3 1 64 
  r1 = mul r1 4 64 
  r1 = add r2 r1 64 
  r1 = load 4 r1 0 
  ret r1 
end P

start preprocess 0:
.entry:
  r1 = load 4 204800 16 
  r1 = call floorlog2 r1 
  store 4 r1 204800 40 
  r1 = load 4 204800 8 
  r1 = call floorlog2 r1 
  store 4 r1 204800 32 
  r1 = load 4 204800 40 
  r1 = add r1 1 32 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r2 = mul 8 r1 64 
  r1 = load 4 204800 32 
  r1 = add r1 1 32 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r1 = mul r2 r1 64 
  r1 = add r1 7 64 
  r1 = udiv r1 8 64 
  r1 = mul r1 8 64 
  r1 = malloc r1 
  store 8 r1 204800 24 
  r3 = load 8 204800 0 
  r2 = load 8 204800 24 
  r1 = load 4 204800 32 
  r1 = add r1 1 32 
  store 8 r3 r2 0 
  r5 = mul 0 1 32 
  br .for.cond 
.for.cond:
  r1 = load 4 204800 40 
  r1 = icmp sle r5 r1 32 
  br r1 .for.body .for.end26 
.for.body:
  r4 = mul 0 1 32 
  br .for.cond8 
.for.cond8:
  r1 = load 4 204800 32 
  r1 = icmp sle r4 r1 32 
  br r1 .for.body11 .for.end 
.for.body11:
  r1 = icmp eq r5 0 32 
  br r1 .land.lhs.true .if.end 
.land.lhs.true:
  r1 = icmp eq r4 0 32 
  br r1 .if.then .if.end 
.if.then:
  br .for.inc 
.if.end:
  r1 = load 4 204800 16 
  r2 = shl 1 r5 32 
  r1 = sub r1 r2 32 
  r1 = add r1 1 32 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r3 = mul 4 r1 64 
  r2 = load 4 204800 8 
  r1 = shl 1 r4 32 
  r1 = sub r2 r1 32 
  r1 = add r1 1 32 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r1 = mul r3 r1 64 
  r1 = add r1 7 64 
  r1 = udiv r1 8 64 
  r1 = mul r1 8 64 
  r3 = malloc r1 
  r2 = load 8 204800 24 
  r1 = load 4 204800 32 
  r1 = add r1 1 32 
  r1 = mul r5 r1 32 
  r1 = add r1 r4 32 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r1 = mul r1 8 64 
  r1 = add r2 r1 64 
  store 8 r3 r1 0 
  br .for.inc 
.for.inc:
  r1 = add r4 1 32 
  r4 = mul r1 1 32 
  br .for.cond8 
.for.end:
  br .for.inc24 
.for.inc24:
  r1 = add r5 1 32 
  r5 = mul r1 1 32 
  br .for.cond 
.for.end26:
  r10 = mul 0 1 32 
  br .for.cond27 
.for.cond27:
  r1 = load 4 204800 40 
  r1 = icmp sle r10 r1 32 
  br r1 .for.body30 .for.end112 
.for.body30:
  r9 = mul 0 1 32 
  br .for.cond31 
.for.cond31:
  r1 = load 4 204800 32 
  r1 = icmp sle r9 r1 32 
  br r1 .for.body34 .for.end109 
.for.body34:
  r1 = icmp eq r10 0 32 
  br r1 .land.lhs.true37 .if.end41 
.land.lhs.true37:
  r1 = icmp eq r9 0 32 
  br r1 .if.then40 .if.end41 
.if.then40:
  br .for.inc107 
.if.end41:
  r2 = load 8 204800 24 
  r1 = load 4 204800 32 
  r1 = add r1 1 32 
  r1 = mul r10 r1 32 
  r1 = add r1 r9 32 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r1 = mul r1 8 64 
  r1 = add r2 r1 64 
  r8 = load 8 r1 0 
  r7 = mul 0 1 32 
  br .for.cond43 
.for.cond43:
  r1 = call height r10 
  r1 = icmp slt r7 r1 32 
  br r1 .for.body47 .for.cond.cleanup 
.for.cond.cleanup:
  br .for.end106 
.for.body47:
  r6 = mul 0 1 32 
  br .for.cond48 
.for.cond48:
  r2 = load 4 204800 8 
  r1 = shl 1 r9 32 
  r1 = sub r2 r1 32 
  r1 = add r1 1 32 
  r1 = icmp slt r6 r1 32 
  br r1 .for.body53 .for.cond.cleanup52 
.for.cond.cleanup52:
  br .for.end103 
.for.body53:
  r1 = icmp ne r9 0 32 
  br r1 .if.then56 .if.else 
.if.then56:
  r3 = sub r9 1 32 
  r2 = load 8 204800 24 
  r1 = load 4 204800 32 
  r1 = add r1 1 32 
  r1 = mul r10 r1 32 
  r1 = add r1 r3 32 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r1 = mul r1 8 64 
  r1 = add r2 r1 64 
  r5 = load 8 r1 0 
  r1 = sub r9 1 32 
  r2 = load 4 204800 8 
  r1 = shl 1 r1 32 
  r1 = sub r2 r1 32 
  r4 = add r1 1 32 
  r1 = mul r7 r4 32 
  r1 = add r1 r6 32 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r2 = mul r5 1 64 
  r1 = mul r1 4 64 
  r1 = add r2 r1 64 
  r3 = load 4 r1 0 
  r1 = mul r7 r4 32 
  r2 = add r1 r6 32 
  r1 = sub r9 1 32 
  r1 = shl 1 r1 32 
  r1 = add r2 r1 32 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r2 = mul r5 1 64 
  r1 = mul r1 4 64 
  r1 = add r2 r1 64 
  r2 = load 4 r1 0 
  r1 = icmp slt r3 r2 32 
  br r1 .cond.true.i29 .cond.false.i30 
.cond.true.i29:
  br .min.exit32 
.cond.false.i30:
  r3 = mul r2 1 32 
  br .min.exit32 
.min.exit32:
  r2 = load 4 204800 8 
  r1 = shl 1 r9 32 
  r1 = sub r2 r1 32 
  r1 = add r1 1 32 
  r1 = mul r7 r1 32 
  r1 = add r1 r6 32 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r2 = mul r8 1 64 
  r1 = mul r1 4 64 
  r1 = add r2 r1 64 
  store 4 r3 r1 0 
  br .if.end100 
.if.else:
  r1 = sub r10 1 32 
  r1 = call A r1 r9 
  r4 = load 8 r1 0 
  r1 = sub r10 1 32 
  r1 = call height r1 
  r1 = call width r9 
  r1 = mul r7 r1 32 
  r1 = add r1 r6 32 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r2 = mul r4 1 64 
  r1 = mul r1 4 64 
  r1 = add r2 r1 64 
  r3 = load 4 r1 0 
  r1 = sub r10 1 32 
  r1 = shl 1 r1 32 
  r2 = add r7 r1 32 
  r1 = call width r9 
  r1 = mul r2 r1 32 
  r1 = add r1 r6 32 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r2 = mul r4 1 64 
  r1 = mul r1 4 64 
  r1 = add r2 r1 64 
  r2 = load 4 r1 0 
  r1 = icmp slt r3 r2 32 
  br r1 .cond.true.i .cond.false.i 
.cond.true.i:
  br .min.exit 
.cond.false.i:
  r3 = mul r2 1 32 
  br .min.exit 
.min.exit:
  r1 = call width r9 
  r1 = mul r7 r1 32 
  r1 = add r1 r6 32 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r2 = mul r8 1 64 
  r1 = mul r1 4 64 
  r1 = add r2 r1 64 
  store 4 r3 r1 0 
  br .if.end100 
.if.end100:
  br .for.inc101 
.for.inc101:
  r1 = add r6 1 32 
  r6 = mul r1 1 32 
  br .for.cond48 
.for.end103:
  br .for.inc104 
.for.inc104:
  r1 = add r7 1 32 
  r7 = mul r1 1 32 
  br .for.cond43 
.for.end106:
  br .for.inc107 
.for.inc107:
  r1 = add r9 1 32 
  r9 = mul r1 1 32 
  br .for.cond31 
.for.end109:
  br .for.inc110 
.for.inc110:
  r1 = add r10 1 32 
  r10 = mul r1 1 32 
  br .for.cond27 
.for.end112:
  ret 0 
end preprocess

start main 0:
.entry:
  ; Init global variables
  r1 = malloc 8 
  r1 = malloc 8 
  r1 = malloc 8 
  r1 = malloc 8 
  r1 = malloc 8 
  r1 = malloc 8 
  r1 = call read 
  store 4 r1 204800 16 
  r1 = call read 
  store 4 r1 204800 8 
  r1 = load 4 204800 16 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r2 = mul 4 r1 64 
  r1 = load 4 204800 8 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r1 = mul r2 r1 64 
  r1 = add r1 7 64 
  r1 = udiv r1 8 64 
  r1 = mul r1 8 64 
  r1 = malloc r1 
  store 8 r1 204800 0 
  r5 = mul 0 1 32 
  br .for.cond 
.for.cond:
  r1 = load 4 204800 16 
  r1 = icmp slt r5 r1 32 
  br r1 .for.body .for.cond.cleanup 
.for.cond.cleanup:
  br .for.end18 
.for.body:
  r4 = mul 0 1 32 
  br .for.cond8 
.for.cond8:
  r1 = load 4 204800 8 
  r1 = icmp slt r4 r1 32 
  br r1 .for.body12 .for.cond.cleanup11 
.for.cond.cleanup11:
  br .for.end 
.for.body12:
  r1 = call read 
  r3 = mul r1 1 32 
  r2 = load 8 204800 0 
  r1 = load 4 204800 8 
  r1 = mul r5 r1 32 
  r1 = add r1 r4 32 
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  r1 = mul r1 4 64 
  r1 = add r2 r1 64 
  store 4 r3 r1 0 
  br .for.inc 
.for.inc:
  r1 = add r4 1 32 
  r4 = mul r1 1 32 
  br .for.cond8 
.for.end:
  br .for.inc16 
.for.inc16:
  r1 = add r5 1 32 
  r5 = mul r1 1 32 
  br .for.cond 
.for.end18:
  call preprocess 
  r1 = call read 
  br .while.cond 
.while.cond:
  r13 = add r1 4294967295 32 
  r1 = icmp ne r1 0 32 
  br r1 .while.body .while.end 
.while.body:
  r1 = call read 
  r12 = mul r1 1 32 
  r1 = call read 
  r11 = mul r1 1 32 
  r1 = call read 
  r10 = mul r1 1 32 
  r1 = call read 
  r9 = mul r1 1 32 
  r1 = sub r11 r12 32 
  r4 = add r1 1 32 
  r5 = mul 0 1 32 
  r3 = mul 31 1 32 
  br .for.cond.i.i 
.for.cond.i.i:
  r1 = icmp sge r3 0 32 
  br r1 .for.body.i.i .for.cond.cleanup.i.i 
.for.cond.cleanup.i.i:
  br .floorlog2.exit 
.for.body.i.i:
  r1 = shl 1 r3 32 
  r1 = and r4 r1 32 
  r1 = icmp ne r1 0 32 
  br r1 .if.then.i.i .if.end.i.i 
.if.then.i.i:
  br .floorlog2.exit 
.if.end.i.i:
  r2 = add r5 1 32 
  r1 = add r3 4294967295 32 
  r3 = mul r1 1 32 
  r5 = mul r2 1 32 
  br .for.cond.i.i 
.floorlog2.exit:
  r1 = sub 32 r5 32 
  r8 = sub r1 1 32 
  r1 = sub r9 r10 32 
  r5 = add r1 1 32 
  r4 = mul 0 1 32 
  r3 = mul 31 1 32 
  br .for.cond.i.i6 
.for.cond.i.i6:
  r1 = icmp sge r3 0 32 
  br r1 .for.body.i.i11 .for.cond.cleanup.i.i7 
.for.cond.cleanup.i.i7:
  br .floorlog2.exit18 
.for.body.i.i11:
  r1 = shl 1 r3 32 
  r1 = and r5 r1 32 
  r1 = icmp ne r1 0 32 
  br r1 .if.then.i.i12 .if.end.i.i15 
.if.then.i.i12:
  br .floorlog2.exit18 
.if.end.i.i15:
  r2 = add r4 1 32 
  r1 = add r3 4294967295 32 
  r3 = mul r1 1 32 
  r4 = mul r2 1 32 
  br .for.cond.i.i6 
.floorlog2.exit18:
  r1 = sub 32 r4 32 
  r6 = sub r1 1 32 
  r7 = call P r8 r6 r12 r10 
  r2 = add r11 1 32 
  r1 = shl 1 r8 32 
  r1 = sub r2 r1 32 
  r5 = call P r8 r6 r1 r10 
  r2 = add r9 1 32 
  r1 = shl 1 r6 32 
  r1 = sub r2 r1 32 
  r4 = call P r8 r6 r12 r1 
  r2 = add r11 1 32 
  r1 = shl 1 r8 32 
  r3 = sub r2 r1 32 
  r2 = add r9 1 32 
  r1 = shl 1 r6 32 
  r1 = sub r2 r1 32 
  r2 = call P r8 r6 r3 r1 
  r1 = icmp slt r7 r5 32 
  br r1 .cond.true.i20 .cond.false.i21 
.cond.true.i20:
  r3 = mul r7 1 32 
  br .min.exit23 
.cond.false.i21:
  r3 = mul r5 1 32 
  br .min.exit23 
.min.exit23:
  r1 = icmp slt r4 r2 32 
  br r1 .cond.true.i25 .cond.false.i26 
.cond.true.i25:
  r1 = mul r4 1 32 
  br .min.exit28 
.cond.false.i26:
  r1 = mul r2 1 32 
  br .min.exit28 
.min.exit28:
  r2 = icmp slt r3 r1 32 
  br r2 .cond.true.i .cond.false.i 
.cond.true.i:
  r1 = mul r3 1 32 
  br .min.exit 
.cond.false.i:
  br .min.exit 
.min.exit:
  r1 = mul r1 4294967296 64 
  r1 = sdiv r1 4294967296 64 
  call write r1 
  r1 = mul r13 1 32 
  br .while.cond 
.while.end:
  ret 0 
end main

