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


#include "snes9x.h"
#include "memmap.h"
#include "screenshot.h"

#include <QuickTime/QuickTime.h>

#include "mac-prefix.h"
#include "mac-file.h"
#include "mac-gworld.h"
#include "mac-os.h"
#include "mac-render.h"
#include "mac-screenshot.h"

static Handle GetScreenAsRawHandle (int, int);
static void ExportCGImageToPNGFile (CGImageRef, const char *);

#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
typedef struct QDPict* QDPictRef;
extern "C" QDPictRef QDPictCreateWithProvider (CGDataProviderRef provider);
extern "C" void QDPictRelease (QDPictRef pictRef);
extern "C" OSStatus QDPictDrawToCGContext (CGContextRef ctx, CGRect rect, QDPictRef pictRef);
#endif

static Handle GetScreenAsRawHandle (int destWidth, int destHeight)
{
	CGContextRef	ctx;
	CGColorSpaceRef	color;
	CGImageRef		image;
	Handle			data = NULL;

	image = CreateGameScreenCGImage();
	if (image)
	{
		data = NewHandleClear(destWidth * destHeight * 2);
		if (data)
		{
			HLock(data);

			color = CGColorSpaceCreateDeviceRGB();
			if (color)
			{
				ctx = CGBitmapContextCreate(*data, destWidth, destHeight, 5, destWidth * 2, color, kCGImageAlphaNoneSkipFirst | ((systemVersion >= 0x1040) ? kCGBitmapByteOrder16Big : 0));
				if (ctx)
				{
					CGContextDrawImage(ctx, CGRectMake(0.0f, 0.0f, (float) destWidth, (float) destHeight), image);
					CGContextRelease(ctx);
				}

				CGColorSpaceRelease(color);
			}

			HUnlock(data);
		}

		CGImageRelease(image);
	}

	return (data);
}

void WriteThumbnailToResourceFork (FSRef *ref, int destWidth, int destHeight)
{
	OSStatus		err;
	HFSUniStr255	fork;
	SInt16			resf;

	err = FSGetResourceForkName(&fork);
	if (err == noErr)
	{
		err = FSCreateResourceFork(ref, fork.length, fork.unicode, 0);
		if ((err == noErr) || (err == errFSForkExists))
		{
			err = FSOpenResourceFile(ref, fork.length, fork.unicode, fsWrPerm, &resf);
			if (err == noErr)
			{
				Handle	pict;

				pict = GetScreenAsRawHandle(destWidth, destHeight);
				if (pict)
				{
					AddResource(pict, 'Thum', 128, "\p");
					WriteResource(pict);
					ReleaseResource(pict);
				}

				CloseResFile(resf);
			}
		}
	}
}

static void ExportCGImageToPNGFile (CGImageRef image, const char *path)
{
	OSStatus				err;
	GraphicsExportComponent	exporter;
	CFStringRef				str;
	CFURLRef				url;
	Handle					dataRef;
	OSType					dataRefType;

	str = CFStringCreateWithCString(kCFAllocatorDefault, path, kCFStringEncodingUTF8);
	if (str)
	{
		url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, str, kCFURLPOSIXPathStyle, false);
		if (url)
		{
			err = QTNewDataReferenceFromCFURL(url, 0, &dataRef, &dataRefType);
			if (err == noErr)
			{
				err = OpenADefaultComponent(GraphicsExporterComponentType, kQTFileTypePNG, &exporter);
				if (err == noErr)
				{
					err = GraphicsExportSetInputCGImage(exporter, image);
					if (err == noErr)
					{
						err = GraphicsExportSetOutputDataReference(exporter, dataRef, dataRefType);
						if (err == noErr)
							err = GraphicsExportDoExport(exporter, NULL);
					}

					CloseComponent(exporter);
				}

				DisposeHandle(dataRef);
			}

			CFRelease(url);
		}

		CFRelease(str);
	}
}

CGImageRef CreateGameScreenCGImage (void)
{
	CGDataProviderRef	prov;
	CGColorSpaceRef		color;
	CGImageRef			image = NULL;
	int					rowbytes;

	rowbytes = IPPU.RenderedScreenWidth * 2;

	prov = CGDataProviderCreateWithData(NULL, GFX.Screen, 512 * 2 * 478, NULL);
	if (prov)
	{
		color = CGColorSpaceCreateDeviceRGB();
		if (color)
		{
			image = CGImageCreate(IPPU.RenderedScreenWidth, IPPU.RenderedScreenHeight, 5, 16, rowbytes, color, kCGImageAlphaNoneSkipFirst | ((systemVersion >= 0x1040) ? kCGBitmapByteOrder16Host : 0), prov, NULL, 1, kCGRenderingIntentDefault);
			CGColorSpaceRelease(color);
		}

		CGDataProviderRelease(prov);
	}

	return (image);
}

