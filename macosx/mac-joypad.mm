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

#include <map>

#include <pthread.h>

#include "port.h"

#include <IOKit/hid/IOHIDDevice.h>
#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDUsageTables.h>
#include "HID_Utilities_External.h"

#include "mac-prefix.h"
#include "mac-dialog.h"
#include "mac-os.h"
#include "mac-joypad.h"

std::unordered_set<JoypadDevice> allDevices;
std::unordered_map<JoypadDevice, std::map<uint8, std::map<int8, S9xButtonCode>>> defaultAxes;
std::unordered_map<JoypadDevice, std::map<uint8, S9xButtonCode>> defaultButtons;
std::unordered_map<JoypadDevice, std::map<uint8, S9xButtonCode>> defaultHatValues;
std::unordered_map<JoypadDevice, int8> playerNumByDevice;
std::unordered_map<uint32, int8> deviceIndexByPort;
std::unordered_map<JoypadCookie, JoypadCookieInfo> infoByCookie;
std::unordered_map<JoypadInput, S9xButtonCode> buttonCodeByJoypadInput;
std::unordered_map<JoypadDevice, std::string> namesByDevice;

@interface NSData (S9xHexString)
+(id)s9x_dataWithHexString:(NSString *)hex;
@end

@implementation NSData (S9xHexString)

// Not efficent
+ (id)s9x_dataWithHexString:(NSString *)hex
{
    char buf[3];
    buf[2] = '\0';
    NSAssert(0 == [hex length] % 2, @"Hex strings should have an even number of digits (%@)", hex);
    unsigned char *bytes = (unsigned char *)malloc([hex length]/2);
    unsigned char *bp = bytes;
    for (CFIndex i = 0; i < [hex length]; i += 2) {
        buf[0] = [hex characterAtIndex:i];
        buf[1] = [hex characterAtIndex:i+1];
        char *b2 = NULL;
        *bp++ = strtol(buf, &b2, 16);
        NSAssert(b2 == buf + 2, @"String should be all hex digits: %@ (bad digit around %ld)", hex, (long)i);
    }

    return [NSData dataWithBytesNoCopy:bytes length:[hex length]/2 freeWhenDone:YES];
}

@end

IOHIDManagerRef hidManager = NULL;

std::unordered_set<struct JoypadDevice> ListJoypads (void) {
    return allDevices;
}

std::string NameForDevice(struct JoypadDevice device) {
    auto it = namesByDevice.find(device);
    if (it != namesByDevice.end())
    {
        return it->second;
    }

    return "";
}

