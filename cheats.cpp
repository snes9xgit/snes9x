/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <ctype.h>
#include "snes9x.h"
#include "memmap.h"
#include "cheats.h"

#define WRAM_BITS	ALL_BITS
#define SRAM_BITS	ALL_BITS + (0x20000 >> 5)
#define IRAM_BITS	ALL_BITS + (0x30000 >> 5)

#define BIT_CLEAR(a, v)	(a)[(v) >> 5] &= ~(1 << ((v) & 31))

#define TEST_BIT(a, v)	((a)[(v) >> 5] & (1 << ((v) & 31)))

#define _S9XCHTC(c, a, b) \
	((c) == S9X_LESS_THAN             ? (a) <  (b) : \
	 (c) == S9X_GREATER_THAN          ? (a) >  (b) : \
	 (c) == S9X_LESS_THAN_OR_EQUAL    ? (a) <= (b) : \
	 (c) == S9X_GREATER_THAN_OR_EQUAL ? (a) >= (b) : \
	 (c) == S9X_EQUAL                 ? (a) == (b) : \
	                                    (a) != (b))

#define _S9XCHTD(s, m, o) \
	((s) == S9X_8_BITS  ? ((uint8)   (*((m) + (o)))) : \
	 (s) == S9X_16_BITS ? ((uint16)  (*((m) + (o)) + (*((m) + (o) + 1) << 8))) : \
	 (s) == S9X_24_BITS ? ((uint32)  (*((m) + (o)) + (*((m) + (o) + 1) << 8) + (*((m) + (o) + 2) << 16))) : \
	                      ((uint32)  (*((m) + (o)) + (*((m) + (o) + 1) << 8) + (*((m) + (o) + 2) << 16) + (*((m) + (o) + 3) << 24))))

#define _S9XCHTDS(s, m, o) \
	((s) == S9X_8_BITS  ?  ((int8)   (*((m) + (o)))) : \
	 (s) == S9X_16_BITS ?  ((int16)  (*((m) + (o)) + (*((m) + (o) + 1) << 8))) : \
	 (s) == S9X_24_BITS ? (((int32) ((*((m) + (o)) + (*((m) + (o) + 1) << 8) + (*((m) + (o) + 2) << 16)) << 8)) >> 8): \
                           ((int32)  (*((m) + (o)) + (*((m) + (o) + 1) << 8) + (*((m) + (o) + 2) << 16) + (*((m) + (o) + 3) << 24))))

static void S9xSetBitRange (uint32 *bits, int start, int count)
{
	// start is 32-aligned for all three regions (0x00000/0x20000/0x30000)
	int full = count >> 5;
	if (full)
		memset(bits + (start >> 5), 0xff, full * sizeof(uint32));

	int rem = count & 31;
	if (rem)
		bits[(start >> 5) + full] = (1u << rem) - 1;
}

void S9xStartCheatSearch (SCheatData *d)
{
	if (d->RAM && d->ram_size)
		memmove(d->CWRAM, d->RAM, d->ram_size);
	if (d->SRAM && d->sram_size)
		memmove(d->CSRAM, d->SRAM, d->sram_size);
	if (d->IRAM && d->iram_size)
		memmove(d->CIRAM, d->IRAM, d->iram_size);

	memset((char *) d->ALL_BITS, 0, 0x32000 >> 3);
	S9xSetBitRange(d->ALL_BITS, 0x00000, d->RAM  ? d->ram_size  : 0);
	S9xSetBitRange(d->ALL_BITS, 0x20000, d->SRAM ? d->sram_size : 0);
	S9xSetBitRange(d->ALL_BITS, 0x30000, d->IRAM ? d->iram_size : 0);
}

static void S9xClearTrailingBits (SCheatData *d, int l)
{
	int	i;
	int	ram_n  = (int) d->ram_size;
	int	sram_n = (int) d->sram_size;
	int	iram_n = (int) d->iram_size;

	for (i = ram_n - l < 0 ? 0 : ram_n - l; i < ram_n; i++)
		BIT_CLEAR(d->WRAM_BITS, i);

	for (i = sram_n - l < 0 ? 0 : sram_n - l; i < sram_n; i++)
		BIT_CLEAR(d->SRAM_BITS, i);

	for (i = iram_n - l < 0 ? 0 : iram_n - l; i < iram_n; i++)
		BIT_CLEAR(d->IRAM_BITS, i);
}

