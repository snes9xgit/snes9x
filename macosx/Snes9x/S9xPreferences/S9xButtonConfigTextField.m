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

#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>
#import "S9xButtonConfigTextField.h"

@implementation S9xButtonConfigTextField

- (void)awakeFromNib
{
    self.delegate = self;
    self.placeholderString = @"";
    [[self cell] setSearchButtonCell:nil];

    NSButtonCell *cancelButton = [[self cell] cancelButtonCell];
    cancelButton.target = self;
    cancelButton.action = @selector(clearSearch:);
}

- (void)setKeyCode:(CGKeyCode)keyCode
{
    if (keyCode == (CGKeyCode)-1)
    {
        _keyCode = keyCode;
        self.stringValue = @"";
        return;
    }

    NSString *stringValue = nil;

    switch (keyCode)
    {
        case kVK_Shift:
            stringValue = NSLocalizedString(@"Shift", @"");
            break;

        case kVK_Command:
            stringValue = NSLocalizedString(@"Command", @"");
            break;

        case kVK_Control:
            stringValue = NSLocalizedString(@"Control", @"");
            break;

        case kVK_Option:
            stringValue = NSLocalizedString(@"Option", @"");
            break;

        case kVK_F1:
            stringValue = NSLocalizedString(@"F1", @"");
            break;

        case kVK_F2:
            stringValue = NSLocalizedString(@"F2", @"");
            break;

        case kVK_F3:
            stringValue = NSLocalizedString(@"F3", @"");
            break;

        case kVK_F4:
            stringValue = NSLocalizedString(@"F4", @"");
            break;

        case kVK_F5:
            stringValue = NSLocalizedString(@"F5", @"");
            break;

        case kVK_F6:
            stringValue = NSLocalizedString(@"F6", @"");
            break;

        case kVK_F7:
            stringValue = NSLocalizedString(@"F7", @"");
            break;

        case kVK_F8:
            stringValue = NSLocalizedString(@"F8", @"");
            break;

        case kVK_F9:
            stringValue = NSLocalizedString(@"F9", @"");
            break;

        case kVK_F10:
            stringValue = NSLocalizedString(@"F10", @"");
            break;

        case kVK_F11:
            stringValue = NSLocalizedString(@"F11", @"");
            break;

        case kVK_F12:
            stringValue = NSLocalizedString(@"F12", @"");
            break;

        case kVK_F13:
            stringValue = NSLocalizedString(@"F13", @"");
            break;

        case kVK_F14:
            stringValue = NSLocalizedString(@"F14", @"");
            break;

        case kVK_F15:
            stringValue = NSLocalizedString(@"F15", @"");
            break;

        case kVK_F16:
            stringValue = NSLocalizedString(@"F16", @"");
            break;

        case kVK_F17:
            stringValue = NSLocalizedString(@"F17", @"");
            break;

        case kVK_F18:
            stringValue = NSLocalizedString(@"F18", @"");
            break;

        case kVK_F19:
            stringValue = NSLocalizedString(@"F19", @"");
            break;

        case kVK_F20:
            stringValue = NSLocalizedString(@"F20", @"");
            break;

        case kVK_Delete:
            stringValue = NSLocalizedString(@"Delete", @"");
            break;

        case kVK_ForwardDelete:
            stringValue = NSLocalizedString(@"⌦", @"");
            break;

        case kVK_Home:
            stringValue = NSLocalizedString(@"Home", @"");
            break;

        case kVK_End:
            stringValue = NSLocalizedString(@"End", @"");
            break;

        case kVK_PageUp:
            stringValue = NSLocalizedString(@"Page Up", @"");
            break;

        case kVK_PageDown:
            stringValue = NSLocalizedString(@"Page Down", @"");
            break;

        case kVK_Tab:
            stringValue = NSLocalizedString(@"Tab", @"");
            break;

        case kVK_Space:
            stringValue = NSLocalizedString(@"Space", @"");
            break;

        case kVK_ANSI_KeypadClear:
            stringValue = NSLocalizedString(@"Clear", @"");
            break;

        case kVK_LeftArrow:
            stringValue = NSLocalizedString(@"←", @"");
            break;

        case kVK_RightArrow:
            stringValue = NSLocalizedString(@"→", @"");
            break;

        case kVK_UpArrow:
            stringValue = NSLocalizedString(@"↑", @"");
            break;

        case kVK_DownArrow:
            stringValue = NSLocalizedString(@"↓", @"");
            break;

        case kVK_Return:
            stringValue = NSLocalizedString(@"Return", @"");
            break;

        case kVK_ANSI_KeypadEnter:
            stringValue = NSLocalizedString(@"Enter", @"");
            break;

        case kVK_Escape:
            stringValue = NSLocalizedString(@"Escape", @"");
            break;

        default:
        {
            TISInputSourceRef keyboard = TISCopyCurrentKeyboardInputSource();
            CFDataRef layoutData = (CFDataRef)TISGetInputSourceProperty(keyboard, kTISPropertyUnicodeKeyLayoutData);

            if ( layoutData != NULL )
            {
                const UCKeyboardLayout *layout = (const UCKeyboardLayout*)CFDataGetBytePtr(layoutData);
                UniCharCount maxStringLength = 0xFF;
                UniCharCount actualStringLength = 0;
                UniChar unicodeString[maxStringLength];
                uint32_t deadKeyState = 0;

                OSErr status = UCKeyTranslate(layout, keyCode, kUCKeyActionDisplay, kUCKeyTranslateNoDeadKeysBit, LMGetKbdType(), 0, &deadKeyState, maxStringLength, &actualStringLength, unicodeString);

                if (status == noErr && actualStringLength > 0)
                {
                    stringValue = [NSString stringWithCharacters:unicodeString length:actualStringLength];

                    switch (keyCode)
                    {
                        case kVK_ANSI_Keypad0:
                        case kVK_ANSI_Keypad1:
                        case kVK_ANSI_Keypad2:
                        case kVK_ANSI_Keypad3:
                        case kVK_ANSI_Keypad4:
                        case kVK_ANSI_Keypad5:
                        case kVK_ANSI_Keypad6:
                        case kVK_ANSI_Keypad7:
                        case kVK_ANSI_Keypad8:
                        case kVK_ANSI_Keypad9:
                        case kVK_ANSI_KeypadPlus:
                        case kVK_ANSI_KeypadMinus:
                        case kVK_ANSI_KeypadDivide:
                        case kVK_ANSI_KeypadMultiply:
                        case kVK_ANSI_KeypadEquals:
                        case kVK_ANSI_KeypadDecimal:
                            stringValue = [[stringValue stringByAppendingString:@" "] stringByAppendingString:NSLocalizedString(@"(Keypad)", @"")];
                            break;
                    }
                }
            }
        }
    }

    if ( stringValue != nil )
    {
        stringValue = [[stringValue componentsSeparatedByCharactersInSet:NSCharacterSet.controlCharacterSet] componentsJoinedByString:@""];
        stringValue = [[stringValue componentsSeparatedByCharactersInSet:NSCharacterSet.newlineCharacterSet] componentsJoinedByString:@""];
        stringValue = [[stringValue componentsSeparatedByCharactersInSet:NSCharacterSet.illegalCharacterSet] componentsJoinedByString:@""];

        if ( stringValue.length > 0 )
        {
            self.stringValue = stringValue.capitalizedString;
            [self.window makeFirstResponder:self.window.contentView];
        }
    }

    _keyCode = keyCode;
}