void gamepadAction(void *inContext, IOReturn inResult, void *inSender, IOHIDValueRef v) {
    pthread_mutex_lock(&keyLock);

    IOHIDDeviceRef device = (IOHIDDeviceRef) inSender;
    uint32 port = ((NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDLocationIDKey))).unsignedIntValue;

    if (deviceIndexByPort.find(port) == deviceIndexByPort.end())
    {
        pthread_mutex_unlock(&keyLock);
        return;
    }

    IOHIDElementRef element = IOHIDValueGetElement(v);

    NSNumber *vendor = (NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey));
    NSNumber *product = (NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey));

    struct JoypadDevice deviceStruct;
    deviceStruct.vendorID = vendor.unsignedIntValue;
    deviceStruct.productID = product.unsignedIntValue;
    deviceStruct.index = deviceIndexByPort[port];

    if (allDevices.find(deviceStruct) == allDevices.end())
    {
        pthread_mutex_unlock(&keyLock);
        return;
    }

    if ( playerNumByDevice.find(deviceStruct) == playerNumByDevice.end())
    {
        pthread_mutex_unlock(&keyLock);
        return;
    }

    int8 playerNum = playerNumByDevice[deviceStruct];
    if (playerNum < 0 || playerNum >= MAC_MAX_PLAYERS)
    {
        pthread_mutex_unlock(&keyLock);
        return;
    }

    struct JoypadCookie cookieStruct;
    cookieStruct.device = deviceStruct;
    cookieStruct.cookie = (int32_t)IOHIDElementGetCookie(element);

    if (infoByCookie.find(cookieStruct) != infoByCookie.end())
    {
        auto info = infoByCookie[cookieStruct];

        struct JoypadInput inputStruct;
        inputStruct.cookie = cookieStruct;
        inputStruct.value = (int32_t)IOHIDValueGetIntegerValue(v);

        S9xJoypad *objcJoypad = [S9xJoypad new];
        objcJoypad.vendorID = deviceStruct.vendorID;
        objcJoypad.productID = deviceStruct.productID;
        objcJoypad.index = deviceStruct.index;

        S9xJoypadInput *objcInput = [S9xJoypadInput new];
        objcInput.cookie = inputStruct.cookie.cookie;
        objcInput.value =inputStruct.value;

        pthread_mutex_unlock(&keyLock);
        if (info.usage != kHIDUsage_GD_Hatswitch && info.min != info.max)
        {
            if (inputStruct.value <= info.min || inputStruct.value >= info.max)
            {
                if ([inputDelegate handleInput:objcInput fromJoypad:objcJoypad])
                {
                    return;
                }
            }
        }
        else
        {
            if (inputStruct.value >= info.min && inputStruct.value <= info.max)
            {
               if ([inputDelegate handleInput:objcInput fromJoypad:objcJoypad])
               {
                   return;
               }
            }
        }
        pthread_mutex_lock(&keyLock);

        struct JoypadInput oppositeInputStruct = inputStruct;

        if (info.usage != kHIDUsage_GD_Hatswitch && info.min != info.max)
        {
            if (inputStruct.value < info.min)
            {
                inputStruct.value = info.min;
                oppositeInputStruct.value = info.max;
            }
            else if ( inputStruct.value > info.max)
            {
                inputStruct.value = info.max;
                oppositeInputStruct.value = info.min;
            }
            else
            {
                inputStruct.value = info.midpoint;
            }

            if (buttonCodeByJoypadInput.find(inputStruct) != buttonCodeByJoypadInput.end())
            {
                pressedGamepadButtons[playerNum][buttonCodeByJoypadInput[inputStruct]] = true;

				if ( buttonCodeByJoypadInput.find(oppositeInputStruct) != buttonCodeByJoypadInput.end() )
				{
					pressedGamepadButtons[playerNum][buttonCodeByJoypadInput[oppositeInputStruct]] = false;
				}

            }
            else
            {
                oppositeInputStruct.value = info.min;
				if ( buttonCodeByJoypadInput.find(oppositeInputStruct) != buttonCodeByJoypadInput.end() )
				{
					pressedGamepadButtons[playerNum][buttonCodeByJoypadInput[oppositeInputStruct]] = false;
				}
                oppositeInputStruct.value = info.max;

				if ( buttonCodeByJoypadInput.find(oppositeInputStruct) != buttonCodeByJoypadInput.end() )
				{
					pressedGamepadButtons[playerNum][buttonCodeByJoypadInput[oppositeInputStruct]] = false;
				}
            }
        }
        else if (info.usage == kHIDUsage_GD_Hatswitch)
        {
            int32 value = inputStruct.value;

            for (int i = info.min; i <= info.max; i++)
            {
                inputStruct.value = i;
                if (buttonCodeByJoypadInput.find(inputStruct) != buttonCodeByJoypadInput.end())
                {
                     pressedGamepadButtons[playerNum][buttonCodeByJoypadInput[inputStruct]] = false;
                }
            }

			if (value % 2 == 0)
			{
				inputStruct.value = value;
				if (buttonCodeByJoypadInput.find(inputStruct) != buttonCodeByJoypadInput.end())
				{
					pressedGamepadButtons[playerNum][buttonCodeByJoypadInput[inputStruct]] = true;
				}
			}
			else
			{
				for (int i = value - 1; i <= value + 1; i++)
				{
					int button = i;
					if (i < info.min)
					{
						button = info.max;
					}
					else if (i > info.max)
					{
						button = info.min;
					}
					inputStruct.value = button;
					if (buttonCodeByJoypadInput.find(inputStruct) != buttonCodeByJoypadInput.end())
					{
						pressedGamepadButtons[playerNum][buttonCodeByJoypadInput[inputStruct]] = true;
					}
				}
			}
        }
        else
        {
            bool value = (inputStruct.value != 0);
            inputStruct.value = 0;

            if (buttonCodeByJoypadInput.find(inputStruct) != buttonCodeByJoypadInput.end())
            {
                pressedGamepadButtons[playerNum][buttonCodeByJoypadInput[inputStruct]] = value;
            }
        }
    }

    pthread_mutex_unlock(&keyLock);
}