void S9xSearchForChange (SCheatData *d, S9xCheatComparisonType cmp, S9xCheatDataSize size, bool8 is_signed, bool8 update)
{
	int	l, i;
	int	ram_n  = (int) d->ram_size;
	int	sram_n = (int) d->sram_size;
	int	iram_n = (int) d->iram_size;

	switch (size)
	{
		case S9X_8_BITS:	l = 0; break;
		case S9X_16_BITS:	l = 1; break;
		case S9X_24_BITS:	l = 2; break;
		default:
		case S9X_32_BITS:	l = 3; break;
	}

	if (is_signed)
	{
		for (i = 0; i < ram_n - l; i++)
		{
			if (TEST_BIT(d->WRAM_BITS, i) && _S9XCHTC(cmp, _S9XCHTDS(size, d->RAM, i), _S9XCHTDS(size, d->CWRAM, i)))
			{
				if (update)
					d->CWRAM[i] = d->RAM[i];
			}
			else
				BIT_CLEAR(d->WRAM_BITS, i);
		}

		for (i = 0; i < sram_n - l; i++)
		{
			if (TEST_BIT(d->SRAM_BITS, i) && _S9XCHTC(cmp, _S9XCHTDS(size, d->SRAM, i), _S9XCHTDS(size, d->CSRAM, i)))
			{
				if (update)
					d->CSRAM[i] = d->SRAM[i];
			}
			else
				BIT_CLEAR(d->SRAM_BITS, i);
		}

		for (i = 0; i < iram_n - l; i++)
		{
			if (TEST_BIT(d->IRAM_BITS, i) && _S9XCHTC(cmp, _S9XCHTDS(size, d->IRAM, i), _S9XCHTDS(size, d->CIRAM, i)))
			{
				if (update)
					d->CIRAM[i] = d->IRAM[i];
			}
			else
				BIT_CLEAR(d->IRAM_BITS, i);
		}
	}
	else
	{
		for (i = 0; i < ram_n - l; i++)
		{
			if (TEST_BIT(d->WRAM_BITS, i) && _S9XCHTC(cmp, _S9XCHTD(size, d->RAM, i), _S9XCHTD(size, d->CWRAM, i)))
			{
				if (update)
					d->CWRAM[i] = d->RAM[i];
			}
			else
				BIT_CLEAR(d->WRAM_BITS, i);
		}

		for (i = 0; i < sram_n - l; i++)
		{
			if (TEST_BIT(d->SRAM_BITS, i) && _S9XCHTC(cmp, _S9XCHTD(size, d->SRAM, i), _S9XCHTD(size, d->CSRAM, i)))
			{
				if (update)
					d->CSRAM[i] = d->SRAM[i];
			}
			else
				BIT_CLEAR(d->SRAM_BITS, i);
		}

		for (i = 0; i < iram_n - l; i++)
		{
			if (TEST_BIT(d->IRAM_BITS, i) && _S9XCHTC(cmp, _S9XCHTD(size, d->IRAM, i), _S9XCHTD(size, d->CIRAM, i)))
			{
				if (update)
					d->CIRAM[i] = d->IRAM[i];
			}
			else
				BIT_CLEAR(d->IRAM_BITS, i);
		}
	}

	S9xClearTrailingBits(d, l);
}

