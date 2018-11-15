/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _PIXFORM_H_
#define _PIXFORM_H_

#ifdef GFX_MULTI_FORMAT

enum { RGB565, RGB555, BGR565, BGR555, GBR565, GBR555, RGB5551 };

#define BUILD_PIXEL(R, G, B)					((*GFX.BuildPixel) (R, G, B))
#define BUILD_PIXEL2(R, G, B)					((*GFX.BuildPixel2) (R, G, B))
#define DECOMPOSE_PIXEL(PIX, R, G, B)			((*GFX.DecomposePixel) (PIX, R, G, B))

extern uint32	MAX_RED;
extern uint32	MAX_GREEN;
extern uint32	MAX_BLUE;
extern uint32	RED_LOW_BIT_MASK;
extern uint32	GREEN_LOW_BIT_MASK;
extern uint32	BLUE_LOW_BIT_MASK;
extern uint32	RED_HI_BIT_MASK;
extern uint32	GREEN_HI_BIT_MASK;
extern uint32	BLUE_HI_BIT_MASK;
extern uint32	FIRST_COLOR_MASK;
extern uint32	SECOND_COLOR_MASK;
extern uint32	THIRD_COLOR_MASK;
extern uint32	ALPHA_BITS_MASK;
extern uint32	GREEN_HI_BIT;
extern uint32	RGB_LOW_BITS_MASK;
extern uint32	RGB_HI_BITS_MASK;
extern uint32	RGB_HI_BITS_MASKx2;
extern uint32	RGB_REMOVE_LOW_BITS_MASK;
extern uint32	FIRST_THIRD_COLOR_MASK;
extern uint32	TWO_LOW_BITS_MASK;
extern uint32	HIGH_BITS_SHIFTED_TWO_MASK;
extern uint32	SPARE_RGB_BIT_MASK;

#endif

/* RGB565 format */
#define BUILD_PIXEL_RGB565(R, G, B)				(((int) (R) << 11) | ((int) (G) << 6) | (int) (B))
#define BUILD_PIXEL2_RGB565(R, G, B)			(((int) (R) << 11) | ((int) (G) << 5) | (int) (B))
#define DECOMPOSE_PIXEL_RGB565(PIX, R, G, B)	{ (R) = (PIX) >> 11; (G) = ((PIX) >> 6) & 0x1f; (B) = (PIX) & 0x1f; }
#define SPARE_RGB_BIT_MASK_RGB565				(1 << 5)

#define MAX_RED_RGB565				31
#define MAX_GREEN_RGB565			63
#define MAX_BLUE_RGB565				31
#define RED_LOW_BIT_MASK_RGB565		0x0800
#define GREEN_LOW_BIT_MASK_RGB565	0x0020
#define BLUE_LOW_BIT_MASK_RGB565	0x0001
#define RED_HI_BIT_MASK_RGB565		0x8000
#define GREEN_HI_BIT_MASK_RGB565	0x0400
#define BLUE_HI_BIT_MASK_RGB565		0x0010
#define FIRST_COLOR_MASK_RGB565		0xF800
#define SECOND_COLOR_MASK_RGB565	0x07E0
#define THIRD_COLOR_MASK_RGB565		0x001F
#define ALPHA_BITS_MASK_RGB565		0x0000

/* RGB555 format */
#define BUILD_PIXEL_RGB555(R, G, B)				(((int) (R) << 10) | ((int) (G) << 5) | (int) (B))
#define BUILD_PIXEL2_RGB555(R, G, B)			(((int) (R) << 10) | ((int) (G) << 5) | (int) (B))
#define DECOMPOSE_PIXEL_RGB555(PIX, R, G, B)	{ (R) = (PIX) >> 10; (G) = ((PIX) >> 5) & 0x1f; (B) = (PIX) & 0x1f; }
#define SPARE_RGB_BIT_MASK_RGB555				(1 << 15)

#define MAX_RED_RGB555				31
#define MAX_GREEN_RGB555			31
#define MAX_BLUE_RGB555				31
#define RED_LOW_BIT_MASK_RGB555		0x0400
#define GREEN_LOW_BIT_MASK_RGB555	0x0020
#define BLUE_LOW_BIT_MASK_RGB555	0x0001
#define RED_HI_BIT_MASK_RGB555		0x4000
#define GREEN_HI_BIT_MASK_RGB555	0x0200
#define BLUE_HI_BIT_MASK_RGB555		0x0010
#define FIRST_COLOR_MASK_RGB555		0x7C00
#define SECOND_COLOR_MASK_RGB555	0x03E0
#define THIRD_COLOR_MASK_RGB555		0x001F
#define ALPHA_BITS_MASK_RGB555		0x0000