void findControls(struct JoypadDevice &device, NSDictionary *properties, NSMutableArray<NSDictionary *> *buttons, NSMutableArray<NSDictionary *> *axes, NSMutableDictionary *hat)
{
    if (properties == nil)
    {
        return;
	}

    int usagePage = [properties[@kIOHIDElementUsagePageKey] intValue];
    int usage = [properties[@kIOHIDElementUsageKey] intValue];
    if (usagePage == kHIDPage_Button ||
        usagePage == kHIDPage_Consumer ||
        (usagePage == kHIDPage_GenericDesktop && (usage == kHIDUsage_GD_DPadUp || usage == kHIDUsage_GD_DPadDown || usage == kHIDUsage_GD_DPadLeft || usage == kHIDUsage_GD_DPadRight ||
                                                  usage == kHIDUsage_GD_Start || usage == kHIDUsage_GD_Select || usage == kHIDUsage_GD_SystemMainMenu)))
    {
        [buttons addObject:properties];
    }
    else if (usagePage == kHIDPage_GenericDesktop && (usage == kHIDUsage_GD_X || usage == kHIDUsage_GD_Y || usage == kHIDUsage_GD_Z || usage == kHIDUsage_GD_Rx || usage == kHIDUsage_GD_Ry || usage == kHIDUsage_GD_Rz))
    {
        [axes addObject:properties];
    }
    else if (usagePage == kHIDPage_GenericDesktop && usage == kHIDUsage_GD_Hatswitch)
    {
        [hat setDictionary:properties];
    }

	for ( NSDictionary *child in properties[@kIOHIDElementKey] )
	{
		findControls(device, child, buttons, axes, hat);
	}
}

