/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

/***********************************************************************************
  SNES9X for Mac OS (c) Copyright John Stiles

  Snes9x for Mac OS X

  (c) Copyright 2001 - 2011  zones
  (c) Copyright 2002 - 2005  107
  (c) Copyright 2002         PB1400c
  (c) Copyright 2004         Alexander and Sander
  (c) Copyright 2004 - 2005  Steven Seeger
  (c) Copyright 2005         Ryan Vogt
 ***********************************************************************************/


#include "port.h"

#include "mac-prefix.h"
#include "mac-os.h"
#include "mac-gworld.h"

#define kIconSize	16

static void SetIconImage (CGImageRef, CGRect, int);
#ifdef MAC_PANTHER_SUPPORT
static IconRef CreateIconRefFromImage (CGImageRef, CGRect);
#endif


void DrawSubCGImage (CGContextRef ctx, CGImageRef image, CGRect src, CGRect dst)
{
    float	w = (float) CGImageGetWidth(image);
    float	h = (float) CGImageGetHeight(image);

	CGRect	drawRect = CGRectMake(0.0f, 0.0f, w, h);

	if (!CGRectEqualToRect(src, dst))
	{
		float	sx = CGRectGetWidth(dst)  / CGRectGetWidth(src);
		float	sy = CGRectGetHeight(dst) / CGRectGetHeight(src);
		float	dx = CGRectGetMinX(dst) - (CGRectGetMinX(src) * sx);
		float	dy = CGRectGetMinY(dst) - (CGRectGetMinY(src) * sy);

		drawRect = CGRectMake(dx, dy, w * sx, h * sy);
	}

	CGContextSaveGState(ctx);
	CGContextClipToRect(ctx, dst);
	CGContextDrawImage(ctx, drawRect, image);
	CGContextRestoreGState(ctx);
}

static void SetIconImage (CGImageRef image, CGRect rct, int n)
{
	if (systemVersion >= 0x1040)
		macIconImage[n] = CGImageCreateWithImageInRect(image, rct);
#ifdef MAC_PANTHER_SUPPORT
	else
		macIconRef[n] = CreateIconRefFromImage(image, rct);
#endif
}

void CreateIconImages (void)
{
	CGDataProviderRef	prov;
	CGImageRef			image;
	CFURLRef			url;

	image = NULL;
	memset(macIconImage, 0, sizeof(macIconImage));
#ifdef MAC_PANTHER_SUPPORT
	if (systemVersion < 0x1040)
		memset(macIconRef, 0, sizeof(macIconRef));
#endif

	url = CFBundleCopyResourceURL(CFBundleGetMainBundle(), CFSTR("icons"),  CFSTR("png"), NULL);
	if (url)
	{
		prov = CGDataProviderCreateWithURL(url);
		if (prov)
		{
			image = CGImageCreateWithPNGDataProvider(prov, NULL, true, kCGRenderingIntentDefault);
			CGDataProviderRelease(prov);
		}

		CFRelease(url);
	}

	if (image)
	{
		int	x, y, v = 0, n = 0;

		macPadIconIndex = n;
		for (y = 0; y < 8; y++)
		{
			for (x = 0; x < 12; x++)
				SetIconImage(image, CGRectMake(x * kIconSize, v, kIconSize, kIconSize), n++);
			v += kIconSize;
		}

		macLegendIconIndex = n;
		for (x = 0; x < 2; x++)
			SetIconImage(image, CGRectMake(x * kIconSize, v, kIconSize, kIconSize), n++);
		v += kIconSize;

		macMusicBoxIconIndex = n;
		for (x = 0; x < 3; x++)
			SetIconImage(image, CGRectMake(x * kIconSize, v, kIconSize, kIconSize), n++);
		v += kIconSize;

		macFunctionIconIndex = n;
		for (x = 0; x < 17; x++)
			SetIconImage(image, CGRectMake(x * kIconSize, v, kIconSize, kIconSize), n++);

		CGImageRelease(image);
		
	#ifdef MAC_PANTHER_SUPPORT
		if (systemVersion < 0x1040)
		{
			CGColorSpaceRef		color;
			CGContextRef		ctx;
			CGRect				rct;
			static UInt32		data[2][kIconSize * kIconSize];

			rct = CGRectMake(0, 0, kIconSize, kIconSize);

			color = CGColorSpaceCreateDeviceRGB();
			if (color)
			{
				for (int i = 0; i < 2; i++)
				{
					ctx = CGBitmapContextCreate(data[i], kIconSize, kIconSize, 8, kIconSize * 4, color, kCGImageAlphaNoneSkipFirst);
					if (ctx)
					{
						PlotIconRefInContext(ctx, &rct, kAlignNone, kTransformNone, NULL, kPlotIconRefNormalFlags, macIconRef[macLegendIconIndex + i]);
						CGContextRelease(ctx);

						prov = CGDataProviderCreateWithData(NULL, data[i], kIconSize * kIconSize * 4, NULL);
						if (prov)
						{
							macIconImage[macLegendIconIndex + i] = CGImageCreate(kIconSize, kIconSize, 8, 32, kIconSize * 4, color, kCGImageAlphaNoneSkipFirst, prov, NULL, 1, kCGRenderingIntentDefault);
							CGDataProviderRelease(prov);
						}
					}
				}

				CGColorSpaceRelease(color);
			}
		}
	#endif
	}
}

