/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "bml.h"
#include "cheats.h"
#include "snes9x.h"
#include "memmap.h"

static inline uint8 S9xGetByteFree(uint32 Address)
{
    int block = (Address & 0xffffff) >> MEMMAP_SHIFT;
    uint8 *GetAddress = Memory.Map[block];
    uint8 byte;

    if (GetAddress >= (uint8 *)CMemory::MAP_LAST)
    {
        byte = *(GetAddress + (Address & 0xffff));
        return (byte);
    }

    switch ((pint)GetAddress)
    {
    case CMemory::MAP_CPU:
        byte = S9xGetCPU(Address & 0xffff);
        return (byte);

    case CMemory::MAP_PPU:
        if (CPU.InDMAorHDMA && (Address & 0xff00) == 0x2100)
            return (OpenBus);

        byte = S9xGetPPU(Address & 0xffff);
        return (byte);

    case CMemory::MAP_LOROM_SRAM:
    case CMemory::MAP_SA1RAM:
        // Address & 0x7fff   : offset into bank
        // Address & 0xff0000 : bank
        // bank >> 1 | offset : SRAM address, unbound
        // unbound & SRAMMask : SRAM offset
        byte = *(Memory.SRAM + ((((Address & 0xff0000) >> 1) | (Address & 0x7fff)) & Memory.SRAMMask));
        return (byte);

    case CMemory::MAP_LOROM_SRAM_B:
        byte = *(Multi.sramB + ((((Address & 0xff0000) >> 1) | (Address & 0x7fff)) & Multi.sramMaskB));
        return (byte);

    case CMemory::MAP_HIROM_SRAM:
    case CMemory::MAP_RONLY_SRAM:
        byte = *(Memory.SRAM + (((Address & 0x7fff) - 0x6000 + ((Address & 0x1f0000) >> 3)) & Memory.SRAMMask));
        return (byte);

    case CMemory::MAP_BWRAM:
        byte = *(Memory.BWRAM + ((Address & 0x7fff) - 0x6000));
        return (byte);

    case CMemory::MAP_DSP:
        byte = S9xGetDSP(Address & 0xffff);
        return (byte);

    case CMemory::MAP_SPC7110_ROM:
        byte = S9xGetSPC7110Byte(Address);
        return (byte);

    case CMemory::MAP_SPC7110_DRAM:
        byte = S9xGetSPC7110(0x4800);
        return (byte);

    case CMemory::MAP_C4:
        byte = S9xGetC4(Address & 0xffff);
        return (byte);

    case CMemory::MAP_OBC_RAM:
        byte = S9xGetOBC1(Address & 0xffff);
        return (byte);

    case CMemory::MAP_SETA_DSP:
        byte = S9xGetSetaDSP(Address);
        return (byte);

    case CMemory::MAP_SETA_RISC:
        byte = S9xGetST018(Address);
        return (byte);

    case CMemory::MAP_BSX:
        byte = S9xGetBSX(Address);
        return (byte);

    case CMemory::MAP_NONE:
    default:
        byte = OpenBus;
        return (byte);
    }
}

