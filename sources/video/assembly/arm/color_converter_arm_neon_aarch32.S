//
//
//  IPV-MediaEngine
//  color_converter_arm_neon.s
//  Created by Rajib Chandra Das on 7/26/17.
//
//

#ifdef HAVE_NEON

#ifdef ASSEMBLY_IOS
.macro NEON_ARM_FUNC_BEGIN
.syntax unified
.text
.extern printf
.align 2
.arm
.globl _$0
_$0:
.endm
#endif


#ifdef ASSEMBLY_ANDROID
.macro NEON_ARM_FUNC_BEGIN func_name
.syntax unified
.text
.extern printf
.align 2
.arm
.global \func_name
\func_name:
.endm
#endif



.macro NEON_ARM_FUNC_END
mov pc, lr
.endm


NEON_ARM_FUNC_BEGIN convert_nv12_to_i420_arm_neon
# r0: Ptr to source data
# r1: Ptr to destination data
# r2: height
# r3: width
push {r4-r8,lr}
mul r4, r2, r3
mov r5, r4

### Y-Values
ands r8, r4, #7
beq .convert_nv12_to_i420_arm_neon_loop_Y
vld1.u8 {d0}, [r0], r8
vst1.u8 {d0}, [r1], r8
subs r4, r4, r8

.convert_nv12_to_i420_arm_neon_loop_Y:
vld1.u8  {d0}, [r0]!
vst1.u8  {d0}, [r1]!
subs    r4, r4, #8
bne  .convert_nv12_to_i420_arm_neon_loop_Y

mov r6, r0
mov r7, r0

### U-Values
lsrs r4, r5, #1
ands r8, r4, #15
beq .convert_nv12_to_i420_arm_neon_loop_U
vld2.u8 {d0,d1}, [r6], r8
subs r4, r4, r8
lsrs r8, r8, #1
vst1.u8 {d0}, [r1], r8

.convert_nv12_to_i420_arm_neon_loop_U:
vld2.u8 {d0, d1}, [r6]!
vst1.u8 {d0}, [r1]!
subs    r4, r4, #16
bne  .convert_nv12_to_i420_arm_neon_loop_U


### V-Values
lsrs r4, r5, #1
ands r8, r4, #15
beq .convert_nv12_to_i420_arm_neon_loop_V
vld2.u8 {d0,d1}, [r7], r8
subs r4, r4, r8
lsrs r8, r8, #1
vst1.u8 {d1}, [r1], r8

.convert_nv12_to_i420_arm_neon_loop_V:
vld2.u8 {d0, d1}, [r7]!
vst1.u8 {d1}, [r1]!
subs    r4, r4, #16
bne  .convert_nv12_to_i420_arm_neon_loop_V

pop { r4-r8, pc }

NEON_ARM_FUNC_END

#ifdef ASSEMBLY_IOS
.section	__TEXT,__cstring,cstring_literals
output:                               ; @.str
.asciz	"Is this ok.. The Value FF is: %d Yes I am done\n"
#endif

NEON_ARM_FUNC_BEGIN crop_yuv420_arm_neon
#r0 src data
#r1 dst data
#r2 parameters sequence: { inHeight, inWidth, startXDiff, endXDiff, startYDiff, endYDiff, outHeight, outWidth}
push {r3-r11,lr}
vld1.u32 {d0}, [r2]!
vmov r3, r4, d0
vld1.u32 {d1}, [r2]!
vmov r5, r6, d1
vld1.u32 {d2}, [r2]!
vmov r7, r8, d2
vld1.u32 {d3}, [r2]!

#r3 = inHeight, r4 = inWidth, r5 = startXDiff and j, r6 = endXDiff, r7 = startYDiff and i, r8 = endYDiff
#r9 = using as a temporary variable

#Start Process Y
mul r9, r7, r4
add r0, r0, r9

sub r8, r3, r8
.crop_yuv420_arm_neon_Y_Height:

vmov r5, r6, d1
add r0, r0, r5
sub r6, r4, r6

sub r9, r6, r5

ands r9, r9, #7
beq .crop_yuv420_arm_neon_Y_Width
vld1.u8 {d4}, [r0], r9
vst1.u8 {d4}, [r1], r9
add r5, r5, r9
#add r0, r0, r9

cmp r5, r6
beq .crop_yuv420_arm_neon_Y_Width_end

.crop_yuv420_arm_neon_Y_Width:
vld1.u8 {d4}, [r0]!
vst1.u8 {d4}, [r1]!

add r5, r5, #8
cmp r5, r6
bne .crop_yuv420_arm_neon_Y_Width
.crop_yuv420_arm_neon_Y_Width_end:
vmov r5, r6, d1
add r0, r0, r6

add r7, r7, #1
cmp r7, r8
bne .crop_yuv420_arm_neon_Y_Height

vmov r3, r4, d0
vmov r5, r6, d1
vmov r7, r8, d2

mul r9, r8, r4
add r0, r0, r9




vshr.u32 d0, d0, #1
vshr.u32 d1, d1, #1
vshr.u32 d2, d2, #1

mov r11, #0

#Now Ready to process UV
.crop_yuv420_arm_neon_UV_Process:
vmov r3, r4, d0
vmov r5, r6, d1
vmov r7, r8, d2

mul r9, r7, r4
add r0, r0, r9
sub r8, r3, r8
.crop_yuv420_arm_neon_UV_Height:

vmov r5, r6, d1
add r0, r0, r5
sub r6, r4, r6

sub r9, r6, r5
ands r9, r9, #7
beq .crop_yuv420_arm_neon_UV_Width
vld1.u8 {d4}, [r0], r9
vst1.u8 {d4}, [r1], r9

add r5, r5, r9
cmp r5, r6
beq .crop_yuv420_arm_neon_UV_Width_end

.crop_yuv420_arm_neon_UV_Width:
vld1.u8 {d4}, [r0]!
vst1.u8 {d4}, [r1]!
add r5, r5, #8
cmp r5, r6
bne .crop_yuv420_arm_neon_UV_Width

.crop_yuv420_arm_neon_UV_Width_end:
vmov r5, r6, d1
add r0, r0, r6
add r7, r7, #1
cmp r7, r8
bne .crop_yuv420_arm_neon_UV_Height

vmov r3, r4, d0
vmov r5, r6, d1
vmov r7, r8, d2

mul r9, r8, r4
add r0, r0, r9
add r11, r11, #1
cmp r11, #2
bne .crop_yuv420_arm_neon_UV_Process

pop { r3-r11, pc }
NEON_ARM_FUNC_END


#endif