/* BGR565 format */
#define BUILD_PIXEL_BGR565(R, G, B)				(((int) (B) << 11) | ((int) (G) << 6) | (int) (R))
#define BUILD_PIXEL2_BGR565(R, G, B)			(((int) (B) << 11) | ((int) (G) << 5) | (int) (R))
#define DECOMPOSE_PIXEL_BGR565(PIX, R, G, B)	{ (B) = (PIX) >> 11; (G) = ((PIX) >> 6) & 0x1f; (R) = (PIX) & 0x1f; }
#define SPARE_RGB_BIT_MASK_BGR565				(1 << 5)

#define MAX_RED_BGR565				31
#define MAX_GREEN_BGR565			63
#define MAX_BLUE_BGR565				31
#define RED_LOW_BIT_MASK_BGR565		0x0001
#define GREEN_LOW_BIT_MASK_BGR565	0x0040
#define BLUE_LOW_BIT_MASK_BGR565	0x0800
#define RED_HI_BIT_MASK_BGR565		0x0010
#define GREEN_HI_BIT_MASK_BGR565	0x0400
#define BLUE_HI_BIT_MASK_BGR565		0x8000
#define FIRST_COLOR_MASK_BGR565		0xF800
#define SECOND_COLOR_MASK_BGR565	0x07E0
#define THIRD_COLOR_MASK_BGR565		0x001F
#define ALPHA_BITS_MASK_BGR565		0x0000

/* BGR555 format */
#define BUILD_PIXEL_BGR555(R, G, B)				(((int) (B) << 10) | ((int) (G) << 5) | (int) (R))
#define BUILD_PIXEL2_BGR555(R, G, B)			(((int) (B) << 10) | ((int) (G) << 5) | (int) (R))
#define DECOMPOSE_PIXEL_BGR555(PIX, R, G, B)	{ (B) = (PIX) >> 10; (G) = ((PIX) >> 5) & 0x1f; (R) = (PIX) & 0x1f; }
#define SPARE_RGB_BIT_MASK_BGR555				(1 << 15)

#define MAX_RED_BGR555				31
#define MAX_GREEN_BGR555			31
#define MAX_BLUE_BGR555				31
#define RED_LOW_BIT_MASK_BGR555		0x0001
#define GREEN_LOW_BIT_MASK_BGR555	0x0020
#define BLUE_LOW_BIT_MASK_BGR555	0x0400
#define RED_HI_BIT_MASK_BGR555		0x0010
#define GREEN_HI_BIT_MASK_BGR555	0x0200
#define BLUE_HI_BIT_MASK_BGR555		0x4000
#define FIRST_COLOR_MASK_BGR555		0x7C00
#define SECOND_COLOR_MASK_BGR555	0x03E0
#define THIRD_COLOR_MASK_BGR555		0x001F
#define ALPHA_BITS_MASK_BGR555		0x0000

/* GBR565 format */
#define BUILD_PIXEL_GBR565(R, G, B)				(((int) (G) << 11) | ((int) (B) << 6) | (int) (R))
#define BUILD_PIXEL2_GBR565(R, G, B)			(((int) (G) << 11) | ((int) (B) << 5) | (int) (R))
#define DECOMPOSE_PIXEL_GBR565(PIX, R, G, B)	{ (G) = (PIX) >> 11; (B) = ((PIX) >> 6) & 0x1f; (R) = (PIX) & 0x1f; }
#define SPARE_RGB_BIT_MASK_GBR565				(1 << 5)