- (void)controlTextDidChange:(NSNotification *)obj
{
    self.stringValue = @"";
}

- (void)keyUp:(NSEvent *)event
{
    if (!self.disableKeyboardInput )
    {
        [self setKeyCode:event.keyCode];
    }
}

- (void)flagsChanged:(NSEvent *)event
{
    if (self.disableKeyboardInput)
    {
        return;
    }

    NSEventModifierFlags flags = event.modifierFlags;

    if ( flags & NSEventModifierFlagShift )
    {
        [self setKeyCode:kVK_Shift];
    }
    else if ( flags & NSEventModifierFlagOption )
    {
        [self setKeyCode:kVK_Option];
    }
    else if ( flags & NSEventModifierFlagCommand )
    {
        [self setKeyCode:kVK_Command];
    }
    else if ( flags & NSEventModifierFlagControl )
    {
        [self setKeyCode:kVK_Control];
    }
}

- (void)mouseDown:(NSEvent *)event
{
    [super mouseDown:event];
    [self.currentEditor selectAll:self];
}

- (void)clearSearch:(id)sender
{
    self.stringValue = @"";

    if (self.disableKeyboardInput)
    {
        self.joypadInput = nil;
    }
    else
    {
        self.keyCode = -1;
    }
}

- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)commandSelector
{
    if (self.disableKeyboardInput)
    {
        return NO;
    }

    if (commandSelector == @selector(insertTab:))
    {
        [self setKeyCode:kVK_Tab];
        return YES;
    }
    else if (commandSelector == @selector(cancelOperation:))
    {
        [self setKeyCode:kVK_Escape];
        return YES;
    }

    return NO;
}

- (BOOL)control:(NSControl *)control textShouldBeginEditing:(NSText *)fieldEditor
{
    return !self.disableKeyboardInput;
}

@end