CGImageRef CreateBlitScreenCGImage (int width, int height, int rowbytes, uint8 *buffer)
{
	CGDataProviderRef	prov;
	CGColorSpaceRef		color;
	CGImageRef			image = NULL;

	prov = CGDataProviderCreateWithData(NULL, buffer, rowbytes * height, NULL);
	if (prov)
	{
		color = CGColorSpaceCreateDeviceRGB();
		if (color)
		{
			image = CGImageCreate(width, height, 5, 16, rowbytes, color, kCGImageAlphaNoneSkipFirst | ((systemVersion >= 0x1040) ? kCGBitmapByteOrder16Host : 0), prov, NULL, 1, kCGRenderingIntentDefault);
			CGColorSpaceRelease(color);
		}

		CGDataProviderRelease(prov);
	}

	return (image);
}

void DrawThumbnailResource (FSRef *ref, CGContextRef ctx, CGRect bounds)
{
	OSStatus			err;
	CGDataProviderRef	prov;
	CGColorSpaceRef		color;
	CGImageRef			image;
	QDPictRef			qdpr;
	Handle				pict;
	HFSUniStr255		fork;
	SInt16				resf;
	Size				size;

	CGContextSaveGState(ctx);

	CGContextSetRGBFillColor(ctx, 0.0f, 0.0f, 0.0f, 1.0f);
	CGContextFillRect(ctx, bounds);

	err = FSGetResourceForkName(&fork);
	if (err == noErr)
	{
		err = FSOpenResourceFile(ref, fork.length, fork.unicode, fsRdPerm, &resf);
		if (err == noErr)
		{
			pict = Get1Resource('PICT', 128);
			if (pict)
			{
				HLock(pict);

				size = GetHandleSize(pict);
				prov = CGDataProviderCreateWithData(NULL, (void *) *pict, size, NULL);
				if (prov)
				{
					qdpr = QDPictCreateWithProvider(prov);
					if (qdpr)
					{
						QDPictDrawToCGContext(ctx, bounds, qdpr);
						QDPictRelease(qdpr);
					}

					CGDataProviderRelease(prov);
				}

				HUnlock(pict);
				ReleaseResource(pict);
			}
			else
			{
				pict = Get1Resource('Thum', 128);
				if (pict)
				{
					HLock(pict);

					size = GetHandleSize(pict);
					prov = CGDataProviderCreateWithData(NULL, (void *) *pict, size, NULL);
					if (prov)
					{
						color = CGColorSpaceCreateDeviceRGB();
						if (color)
						{
							image = CGImageCreate(128, 120, 5, 16, 256, color, kCGImageAlphaNoneSkipFirst | ((systemVersion >= 0x1040) ? kCGBitmapByteOrder16Big : 0), prov, NULL, 0, kCGRenderingIntentDefault);
							if (image)
							{
								CGContextDrawImage(ctx, bounds, image);
								CGImageRelease(image);
							}

							CGColorSpaceRelease(color);
						}

						CGDataProviderRelease(prov);
					}

					HUnlock(pict);
					ReleaseResource(pict);
				}
			}

			CloseResFile(resf);
		}
	}

	CGContextRestoreGState(ctx);
}

bool8 S9xDoScreenshot (int width, int height)
{
	Settings.TakeScreenshot = false;

	uint16	*data;

	data = (uint16 *) malloc(512 * 478 * 2);
	if (data)
	{
		uint16	*sp, *dp;

		if (width > 256 && height > 239)
		{
			for (int y = 0; y < height; y++)
			{
				sp = GFX.Screen + y * GFX.RealPPL;
				dp = data + y * 512;

				for (int x = 0; x < width; x++)
					*dp++ = *sp++;
			}
		}
		else
		if (width > 256)
		{
			for (int y = 0; y < height; y++)
			{
				sp = GFX.Screen + y * GFX.RealPPL;
				dp = data + y * 2 * 512;

				for (int x = 0; x < width; x++)
				{
					*dp = *(dp + 512) = *sp++;
					dp++;
				}
			}
		}
		else
		{
			for (int y = 0; y < height; y++)
			{
				sp = GFX.Screen + y * GFX.RealPPL;
				dp = data + y * 2 * 512;

				for (int x = 0; x < width; x++)
				{
					*dp = *(dp + 1) = *(dp + 512) = *(dp + 512 + 1) = *sp++;
					dp += 2;
				}
			}
		}

		CGImageRef	image;

		image = CreateBlitScreenCGImage(512, (height > 239) ? height : (height * 2), 1024, (uint8 *) data);
		if (image)
		{
			ExportCGImageToPNGFile(image, S9xGetPNGFilename());
			CGImageRelease(image);
		}

		free(data);
	}

	return (true);
}
