; ModuleID = '/tmp/a.ll'
source_filename = "matmul3/src/matmul3.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx11.0.0"

; Function Attrs: nounwind ssp uwtable
define dso_local void @matmul(i32 %dim, i64* %c, i64* %a, i64* %b) #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc423, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %add424, %for.inc423 ]
  %cmp = icmp ult i32 %i.0, %dim
  br i1 %cmp, label %for.body, label %for.end425

for.body:                                         ; preds = %for.cond
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc420, %for.body
  %j.0 = phi i32 [ 0, %for.body ], [ %add421, %for.inc420 ]
  %cmp2 = icmp ult i32 %j.0, %dim
  br i1 %cmp2, label %for.body3, label %for.end422

for.body3:                                        ; preds = %for.cond1
  br label %for.cond4

for.cond4:                                        ; preds = %for.inc, %for.body3
  %k.0 = phi i32 [ 0, %for.body3 ], [ %add419, %for.inc ]
  %cmp5 = icmp ult i32 %k.0, %dim
  br i1 %cmp5, label %for.body6, label %for.end

for.body6:                                        ; preds = %for.cond4
  %add = add i32 %i.0, 0
  %mul = mul i32 %add, %dim
  %add7 = add i32 %mul, %k.0
  %add8 = add i32 %add7, 0
  %idxprom = zext i32 %add8 to i64
  %arrayidx = getelementptr inbounds i64, i64* %a, i64 %idxprom
  %0 = load i64, i64* %arrayidx, align 8
  %add9 = add i32 %i.0, 0
  %mul10 = mul i32 %add9, %dim
  %add11 = add i32 %mul10, %k.0
  %add12 = add i32 %add11, 1
  %idxprom13 = zext i32 %add12 to i64
  %arrayidx14 = getelementptr inbounds i64, i64* %a, i64 %idxprom13
  %1 = load i64, i64* %arrayidx14, align 8
  %add15 = add i32 %i.0, 0
  %mul16 = mul i32 %add15, %dim
  %add17 = add i32 %mul16, %k.0
  %add18 = add i32 %add17, 2
  %idxprom19 = zext i32 %add18 to i64
  %arrayidx20 = getelementptr inbounds i64, i64* %a, i64 %idxprom19
  %2 = load i64, i64* %arrayidx20, align 8
  %add21 = add i32 %i.0, 0
  %mul22 = mul i32 %add21, %dim
  %add23 = add i32 %mul22, %k.0
  %add24 = add i32 %add23, 3
  %idxprom25 = zext i32 %add24 to i64
  %arrayidx26 = getelementptr inbounds i64, i64* %a, i64 %idxprom25
  %3 = load i64, i64* %arrayidx26, align 8
  %add27 = add i32 %i.0, 1
  %mul28 = mul i32 %add27, %dim
  %add29 = add i32 %mul28, %k.0
  %add30 = add i32 %add29, 0
  %idxprom31 = zext i32 %add30 to i64
  %arrayidx32 = getelementptr inbounds i64, i64* %a, i64 %idxprom31
  %4 = load i64, i64* %arrayidx32, align 8
  %add33 = add i32 %i.0, 1
  %mul34 = mul i32 %add33, %dim
  %add35 = add i32 %mul34, %k.0
  %add36 = add i32 %add35, 1
  %idxprom37 = zext i32 %add36 to i64
  %arrayidx38 = getelementptr inbounds i64, i64* %a, i64 %idxprom37
  %5 = load i64, i64* %arrayidx38, align 8
  %add39 = add i32 %i.0, 1
  %mul40 = mul i32 %add39, %dim
  %add41 = add i32 %mul40, %k.0
  %add42 = add i32 %add41, 2
  %idxprom43 = zext i32 %add42 to i64
  %arrayidx44 = getelementptr inbounds i64, i64* %a, i64 %idxprom43
  %6 = load i64, i64* %arrayidx44, align 8
  %add45 = add i32 %i.0, 1
  %mul46 = mul i32 %add45, %dim
  %add47 = add i32 %mul46, %k.0
  %add48 = add i32 %add47, 3
  %idxprom49 = zext i32 %add48 to i64
  %arrayidx50 = getelementptr inbounds i64, i64* %a, i64 %idxprom49
  %7 = load i64, i64* %arrayidx50, align 8
  %add51 = add i32 %i.0, 2
  %mul52 = mul i32 %add51, %dim
  %add53 = add i32 %mul52, %k.0
  %add54 = add i32 %add53, 0
  %idxprom55 = zext i32 %add54 to i64
  %arrayidx56 = getelementptr inbounds i64, i64* %a, i64 %idxprom55
  %8 = load i64, i64* %arrayidx56, align 8
  %add57 = add i32 %i.0, 2
  %mul58 = mul i32 %add57, %dim
  %add59 = add i32 %mul58, %k.0
  %add60 = add i32 %add59, 1
  %idxprom61 = zext i32 %add60 to i64
  %arrayidx62 = getelementptr inbounds i64, i64* %a, i64 %idxprom61
  %9 = load i64, i64* %arrayidx62, align 8
  %add63 = add i32 %i.0, 2
  %mul64 = mul i32 %add63, %dim
  %add65 = add i32 %mul64, %k.0
  %add66 = add i32 %add65, 2
  %idxprom67 = zext i32 %add66 to i64
  %arrayidx68 = getelementptr inbounds i64, i64* %a, i64 %idxprom67
  %10 = load i64, i64* %arrayidx68, align 8
  %add69 = add i32 %i.0, 2
  %mul70 = mul i32 %add69, %dim
  %add71 = add i32 %mul70, %k.0
  %add72 = add i32 %add71, 3
  %idxprom73 = zext i32 %add72 to i64
  %arrayidx74 = getelementptr inbounds i64, i64* %a, i64 %idxprom73
  %11 = load i64, i64* %arrayidx74, align 8
  %add75 = add i32 %i.0, 3
  %mul76 = mul i32 %add75, %dim
  %add77 = add i32 %mul76, %k.0
  %add78 = add i32 %add77, 0
  %idxprom79 = zext i32 %add78 to i64
  %arrayidx80 = getelementptr inbounds i64, i64* %a, i64 %idxprom79
  %12 = load i64, i64* %arrayidx80, align 8
  %add81 = add i32 %i.0, 3
  %mul82 = mul i32 %add81, %dim
  %add83 = add i32 %mul82, %k.0
  %add84 = add i32 %add83, 1
  %idxprom85 = zext i32 %add84 to i64
  %arrayidx86 = getelementptr inbounds i64, i64* %a, i64 %idxprom85
  %13 = load i64, i64* %arrayidx86, align 8
  %add87 = add i32 %i.0, 3
  %mul88 = mul i32 %add87, %dim
  %add89 = add i32 %mul88, %k.0
  %add90 = add i32 %add89, 2
  %idxprom91 = zext i32 %add90 to i64
  %arrayidx92 = getelementptr inbounds i64, i64* %a, i64 %idxprom91
  %14 = load i64, i64* %arrayidx92, align 8
  %add93 = add i32 %i.0, 3
  %mul94 = mul i32 %add93, %dim
  %add95 = add i32 %mul94, %k.0
  %add96 = add i32 %add95, 3
  %idxprom97 = zext i32 %add96 to i64
  %arrayidx98 = getelementptr inbounds i64, i64* %a, i64 %idxprom97
  %15 = load i64, i64* %arrayidx98, align 8
  %add99 = add i32 %k.0, 0
  %mul100 = mul i32 %add99, %dim
  %add101 = add i32 %mul100, %j.0
  %add102 = add i32 %add101, 0
  %idxprom103 = zext i32 %add102 to i64
  %arrayidx104 = getelementptr inbounds i64, i64* %b, i64 %idxprom103
  %16 = load i64, i64* %arrayidx104, align 8
  %add105 = add i32 %k.0, 0
  %mul106 = mul i32 %add105, %dim
  %add107 = add i32 %mul106, %j.0
  %add108 = add i32 %add107, 1
  %idxprom109 = zext i32 %add108 to i64
  %arrayidx110 = getelementptr inbounds i64, i64* %b, i64 %idxprom109
  %17 = load i64, i64* %arrayidx110, align 8
  %add111 = add i32 %k.0, 0
  %mul112 = mul i32 %add111, %dim
  %add113 = add i32 %mul112, %j.0
  %add114 = add i32 %add113, 2
  %idxprom115 = zext i32 %add114 to i64
  %arrayidx116 = getelementptr inbounds i64, i64* %b, i64 %idxprom115
  %18 = load i64, i64* %arrayidx116, align 8
  %add117 = add i32 %k.0, 0
  %mul118 = mul i32 %add117, %dim
  %add119 = add i32 %mul118, %j.0
  %add120 = add i32 %add119, 3
  %idxprom121 = zext i32 %add120 to i64
  %arrayidx122 = getelementptr inbounds i64, i64* %b, i64 %idxprom121
  %19 = load i64, i64* %arrayidx122, align 8
  %add123 = add i32 %k.0, 1
  %mul124 = mul i32 %add123, %dim
  %add125 = add i32 %mul124, %j.0
  %add126 = add i32 %add125, 0
  %idxprom127 = zext i32 %add126 to i64
  %arrayidx128 = getelementptr inbounds i64, i64* %b, i64 %idxprom127
  %20 = load i64, i64* %arrayidx128, align 8
  %add129 = add i32 %k.0, 1
  %mul130 = mul i32 %add129, %dim
  %add131 = add i32 %mul130, %j.0
  %add132 = add i32 %add131, 1
  %idxprom133 = zext i32 %add132 to i64
  %arrayidx134 = getelementptr inbounds i64, i64* %b, i64 %idxprom133
  %21 = load i64, i64* %arrayidx134, align 8
  %add135 = add i32 %k.0, 1
  %mul136 = mul i32 %add135, %dim
  %add137 = add i32 %mul136, %j.0
  %add138 = add i32 %add137, 2
  %idxprom139 = zext i32 %add138 to i64
  %arrayidx140 = getelementptr inbounds i64, i64* %b, i64 %idxprom139
  %22 = load i64, i64* %arrayidx140, align 8
  %add141 = add i32 %k.0, 1
  %mul142 = mul i32 %add141, %dim
  %add143 = add i32 %mul142, %j.0
  %add144 = add i32 %add143, 3
  %idxprom145 = zext i32 %add144 to i64
  %arrayidx146 = getelementptr inbounds i64, i64* %b, i64 %idxprom145
  %23 = load i64, i64* %arrayidx146, align 8
  %add147 = add i32 %k.0, 2
  %mul148 = mul i32 %add147, %dim
  %add149 = add i32 %mul148, %j.0
  %add150 = add i32 %add149, 0
  %idxprom151 = zext i32 %add150 to i64
  %arrayidx152 = getelementptr inbounds i64, i64* %b, i64 %idxprom151
  %24 = load i64, i64* %arrayidx152, align 8
  %add153 = add i32 %k.0, 2
  %mul154 = mul i32 %add153, %dim
  %add155 = add i32 %mul154, %j.0
  %add156 = add i32 %add155, 1
  %idxprom157 = zext i32 %add156 to i64
  %arrayidx158 = getelementptr inbounds i64, i64* %b, i64 %idxprom157
  %25 = load i64, i64* %arrayidx158, align 8
  %add159 = add i32 %k.0, 2
  %mul160 = mul i32 %add159, %dim
  %add161 = add i32 %mul160, %j.0
  %add162 = add i32 %add161, 2
  %idxprom163 = zext i32 %add162 to i64
  %arrayidx164 = getelementptr inbounds i64, i64* %b, i64 %idxprom163
  %26 = load i64, i64* %arrayidx164, align 8
  %add165 = add i32 %k.0, 2
  %mul166 = mul i32 %add165, %dim
  %add167 = add i32 %mul166, %j.0
  %add168 = add i32 %add167, 3
  %idxprom169 = zext i32 %add168 to i64
  %arrayidx170 = getelementptr inbounds i64, i64* %b, i64 %idxprom169
  %27 = load i64, i64* %arrayidx170, align 8
  %add171 = add i32 %k.0, 3
  %mul172 = mul i32 %add171, %dim
  %add173 = add i32 %mul172, %j.0
  %add174 = add i32 %add173, 0
  %idxprom175 = zext i32 %add174 to i64
  %arrayidx176 = getelementptr inbounds i64, i64* %b, i64 %idxprom175
  %28 = load i64, i64* %arrayidx176, align 8
  %add177 = add i32 %k.0, 3
  %mul178 = mul i32 %add177, %dim
  %add179 = add i32 %mul178, %j.0
  %add180 = add i32 %add179, 1
  %idxprom181 = zext i32 %add180 to i64
  %arrayidx182 = getelementptr inbounds i64, i64* %b, i64 %idxprom181
  %29 = load i64, i64* %arrayidx182, align 8
  %add183 = add i32 %k.0, 3
  %mul184 = mul i32 %add183, %dim
  %add185 = add i32 %mul184, %j.0
  %add186 = add i32 %add185, 2
  %idxprom187 = zext i32 %add186 to i64
  %arrayidx188 = getelementptr inbounds i64, i64* %b, i64 %idxprom187
  %30 = load i64, i64* %arrayidx188, align 8
  %add189 = add i32 %k.0, 3
  %mul190 = mul i32 %add189, %dim
  %add191 = add i32 %mul190, %j.0
  %add192 = add i32 %add191, 3
  %idxprom193 = zext i32 %add192 to i64
  %arrayidx194 = getelementptr inbounds i64, i64* %b, i64 %idxprom193
  %31 = load i64, i64* %arrayidx194, align 8
  %mul195 = mul i64 %0, %16
  %mul196 = mul i64 %1, %20
  %add197 = add i64 %mul195, %mul196
  %mul198 = mul i64 %2, %24
  %add199 = add i64 %add197, %mul198
  %mul200 = mul i64 %3, %28
  %add201 = add i64 %add199, %mul200
  %add202 = add i32 %i.0, 0
  %mul203 = mul i32 %add202, %dim
  %add204 = add i32 %mul203, %j.0
  %add205 = add i32 %add204, 0
  %idxprom206 = zext i32 %add205 to i64
  %arrayidx207 = getelementptr inbounds i64, i64* %c, i64 %idxprom206
  %32 = load i64, i64* %arrayidx207, align 8
  %add208 = add i64 %32, %add201
  store i64 %add208, i64* %arrayidx207, align 8
  %mul209 = mul i64 %0, %17
  %mul210 = mul i64 %1, %21
  %add211 = add i64 %mul209, %mul210
  %mul212 = mul i64 %2, %25
  %add213 = add i64 %add211, %mul212
  %mul214 = mul i64 %3, %29
  %add215 = add i64 %add213, %mul214
  %add216 = add i32 %i.0, 0
  %mul217 = mul i32 %add216, %dim
  %add218 = add i32 %mul217, %j.0
  %add219 = add i32 %add218, 1
  %idxprom220 = zext i32 %add219 to i64
  %arrayidx221 = getelementptr inbounds i64, i64* %c, i64 %idxprom220
  %33 = load i64, i64* %arrayidx221, align 8
  %add222 = add i64 %33, %add215
  store i64 %add222, i64* %arrayidx221, align 8
  %mul223 = mul i64 %0, %18
  %mul224 = mul i64 %1, %22
  %add225 = add i64 %mul223, %mul224
  %mul226 = mul i64 %2, %26
  %add227 = add i64 %add225, %mul226
  %mul228 = mul i64 %3, %30
  %add229 = add i64 %add227, %mul228
  %add230 = add i32 %i.0, 0
  %mul231 = mul i32 %add230, %dim
  %add232 = add i32 %mul231, %j.0
  %add233 = add i32 %add232, 2
  %idxprom234 = zext i32 %add233 to i64
  %arrayidx235 = getelementptr inbounds i64, i64* %c, i64 %idxprom234
  %34 = load i64, i64* %arrayidx235, align 8
  %add236 = add i64 %34, %add229
  store i64 %add236, i64* %arrayidx235, align 8
  %mul237 = mul i64 %0, %19
  %mul238 = mul i64 %1, %23
  %add239 = add i64 %mul237, %mul238
  %mul240 = mul i64 %2, %27
  %add241 = add i64 %add239, %mul240
  %mul242 = mul i64 %3, %31
  %add243 = add i64 %add241, %mul242
  %add244 = add i32 %i.0, 0
  %mul245 = mul i32 %add244, %dim
  %add246 = add i32 %mul245, %j.0
  %add247 = add i32 %add246, 3
  %idxprom248 = zext i32 %add247 to i64
  %arrayidx249 = getelementptr inbounds i64, i64* %c, i64 %idxprom248
  %35 = load i64, i64* %arrayidx249, align 8
  %add250 = add i64 %35, %add243
  store i64 %add250, i64* %arrayidx249, align 8
  %mul251 = mul i64 %4, %16
  %mul252 = mul i64 %5, %20
  %add253 = add i64 %mul251, %mul252
  %mul254 = mul i64 %6, %24
  %add255 = add i64 %add253, %mul254
  %mul256 = mul i64 %7, %28
  %add257 = add i64 %add255, %mul256
  %add258 = add i32 %i.0, 1
  %mul259 = mul i32 %add258, %dim
  %add260 = add i32 %mul259, %j.0
  %add261 = add i32 %add260, 0
  %idxprom262 = zext i32 %add261 to i64
  %arrayidx263 = getelementptr inbounds i64, i64* %c, i64 %idxprom262
  %36 = load i64, i64* %arrayidx263, align 8
  %add264 = add i64 %36, %add257
  store i64 %add264, i64* %arrayidx263, align 8
  %mul265 = mul i64 %4, %17
  %mul266 = mul i64 %5, %21
  %add267 = add i64 %mul265, %mul266
  %mul268 = mul i64 %6, %25
  %add269 = add i64 %add267, %mul268
  %mul270 = mul i64 %7, %29
  %add271 = add i64 %add269, %mul270
  %add272 = add i32 %i.0, 1
  %mul273 = mul i32 %add272, %dim
  %add274 = add i32 %mul273, %j.0
  %add275 = add i32 %add274, 1
  %idxprom276 = zext i32 %add275 to i64
  %arrayidx277 = getelementptr inbounds i64, i64* %c, i64 %idxprom276
  %37 = load i64, i64* %arrayidx277, align 8
  %add278 = add i64 %37, %add271
  store i64 %add278, i64* %arrayidx277, align 8
  %mul279 = mul i64 %4, %18
  %mul280 = mul i64 %5, %22
  %add281 = add i64 %mul279, %mul280
  %mul282 = mul i64 %6, %26
  %add283 = add i64 %add281, %mul282
  %mul284 = mul i64 %7, %30
  %add285 = add i64 %add283, %mul284
  %add286 = add i32 %i.0, 1
  %mul287 = mul i32 %add286, %dim
  %add288 = add i32 %mul287, %j.0
  %add289 = add i32 %add288, 2
  %idxprom290 = zext i32 %add289 to i64
  %arrayidx291 = getelementptr inbounds i64, i64* %c, i64 %idxprom290
  %38 = load i64, i64* %arrayidx291, align 8
  %add292 = add i64 %38, %add285
  store i64 %add292, i64* %arrayidx291, align 8
  %mul293 = mul i64 %4, %19
  %mul294 = mul i64 %5, %23
  %add295 = add i64 %mul293, %mul294
  %mul296 = mul i64 %6, %27
  %add297 = add i64 %add295, %mul296
  %mul298 = mul i64 %7, %31
  %add299 = add i64 %add297, %mul298
  %add300 = add i32 %i.0, 1
  %mul301 = mul i32 %add300, %dim
  %add302 = add i32 %mul301, %j.0
  %add303 = add i32 %add302, 3
  %idxprom304 = zext i32 %add303 to i64
  %arrayidx305 = getelementptr inbounds i64, i64* %c, i64 %idxprom304
  %39 = load i64, i64* %arrayidx305, align 8
  %add306 = add i64 %39, %add299
  store i64 %add306, i64* %arrayidx305, align 8
  %mul307 = mul i64 %8, %16
  %mul308 = mul i64 %9, %20
  %add309 = add i64 %mul307, %mul308
  %mul310 = mul i64 %10, %24
  %add311 = add i64 %add309, %mul310
  %mul312 = mul i64 %11, %28
  %add313 = add i64 %add311, %mul312
  %add314 = add i32 %i.0, 2
  %mul315 = mul i32 %add314, %dim
  %add316 = add i32 %mul315, %j.0
  %add317 = add i32 %add316, 0
  %idxprom318 = zext i32 %add317 to i64
  %arrayidx319 = getelementptr inbounds i64, i64* %c, i64 %idxprom318
  %40 = load i64, i64* %arrayidx319, align 8
  %add320 = add i64 %40, %add313
  store i64 %add320, i64* %arrayidx319, align 8
  %mul321 = mul i64 %8, %17
  %mul322 = mul i64 %9, %21
  %add323 = add i64 %mul321, %mul322
  %mul324 = mul i64 %10, %25
  %add325 = add i64 %add323, %mul324
  %mul326 = mul i64 %11, %29
  %add327 = add i64 %add325, %mul326
  %add328 = add i32 %i.0, 2
  %mul329 = mul i32 %add328, %dim
  %add330 = add i32 %mul329, %j.0
  %add331 = add i32 %add330, 1
  %idxprom332 = zext i32 %add331 to i64
  %arrayidx333 = getelementptr inbounds i64, i64* %c, i64 %idxprom332
  %41 = load i64, i64* %arrayidx333, align 8
  %add334 = add i64 %41, %add327
  store i64 %add334, i64* %arrayidx333, align 8
  %mul335 = mul i64 %8, %18
  %mul336 = mul i64 %9, %22
  %add337 = add i64 %mul335, %mul336
  %mul338 = mul i64 %10, %26
  %add339 = add i64 %add337, %mul338
  %mul340 = mul i64 %11, %30
  %add341 = add i64 %add339, %mul340
  %add342 = add i32 %i.0, 2
  %mul343 = mul i32 %add342, %dim
  %add344 = add i32 %mul343, %j.0
  %add345 = add i32 %add344, 2
  %idxprom346 = zext i32 %add345 to i64
  %arrayidx347 = getelementptr inbounds i64, i64* %c, i64 %idxprom346
  %42 = load i64, i64* %arrayidx347, align 8
  %add348 = add i64 %42, %add341
  store i64 %add348, i64* %arrayidx347, align 8
  %mul349 = mul i64 %8, %19
  %mul350 = mul i64 %9, %23
  %add351 = add i64 %mul349, %mul350
  %mul352 = mul i64 %10, %27
  %add353 = add i64 %add351, %mul352
  %mul354 = mul i64 %11, %31
  %add355 = add i64 %add353, %mul354
  %add356 = add i32 %i.0, 2
  %mul357 = mul i32 %add356, %dim
  %add358 = add i32 %mul357, %j.0
  %add359 = add i32 %add358, 3
  %idxprom360 = zext i32 %add359 to i64
  %arrayidx361 = getelementptr inbounds i64, i64* %c, i64 %idxprom360
  %43 = load i64, i64* %arrayidx361, align 8
  %add362 = add i64 %43, %add355
  store i64 %add362, i64* %arrayidx361, align 8
  %mul363 = mul i64 %12, %16
  %mul364 = mul i64 %13, %20
  %add365 = add i64 %mul363, %mul364
  %mul366 = mul i64 %14, %24
  %add367 = add i64 %add365, %mul366
  %mul368 = mul i64 %15, %28
  %add369 = add i64 %add367, %mul368
  %add370 = add i32 %i.0, 3
  %mul371 = mul i32 %add370, %dim
  %add372 = add i32 %mul371, %j.0
  %add373 = add i32 %add372, 0
  %idxprom374 = zext i32 %add373 to i64
  %arrayidx375 = getelementptr inbounds i64, i64* %c, i64 %idxprom374
  %44 = load i64, i64* %arrayidx375, align 8
  %add376 = add i64 %44, %add369
  store i64 %add376, i64* %arrayidx375, align 8
  %mul377 = mul i64 %12, %17
  %mul378 = mul i64 %13, %21
  %add379 = add i64 %mul377, %mul378
  %mul380 = mul i64 %14, %25
  %add381 = add i64 %add379, %mul380
  %mul382 = mul i64 %15, %29
  %add383 = add i64 %add381, %mul382
  %add384 = add i32 %i.0, 3
  %mul385 = mul i32 %add384, %dim
  %add386 = add i32 %mul385, %j.0
  %add387 = add i32 %add386, 1
  %idxprom388 = zext i32 %add387 to i64
  %arrayidx389 = getelementptr inbounds i64, i64* %c, i64 %idxprom388
  %45 = load i64, i64* %arrayidx389, align 8
  %add390 = add i64 %45, %add383
  store i64 %add390, i64* %arrayidx389, align 8
  %mul391 = mul i64 %12, %18
  %mul392 = mul i64 %13, %22
  %add393 = add i64 %mul391, %mul392
  %mul394 = mul i64 %14, %26
  %add395 = add i64 %add393, %mul394
  %mul396 = mul i64 %15, %30
  %add397 = add i64 %add395, %mul396
  %add398 = add i32 %i.0, 3
  %mul399 = mul i32 %add398, %dim
  %add400 = add i32 %mul399, %j.0
  %add401 = add i32 %add400, 2
  %idxprom402 = zext i32 %add401 to i64
  %arrayidx403 = getelementptr inbounds i64, i64* %c, i64 %idxprom402
  %46 = load i64, i64* %arrayidx403, align 8
  %add404 = add i64 %46, %add397
  store i64 %add404, i64* %arrayidx403, align 8
  %mul405 = mul i64 %12, %19
  %mul406 = mul i64 %13, %23
  %add407 = add i64 %mul405, %mul406
  %mul408 = mul i64 %14, %27
  %add409 = add i64 %add407, %mul408
  %mul410 = mul i64 %15, %31
  %add411 = add i64 %add409, %mul410
  %add412 = add i32 %i.0, 3
  %mul413 = mul i32 %add412, %dim
  %add414 = add i32 %mul413, %j.0
  %add415 = add i32 %add414, 3
  %idxprom416 = zext i32 %add415 to i64
  %arrayidx417 = getelementptr inbounds i64, i64* %c, i64 %idxprom416
  %47 = load i64, i64* %arrayidx417, align 8
  %add418 = add i64 %47, %add411
  store i64 %add418, i64* %arrayidx417, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body6
  %add419 = add i32 %k.0, 4
  br label %for.cond4, !llvm.loop !4

for.end:                                          ; preds = %for.cond4
  br label %for.inc420

for.inc420:                                       ; preds = %for.end
  %add421 = add i32 %j.0, 4
  br label %for.cond1, !llvm.loop !7

for.end422:                                       ; preds = %for.cond1
  br label %for.inc423

for.inc423:                                       ; preds = %for.end422
  %add424 = add i32 %i.0, 4
  br label %for.cond, !llvm.loop !8

for.end425:                                       ; preds = %for.cond
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
