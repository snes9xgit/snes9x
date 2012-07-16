//RamSearch dialog was copied and adapted from GENS11: http://code.google.com/p/gens-rerecording/
//Authors: Upthorn, Nitsuja, adelikat

#ifndef RAM_SEARCH_H
#define RAM_SEARCH_H


extern char rs_type_size;
extern int ResultCount;
typedef unsigned int HWAddressType;

unsigned int sizeConv(unsigned int index,char size, char *prevSize = &rs_type_size, bool usePrev = false);
unsigned int GetRamValue(unsigned int Addr,char Size);
void prune(char Search, char Operater, char Type, int Value, int OperatorParameter);
void CompactAddrs();
void reset_address_info();
void signal_new_frame();
void signal_new_size();
void UpdateRamSearchTitleBar(int percent = 0);
void SetRamSearchUndoType(HWND hDlg, int type);
unsigned int ReadValueAtHardwareAddress(HWAddressType address, unsigned int size);
bool WriteValueAtHardwareAddress(HWAddressType address, unsigned int value, unsigned int size);
bool IsRAMWatchAddressValid(HWAddressType address);
extern int curr_ram_size;
extern bool noMisalign;
extern HWND RamSearchHWnd;


void ResetResults();
void CloseRamWindows(); //Close the Ram Search & Watch windows when rom closes
void ReopenRamWindows(); //Reopen them when a new Rom is loaded
void Update_RAM_Search(); //keeps RAM values up to date in the search and watch windows


#define RW_VIRTUAL_ADDR_SHIFT	24
#define RW_VIRTUAL_ADDR_MASK	((1<<(RW_VIRTUAL_ADDR_SHIFT))-1)
#define RW_VIRTUAL_ADDR_SRAM	1
#define RW_VIRTUAL_ADDR_FILLRAM	2


static inline uint8* RWInternalToSoftwareAddress(HWAddressType address)
{
	if(Settings.StopEmulation)
		return NULL;

	HWAddressType base = address & RW_VIRTUAL_ADDR_MASK;
	HWAddressType type = address >> RW_VIRTUAL_ADDR_SHIFT;

	switch(type) {
	case RW_VIRTUAL_ADDR_SRAM:
		if (Memory.SRAM != NULL && base < 0x20000)
			return &Memory.SRAM[base];
		break;
	case RW_VIRTUAL_ADDR_FILLRAM:
		if (Memory.FillRAM != NULL && base < 0x8000)
			return &Memory.FillRAM[base];
		break;
	default:
		if (Memory.RAM != NULL && address >= 0x7e0000 && address <= 0x7fffff)
			return &Memory.RAM[address & 0x1ffff];
	}
	return NULL;
}

static inline HWAddressType DisplayToRWInternalAddress(const TCHAR* str)
{
	HWAddressType base, type = 0;
	switch(str[0]) {
	case 'S':
	case 's':
		base = _tcstoul(&str[1], NULL, 16);
		type = RW_VIRTUAL_ADDR_SRAM;
		break;
	case 'I':
	case 'i':
		base = _tcstoul(&str[1], NULL, 16);
		type = RW_VIRTUAL_ADDR_FILLRAM;
		break;
	default:
		base = _tcstoul(str, NULL, 16);
	}
	return (type << RW_VIRTUAL_ADDR_SHIFT) | (base & RW_VIRTUAL_ADDR_MASK);
}

static inline const TCHAR* RWInternalToDisplayAddress(HWAddressType address)
{
	static TCHAR str[11];
	HWAddressType base = address & RW_VIRTUAL_ADDR_MASK;
	HWAddressType type = address >> RW_VIRTUAL_ADDR_SHIFT;

	// must return 6 characters
	switch(type) {
	case RW_VIRTUAL_ADDR_SRAM:
		_stprintf(str, TEXT("s%05X"), base);
		break;
	case RW_VIRTUAL_ADDR_FILLRAM:
		_stprintf(str, TEXT("i%05X"), base);
		break;
	default:
		_stprintf(str, TEXT("%06X"), base);
	}
	return str;
}

static inline HWAddressType RWInternalToHardwareAddress(HWAddressType address)
{
	HWAddressType base = address & RW_VIRTUAL_ADDR_MASK;
	HWAddressType type = address >> RW_VIRTUAL_ADDR_SHIFT;

	switch(type) {
	case RW_VIRTUAL_ADDR_SRAM:
		return 0x700000 | (base & 0x1ffff); // FIXME: incorrect, but better than nothing
	case RW_VIRTUAL_ADDR_FILLRAM: // FIXME
	default:
		return base;
	}
}


#endif