void ParseDefaults (void)
{
    NSString *contents = [NSString stringWithContentsOfURL:[[NSBundle bundleForClass:[S9xEngine class]] URLForResource:@"gamecontrollerdb" withExtension:@"txt"] encoding:NSUTF8StringEncoding error:NULL];

    for ( NSString *line in [contents componentsSeparatedByString:@"\n"])
    {
        NSMutableArray<NSString *> *components = [[line componentsSeparatedByString:@","] mutableCopy];
        if (components.count > 0)
        {
            [components removeLastObject];
        }

        if (![components.lastObject isEqualToString:@"platform:Mac OS X"])
        {
            continue;
        }

        [components removeLastObject];

        if (components.firstObject.length != 32)
        {
            continue;
        }

        NSData *guidData = [NSData s9x_dataWithHexString:components.firstObject];
        uint16 *bytes = (uint16 *)guidData.bytes;

        struct JoypadDevice key;
        key.vendorID = bytes[2];
        key.productID = bytes[4];
        key.index = 0;

        [components removeObjectAtIndex:0];
        [components removeObjectAtIndex:0];

        for (NSString *component in components)
        {
            NSArray<NSString *> *subcomponents = [component componentsSeparatedByString:@":"];

            if (subcomponents.count != 2)
            {
                continue;
            }

            NSString *control = subcomponents.lastObject;
            NSString *codeString = subcomponents.firstObject;

            if ([control hasPrefix:@"b"])
            {
                control = [control substringFromIndex:1];
                int buttonNum = control.intValue;
                int16 code = -1;

                // Buttons are mirrored horizontally, since the config uses the Xbox controller as a reference.
                if ([codeString isEqualToString:@"a"])
                {
                    code = kB;
                }
                else if ([codeString isEqualToString:@"b"])
                {
                    code = kA;
                }
                else if ([codeString isEqualToString:@"x"])
                {
                    code = kY;
                }
                else if ([codeString isEqualToString:@"y"])
                {
                    code = kX;
                }
                else if ([codeString isEqualToString:@"start"])
                {
                    code = kStart;
                }
                else if ([codeString isEqualToString:@"back"])
                {
                    code = kSelect;
                }
                else if ([codeString isEqualToString:@"rightshoulder"])
                {
                    code = kR;
                }
                else if ([codeString isEqualToString:@"leftshoulder"])
                {
                    code = kL;
                }
                else if ([codeString isEqualToString:@"dpup"])
                {
                    code = kUp;
                }
                else if ([codeString isEqualToString:@"dpdown"])
                {
                    code = kDown;
                }
                else if ([codeString isEqualToString:@"dpleft"])
                {
                    code = kLeft;
                }
                else if ([codeString isEqualToString:@"dpright"])
                {
                    code = kRight;
                }

                if (code >= 0)
                {
                    defaultButtons[key][buttonNum] = (S9xButtonCode)code;
                }
            }
            else if ([control hasPrefix:@"h0."])
            {
                control = [control substringFromIndex:3];
                int value = control.intValue;
                int16 code = -1;

                if ([codeString isEqualToString:@"dpup"])
                {
                    code = kUp;
                }
                else if ([codeString isEqualToString:@"dpdown"])
                {
                    code = kDown;
                }
                else if ([codeString isEqualToString:@"dpleft"])
                {
                    code = kLeft;
                }
                else if ([codeString isEqualToString:@"dpright"])
                {
                    code = kRight;
                }

                if (code >= 0)
                {
                    defaultHatValues[key][value] = (S9xButtonCode)code;
                }
            }
            else if ([control hasPrefix:@"a"] || [control hasPrefix:@"+a"] || [control hasPrefix:@"-a"])
            {
                BOOL negative = [control hasPrefix:@"-"];

                if ( negative || [control hasPrefix:@"+"])
                {
                    control = [control substringFromIndex:2];
                }
                else
                {
                    control = [control substringFromIndex:1];
                }

                int axisNum = control.intValue;
                int16 code = -1;

                if ([codeString isEqualToString:@"dpup"])
                {
                    code = kUp;
                }
                else if ([codeString isEqualToString:@"dpdown"])
                {
                    code = kDown;
                }
                else if ([codeString isEqualToString:@"dpleft"])
                {
                    code = kLeft;
                }
                else if ([codeString isEqualToString:@"dpright"])
                {
                    code = kRight;
                }

                if (code >= 0)
                {
                    defaultAxes[key][axisNum][negative ? -1 : 1] = (S9xButtonCode)code;
                }
                else
                {
                    if ([codeString isEqualToString:@"leftx"])
                    {
                        defaultAxes[key][axisNum][-1] = kLeft;
                        defaultAxes[key][axisNum][-1] = kRight;
                    }
                    else if ([codeString isEqualToString:@"lefty"])
                    {
                        defaultAxes[key][axisNum][-1] = kUp;
                        defaultAxes[key][axisNum][-1] = kDown;
                    }
                }
            }
        }
    }
}

void SetDefaultButtonCodeForJoypadControl(struct JoypadInput &input, S9xButtonCode buttonCode)
{
    SetButtonCodeForJoypadControl(input.cookie.device.vendorID, input.cookie.device.productID, input.cookie.device.index, input.cookie.cookie, input.value, buttonCode, false, NULL);
}