#define MAX_RED_GBR565				31
#define MAX_GREEN_GBR565			31
#define MAX_BLUE_GBR565				63
#define RED_LOW_BIT_MASK_GBR565		0x0001
#define GREEN_LOW_BIT_MASK_GBR565	0x0800
#define BLUE_LOW_BIT_MASK_GBR565	0x0040
#define RED_HI_BIT_MASK_GBR565		0x0010
#define GREEN_HI_BIT_MASK_GBR565	0x8000
#define BLUE_HI_BIT_MASK_GBR565		0x0400
#define FIRST_COLOR_MASK_GBR565		0xF800
#define SECOND_COLOR_MASK_GBR565	0x07E0
#define THIRD_COLOR_MASK_GBR565		0x001F
#define ALPHA_BITS_MASK_GBR565		0x0000

/* GBR555 format */
#define BUILD_PIXEL_GBR555(R, G, B)				(((int) (G) << 10) | ((int) (B) << 5) | (int) (R))
#define BUILD_PIXEL2_GBR555(R, G, B)			(((int) (G) << 10) | ((int) (B) << 5) | (int) (R))
#define DECOMPOSE_PIXEL_GBR555(PIX, R, G, B)	{ (G) = (PIX) >> 10; (B) = ((PIX) >> 5) & 0x1f; (R) = (PIX) & 0x1f; }
#define SPARE_RGB_BIT_MASK_GBR555				(1 << 15)

#define MAX_RED_GBR555				31
#define MAX_GREEN_GBR555			31
#define MAX_BLUE_GBR555				31
#define RED_LOW_BIT_MASK_GBR555		0x0001
#define GREEN_LOW_BIT_MASK_GBR555	0x0400
#define BLUE_LOW_BIT_MASK_GBR555	0x0020
#define RED_HI_BIT_MASK_GBR555		0x0010
#define GREEN_HI_BIT_MASK_GBR555	0x4000
#define BLUE_HI_BIT_MASK_GBR555		0x0200
#define FIRST_COLOR_MASK_GBR555		0x7C00
#define SECOND_COLOR_MASK_GBR555	0x03E0
#define THIRD_COLOR_MASK_GBR555		0x001F
#define ALPHA_BITS_MASK_GBR555		0x0000

/* RGB5551 format */
#define BUILD_PIXEL_RGB5551(R, G, B)			(((int) (R) << 11) | ((int) (G) << 6) | (int) ((B) << 1) | 1)
#define BUILD_PIXEL2_RGB5551(R, G, B)			(((int) (R) << 11) | ((int) (G) << 6) | (int) ((B) << 1) | 1)
#define DECOMPOSE_PIXEL_RGB5551(PIX, R, G, B)	{ (R) = (PIX) >> 11; (G) = ((PIX) >> 6) & 0x1f; (B) = ((PIX) >> 1) & 0x1f; }
#define SPARE_RGB_BIT_MASK_RGB5551				(1)

#define MAX_RED_RGB5551				31
#define MAX_GREEN_RGB5551			31
#define MAX_BLUE_RGB5551			31
#define RED_LOW_BIT_MASK_RGB5551	0x0800
#define GREEN_LOW_BIT_MASK_RGB5551	0x0040
#define BLUE_LOW_BIT_MASK_RGB5551	0x0002
#define RED_HI_BIT_MASK_RGB5551		0x8000
#define GREEN_HI_BIT_MASK_RGB5551	0x0400
#define BLUE_HI_BIT_MASK_RGB5551	0x0020
#define FIRST_COLOR_MASK_RGB5551	0xf800
#define SECOND_COLOR_MASK_RGB5551	0x07c0
#define THIRD_COLOR_MASK_RGB5551	0x003e
#define ALPHA_BITS_MASK_RGB5551		0x0001

#ifndef GFX_MULTI_FORMAT

#define CONCAT(X, Y)	X##Y

// C pre-processor needs a two stage macro define to enable it to concat
// to macro names together to form the name of another macro.
#define BUILD_PIXEL_D(F, R, G, B)			CONCAT(BUILD_PIXEL_,F) (R, G, B)
#define BUILD_PIXEL2_D(F, R, G, B)			CONCAT(BUILD_PIXEL2_,F) (R, G, B)
#define DECOMPOSE_PIXEL_D(F, PIX, R, G, B)	CONCAT(DECOMPOSE_PIXEL_,F) (PIX, R, G, B)

#define BUILD_PIXEL(R, G, B)				BUILD_PIXEL_D(PIXEL_FORMAT, R, G, B)
#define BUILD_PIXEL2(R, G, B)				BUILD_PIXEL2_D(PIXEL_FORMAT, R, G, B)
#define DECOMPOSE_PIXEL(PIX, R, G, B)		DECOMPOSE_PIXEL_D(PIXEL_FORMAT, PIX, R, G, B)

