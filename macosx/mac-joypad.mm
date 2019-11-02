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
#include <unordered_map>
#include <unordered_set>

#include "port.h"

#include <IOKit/hid/IOHIDDevice.h>
#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDUsageTables.h>
#include "HID_Utilities_External.h"

#include "mac-prefix.h"
#include "mac-dialog.h"
#include "mac-os.h"
#include "mac-joypad.h"

#define	kUp(i)					(i * 4)
#define	kDn(i)					(i * 4 + 1)
#define	kLf(i)					(i * 4 + 2)
#define	kRt(i)					(i * 4 + 3)

#define	kPadElemTypeNone		0
#define	kPadElemTypeHat4		1
#define	kPadElemTypeHat8		2
#define	kPadElemTypeAxis		3
#define	kPadElemTypeButton		4
#define	kPadElemTypeOtherHat4	5
#define	kPadElemTypeOtherHat8	6

#define	kPadXAxis				1
#define	kPadYAxis				0
#define	kPadHat					0

#define	kMaskUp					0x0800
#define	kMaskDn					0x0400
#define	kMaskLf					0x0200
#define	kMaskRt					0x0100

typedef	hu_device_t		*pRecDevice;
typedef	hu_element_t	*pRecElement;

struct JoypadDevice {
    uint16 vendorID;
    uint16 productID;
    uint32 index;

    bool operator==(const struct JoypadDevice &o) const
    {
        return vendorID == o.vendorID && productID == o.productID && index == o.index;
    }

    bool operator<(const struct JoypadDevice &o) const
    {
        return vendorID < o.vendorID || productID < o.productID || index < o.index;
    }
};

struct JoypadCookie {
    struct JoypadDevice device;
    uint32 cookie;

    JoypadCookie() {}

    struct JoypadCookie &operator=(const struct JoypadCookie &o)
    {
        device = o.device;
        cookie = o.cookie;
        return *this;
    }

    bool operator==(const struct JoypadCookie &o) const
    {
        return device == o.device && cookie == o.cookie;
    }

    bool operator<(const struct JoypadCookie &o) const
    {
        return device < o.device || cookie < o.cookie;
    }
};

struct JoypadCookieInfo {
    uint32 usage;
    uint32 index;
    int32 midpoint;
    int32 min;
    int32 max;
};

struct JoypadInput {
    struct JoypadCookie cookie;
    int32 value;

    bool operator==(const struct JoypadInput &o) const
    {
        return cookie == o.cookie && value == o.value;
    }

    bool operator<(const struct JoypadInput &o) const
    {
        return cookie < o.cookie || value < o.value;
    }
};

namespace std {
    template <>
    struct hash<struct JoypadDevice>
    {
        std::size_t operator()(const JoypadDevice& k) const
        {
            return k.vendorID ^ k.productID ^ k.index;
        }
    };

    template <>
    struct hash<struct JoypadCookie>
    {
        std::size_t operator()(const JoypadCookie& k) const
        {
            return std::hash<struct JoypadDevice>()(k.device) ^ k.cookie;
        }
    };

    template <>
    struct hash<struct JoypadInput>
    {
        std::size_t operator()(const JoypadInput& k) const
        {
            return std::hash<struct JoypadCookie>()(k.cookie) ^ k.value;
        }
    };
}

std::unordered_set<JoypadDevice> allDevices;
std::unordered_map<JoypadDevice, std::map<uint8, std::map<int8, S9xButtonCode>>> defaultAxes;
std::unordered_map<JoypadDevice, std::map<uint8, S9xButtonCode>> defaultButtons;
std::unordered_map<JoypadDevice, std::map<uint8, S9xButtonCode>> defaultHatValues;
// TODO: Hook these next two up
std::unordered_map<JoypadDevice, int8> playerNumByDevice;
std::unordered_map<uint32, int8> deviceIndexByPort;
std::unordered_map<JoypadCookie, JoypadCookieInfo> infoByCookie;
std::unordered_map<JoypadInput, S9xButtonCode> buttonCodeByJoypadInput;

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