void AddDevice (IOHIDDeviceRef device)
{
    NSNumber *vendor = (NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey));
    NSNumber *product = (NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey));
    NSString *name = (NSString *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));

    NSMutableArray<NSDictionary *> *buttons = [NSMutableArray new];
    NSMutableArray<NSDictionary *> *axes = [NSMutableArray new];
    NSMutableDictionary<NSString *, NSNumber *> *hat = [NSMutableDictionary new];

    struct JoypadDevice deviceStruct;
    deviceStruct.vendorID = vendor.unsignedIntValue;
    deviceStruct.productID = product.unsignedIntValue;
    deviceStruct.index = 0;

    struct JoypadDevice defaultsKey = deviceStruct;

    while (allDevices.find(deviceStruct) != allDevices.end())
    {
        deviceStruct.index += 1;
    }

	if (name == nil)
	{
		name = @"Unknown Device";
	}

    allDevices.insert(deviceStruct);
    std::string s = std::string(name.UTF8String);

    if (deviceStruct.index > 0)
    {
        s = s + " (" + std::to_string(deviceStruct.index + 1) + ")";
    }

    namesByDevice[deviceStruct] = s;
    uint32_t port = ((NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDLocationIDKey))).unsignedIntValue;
    deviceIndexByPort[port] = deviceStruct.index;

    CFMutableDictionaryRef properties = NULL;

    IORegistryEntryCreateCFProperties(IOHIDDeviceGetService(device), &properties, kCFAllocatorDefault, kNilOptions);

    for ( NSDictionary *child in ((__bridge NSDictionary *)properties)[@kIOHIDElementKey] )
    {
        findControls(deviceStruct, child, buttons, axes, hat);
    }

    NSComparisonResult (^comparitor)(NSDictionary *a, NSDictionary *b) = ^NSComparisonResult(NSDictionary *a, NSDictionary *b)
    {
        NSNumber *usageA = a[@kIOHIDElementUsageKey];
        NSNumber *usageB = b[@kIOHIDElementUsageKey];

        return [usageA compare:usageB];
    };

    [buttons sortWithOptions:NSSortStable usingComparator:comparitor];

    uint32 buttonIndex = 0;
    for (NSDictionary<NSString *, NSNumber *> *buttonDict in buttons)
    {
        struct JoypadCookie cookie;
        struct JoypadCookieInfo info;

        cookie.device = deviceStruct;
        cookie.cookie = buttonDict[@kIOHIDElementCookieKey].unsignedIntValue;

        info.usage = kHIDUsage_Undefined;
        info.min = 0;
        info.max = 0;
        info.midpoint = 0;

        if (defaultButtons.find(defaultsKey) != defaultButtons.end())
        {
            std::map<uint8, S9xButtonCode> &buttonMap = defaultButtons[defaultsKey];
            if (buttonMap.find(buttonIndex) != buttonMap.end())
            {
                struct JoypadInput input;
                input.cookie = cookie;
                input.value = 0;
                SetDefaultButtonCodeForJoypadControl(input, buttonMap[buttonIndex]);
            }
        }

        info.index = buttonIndex++;
        infoByCookie[cookie] = info;
    }

    [axes sortWithOptions:NSSortStable usingComparator:comparitor];

    uint32 axisIndex = 0;
    const float deadZone = 0.3;
    for ( NSDictionary<NSString *, NSNumber *> *axisDict in axes)
    {
        struct JoypadCookie cookie;
        struct JoypadCookieInfo info;

        cookie.device = deviceStruct;
        cookie.cookie = axisDict[@kIOHIDElementCookieKey].unsignedIntValue;

        info.min = axisDict[@kIOHIDElementMinKey].intValue;
        info.max = axisDict[@kIOHIDElementMaxKey].intValue;
        info.midpoint = (info.min + info.max) / 2;
        info.max = info.midpoint + ((info.max - info.midpoint) * deadZone);
        info.min = info.midpoint - ((info.midpoint - info.min) * deadZone);

        if (defaultAxes.find(defaultsKey) != defaultAxes.end())
        {
            auto axisMap = defaultAxes[defaultsKey];

            if (axisMap.find(axisIndex) != axisMap.end())
            {
                {
                    struct JoypadInput input;
                    input.cookie = cookie;
                    input.value = info.min;
                    SetDefaultButtonCodeForJoypadControl(input, axisMap[axisIndex][-1]);
                }

                {
                    struct JoypadInput input;
                    input.cookie = cookie;
                    input.value = info.max;
                    SetDefaultButtonCodeForJoypadControl(input, axisMap[axisIndex][1]);
                }
            }
        }

        info.usage = axisDict[@kIOHIDElementUsageKey].intValue;
        info.index = axisIndex++;
        infoByCookie[cookie] = info;
    }

    if ([hat count] >= 0)
    {
        struct JoypadCookie cookie;
        struct JoypadCookieInfo info;

        cookie.device = deviceStruct;
        cookie.cookie = hat[@kIOHIDElementCookieKey].unsignedIntValue;

        info.usage = kHIDUsage_GD_Hatswitch;
        info.min = hat[@kIOHIDElementMinKey].intValue;
        info.max = hat[@kIOHIDElementMaxKey].intValue;
        info.midpoint = 0;

        if (defaultHatValues.find(defaultsKey) != defaultHatValues.end())
        {
            for (auto value : defaultHatValues[defaultsKey])
            {
                struct JoypadInput input;
                input.cookie = cookie;
                input.value = value.first;
                SetDefaultButtonCodeForJoypadControl(input, value.second);
            }
        }

        info.index = 0;
        infoByCookie[cookie] = info;
    }

    CFRelease(properties);
}