#define MAX_RED_D(F)						CONCAT(MAX_RED_, F)
#define MAX_GREEN_D(F)						CONCAT(MAX_GREEN_, F)
#define MAX_BLUE_D(F)						CONCAT(MAX_BLUE_, F)
#define RED_LOW_BIT_MASK_D(F)				CONCAT(RED_LOW_BIT_MASK_, F)
#define GREEN_LOW_BIT_MASK_D(F)				CONCAT(GREEN_LOW_BIT_MASK_, F)
#define BLUE_LOW_BIT_MASK_D(F)				CONCAT(BLUE_LOW_BIT_MASK_, F)
#define RED_HI_BIT_MASK_D(F)				CONCAT(RED_HI_BIT_MASK_, F)
#define GREEN_HI_BIT_MASK_D(F)				CONCAT(GREEN_HI_BIT_MASK_, F)
#define BLUE_HI_BIT_MASK_D(F)				CONCAT(BLUE_HI_BIT_MASK_, F)
#define FIRST_COLOR_MASK_D(F)				CONCAT(FIRST_COLOR_MASK_, F)
#define SECOND_COLOR_MASK_D(F)				CONCAT(SECOND_COLOR_MASK_, F)
#define THIRD_COLOR_MASK_D(F)				CONCAT(THIRD_COLOR_MASK_, F)
#define ALPHA_BITS_MASK_D(F)				CONCAT(ALPHA_BITS_MASK_, F)

#define MAX_RED								MAX_RED_D(PIXEL_FORMAT)
#define MAX_GREEN							MAX_GREEN_D(PIXEL_FORMAT)
#define MAX_BLUE							MAX_BLUE_D(PIXEL_FORMAT)
#define RED_LOW_BIT_MASK					RED_LOW_BIT_MASK_D(PIXEL_FORMAT)
#define GREEN_LOW_BIT_MASK					GREEN_LOW_BIT_MASK_D(PIXEL_FORMAT)
#define BLUE_LOW_BIT_MASK					BLUE_LOW_BIT_MASK_D(PIXEL_FORMAT)
#define RED_HI_BIT_MASK						RED_HI_BIT_MASK_D(PIXEL_FORMAT)
#define GREEN_HI_BIT_MASK					GREEN_HI_BIT_MASK_D(PIXEL_FORMAT)
#define BLUE_HI_BIT_MASK					BLUE_HI_BIT_MASK_D(PIXEL_FORMAT)
#define FIRST_COLOR_MASK					FIRST_COLOR_MASK_D(PIXEL_FORMAT)
#define SECOND_COLOR_MASK					SECOND_COLOR_MASK_D(PIXEL_FORMAT)
#define THIRD_COLOR_MASK					THIRD_COLOR_MASK_D(PIXEL_FORMAT)
#define ALPHA_BITS_MASK						ALPHA_BITS_MASK_D(PIXEL_FORMAT)

#define GREEN_HI_BIT						((MAX_GREEN + 1) >> 1)
#define RGB_LOW_BITS_MASK					(RED_LOW_BIT_MASK | GREEN_LOW_BIT_MASK | BLUE_LOW_BIT_MASK)
#define RGB_HI_BITS_MASK					(RED_HI_BIT_MASK | GREEN_HI_BIT_MASK | BLUE_HI_BIT_MASK)
#define RGB_HI_BITS_MASKx2					((RED_HI_BIT_MASK | GREEN_HI_BIT_MASK | BLUE_HI_BIT_MASK) << 1)
#define RGB_REMOVE_LOW_BITS_MASK			(~RGB_LOW_BITS_MASK)
#define FIRST_THIRD_COLOR_MASK				(FIRST_COLOR_MASK | THIRD_COLOR_MASK)
#define TWO_LOW_BITS_MASK					(RGB_LOW_BITS_MASK | (RGB_LOW_BITS_MASK << 1))
#define HIGH_BITS_SHIFTED_TWO_MASK			(((FIRST_COLOR_MASK | SECOND_COLOR_MASK | THIRD_COLOR_MASK) & ~TWO_LOW_BITS_MASK ) >> 2)

#endif

#endif
