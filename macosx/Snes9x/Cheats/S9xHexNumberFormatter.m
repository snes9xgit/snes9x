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
  (c) Copyright 2019 - 2023  Michael Donald Buckley
 ***********************************************************************************/

#import "S9xHexNumberFormatter.h"

@implementation S9xHexNumberFormatter

+ (NSUInteger)maxLength
{
	return NSUIntegerMax;
}

- (BOOL)getObjectValue:(out id  _Nullable __autoreleasing *)obj forString:(NSString *)string errorDescription:(out NSString * _Nullable __autoreleasing *)error
{
	if (string.length > 0)
	{
		unsigned int value = 0;
		NSScanner *scanner = [NSScanner scannerWithString:string];
		[scanner scanHexInt:&value];
		*obj = @(value);

		return YES;
	}

	return NO;
}

- (NSNumber *)numberFromString:(NSString *)string
{
	unsigned int value = 0;
	NSScanner *scanner = [NSScanner scannerWithString:string];
	[scanner scanHexInt:&value];
	return @(value);
}

- (NSString *)stringForObjectValue:(id)obj
{
	if ([obj isKindOfClass:NSNumber.class])
	{
		return [NSString stringWithFormat:@"%X", ((NSNumber *)obj).unsignedIntValue];
	}
	else if ([obj isKindOfClass:NSString.class])
	{
		return obj;
	}

	return @"";
}

- (BOOL)isPartialStringValid:(NSString * _Nonnull * _Nonnull)partialStringPtr proposedSelectedRange:(nullable NSRangePointer)proposedSelRangePtr originalString:(NSString *)origString originalSelectedRange:(NSRange)origSelRange errorDescription:(NSString * _Nullable * _Nullable)error
{
	static NSCharacterSet *hexCharacterSet;
	static dispatch_once_t onceToken;

	dispatch_once(&onceToken, ^
	{
		hexCharacterSet = [[NSCharacterSet characterSetWithCharactersInString:@"0123456789ABCDEFabcdef"] invertedSet];
	});

	if ( [*partialStringPtr rangeOfCharacterFromSet:hexCharacterSet].location != NSNotFound )
	{
		*partialStringPtr = nil;
		return NO;
	}

	*partialStringPtr = (*partialStringPtr).uppercaseString;

	if ( (*partialStringPtr).length > self.class.maxLength )
	{
		*partialStringPtr = [*partialStringPtr substringToIndex:self.class.maxLength];
		return NO;
	}

	return YES;
}

@end