void ClearJoypad(uint32 vendorID, uint32 productID, uint32 index)
{
    struct JoypadDevice device;
    device.vendorID = vendorID;
    device.productID = productID;
    device.index = index;

    for (auto it = buttonCodeByJoypadInput.begin(); it != buttonCodeByJoypadInput.end();)
    {
        if (it->first.cookie.device == device)
        {
            buttonCodeByJoypadInput.erase(it++);
        }
        else
        {
            ++it;
        }
    }
}

std::unordered_map<struct JoypadInput, S9xButtonCode> GetJuypadButtons(uint32 vendorID, uint32 productID, uint32 index)
{
    struct JoypadDevice device;
    device.vendorID = vendorID;
    device.productID = productID;
    device.index = index;

    std::unordered_map<struct JoypadInput, S9xButtonCode> joypadButtons;

    for (auto it = buttonCodeByJoypadInput.begin(); it != buttonCodeByJoypadInput.end(); ++it)
    {
        if ( it->first.cookie.device == device)
        {
            joypadButtons[it->first] = it->second;
        }
    }

    return joypadButtons;
}

void SetUpHID (void)
{
    hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);

    if (hidManager != NULL && IOHIDManagerOpen(hidManager, kIOHIDOptionsTypeNone) == kIOReturnSuccess)
    {
        IOHIDManagerRegisterInputValueCallback(hidManager, gamepadAction, NULL);
        IOHIDManagerScheduleWithRunLoop(hidManager, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
        IOHIDManagerSetDeviceMatching(hidManager, NULL);

        ParseDefaults();

        NSSet* devices = (NSSet *)CFBridgingRelease(IOHIDManagerCopyDevices(hidManager));
        NSMutableArray *orderedDevices = [devices.allObjects mutableCopy];

        [orderedDevices removeObjectsAtIndexes:[orderedDevices indexesOfObjectsPassingTest:^BOOL(id obj, NSUInteger idx, BOOL *stop)
        {
            IOHIDDeviceRef device = (__bridge IOHIDDeviceRef)obj;

            NSNumber *usagePage = (NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDPrimaryUsagePageKey));
            NSNumber *usage = (NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDPrimaryUsageKey));

            return usagePage.intValue != kHIDPage_GenericDesktop || (usage.intValue != kHIDUsage_GD_GamePad && usage.intValue != kHIDUsage_GD_Joystick && usage.intValue != kHIDUsage_GD_Mouse);
        }]];

        [orderedDevices sortUsingComparator:^NSComparisonResult(id a, id b)
        {
            NSNumber *vendorA = (NSNumber *)IOHIDDeviceGetProperty((__bridge IOHIDDeviceRef)a, CFSTR(kIOHIDVendorIDKey));

            NSNumber *vendorB = (NSNumber *)IOHIDDeviceGetProperty((__bridge IOHIDDeviceRef)b, CFSTR(kIOHIDVendorIDKey));

            NSComparisonResult result = [vendorA compare:vendorB];

            if (result == NSOrderedSame)
            {
                NSNumber *productA = (NSNumber *)IOHIDDeviceGetProperty((__bridge IOHIDDeviceRef)a, CFSTR(kIOHIDProductIDKey));

                NSNumber *productB = (NSNumber *)IOHIDDeviceGetProperty((__bridge IOHIDDeviceRef)b, CFSTR(kIOHIDProductIDKey));

                result = [productA compare:productB];
            }

            if (result == NSOrderedSame)
            {
                NSNumber *portA = (NSNumber *)IOHIDDeviceGetProperty((__bridge IOHIDDeviceRef)a, CFSTR(kIOHIDLocationIDKey));

                NSNumber *portB = (NSNumber *)IOHIDDeviceGetProperty((__bridge IOHIDDeviceRef)b, CFSTR(kIOHIDLocationIDKey));

                result = [portA compare:portB];
            }

            return result;
        }];

        for (id device in orderedDevices)
        {
            AddDevice((__bridge IOHIDDeviceRef)device);
        }
    }
    else
    {
        printf("Unable to open IOHIDManager\n");
        hidManager = NULL;
    }
}