void S9xSearchForValue (SCheatData *d, S9xCheatComparisonType cmp, S9xCheatDataSize size, uint32 value, bool8 is_signed, bool8 update)
{
	int	l, i;
	int	ram_n  = (int) d->ram_size;
	int	sram_n = (int) d->sram_size;
	int	iram_n = (int) d->iram_size;

	switch (size)
	{
		case S9X_8_BITS:	l = 0; break;
		case S9X_16_BITS:	l = 1; break;
		case S9X_24_BITS:	l = 2; break;
		default:
		case S9X_32_BITS:	l = 3; break;
	}

	if (is_signed)
	{
		for (i = 0; i < ram_n - l; i++)
		{
			if (TEST_BIT(d->WRAM_BITS, i) && _S9XCHTC(cmp, _S9XCHTDS(size, d->RAM, i), (int32) value))
			{
				if (update)
					d->CWRAM[i] = d->RAM[i];
			}
			else
				BIT_CLEAR(d->WRAM_BITS, i);
		}

		for (i = 0; i < sram_n - l; i++)
		{
			if (TEST_BIT(d->SRAM_BITS, i) && _S9XCHTC(cmp, _S9XCHTDS(size, d->SRAM, i), (int32) value))
			{
				if (update)
					d->CSRAM[i] = d->SRAM[i];
			}
			else
				BIT_CLEAR(d->SRAM_BITS, i);
		}

		for (i = 0; i < iram_n - l; i++)
		{
			if (TEST_BIT(d->IRAM_BITS, i) && _S9XCHTC(cmp, _S9XCHTDS(size, d->IRAM, i), (int32) value))
			{
				if (update)
					d->CIRAM[i] = d->IRAM[i];
			}
			else
				BIT_CLEAR(d->IRAM_BITS, i);
		}
	}
	else
	{
		for (i = 0; i < ram_n - l; i++)
		{
			if (TEST_BIT(d->WRAM_BITS, i) && _S9XCHTC(cmp, _S9XCHTD(size, d->RAM, i), value))
			{
				if (update)
					d->CWRAM[i] = d->RAM[i];
			}
			else
				BIT_CLEAR(d->WRAM_BITS, i);
		}

		for (i = 0; i < sram_n - l; i++)
		{
			if (TEST_BIT(d->SRAM_BITS, i) && _S9XCHTC(cmp, _S9XCHTD(size, d->SRAM, i), value))
			{
				if (update)
					d->CSRAM[i] = d->SRAM[i];
			}
			else
				BIT_CLEAR(d->SRAM_BITS, i);
		}

		for (i = 0; i < iram_n - l; i++)
		{
			if (TEST_BIT(d->IRAM_BITS, i) && _S9XCHTC(cmp, _S9XCHTD(size, d->IRAM, i), value))
			{
				if (update)
					d->CIRAM[i] = d->IRAM[i];
			}
			else
				BIT_CLEAR(d->IRAM_BITS, i);
		}
	}

	S9xClearTrailingBits(d, l);
}

void S9xSearchForAddress (SCheatData *d, S9xCheatComparisonType cmp, S9xCheatDataSize size, uint32 value, bool8 update)
{
	int	l, i;
	int	ram_n  = (int) d->ram_size;
	int	sram_n = (int) d->sram_size;
	int	iram_n = (int) d->iram_size;

	switch (size)
	{
		case S9X_8_BITS:	l = 0; break;
		case S9X_16_BITS:	l = 1; break;
		case S9X_24_BITS:	l = 2; break;
		default:
		case S9X_32_BITS:	l = 3; break;
	}

	for (i = 0; i < ram_n - l; i++)
	{
		if (TEST_BIT(d->WRAM_BITS, i) && _S9XCHTC(cmp, i, (int32) value))
		{
			if (update)
				d->CWRAM[i] = d->RAM[i];
		}
		else
			BIT_CLEAR(d->WRAM_BITS, i);
	}

	for (i = 0; i < sram_n - l; i++)
	{
		if (TEST_BIT(d->SRAM_BITS, i) && _S9XCHTC(cmp, i + 0x20000, (int32) value))
		{
			if (update)
				d->CSRAM[i] = d->SRAM[i];
		}
		else
			BIT_CLEAR(d->SRAM_BITS, i);
	}

	for (i = 0; i < iram_n - l; i++)
	{
		if (TEST_BIT(d->IRAM_BITS, i) && _S9XCHTC(cmp, i + 0x30000, (int32) value))
		{
			if (update)
				d->CIRAM[i] = d->IRAM[i];
		}
		else
			BIT_CLEAR(d->IRAM_BITS, i);
	}

	S9xClearTrailingBits(d, l);
}

void S9xOutputCheatSearchResults (SCheatData *d)
{
	int	i;

	for (i = 0; i < (int) d->ram_size; i++)
	{
		if (TEST_BIT(d->WRAM_BITS, i))
			printf("WRAM: %05x: %02x\n", i, d->RAM[i]);
	}

	for (i = 0; i < (int) d->sram_size; i++)
	{
		if (TEST_BIT(d->SRAM_BITS, i))
			printf("SRAM: %04x: %02x\n", i, d->SRAM[i]);
	}

	for (i = 0; i < (int) d->iram_size; i++)
	{
		if (TEST_BIT(d->IRAM_BITS, i))
			printf("IRAM: %05x: %02x\n", i, d->IRAM[i]);
	}
}
