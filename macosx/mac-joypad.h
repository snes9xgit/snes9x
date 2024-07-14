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


#ifndef _mac_joypad_h_
#define _mac_joypad_h_

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "mac-controls.h"

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

void SetUpHID (void);
void ReleaseHID (void);

std::unordered_set<struct JoypadDevice> ListJoypads (void);
std::string NameForDevice(struct JoypadDevice device);

void SetPlayerForJoypad(int8 playerNum, uint32 vendorID, uint32 productID, uint32 index, int8 *oldPlayerNum);
bool SetButtonCodeForJoypadControl(uint32 vendorID, uint32 productID, uint32 index, uint32 cookie, int32 value, S9xButtonCode buttonCode, bool overwrite, S9xButtonCode *oldButtonCode);
void ClearButtonCodeForJoypad(uint32 vendorID, uint32 productID, uint32 index, S9xButtonCode buttonCode);

void ClearJoypad(uint32 vendorID, uint32 productID, uint32 index);
std::unordered_map<struct JoypadInput, S9xButtonCode> GetJoypadButtons(uint32 vendorID, uint32 productID, uint32 index);

std::string LabelForInput(uint32 vendorID, uint32 productID, uint32 cookie, int32 value);

#endif