void ReleaseHID (void)
{
    if ( hidManager != NULL)
    {
        IOHIDManagerClose(hidManager, kIOHIDOptionsTypeNone);
    }
}

void SetPlayerForJoypad(int8 playerNum, uint32 vendorID, uint32 productID, uint32 index, int8 *oldPlayerNum)
{
    struct JoypadDevice device;
    device.vendorID = vendorID;
    device.productID = productID;
    device.index = index;

    if ( oldPlayerNum != NULL )
    {
        *oldPlayerNum = -1;

        if (playerNumByDevice.find(device) != playerNumByDevice.end())
        {
            *oldPlayerNum = playerNumByDevice[device];
        }
    }

    playerNumByDevice[device] = playerNum;
}

bool SetButtonCodeForJoypadControl(uint32 vendorID, uint32 productID, uint32 index, uint32 cookie, int32 value, S9xButtonCode buttonCode, bool overwrite, S9xButtonCode *oldButtonCode)
{
    if (buttonCode < 0 || buttonCode >= kNumButtons)
    {
        return false;
    }

    if (oldButtonCode != NULL)
    {
        *oldButtonCode = (S9xButtonCode)-1;
    }

    struct JoypadDevice device;
    device.vendorID = vendorID;
    device.productID = productID;
    device.index = index;

    struct JoypadCookie cookieStruct;
    cookieStruct.device = device;
    cookieStruct.cookie = cookie;

    if ( infoByCookie.find(cookieStruct) != infoByCookie.end())
    {
        auto info = infoByCookie[cookieStruct];

        if (info.usage != kHIDUsage_GD_Hatswitch && info.min != info.max)
        {
            if (value <= info.min)
            {
                value = info.min;
            }
            else if (value >= info.max)
            {
                value = info.max;
            }
            else
            {
                value = info.midpoint;
            }
        }
    }

    struct JoypadInput input;
    input.cookie = cookieStruct;
    input.value = value;

    if (buttonCodeByJoypadInput.find(input) == buttonCodeByJoypadInput.end())
    {
        overwrite = true;
    }
    else if (overwrite && oldButtonCode != NULL)
    {
        *oldButtonCode = buttonCodeByJoypadInput[input];
    }

    for (auto it = buttonCodeByJoypadInput.begin(); it != buttonCodeByJoypadInput.end();)
    {
        if (it->second == buttonCode && it->first.cookie.device == device)
        {
            if (overwrite)
            {
                buttonCodeByJoypadInput.erase(it++);
            }
            else
            {
                ++it;
            }
        }
        else
        {
            ++it;
        }
    }

    if (overwrite)
    {
        buttonCodeByJoypadInput[input] = buttonCode;
    }

    return true;
}