void ReleaseIconImages (void)
{
	for (int i = 0; i < 118; i++)
	{
		if (systemVersion >= 0x1040)
		{
			if (macIconImage[i])
				CGImageRelease(macIconImage[i]);
		}
	#ifdef MAC_PANTHER_SUPPORT
		else
		{
			if (macIconRef[i])
				ReleaseIconRef(macIconRef[i]);
		}
	#endif
	}

#ifdef MAC_PANTHER_SUPPORT
	if (systemVersion < 0x1040)
	{
		if (macIconImage[macLegendIconIndex])
			CGImageRelease(macIconImage[macLegendIconIndex]);
		if (macIconImage[macLegendIconIndex + 1])
			CGImageRelease(macIconImage[macLegendIconIndex + 1]);
	}
#endif
}

#ifdef MAC_PANTHER_SUPPORT
static IconRef CreateIconRefFromImage (CGImageRef srcImage, CGRect srcRect)
{
	OSStatus			err;
	CGContextRef		cctx, actx;
	CGColorSpaceRef		color;
	CGRect				dstRect;
	IconRef				iconRef;
	IconFamilyHandle	icns;
	Handle				hdl;
	SInt32				size;
	UInt32				rgb[kIconSize * kIconSize];
	UInt8				alp[kIconSize * kIconSize];

	srcRect.origin.y = CGImageGetHeight(srcImage) - srcRect.origin.y - kIconSize;

	color = CGColorSpaceCreateDeviceRGB();
	if (color)
	{
		cctx = CGBitmapContextCreate(rgb, kIconSize, kIconSize, 8, kIconSize * 4, color, kCGImageAlphaNoneSkipFirst);
		if (cctx)
		{
			dstRect = CGRectMake(0, 0, kIconSize, kIconSize);
			DrawSubCGImage(cctx, srcImage, srcRect, dstRect);

			actx = CGBitmapContextCreate(alp, kIconSize, kIconSize, 8, kIconSize, NULL, kCGImageAlphaOnly);
			if (actx)
			{
				DrawSubCGImage(actx, srcImage, srcRect, dstRect);
				CGContextRelease(actx);
			}

			CGContextRelease(cctx);
		}

		CGColorSpaceRelease(color);
	}

	iconRef = NULL;

	size = sizeof(OSType) + sizeof(SInt32);
	icns = (IconFamilyHandle) NewHandle(size);
	if (icns)
	{
		// Big-endian: Panther is for PowerPC only
		(*icns)->resourceType = kIconFamilyType;
		(*icns)->resourceSize = size;

		err = PtrToHand(rgb, &hdl, sizeof(rgb));
		if (err == noErr)
		{
			err = SetIconFamilyData(icns, kSmall32BitData, hdl);
			DisposeHandle(hdl);

			if (err == noErr)
			{
				err = PtrToHand(alp, &hdl, sizeof(alp));
				if (err == noErr)
				{
					err = SetIconFamilyData(icns, kSmall8BitMask, hdl);
					DisposeHandle(hdl);
				}
			}
		}

		if (err == noErr)
			err = GetIconRefFromIconFamilyPtr(*icns, GetHandleSize((Handle) icns), &iconRef);

		DisposeHandle((Handle) icns);
	}

	return (iconRef);
}
#endif