static inline void S9xSetByteFree(uint8 Byte, uint32 Address)
{
    int block = (Address & 0xffffff) >> MEMMAP_SHIFT;
    uint8 *SetAddress = Memory.Map[block];

    if (SetAddress >= (uint8 *)CMemory::MAP_LAST)
    {
        *(SetAddress + (Address & 0xffff)) = Byte;
        return;
    }

    switch ((pint)SetAddress)
    {
    case CMemory::MAP_CPU:
        S9xSetCPU(Byte, Address & 0xffff);
        return;

    case CMemory::MAP_PPU:
        if (CPU.InDMAorHDMA && (Address & 0xff00) == 0x2100)
            return;

        S9xSetPPU(Byte, Address & 0xffff);
        return;

    case CMemory::MAP_LOROM_SRAM:
        if (Memory.SRAMMask)
        {
            *(Memory.SRAM + ((((Address & 0xff0000) >> 1) | (Address & 0x7fff)) & Memory.SRAMMask)) = Byte;
            CPU.SRAMModified = TRUE;
        }

        return;

    case CMemory::MAP_LOROM_SRAM_B:
        if (Multi.sramMaskB)
        {
            *(Multi.sramB + ((((Address & 0xff0000) >> 1) | (Address & 0x7fff)) & Multi.sramMaskB)) = Byte;
            CPU.SRAMModified = TRUE;
        }

        return;

    case CMemory::MAP_HIROM_SRAM:
        if (Memory.SRAMMask)
        {
            *(Memory.SRAM + (((Address & 0x7fff) - 0x6000 + ((Address & 0x1f0000) >> 3)) & Memory.SRAMMask)) = Byte;
            CPU.SRAMModified = TRUE;
        }
        return;

    case CMemory::MAP_BWRAM:
        *(Memory.BWRAM + ((Address & 0x7fff) - 0x6000)) = Byte;
        CPU.SRAMModified = TRUE;
        return;

    case CMemory::MAP_SA1RAM:
        *(Memory.SRAM + (Address & 0xffff)) = Byte;
        return;

    case CMemory::MAP_DSP:
        S9xSetDSP(Byte, Address & 0xffff);
        return;

    case CMemory::MAP_C4:
        S9xSetC4(Byte, Address & 0xffff);
        return;

    case CMemory::MAP_OBC_RAM:
        S9xSetOBC1(Byte, Address & 0xffff);
        return;

    case CMemory::MAP_SETA_DSP:
        S9xSetSetaDSP(Byte, Address);
        return;

    case CMemory::MAP_SETA_RISC:
        S9xSetST018(Byte, Address);
        return;

    case CMemory::MAP_BSX:
        S9xSetBSX(Byte, Address);
        return;

    case CMemory::MAP_NONE:
    default:
        return;
    }
}

void S9xInitWatchedAddress(void)
{
    for (unsigned int i = 0; i < sizeof(watches) / sizeof(watches[0]); i++)
        watches[i].on = false;
}

void S9xInitCheatData(void)
{
    Cheat.RAM = Memory.RAM;
    Cheat.SRAM = Memory.SRAM;
    Cheat.FillRAM = Memory.FillRAM;
}

static inline std::string trim(const std::string &&string)
{
    auto start = string.find_first_not_of(" \t\n\r");
    auto end   = string.find_last_not_of(" \t\n\r");
    if (start != std::string::npos && end != std::string::npos)
        return string.substr(start, end - start + 1);
    return "";
}

void S9xUpdateCheatInMemory(SCheat &c)
{
    uint8 byte;

    if (!c.enabled)
        return;

    byte = S9xGetByteFree(c.address);

    if (byte != c.byte)
    {
        /* The game wrote a different byte to the address, update saved_byte */
        c.saved_byte = byte;

        if (c.conditional)
        {
            if (c.saved_byte != c.cond_byte && c.cond_true)
            {
                /* Condition is now false, let the byte stand */
                c.cond_true = false;
            }
            else if (c.saved_byte == c.cond_byte && !c.cond_true)
            {
                c.cond_true = true;
                S9xSetByteFree(c.byte, c.address);
            }
        }
        else
            S9xSetByteFree(c.byte, c.address);
    }
    else if (c.conditional)
    {
        if (byte == c.cond_byte)
        {
            c.cond_true = true;
            c.saved_byte = byte;
            S9xSetByteFree(c.byte, c.address);
        }
    }
}

void S9xDisableCheat(SCheat &c)
{
    if (!c.enabled)
        return;

    if (!Cheat.enabled)
    {
        c.enabled = false;
        return;
    }

    /* Make sure we restore the up-to-date written byte */
    S9xUpdateCheatInMemory(c);
    c.enabled = false;

    if (c.conditional && !c.cond_true)
        return;

    S9xSetByteFree (c.saved_byte, c.address);
    c.cond_true = false;
}