void ClearButtonCodeForJoypad(uint32 vendorID, uint32 productID, uint32 index, S9xButtonCode buttonCode)
{
    struct JoypadDevice device;
    device.vendorID = vendorID;
    device.productID = productID;
    device.index = index;

    for (auto it = buttonCodeByJoypadInput.begin(); it != buttonCodeByJoypadInput.end();)
    {
        if (it->first.cookie.device == device && it->second == buttonCode)
        {
            buttonCodeByJoypadInput.erase(it++);
        }
        else
        {
            ++it;
        }
    }
}

std::string LabelForInput(uint32 vendorID, uint32 productID, uint32 cookie, int32 value)
{
    struct JoypadDevice deviceStruct;
    deviceStruct.productID = productID;
    deviceStruct.vendorID = vendorID;
    deviceStruct.index = 0;

    struct JoypadCookie cookieStruct;
    cookieStruct.device = deviceStruct;
    cookieStruct.cookie = cookie;

    auto it = infoByCookie.find(cookieStruct);
    if (it != infoByCookie.end())
    {
        auto info = it->second;
        switch(info.usage)
        {
            case kHIDUsage_GD_X:
            {
                if (value <= info.min)
                {
                    return "X-";
                }
                else if (value >= info.max)
                {
                    return "X+";
                }
            }

            case kHIDUsage_GD_Y:
            {
                if (value <= info.min)
                {
                    return "Y-";
                }
                else if (value >= info.max)
                {
                    return "Y+";
                }
            }

            case kHIDUsage_GD_Z:
            {
                if (value <= info.min)
                {
                    return "Z-";
                }
                else if (value >= info.max)
                {
                    return "Z+";
                }
            }

            case kHIDUsage_GD_Rx:
            {
                if (value <= info.min)
                {
                    return "Right X-";
                }
                else if (value >= info.max)
                {
                    return "Right X+";
                }
            }

            case kHIDUsage_GD_Ry:
            {
                if (value <= info.min)
                {
                    return "Right Y-";
                }
                else if (value >= info.max)
                {
                    return "Right Y+";
                }
            }

            case kHIDUsage_GD_Rz:
            {
                if (value <= info.min)
                {
                    return "Right Z-";
                }
                else if (value >= info.max)
                {
                    return "Right Z+";
                }
            }

            case kHIDUsage_GD_Hatswitch:
            {
                auto defaultIT = defaultHatValues.find(deviceStruct);
                if (defaultIT != defaultHatValues.end())
                {
                    auto hatDict = defaultIT->second;
                    auto hatIT = hatDict.find(value);
                    if ( hatIT != hatDict.end())
                    {
                        switch (hatIT->second)
                        {
                            case kUp:
                                return "D-Pad Up";

                            case kDown:
                                return "D-Pad Down";

                            case kLeft:
                                return "D-Pad Left";

                            case kRight:
                                return "D-Pad Right";

                            default:
                                break;
                        }
                    }
                }

                if (value == 0)
                {
                    return "D-Pad Up";
                }
                else if (value == 2)
                {
                    return "D-Pad Right";
                }
                else if (value == 4)
                {
                    return "D-Pad Down";
                }
                else if (value == 6)
                {
                    return "D-Pad Left";
                }
            }

            default:
            {
                return std::string("Button " + std::to_string(info.index));
            }
        }
    }

    return std::to_string(cookie);
}
