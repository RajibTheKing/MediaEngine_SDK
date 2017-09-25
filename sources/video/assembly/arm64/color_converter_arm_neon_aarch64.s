//
//  IPV-MediaEngine
//  color_converter_arm_neon_aarch64.s
//
//
//  Created by Rajib Chandra Das on 7/26/17.
//
//

#if defined(HAVE_NEON_AARCH64)

.macro NEON_ARM_AACH64_FUNC_BEGIN
.text
.extern printf
.align 2
.globl _$0
_$0:
.endm

.macro NEON_ARM_AACH64_FUNC_END
ret
.endm


NEON_ARM_AACH64_FUNC_BEGIN convert_nv12_to_i420_arm_neon_aarch64
# r0: Ptr to source data
# r1: Ptr to destination data
# r2: height
# r3: width

NEON_ARM_AACH64_FUNC_END



NEON_ARM_AACH64_FUNC_BEGIN crop_yuv420_arm_neon_aarch64

NEON_ARM_AACH64_FUNC_END


#endif