void S9xDeleteCheatGroup(uint32 g)
{
    unsigned int i;

    if (g >= Cheat.group.size())
        return;

    for (i = 0; i < Cheat.group[g].cheat.size(); i++)
    {
        S9xDisableCheat(Cheat.group[g].cheat[i]);
    }

    Cheat.group.erase(Cheat.group.begin() + g);
}

void S9xDeleteCheats(void)
{
    for (size_t i = 0; i < Cheat.group.size(); i++)
    {
        S9xDisableCheatGroup(i);
    }

    Cheat.group.clear();
}

void S9xEnableCheat(SCheat &c)
{
    uint8 byte;

    if (c.enabled)
        return;

    c.enabled = true;

    if (!Cheat.enabled)
        return;

    byte = S9xGetByteFree(c.address);

    if (c.conditional)
    {
        if (byte != c.cond_byte)
            return;

        c.cond_true = true;
    }

    c.saved_byte = byte;
    S9xSetByteFree(c.byte, c.address);
}

void S9xEnableCheatGroup(uint32 num)
{
    for (auto &c : Cheat.group[num].cheat)
        S9xEnableCheat(c);

    Cheat.group[num].enabled = true;
}

void S9xDisableCheatGroup(uint32 num)
{
    for (auto &c : Cheat.group[num].cheat)
        S9xDisableCheat(c);

    Cheat.group[num].enabled = false;
}

static bool is_all_hex(const std::string &code)
{
    for (const auto &c : code)
    {
        if ((c < '0' || c > '9') &&
            (c < 'a' || c > 'f') &&
            (c < 'A' || c > 'F'))
            return false;
    }

    return true;
}

bool S9xProActionReplayToRaw(const std::string &code, uint32 &address, uint8 &byte)
{
    if (code.length() != 8 || !is_all_hex(code))
        return false;

    uint32 data = std::strtoul(code.c_str(), nullptr, 16);

    address = data >> 8;
    byte = (uint8)data;

    return true;
}

bool S9xGameGenieToRaw(const std::string &code, uint32 &address, uint8 &byte)
{
    if (code.length() != 9)
        return false;
    if (code[4] != '-')
        return false;
    if (!is_all_hex(code.substr(0, 4)))
        return false;
    if (!is_all_hex(code.substr(5, 4)))
        return false;

    auto new_code = code.substr(0, 4) + code.substr(5, 4);

    static const char *real_hex = "0123456789ABCDEF";
    static const char *genie_hex = "DF4709156BC8A23E";

    for (auto &c : new_code)
    {
        c = toupper(c);

        for (int i = 0; i < 16; i++)
        {
            if (genie_hex[i] == c)
            {
                c = real_hex[i];
                break;
            }
        }
    }

    uint32 data = strtoul(new_code.c_str(), nullptr, 16);
    byte = (uint8)(data >> 24);
    address = data & 0xffffff;
    address = ((address & 0x003c00) << 10) +
              ((address & 0x00003c) << 14) +
              ((address & 0xf00000) >> 8) +
              ((address & 0x000003) << 10) +
              ((address & 0x00c000) >> 6) +
              ((address & 0x0f0000) >> 12) +
              ((address & 0x0003c0) >> 6);

    return true;
}

SCheat S9xTextToCheat(const std::string &text)
{
    SCheat c;
    unsigned int byte = 0;
    unsigned int cond_byte = 0;

    c.enabled     = false;
    c.conditional = false;

    if (S9xGameGenieToRaw(text, c.address, c.byte))
    {
        byte = c.byte;
    }
    else if (S9xProActionReplayToRaw(text, c.address, c.byte))
    {
        byte = c.byte;
    }
    else if (sscanf(text.c_str(), "%x = %x ? %x", &c.address, &cond_byte, &byte) == 3)
    {
        c.conditional = true;
    }
    else if (sscanf(text.c_str(), "%x = %x", &c.address, &byte) == 2)
    {
    }
    else if (sscanf(text.c_str(), "%x / %x / %x", &c.address, &cond_byte, &byte) == 3)
    {
        c.conditional = true;
    }
    else if (sscanf(text.c_str(), "%x / %x", &c.address, &byte) == 2)
    {
    }
    else
    {
        c.address = 0;
        byte = 0;
    }

    c.byte = byte;
    c.cond_byte = cond_byte;

    return c;
}