void gamepadAction(void *inContext, IOReturn inResult, void *inSender, IOHIDValueRef v) {
    os_unfair_lock_lock(&keyLock);

    IOHIDDeviceRef device = (IOHIDDeviceRef) inSender;
    uint32 port = ((NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDLocationIDKey))).unsignedIntValue;

    if (deviceIndexByPort.find(port) == deviceIndexByPort.end())
    {
        os_unfair_lock_unlock(&keyLock);
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
        os_unfair_lock_unlock(&keyLock);
        return;
    }

    if ( playerNumByDevice.find(deviceStruct) == playerNumByDevice.end())
    {
        os_unfair_lock_unlock(&keyLock);
        return;
    }

    int8 playerNum = playerNumByDevice[deviceStruct];
    if (playerNum < 0 || playerNum >= MAC_MAX_PLAYERS)
    {
        os_unfair_lock_unlock(&keyLock);
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

        struct JoypadInput oppositeInputStruct = inputStruct;

        if (info.min != info.max)
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
                pressedGamepadButtons[playerNum][buttonCodeByJoypadInput[oppositeInputStruct]] = false;
            }
            else
            {
                oppositeInputStruct.value = info.min;
                pressedGamepadButtons[playerNum][buttonCodeByJoypadInput[oppositeInputStruct]] = false;
                oppositeInputStruct.value = info.max;
                pressedGamepadButtons[playerNum][buttonCodeByJoypadInput[oppositeInputStruct]] = false;
            }
        }
        else if (info.usage == kHIDUsage_GD_Hatswitch)
        {
            int32 value = inputStruct.value;

            inputStruct.value = 1;
            if (buttonCodeByJoypadInput.find(inputStruct) != buttonCodeByJoypadInput.end())
            {
                pressedGamepadButtons[playerNum][buttonCodeByJoypadInput[inputStruct]] = (value & inputStruct.value);
            }

            inputStruct.value = 2;
            if (buttonCodeByJoypadInput.find(inputStruct) != buttonCodeByJoypadInput.end())
            {
                pressedGamepadButtons[playerNum][buttonCodeByJoypadInput[inputStruct]] = (value & inputStruct.value);
            }

            inputStruct.value = 4;
            if (buttonCodeByJoypadInput.find(inputStruct) != buttonCodeByJoypadInput.end())
            {
                pressedGamepadButtons[playerNum][buttonCodeByJoypadInput[inputStruct]] = (value & inputStruct.value);
            }

            inputStruct.value = 8;
            if (buttonCodeByJoypadInput.find(inputStruct) != buttonCodeByJoypadInput.end())
            {
                pressedGamepadButtons[playerNum][buttonCodeByJoypadInput[inputStruct]] = (value & inputStruct.value);
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

    os_unfair_lock_unlock(&keyLock);
}

void findControls(struct JoypadDevice &device, NSDictionary *properties, NSMutableArray<NSDictionary *> *buttons, NSMutableArray<NSDictionary *> *axes, int64 *hat)
{
    if (properties == nil)
    {
        return;
    }

    int usagePage = [properties[@kIOHIDElementUsagePageKey] intValue];
    int usage = [properties[@kIOHIDElementUsageKey] intValue];
    if (usagePage == kHIDPage_Button)
    {
        [buttons addObject:properties];
    }
    else if (usagePage == kHIDPage_GenericDesktop && (usage == kHIDUsage_GD_X || usage == kHIDUsage_GD_Y || usage == kHIDUsage_GD_Z || usage == kHIDUsage_GD_Rx || usage == kHIDUsage_GD_Ry || usage == kHIDUsage_GD_Rz))
    {
        [axes addObject:properties];
    }
    else if (usagePage == kHIDPage_GenericDesktop && usage == kHIDUsage_GD_Hatswitch)
    {
        if (hat != NULL)
        {
            *hat = [properties[@kIOHIDElementCookieKey] intValue];
        }
    }
    else
    {
        for ( NSDictionary *child in properties[@kIOHIDElementKey] )
        {
            findControls(device, child, buttons, axes, hat);
        }
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

    NSMutableArray<NSDictionary *> *buttons = [NSMutableArray new];
    NSMutableArray<NSDictionary *> *axes = [NSMutableArray new];
    int64 hat = -1;

    struct JoypadDevice deviceStruct;
    deviceStruct.vendorID = vendor.unsignedIntValue;
    deviceStruct.productID = product.unsignedIntValue;
    deviceStruct.index = 0;

    struct JoypadDevice defaultsKey = deviceStruct;

    while (allDevices.find(deviceStruct) != allDevices.end())
    {
        deviceStruct.index += 1;
    }

    allDevices.insert(deviceStruct);
    uint32_t port = ((NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDLocationIDKey))).unsignedIntValue;
    deviceIndexByPort[port] = deviceStruct.index;

    CFMutableDictionaryRef properties = NULL;

    IORegistryEntryCreateCFProperties(IOHIDDeviceGetService(device), &properties, kCFAllocatorDefault, kNilOptions);

    for ( NSDictionary *child in ((__bridge NSDictionary *)properties)[@kIOHIDElementKey] )
    {
        findControls(deviceStruct, child, buttons, axes, &hat);
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

        // TODO: Extend axisIndex into defaultAxes
        info.index = axisIndex++;
        infoByCookie[cookie] = info;
    }

    if (hat >= 0)
    {
        struct JoypadCookie cookie;
        struct JoypadCookieInfo info;

        cookie.device = deviceStruct;
        cookie.cookie = (uint32)hat;

        info.usage = kHIDUsage_GD_Hatswitch;
        info.min = 0;
        info.max = 0;
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

void SetUpHID (void)
{
    hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    IOHIDManagerRegisterInputValueCallback(hidManager, gamepadAction, NULL);
    IOHIDManagerScheduleWithRunLoop(hidManager, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
    IOHIDManagerSetDeviceMatching(hidManager, NULL);

    ParseDefaults();

    if (hidManager != NULL && IOHIDManagerOpen(hidManager, kIOHIDOptionsTypeNone) == kIOReturnSuccess)
    {
        NSSet* devices = (NSSet *)CFBridgingRelease(IOHIDManagerCopyDevices(hidManager));
        NSMutableArray *orderedDevices = [devices.allObjects mutableCopy];

        [orderedDevices removeObjectsAtIndexes:[orderedDevices indexesOfObjectsPassingTest:^BOOL(id obj, NSUInteger idx, BOOL *stop)
        {
            IOHIDDeviceRef device = (__bridge IOHIDDeviceRef)obj;

            NSNumber *usagePage = (NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDPrimaryUsagePageKey));
            NSNumber *usage = (NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDPrimaryUsageKey));

            return usagePage.intValue != kHIDPage_GenericDesktop || (usage.intValue != kHIDUsage_GD_GamePad && usage.intValue != kHIDUsage_GD_Joystick);
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

        if (orderedDevices.count == 1)
        {
            const struct JoypadDevice &deviceStruct = *(allDevices.begin());
            SetPlayerForJoypad(0, deviceStruct.vendorID, deviceStruct.productID, deviceStruct.index, NULL);
        }
    }
    else
    {
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

void SetPlayerForJoypad(int8 playerNum, uint32 vendorID, uint32 productID, uint8 index, int8 *oldPlayerNum)
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

void SetButtonCodeForJoypadControl(uint32 vendorID, uint32 productID, uint8 index, uint32 cookie, int32 value, S9xButtonCode buttonCode, bool overwrite, S9xButtonCode *oldButtonCode)
{
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

        if ( info.min != info.max )
        {
            if (value < info.min)
            {
                value = info.min;
            }
            else if (value > info.max)
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

    if (overwrite)
    {
        buttonCodeByJoypadInput[input] = buttonCode;
    }
}
