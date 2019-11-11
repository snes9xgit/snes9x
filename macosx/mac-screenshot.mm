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
  (c) Copyright 2019         Michael Donald Buckley
 ***********************************************************************************/

#include <sys/xattr.h>

#include "snes9x.h"
#include "memmap.h"
#include "screenshot.h"

#include "mac-prefix.h"
#include "mac-file.h"
#include "mac-gworld.h"
#include "mac-os.h"
#include "mac-render.h"
#include "mac-screenshot.h"

extern "C" {
#include "FakeResources.h"
}

const char *extendedAttributeName = "com.snes9x.preview";

unsigned char *CGImageToPNGData (CGImageRef image, CFIndex *outLength);

unsigned char *CGImageToPNGData (CGImageRef image, CFIndex *outLength)
{
    if (!outLength)
    {
        return NULL;
    }

    CFMutableDataRef dataRef = CFDataCreateMutable(kCFAllocatorDefault, 0);
    if (dataRef)
    {
        CGImageDestinationRef dest = CGImageDestinationCreateWithData(dataRef, kUTTypePNG, 1, NULL);

        if (dest)
        {
            CGImageDestinationAddImage(dest, image, NULL);
            CGImageDestinationFinalize(dest);
            CFRelease(dest);

            *outLength = CFDataGetLength(dataRef);
            unsigned char *data = (unsigned char *)malloc(*outLength);

            if (data)
            {
                CFDataGetBytes(dataRef, CFRangeMake(0, *outLength), data);
                return data;
            }
        }
        else
        {
            CFRelease(dataRef);
        }
    }

    return NULL;
}

static void ExportCGImageToPNGFile (CGImageRef image, const char *path)
{
	CFStringRef str;
	CFURLRef    url;

	str = CFStringCreateWithCString(kCFAllocatorDefault, path, kCFStringEncodingUTF8);
	if (str)
	{
		url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, str, kCFURLPOSIXPathStyle, false);
		if (url)
		{
            CFIndex length = 0;
            unsigned char *data = CGImageToPNGData(image, &length);
            if (data)
            {
                FILE *f = fopen(path, "wb");
                if (f)
                {
                    fwrite(data, length, 1, f);
                    fclose(f);
                }

                free(data);
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
			image = CGImageCreate(IPPU.RenderedScreenWidth, IPPU.RenderedScreenHeight, 5, 16, rowbytes, color, kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder16Host, prov, NULL, 1, kCGRenderingIntentDefault);
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
			image = CGImageCreate(width, height, 5, 16, rowbytes, color, kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder16Host, prov, NULL, 1, kCGRenderingIntentDefault);
			CGColorSpaceRelease(color);
		}

		CGDataProviderRelease(prov);
	}

	return (image);
}

void WriteThumbnailToExtendedAttribute (const char *path, int destWidth, int destHeight)
{
    CGContextRef    ctx;
    CGColorSpaceRef color = NULL;
    char            *data[destWidth * destHeight * 2];
    CGImageRef      image = CreateGameScreenCGImage();

    if (image)
    {
        color = CGColorSpaceCreateDeviceRGB();
        if (color)
        {
            ctx = CGBitmapContextCreate(data, destWidth, destHeight, 5, destWidth * 2, color, kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder16Big);

            if (ctx)
            {
                CGContextDrawImage(ctx, CGRectMake(0.0f, 0.0f, (float) destWidth, (float) destHeight), image);
                CGContextRelease(ctx);

                CFIndex length = 0;
                unsigned char *data = CGImageToPNGData(image, &length);
                if (data)
                {
                    setxattr(path, extendedAttributeName, data, length, 0, 0);
                    free(data);
                }
            }

            CGColorSpaceRelease(color);
        }

        CGImageRelease(image);
    }
}

void DrawThumbnailFromExtendedAttribute (const char *path, CGContextRef ctx, CGRect bounds)
{
    CGContextSaveGState(ctx);

    CGContextSetRGBFillColor(ctx, 0.0f, 0.0f, 0.0f, 1.0f);
    CGContextFillRect(ctx, bounds);

    ssize_t size = getxattr(path, extendedAttributeName, NULL, 0, 0, 0);

    if ( size > 0 )
    {
        unsigned char *buffer = (unsigned char *)malloc(size);
        if (buffer != NULL)
        {
            getxattr(path, extendedAttributeName, buffer, size, 0, 0);
            NSData *data = [NSData dataWithBytes:buffer length:size];
            if (data)
            {
                NSImage *image = [[NSImage alloc] initWithData:data];

                if (image)
                {
                    CGContextDrawImage(ctx, bounds, [image CGImageForProposedRect:NULL context:[NSGraphicsContext currentContext] hints:nil]);
                }
            }

            free(buffer);
        }
    }
    else
    {
        struct FakeResourceMap *resourceMap = FakeResFileOpen(std::string(std::string(path) + "/..namedfork/rsrc").c_str(), "r");
        if (resourceMap != NULL)
        {
            int16 fileNum = FakeCurResFile();
            Handle pict = FakeGet1Resource('PICT', 128);
            if (pict)
            {
                Size size = FakeGetHandleSize(pict);
                NSData *imageData = [NSData dataWithBytes:*pict length:size];

                if (imageData)
                {
                    NSImage *image = [[NSImage alloc] initWithData:imageData];

                    if (image)
                    {
                        CGContextDrawImage(ctx, bounds, [image CGImageForProposedRect:NULL context:[NSGraphicsContext currentContext] hints:nil]);
                    }
                }
            }
            else
            {
                pict = FakeGet1Resource('Thum', 128);
                Size size = FakeGetHandleSize(pict);
                NSData *imageData = [NSData dataWithBytes:*pict length:size];

                if (imageData)
                {
                    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, (__bridge CFDataRef)imageData, size, NULL);

                    if (provider)
                    {
                        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

                        if (colorSpace)
                        {
                            CGImageRef image = CGImageCreate(128, 120, 5, 16, 256, colorSpace, kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder16Big, provider, NULL, 0, kCGRenderingIntentDefault);

                            if (image)
                            {
                                CGContextDrawImage(ctx, bounds, image);
                                CGImageRelease(image);
                            }

                            CGColorSpaceRelease(colorSpace);
                        }

                        CFRelease(provider);
                    }
                }
            }

            FakeCloseResFile(fileNum);
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