std::vector<std::string> split_string(const std::string &str, unsigned char delim)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    size_t index;

    while (pos < str.length())
    {
        index = str.find(delim, pos);
        if (index == std::string::npos)
        {
            if (pos < str.length())
            {
                tokens.push_back(trim(str.substr(pos)));
            }

            break;
        }
        else if (index > pos)
        {
            tokens.push_back(trim(str.substr(pos, index - pos)));
        }

        pos = index + 1;
    }

    return tokens;
}

SCheatGroup S9xCreateCheatGroup(const std::string &name, const std::string &cheat)
{
    SCheatGroup g;

    g.name = name;
    g.enabled = false;

    auto cheats = split_string(cheat, '+');
    for (const auto &c : cheats)
    {
        SCheat new_cheat = S9xTextToCheat(c);
        if (new_cheat.address)
            g.cheat.push_back(new_cheat);
    }

    return g;
}

int S9xAddCheatGroup(const std::string &name, const std::string &cheat)
{
    SCheatGroup g = S9xCreateCheatGroup(name, cheat);
    if (g.cheat.size() == 0)
        return -1;

    Cheat.group.push_back(g);

    return Cheat.group.size() - 1;
}

int S9xModifyCheatGroup(uint32 num, const std::string &name, const std::string &cheat)
{
	if (num >= Cheat.group.size())
		return -1;

    S9xDisableCheatGroup(num);

    Cheat.group[num] = S9xCreateCheatGroup(name, cheat);

    return num;
}

std::string S9xCheatToText(const SCheat &c)
{
    char output[256]{};

    if (c.conditional)
        sprintf(output, "%06x=%02x?%02x", c.address, c.cond_byte, c.byte);
    else
        sprintf(output, "%06x=%02x", c.address, c.byte);

    return std::string(output);
}

std::string S9xCheatGroupToText(SCheatGroup &g)
{
    std::string text = "";

    for (size_t i = 0; i < g.cheat.size(); i++)
    {
        text += S9xCheatToText(g.cheat[i]);
        if (i != g.cheat.size() - 1)
            text += "+";
    }

    return text;
}

std::string S9xCheatValidate(const std::string &code_string)
{
    SCheatGroup g = S9xCreateCheatGroup("temp", code_string);

    if (g.cheat.size() > 0)
    {
        return S9xCheatGroupToText(g);
    }

    return "";
}

std::string S9xCheatGroupToText(uint32 num)
{
    if (num >= Cheat.group.size())
        return "";

    return S9xCheatGroupToText(Cheat.group[num]);
}

void S9xUpdateCheatsInMemory(void)
{
    if (!Cheat.enabled)
        return;

    for (auto &group : Cheat.group)
        for (auto &cheat : group.cheat)
            S9xUpdateCheatInMemory(cheat);
}

static bool S9xCheatIsDuplicate(const std::string &name, const std::string &code)
{
    for (size_t i = 0; i < Cheat.group.size(); i++)
    {
        if (Cheat.group[i].name == name)
        {
            auto code_string = S9xCheatGroupToText(i);
            auto validated_string = S9xCheatValidate(code);

            if (validated_string == code_string)
                return true;
        }
    }

    return false;
}

static void S9xLoadCheatsFromBMLNode(bml_node &n)
{
    for (auto &c : n.child)
    {
        if (strcasecmp(c.name.c_str(), "cheat"))
            continue;

        auto subnode = c.find_subnode("code");
        if (!subnode)
            continue;
        std::string code = subnode->data;

        std::string name;
        subnode = c.find_subnode("name");
        if (subnode)
            name = subnode->data;

        bool enable = false;
        if (c.find_subnode("enable"))
            enable = true;

        if (S9xCheatIsDuplicate(name, code))
            continue;

        auto index = S9xAddCheatGroup(name, code);
        if (enable)
            S9xEnableCheatGroup(index);
    }

    return;
}

static bool8 S9xLoadCheatFileClassic(const std::string &filename)
{
    FILE *fs;
    uint8 data[28];

    fs = fopen(filename.c_str(), "rb");
    if (!fs)
        return (FALSE);

    while (fread(data, 1, 28, fs) == 28)
    {
        SCheat c;
        c.enabled = (data[0] & 4) == 0;
        c.byte = data[1];
        c.address = data[2] | (data[3] << 8) | (data[4] << 16);

        std::string name((const char *)&data[8], 20);
        char code[32]{};
        sprintf(code, "%x=%x", c.address, c.byte);
        std::string cheat(code);
        S9xAddCheatGroup(name, cheat);

        if (c.enabled)
            S9xEnableCheatGroup(Cheat.group.size() - 1);
    }

    fclose(fs);

    return TRUE;
}

bool8 S9xLoadCheatFile(const std::string &filename)
{
    bml_node bml;
    if (!bml.parse_file(filename))
    {
        return S9xLoadCheatFileClassic(filename);
    }

    bml_node *n = bml.find_subnode("cheat");
    if (n)
    {
        S9xLoadCheatsFromBMLNode(bml);
    }

    if (!n)
    {
        return S9xLoadCheatFileClassic(filename);
    }

    return (TRUE);
}

bool8 S9xSaveCheatFile(const std::string &filename)
{
    unsigned int i;
    FILE *file = NULL;

    if (Cheat.group.size() == 0)
    {
        remove(filename.c_str());
        return TRUE;
    }

    file = fopen(filename.c_str(), "w");

    if (!file)
        return FALSE;

    for (i = 0; i < Cheat.group.size(); i++)
    {
        fprintf(file,
                "cheat\n"
                "  name: %s\n"
                "  code: %s\n"
                "%s\n",
                Cheat.group[i].name.c_str(),
                S9xCheatGroupToText(i).c_str(),
                Cheat.group[i].enabled ? "  enable\n" : "");
    }

    fclose(file);

    return TRUE;
}

void S9xCheatsDisable(void)
{
    unsigned int i;

    if (!Cheat.enabled)
        return;

    for (i = 0; i < Cheat.group.size(); i++)
    {
        if (Cheat.group[i].enabled)
        {
            S9xDisableCheatGroup(i);
            Cheat.group[i].enabled = TRUE;
        }
    }

    Cheat.enabled = FALSE;
}

void S9xCheatsEnable(void)
{
    unsigned int i;

    if (Cheat.enabled)
        return;

    Cheat.enabled = TRUE;

    for (i = 0; i < Cheat.group.size(); i++)
    {
        if (Cheat.group[i].enabled)
        {
            Cheat.group[i].enabled = FALSE;
            S9xEnableCheatGroup(i);
        }
    }
}

int S9xImportCheatsFromDatabase(const std::string &filename)
{
    char sha256_txt[65];
    char hextable[] = "0123456789abcdef";
    unsigned int i;

    bml_node bml;
    if (!bml.parse_file(filename))
        return -1; // No file

    for (i = 0; i < 32; i++)
    {
        sha256_txt[i * 2]     = hextable[Memory.ROMSHA256[i] >> 4];
        sha256_txt[i * 2 + 1] = hextable[Memory.ROMSHA256[i] & 0xf];
    }
    sha256_txt[64] = '\0';

    for (auto &c : bml.child)
    {
        if (!strcasecmp(c.name.c_str(), "cartridge"))
        {
            auto n = c.find_subnode("sha256");

            if (n && !strcasecmp(n->data.c_str(), sha256_txt))
            {
                S9xLoadCheatsFromBMLNode(c);
                return 0;
            }
        }
    }

    return -2; /* No codes */
}
