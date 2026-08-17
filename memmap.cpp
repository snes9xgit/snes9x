/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <string>
#include <iomanip>
#include <sstream>
#include <numeric>
#include <assert.h>

#include "snes9x.h"
#ifdef UNZIP_SUPPORT
#  ifdef SYSTEM_ZIP
#    include <minizip/unzip.h>
#  else
#    include "unzip/unzip.h"
#  endif
#endif

#ifdef JMA_SUPPORT
#include "jma/s9x-jma.h"
#endif

#include <ctype.h>
#include <sys/stat.h>

#include "memmap.h"
#include "apu/apu.h"
#include "cheats.h"
#include "sgb/sgb.h"
#include "fxemu.h"
#include "sdd1.h"
#include "srtc.h"
#include "controls.h"
#include "cheats.h"
#include "movie.h"
#include "display.h"
#include "sha256.h"
#include "snapshot.h"
#include "sfcbox.h"
#include "voicekun.h"

#ifndef SET_UI_COLOR
#define SET_UI_COLOR(r, g, b) ;
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

static bool8	stopMovie = TRUE;

// from NSRT
static const char	*nintendo_licensees[] =
{
	"Unlicensed",
	"Nintendo",
	"Rocket Games/Ajinomoto",
	"Imagineer-Zoom",
	"Gray Matter",
	"Zamuse",
	"Falcom",
	NULL,
	"Capcom",
	"Hot B Co.",
	"Jaleco",
	"Coconuts Japan",
	"Coconuts Japan/G.X.Media",
	"Micronet",
	"Technos",
	"Mebio Software",
	"Shouei System",
	"Starfish",
	NULL,
	"Mitsui Fudosan/Dentsu",
	NULL,
	"Warashi Inc.",
	NULL,
	"Nowpro",
	NULL,
	"Game Village",
	"IE Institute",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Banarex",
	"Starfish",
	"Infocom",
	"Electronic Arts Japan",
	NULL,
	"Cobra Team",
	"Human/Field",
	"KOEI",
	"Hudson Soft",
	"S.C.P./Game Village",
	"Yanoman",
	NULL,
	"Tecmo Products",
	"Japan Glary Business",
	"Forum/OpenSystem",
	"Virgin Games (Japan)",
	"SMDE",
	"Yojigen",
	NULL,
	"Daikokudenki",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Creatures Inc.",
	"TDK Deep Impresion",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Destination Software/KSS",
	"Sunsoft/Tokai Engineering",
	"POW (Planning Office Wada)/VR 1 Japan",
	"Micro World",
	NULL,
	"San-X",
	"Enix",
	"Loriciel/Electro Brain",
	"Kemco Japan",
	"Seta Co.,Ltd.",
	"Culture Brain",
	"Irem Corp.",
	"Palsoft",
	"Visit Co., Ltd.",
	"Intec",
	"System Sacom",
	"Poppo",
	"Ubisoft Japan",
	NULL,
	"Media Works",
	"NEC InterChannel",
	"Tam",
	"Gajin/Jordan",
	"Smilesoft",
	NULL,
	NULL,
	"Mediakite",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Viacom",
	"Carrozzeria",
	"Dynamic",
	NULL,
	"Magifact",
	"Hect",
	"Codemasters",
	"Taito/GAGA Communications",
	"Laguna",
	"Telstar Fun & Games/Event/Taito",
	NULL,
	"Arcade Zone Ltd.",
	"Entertainment International/Empire Software",
	"Loriciel",
	"Gremlin Graphics",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Seika Corp.",
	"UBI SOFT Entertainment Software",
	"Sunsoft US",
	NULL,
	"Life Fitness",
	NULL,
	"System 3",
	"Spectrum Holobyte",
	NULL,
	"Irem",
	NULL,
	"Raya Systems",
	"Renovation Products",
	"Malibu Games",
	NULL,
	"Eidos/U.S. Gold",
	"Playmates Interactive",
	NULL,
	NULL,
	"Fox Interactive",
	"Time Warner Interactive",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Disney Interactive",
	NULL,
	"Black Pearl",
	NULL,
	"Advanced Productions",
	NULL,
	NULL,
	"GT Interactive",
	"RARE",
	"Crave Entertainment",
	"Absolute Entertainment",
	"Acclaim",
	"Activision",
	"American Sammy",
	"Take 2/GameTek",
	"Hi Tech",
	"LJN Ltd.",
	NULL,
	"Mattel",
	NULL,
	"Mindscape/Red Orb Entertainment",
	"Romstar",
	"Taxan",
	"Midway/Tradewest",
	NULL,
	"American Softworks Corp.",
	"Majesco Sales Inc.",
	"3DO",
	NULL,
	NULL,
	"Hasbro",
	"NewKidCo",
	"Telegames",
	"Metro3D",
	NULL,
	"Vatical Entertainment",
	"LEGO Media",
	NULL,
	"Xicat Interactive",
	"Cryo Interactive",
	NULL,
	NULL,
	"Red Storm Entertainment",
	"Microids",
	NULL,
	"Conspiracy/Swing",
	"Titus",
	"Virgin Interactive",
	"Maxis",
	NULL,
	"LucasArts Entertainment",
	NULL,
	NULL,
	"Ocean",
	NULL,
	"Electronic Arts",
	NULL,
	"Laser Beam",
	NULL,
	NULL,
	"Elite Systems",
	"Electro Brain",
	"The Learning Company",
	"BBC",
	NULL,
	"Software 2000",
	NULL,
	"BAM! Entertainment",
	"Studio 3",
	NULL,
	NULL,
	NULL,
	"Classified Games",
	NULL,
	"TDK Mediactive",
	NULL,
	"DreamCatcher",
	"JoWood Produtions",
	"SEGA",
	"Wannado Edition",
	"LSP (Light & Shadow Prod.)",
	"ITE Media",
	"Infogrames",
	"Interplay",
	"JVC (US)",
	"Parker Brothers",
	NULL,
	"SCI (Sales Curve Interactive)/Storm",
	NULL,
	NULL,
	"THQ Software",
	"Accolade Inc.",
	"Triffix Entertainment",
	NULL,
	"Microprose Software",
	"Universal Interactive/Sierra/Simon & Schuster",
	NULL,
	"Kemco",
	"Rage Software",
	"Encore",
	NULL,
	"Zoo",
	"Kiddinx",
	"Simon & Schuster Interactive",
	"Asmik Ace Entertainment Inc./AIA",
	"Empire Interactive",
	NULL,
	NULL,
	"Jester Interactive",
	NULL,
	"Rockstar Games",
	"Scholastic",
	"Ignition Entertainment",
	"Summitsoft",
	"Stadlbauer",
	NULL,
	NULL,
	NULL,
	"Misawa",
	"Teichiku",
	"Namco Ltd.",
	"LOZC",
	"KOEI",
	NULL,
	"Tokuma Shoten Intermedia",
	"Tsukuda Original",
	"DATAM-Polystar",
	NULL,
	NULL,
	"Bullet-Proof Software",
	"Vic Tokai Inc.",
	NULL,
	"Character Soft",
	"I'Max",
	"Saurus",
	NULL,
	NULL,
	"General Entertainment",
	NULL,
	NULL,
	"I'Max",
	"Success",
	NULL,
	"SEGA Japan",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Takara",
	"Chun Soft",
	"Video System Co., Ltd./McO'River",
	"BEC",
	NULL,
	"Varie",
	"Yonezawa/S'pal",
	"Kaneko",
	NULL,
	"Victor Interactive Software/Pack-in-Video",
	"Nichibutsu/Nihon Bussan",
	"Tecmo",
	"Imagineer",
	NULL,
	NULL,
	"Nova",
	"Den'Z",
	"Bottom Up",
	NULL,
	"TGL (Technical Group Laboratory)",
	NULL,
	"Hasbro Japan",
	NULL,
	"Marvelous Entertainment",
	NULL,
	"Keynet Inc.",
	"Hands-On Entertainment",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Telenet",
	"Hori",
	NULL,
	NULL,
	"Konami",
	"K.Amusement Leasing Co.",
	"Kawada",
	"Takara",
	NULL,
	"Technos Japan Corp.",
	"JVC (Europe/Japan)/Victor Musical Industries",
	NULL,
	"Toei Animation",
	"Toho",
	NULL,
	"Namco",
	"Media Rings Corp.",
	"J-Wing",
	NULL,
	"Pioneer LDC",
	"KID",
	"Mediafactory",
	NULL,
	NULL,
	NULL,
	"Infogrames Hudson",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Acclaim Japan",
	"ASCII Co./Nexoft",
	"Bandai",
	NULL,
	"Enix",
	NULL,
	"HAL Laboratory/Halken",
	"SNK",
	NULL,
	"Pony Canyon Hanbai",
	"Culture Brain",
	"Sunsoft",
	"Toshiba EMI",
	"Sony Imagesoft",
	NULL,
	"Sammy",
	"Magical",
	"Visco",
	NULL,
	"Compile",
	NULL,
	"MTO Inc.",
	NULL,
	"Sunrise Interactive",
	NULL,
	"Global A Entertainment",
	"Fuuki",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Taito",
	NULL,
	"Kemco",
	"Square",
	"Tokuma Shoten",
	"Data East",
	"Tonkin House",
	NULL,
	"KOEI",
	NULL,
	"Konami/Ultra/Palcom",
	"NTVIC/VAP",
	"Use Co., Ltd.",
	"Meldac",
	"Pony Canyon (Japan)/FCI (US)",
	"Angel/Sotsu Agency/Sunrise",
	"Yumedia/Aroma Co., Ltd.",
	NULL,
	NULL,
	"Boss",
	"Axela/Crea-Tech",
	"Sekaibunka-Sha/Sumire kobo/Marigul Management Inc.",
	"Konami Computer Entertainment Osaka",
	NULL,
	NULL,
	"Enterbrain",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Taito/Disco",
	"Sofel",
	"Quest Corp.",
	"Sigma",
	"Ask Kodansha",
	NULL,
	"Naxat",
	"Copya System",
	"Capcom Co., Ltd.",
	"Banpresto",
	"TOMY",
	"Acclaim/LJN Japan",
	NULL,
	"NCS",
	"Human Entertainment",
	"Altron",
	"Jaleco",
	"Gaps Inc.",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Elf",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Jaleco",
	NULL,
	"Yutaka",
	"Varie",
	"T&ESoft",
	"Epoch Co., Ltd.",
	NULL,
	"Athena",
	"Asmik",
	"Natsume",
	"King Records",
	"Atlus",
	"Epic/Sony Records (Japan)",
	NULL,
	"IGS (Information Global Service)",
	NULL,
	"Chatnoir",
	"Right Stuff",
	NULL,
	"NTT COMWARE",
	NULL,
	"Spike",
	"Konami Computer Entertainment Tokyo",
	"Alphadream Corp.",
	NULL,
	"Sting",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"A Wave",
	"Motown Software",
	"Left Field Entertainment",
	"Extreme Entertainment Group",
	"TecMagik",
	NULL,
	NULL,
	NULL,
	NULL,
	"Cybersoft",
	NULL,
	"Psygnosis",
	NULL,
	NULL,
	"Davidson/Western Tech.",
	"Unlicensed",
	NULL,
	NULL,
	NULL,
	NULL,
	"The Game Factory Europe",
	"Hip Games",
	"Aspyr",
	NULL,
	NULL,
	"Mastiff",
	"iQue",
	"Digital Tainment Pool",
	"XS Games",
	"Daiwon",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"PCCW Japan",
	NULL,
	NULL,
	"KiKi Co. Ltd.",
	"Open Sesame Inc.",
	"Sims",
	"Broccoli",
	"Avex",
	"D3 Publisher",
	NULL,
	"Konami Computer Entertainment Japan",
	NULL,
	"Square-Enix",
	"KSG",
	"Micott & Basara Inc.",
	NULL,
	"Orbital Media",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"The Game Factory USA",
	NULL,
	NULL,
	"Treasure",
	"Aruze",
	"Ertain",
	"SNK Playmore",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Yojigen"
};

static const uint32	crc32Table[256] =
{
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static void S9xDeinterleaveType1 (int, uint8 *);
static void S9xDeinterleaveType2 (int, uint8 *);
static void S9xDeinterleaveGD24 (int, uint8 *);
static bool8 allASCII (uint8 *, int);
static bool8 is_SufamiTurbo_BIOS (const uint8 *, uint32);
static bool8 is_SufamiTurbo_Cart (const uint8 *, uint32);
static bool8 is_BSCart_BIOS (const uint8 *, uint32);
static bool8 is_BSCartSA1_BIOS(const uint8 *, uint32);
static bool8 is_GNEXT_Add_On (const uint8 *, uint32);
static bool8 is_SGB_BIOS      (const uint8 *data, uint32 size, uint8 *out_mode /*1 or 2*/);
static bool8 FindSGB_BIOS     (uint8 mode, const char *gb_rom_path, std::string &out_path);
static bool8 FindSGB_BootROM  (uint8 mode, const char *gb_rom_path, std::string &out_path);
static bool8 LoadSGBBootROM   (const char *path, std::vector<uint8> &out_bytes);
static uint32 caCRC32 (uint8 *, uint32, uint32 crc32 = 0xffffffff);
static bool8 ReadUPSPatch (Stream *, long, int32 &);
static long ReadInt (Stream *, unsigned);
static bool8 ReadIPSPatch (Stream *, long, int32 &);
#ifdef UNZIP_SUPPORT
static int unzFindExtension (unzFile &, const char *, bool restart = TRUE, bool print = TRUE, bool allowExact = FALSE);
#endif

// deinterleave

static void S9xDeinterleaveType1 (int size, uint8 *base)
{
	Settings.DisplayColor = BUILD_PIXEL(0, 31, 0);
	SET_UI_COLOR(0, 255, 0);

	uint8	blocks[256];
	int		nblocks = size >> 16;

	for (int i = 0; i < nblocks; i++)
	{
		blocks[i * 2] = i + nblocks;
		blocks[i * 2 + 1] = i;
	}

	uint8	*tmp = (uint8 *) malloc(0x8000);
	if (tmp)
	{
		for (int i = 0; i < nblocks * 2; i++)
		{
			for (int j = i; j < nblocks * 2; j++)
			{
				if (blocks[j] == i)
				{
					memmove(tmp, &base[blocks[j] * 0x8000], 0x8000);
					memmove(&base[blocks[j] * 0x8000], &base[blocks[i] * 0x8000], 0x8000);
					memmove(&base[blocks[i] * 0x8000], tmp, 0x8000);
					uint8	b = blocks[j];
					blocks[j] = blocks[i];
					blocks[i] = b;
					break;
				}
			}
		}

		free(tmp);
	}
}

static void S9xDeinterleaveType2 (int size, uint8 *base)
{
	// for odd Super FX images
	Settings.DisplayColor = BUILD_PIXEL(31, 14, 6);
	SET_UI_COLOR(255, 119, 25);

	uint8	blocks[256];
	int		nblocks = size >> 16;
	int		step = 64;

	while (nblocks <= step)
		step >>= 1;
	nblocks = step;

	for (int i = 0; i < nblocks * 2; i++)
		blocks[i] = (i & ~0xf) | ((i & 3) << 2) | ((i & 12) >> 2);

	uint8	*tmp = (uint8 *) malloc(0x10000);
	if (tmp)
	{
		for (int i = 0; i < nblocks * 2; i++)
		{
			for (int j = i; j < nblocks * 2; j++)
			{
				if (blocks[j] == i)
				{
					memmove(tmp, &base[blocks[j] * 0x10000], 0x10000);
					memmove(&base[blocks[j] * 0x10000], &base[blocks[i] * 0x10000], 0x10000);
					memmove(&base[blocks[i] * 0x10000], tmp, 0x10000);
					uint8	b = blocks[j];
					blocks[j] = blocks[i];
					blocks[i] = b;
					break;
				}
			}
		}

		free(tmp);
	}
}

static void S9xDeinterleaveGD24 (int size, uint8 *base)
{
	// for 24Mb images dumped with Game Doctor
	if (size != 0x300000)
		return;

	Settings.DisplayColor = BUILD_PIXEL(0, 31, 31);
	SET_UI_COLOR(0, 255, 255);

	uint8	*tmp = (uint8 *) malloc(0x80000);
	if (tmp)
	{
		memmove(tmp, &base[0x180000], 0x80000);
		memmove(&base[0x180000], &base[0x200000], 0x80000);
		memmove(&base[0x200000], &base[0x280000], 0x80000);
		memmove(&base[0x280000], tmp, 0x80000);

		free(tmp);

		S9xDeinterleaveType1(size, base);
	}
}

// allocation and deallocation

bool8 CMemory::Init (void)
{
	IPPU.TileCache[TILE_2BIT]       = (uint8 *) malloc(MAX_2BIT_TILES * 64);
	IPPU.TileCache[TILE_4BIT]       = (uint8 *) malloc(MAX_4BIT_TILES * 64);
	IPPU.TileCache[TILE_8BIT]       = (uint8 *) malloc(MAX_8BIT_TILES * 64);
	IPPU.TileCache[TILE_2BIT_EVEN]  = (uint8 *) malloc(MAX_2BIT_TILES * 64);
	IPPU.TileCache[TILE_2BIT_ODD]   = (uint8 *) malloc(MAX_2BIT_TILES * 64);
	IPPU.TileCache[TILE_4BIT_EVEN]  = (uint8 *) malloc(MAX_4BIT_TILES * 64);
	IPPU.TileCache[TILE_4BIT_ODD]   = (uint8 *) malloc(MAX_4BIT_TILES * 64);

	IPPU.TileCached[TILE_2BIT]      = (uint8 *) malloc(MAX_2BIT_TILES);
	IPPU.TileCached[TILE_4BIT]      = (uint8 *) malloc(MAX_4BIT_TILES);
	IPPU.TileCached[TILE_8BIT]      = (uint8 *) malloc(MAX_8BIT_TILES);
	IPPU.TileCached[TILE_2BIT_EVEN] = (uint8 *) malloc(MAX_2BIT_TILES);
	IPPU.TileCached[TILE_2BIT_ODD]  = (uint8 *) malloc(MAX_2BIT_TILES);
	IPPU.TileCached[TILE_4BIT_EVEN] = (uint8 *) malloc(MAX_4BIT_TILES);
	IPPU.TileCached[TILE_4BIT_ODD]  = (uint8 *) malloc(MAX_4BIT_TILES);

	if (!IPPU.TileCache[TILE_2BIT]       ||
		!IPPU.TileCache[TILE_4BIT]       ||
		!IPPU.TileCache[TILE_8BIT]       ||
		!IPPU.TileCache[TILE_2BIT_EVEN]  ||
		!IPPU.TileCache[TILE_2BIT_ODD]   ||
		!IPPU.TileCache[TILE_4BIT_EVEN]  ||
		!IPPU.TileCache[TILE_4BIT_ODD]   ||
		!IPPU.TileCached[TILE_2BIT]      ||
		!IPPU.TileCached[TILE_4BIT]      ||
		!IPPU.TileCached[TILE_8BIT]      ||
		!IPPU.TileCached[TILE_2BIT_EVEN] ||
		!IPPU.TileCached[TILE_2BIT_ODD]  ||
		!IPPU.TileCached[TILE_4BIT_EVEN] ||
		!IPPU.TileCached[TILE_4BIT_ODD])
    {
		Deinit();
		return (FALSE);
    }

	ROMStorage.resize(MAX_ROM_SIZE + 0x200 + 0x8000);
	std::fill(ROMStorage.begin(), ROMStorage.end(), 0);
	SRAMStorage.resize(SRAM_SIZE);
	std::fill(SRAMStorage.begin(), SRAMStorage.end(), 0);
	SRAM = &SRAMStorage[0];
	memset(RAM, 0,  sizeof(RAM));
	memset(VRAM, 0, sizeof(VRAM));

	memset(IPPU.TileCache[TILE_2BIT], 0,       MAX_2BIT_TILES * 64);
	memset(IPPU.TileCache[TILE_4BIT], 0,       MAX_4BIT_TILES * 64);
	memset(IPPU.TileCache[TILE_8BIT], 0,       MAX_8BIT_TILES * 64);
	memset(IPPU.TileCache[TILE_2BIT_EVEN], 0,  MAX_2BIT_TILES * 64);
	memset(IPPU.TileCache[TILE_2BIT_ODD], 0,   MAX_2BIT_TILES * 64);
	memset(IPPU.TileCache[TILE_4BIT_EVEN], 0,  MAX_4BIT_TILES * 64);
	memset(IPPU.TileCache[TILE_4BIT_ODD], 0,   MAX_4BIT_TILES * 64);

	memset(IPPU.TileCached[TILE_2BIT], 0,      MAX_2BIT_TILES);
	memset(IPPU.TileCached[TILE_4BIT], 0,      MAX_4BIT_TILES);
	memset(IPPU.TileCached[TILE_8BIT], 0,      MAX_8BIT_TILES);
	memset(IPPU.TileCached[TILE_2BIT_EVEN], 0, MAX_2BIT_TILES);
	memset(IPPU.TileCached[TILE_2BIT_ODD], 0,  MAX_2BIT_TILES);
	memset(IPPU.TileCached[TILE_4BIT_EVEN], 0, MAX_4BIT_TILES);
	memset(IPPU.TileCached[TILE_4BIT_ODD], 0,  MAX_4BIT_TILES);

	// FillRAM uses first 32K of ROM image area, otherwise space just
	// wasted. Might be read by the SuperFX code.

	FillRAM = &ROMStorage[0];

	// Add 0x8000 to ROM image pointer to stop SuperFX code accessing
	// unallocated memory (can cause crash on some ports).

	ROM = &ROMStorage[0x8000];

	C4RAM   = ROM + 0x400000 + 8192 * 8; // C4
	OBC1RAM = ROM + 0x400000; // OBC1
	BIOSROM = ROM + 0x300000; // BS
	BSRAM   = ROM + 0x400000; // BS

	SuperFX.pvRegisters = FillRAM + 0x3000;
	SuperFX.nRamBanks   = 2; // Most only use 1.  1=64KB=512Mb, 2=128KB=1024Mb
	SuperFX.pvRam       = SRAM;
	SuperFX.nRomBanks   = (2 * 1024 * 1024) / (32 * 1024);
	SuperFX.pvRom       = (uint8 *) ROM;

	PostRomInitFunc = NULL;

	return (TRUE);
}

void CMemory::Deinit (void)
{
	ROM = NULL;

	for (int t = 0; t < 7; t++)
	{
		if (IPPU.TileCache[t])
		{
			free(IPPU.TileCache[t]);
			IPPU.TileCache[t] = NULL;
		}

		if (IPPU.TileCached[t])
		{
			free(IPPU.TileCached[t]);
			IPPU.TileCached[t] = NULL;
		}
	}
}

// file management and ROM detection

static bool8 allASCII (uint8 *b, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (b[i] < 32 || b[i] > 126)
			return (FALSE);
	}

	return (TRUE);
}

static bool8 is_SufamiTurbo_BIOS (const uint8 *data, uint32 size)
{
	if (size == 0x40000 &&
		strncmp((char *) data, "BANDAI SFC-ADX", 14) == 0 && strncmp((char * ) (data + 0x10), "SFC-ADX BACKUP", 14) == 0)
		return (TRUE);
	else
		return (FALSE);
}

static bool8 is_SufamiTurbo_Cart (const uint8 *data, uint32 size)
{
	if (size >= 0x80000 && size <= 0x100000 &&
		strncmp((char *) data, "BANDAI SFC-ADX", 14) == 0 && strncmp((char * ) (data + 0x10), "SFC-ADX BACKUP", 14) != 0)
		return (TRUE);
	else
		return (FALSE);
}

static bool8 is_BSCart_BIOS(const uint8 *data, uint32 size)
{
	if ((data[0x7FB2] == 0x5A) && (data[0x7FB5] != 0x20) && (data[0x7FDA] == 0x33))
	{
		Memory.LoROM = TRUE;
		Memory.HiROM = FALSE;

		return (TRUE);
	}
	else if ((data[0xFFB2] == 0x5A) && (data[0xFFB5] != 0x20) && (data[0xFFDA] == 0x33))
	{
		Memory.LoROM = FALSE;
		Memory.HiROM = TRUE;

		return (TRUE);
	}
	else
		return (FALSE);
}

static bool8 is_BSCartSA1_BIOS (const uint8 *data, uint32 size)
{
	//Same basic check as BSCart
	if (!is_BSCart_BIOS(data, size))
		return (FALSE);

	//Checks if the game is Itoi's Bass Fishing No. 1 (ZBPJ) or SD Gundam G-NEXT (ZX3J)
	if (strncmp((char *)(data + 0x7fb2), "ZBPJ", 4) == 0 || strncmp((char *)(data + 0x7fb2), "ZX3J", 4) == 0)
		return (TRUE);
	else
		return (FALSE);
}

static bool8 is_GNEXT_Add_On (const uint8 *data, uint32 size)
{
	if (size == 0x80000)
		return (TRUE);
	else
		return (FALSE);
}

// SGB1 / SGB2 BIOS detection. Both ship as a standard LoROM SNES cart with
// the header at 0x7FC0. SGB1 title reads "Super GAMEBOY" (13 chars),
// SGB2 appends a '2'. Canonical file sizes are 256 KB / 512 KB respectively
// but we only validate the title here — callers typically read a partial
// buffer (header-sized) so ROM-size checks live outside this helper.
static bool8 is_SGB_BIOS (const uint8 *data, uint32 size, uint8 *out_mode)
{
	if (!data || size < 0x8000) return (FALSE);
	if (memcmp(data + 0x7FC0, "Super GAMEBOY", 13) != 0) return (FALSE);
	const uint8 mode = (data[0x7FC0 + 13] == '2') ? 2 : 1;
	if (out_mode) *out_mode = mode;
	return (TRUE);
}

// Look for an SGB BIOS file matching `mode`. Search order:
//   1. Same directory as the GB ROM (if path known)
//   2. BIOS_DIR — snes9x's configured BIOS search path
//   3. Current working directory
// Candidate filenames per mode are tried in order; the header is verified
// before a match is accepted. Writes the full path to `out_path` on hit.
static bool8 FindSGB_BIOS (uint8 mode, const char *gb_rom_path, std::string &out_path)
{
	static const char *sgb1_names[] = {
		"sgb.sfc", "SGB.sfc", "sgb1.sfc", "SGB1.sfc",
		"Super Game Boy (World).sfc", nullptr
	};
	static const char *sgb2_names[] = {
		"sgb2.sfc", "SGB2.sfc",
		"Super Game Boy 2 (Japan).sfc", nullptr
	};
	const char **names = (mode == 2) ? sgb2_names : sgb1_names;

	std::vector<std::string> dirs;
	if (gb_rom_path && *gb_rom_path)
	{
		std::string p(gb_rom_path);
		const size_t sep = p.find_last_of("/\\");
		if (sep != std::string::npos) dirs.push_back(p.substr(0, sep));
	}
	dirs.push_back(S9xGetDirectory(BIOS_DIR));
	dirs.push_back(".");

	for (const auto &dir : dirs)
	{
		for (int i = 0; names[i]; ++i)
		{
			std::string full = dir.empty() ? names[i] : (dir + SLASH_STR + names[i]);
			FILE *f = fopen(full.c_str(), "rb");
			if (!f) continue;

			uint8 hdr[0x8000];
			const size_t n = fread(hdr, 1, sizeof hdr, f);
			fclose(f);
			if (n < 0x8000) continue;

			uint8 got_mode = 0;
			if (is_SGB_BIOS(hdr, static_cast<uint32>(n), &got_mode) && got_mode == mode)
			{
				out_path = full;
				return (TRUE);
			}
		}
	}
	// On miss, stash the search roots into out_path so the caller can
	// show the user which directories were probed.
	out_path.clear();
	for (const auto &dir : dirs)
	{
		if (!out_path.empty()) out_path += " ; ";
		out_path += dir.empty() ? std::string(".") : dir;
	}
	return (FALSE);
}

bool8 S9xSGBBIOSAvailable(uint8 mode, const char *gb_rom_path)
{
	std::string dummy;
	return FindSGB_BIOS(mode, gb_rom_path, dummy);
}

// Locate the 256-byte GB-side boot ROM that accompanies the SGB BIOS.
// Common names and the same directory-search order as the .sfc.
static bool8 FindSGB_BootROM (uint8 mode, const char *gb_rom_path, std::string &out_path)
{
	static const char *sgb1_names[] = {
		"sgb.boot.rom", "sgb1.boot.rom", "sgb_bios.bin", nullptr
	};
	static const char *sgb2_names[] = {
		"sgb2.boot.rom", "sgb2_bios.bin", nullptr
	};
	const char **names = (mode == 2) ? sgb2_names : sgb1_names;

	std::vector<std::string> dirs;
	if (gb_rom_path && *gb_rom_path)
	{
		std::string p(gb_rom_path);
		const size_t sep = p.find_last_of("/\\");
		if (sep != std::string::npos) dirs.push_back(p.substr(0, sep));
	}
	dirs.push_back(S9xGetDirectory(BIOS_DIR));
	dirs.push_back(".");

	for (const auto &dir : dirs)
	{
		for (int i = 0; names[i]; ++i)
		{
			std::string full = dir.empty() ? names[i] : (dir + SLASH_STR + names[i]);
			FILE *f = fopen(full.c_str(), "rb");
			if (!f) continue;
			fseek(f, 0, SEEK_END);
			const long sz = ftell(f);
			fclose(f);
			if (sz == 256)
			{
				out_path = full;
				return (TRUE);
			}
		}
	}
	return (FALSE);
}

static bool8 LoadSGBBootROM (const char *path, std::vector<uint8> &out_bytes)
{
	FILE *f = fopen(path, "rb");
	if (!f) return (FALSE);
	out_bytes.assign(256, 0);
	const size_t got = fread(out_bytes.data(), 1, 256, f);
	fclose(f);
	return got == 256;
}

int CMemory::ScoreHiROM (bool8 skip_header, int32 romoff)
{
	uint8	*buf = ROM + 0xff00 + romoff + (skip_header ? 0x200 : 0);
	int		score = 0;

	// Check for extended HiROM expansion used in Mother 2 Deluxe et al.
	// Looks for size byte 13 (8MB) and an actual ROM size greater than 4MB
	if (buf[0xd7] == 13 && CalculatedSize > 1024 * 1024 * 4)
		score += 3;

	if (buf[0xd5] & 0x1)
		score += 2;

	// Mode23 is SA-1
	if (buf[0xd5] == 0x23)
		score -= 2;

	if (buf[0xd4] == 0x20)
		score += 2;

	if ((buf[0xdc] + (buf[0xdd] << 8)) + (buf[0xde] + (buf[0xdf] << 8)) == 0xffff)
	{
		score += 2;
		if (0 != (buf[0xde] + (buf[0xdf] << 8)))
			score++;
	}

	if (buf[0xda] == 0x33)
		score += 2;

	if ((buf[0xd5] & 0xf) < 4)
		score += 2;

	if (!(buf[0xfd] & 0x80))
		score -= 6;

	if ((buf[0xfc] + (buf[0xfd] << 8)) > 0xffb0)
		score -= 2; // reduced after looking at a scan by Cowering

	if (CalculatedSize > 1024 * 1024 * 3)
		score += 4;

	if (buf[0xd7] > 12)
		score -= 1;

	if (!allASCII(&buf[0xb0], 6))
		score -= 1;

	if (!allASCII(&buf[0xc0], ROM_NAME_LEN - 1))
		score -= 1;

	return (score);
}

int CMemory::ScoreLoROM (bool8 skip_header, int32 romoff)
{
	uint8	*buf = ROM + 0x7f00 + romoff + (skip_header ? 0x200 : 0);
	int		score = 0;

	if (!(buf[0xd5] & 0x1))
		score += 3;

	// Mode23 is SA-1
	if (buf[0xd5] == 0x23)
		score += 2;

	if ((buf[0xdc] + (buf[0xdd] << 8)) + (buf[0xde] + (buf[0xdf] << 8)) == 0xffff)
	{
		score += 2;
		if (0 != (buf[0xde] + (buf[0xdf] << 8)))
			score++;
	}

	if (buf[0xda] == 0x33)
		score += 2;

	if ((buf[0xd5] & 0xf) < 4)
		score += 2;

	if (!(buf[0xfd] & 0x80))
		score -= 6;

	if ((buf[0xfc] + (buf[0xfd] << 8)) > 0xffb0)
		score -= 2; // reduced per Cowering suggestion

	if (CalculatedSize <= 1024 * 1024 * 16)
		score += 2;

	if ((1 << (buf[0xd7] - 7)) > 48)
		score -= 1;

	if (!allASCII(&buf[0xb0], 6))
		score -= 1;

	if (!allASCII(&buf[0xc0], ROM_NAME_LEN - 1))
		score -= 1;

	return (score);
}

int CMemory::First512BytesCountZeroes() const
{
	const uint8 *buf = ROM;
	int zeroCount = 0;
	for (int i = 0; i < 512; i++)
	{
		if (buf[i] == 0)
		{
			zeroCount++;
		}
	}
	return zeroCount;
}

uint32 CMemory::HeaderRemove (uint32 size, uint8 *buf)
{
	uint32	calc_size = (size / 0x2000) * 0x2000;

	if ((size - calc_size == 512 && !Settings.ForceNoHeader) || Settings.ForceHeader)
	{
		uint8	*NSRTHead = buf + 0x1D0; // NSRT Header Location

		// detect NSRT header
		if (!strncmp("NSRT", (char *) &NSRTHead[24], 4))
		{
			if (NSRTHead[28] == 22)
			{
				if (((std::accumulate(NSRTHead, NSRTHead + sizeof(NSRTHeader), 0) & 0xFF) == NSRTHead[30]) &&
					(NSRTHead[30] + NSRTHead[31] == 255) && ((NSRTHead[0] & 0x0F) <= 13) &&
					(((NSRTHead[0] & 0xF0) >> 4) <= 3) && ((NSRTHead[0] & 0xF0) >> 4))
					memcpy(NSRTHeader, NSRTHead, sizeof(NSRTHeader));
			}
		}

		memmove(buf, buf + 512, calc_size);
		HeaderCount++;
		size -= 512;
	}

	return (size);
}

uint32 CMemory::FileLoader (uint8 *buffer, const char *filename, uint32 maxsize)
{
	// <- ROM size without header
	// ** Memory.HeaderCount
	// ** Memory.ROMFilename

	uint32	totalSize = 0;
	memset(NSRTHeader, 0, sizeof(NSRTHeader));
	HeaderCount = 0;

	auto path = splitpath(filename);

	int	nFormat = FILE_DEFAULT;
	if (path.ext_is(".zip") || path.ext_is(".msu1"))
		nFormat = FILE_ZIP;
	else if (path.ext_is(".jma"))
		nFormat = FILE_JMA;

	switch (nFormat)
	{
		case FILE_ZIP:
		{
		#ifdef UNZIP_SUPPORT
			if (!LoadZip(filename, &totalSize, buffer, maxsize + 0x200))
			{
			 	S9xMessage(S9X_ERROR, S9X_ROM_INFO, "Invalid Zip archive.");
				return (0);
			}

			ROMFilename = filename;
		#else
			S9xMessage(S9X_ERROR, S9X_ROM_INFO, "This binary was not created with Zip support.");
			return (0);
		#endif
			break;
		}

		case FILE_JMA:
		{
		#ifdef JMA_SUPPORT
			size_t	size = load_jma_file(filename, buffer);
			if (!size)
			{
			 	S9xMessage(S9X_ERROR, S9X_ROM_INFO, "Invalid JMA archive.");
				return (0);
			}

			totalSize = HeaderRemove(size, buffer);

			ROMFilename = filename;
		#else
			S9xMessage(S9X_ERROR, S9X_ROM_INFO, "This binary was not created with JMA support.");
			return (0);
		#endif
			break;
		}

		case FILE_DEFAULT:
		default:
		{
			STREAM	fp = OPEN_STREAM(filename, "rb");
			if (!fp)
				return (0);

			ROMFilename = filename;

			uint32	size = 0;

			size = READ_STREAM(buffer, maxsize + 0x200, fp);
			CLOSE_STREAM(fp);

			totalSize = HeaderRemove(size, buffer);

			break;
		}
	}

	if (HeaderCount == 0)
		S9xMessage(S9X_INFO, S9X_HEADERS_INFO, "No ROM file header found.");
	else if (HeaderCount == 1)
		S9xMessage(S9X_INFO, S9X_HEADERS_INFO, "Found ROM file header (and ignored it).");
	else
		S9xMessage(S9X_INFO, S9X_HEADERS_INFO, "Found multiple ROM file headers (and ignored them).");

	return ((uint32) totalSize);
}

bool8 CMemory::LoadROMMem (const uint8 *source, uint32 sourceSize, const char* optional_rom_filename /*= NULL*/)
{
    if(!source || sourceSize > MAX_ROM_SIZE)
        return FALSE;

    if (optional_rom_filename)
        ROMFilename = optional_rom_filename;
    else
        ROMFilename = "MemoryROM";

    // In-memory GB/SGB detection — mirror LoadROM so libretro and other
    // in-memory callers route .gb/.gbc carts (including ones wrapped in a
    // container) into the SGB subsystem instead of the 65816 parser.
    {
        int gb = LoadGBFromBytes(source, sourceSize, optional_rom_filename);
        if (gb > 0) return TRUE;
        if (gb < 0) return FALSE;
    }

    // Not a GB ROM — tear down any previous SGB session so a SNES ROM loaded
    // after a GB ROM runs on the 65816 path.
    if (Settings.SuperGameBoy || Settings.SGB_BIOSModeActive)
    {
        S9xSGBDeinit();
        Settings.SuperGameBoy       = FALSE;
        Settings.SGB_BIOSModeActive = FALSE;
    }
    Settings.GBRomPath[0] = '\0';

    // LoadROMInt only ever needs one retry (the interleave-detection
    // flip-flop); bound the loop so a deterministic failure — e.g. an
    // SFC-Box image without its KROM BIOS — reports instead of spinning.
    int retries = 0;
    do
    {
        memset(ROM,0, MAX_ROM_SIZE);
        memset(&Multi, 0,sizeof(Multi));
        memcpy(ROM,source,sourceSize);

        if (LoadROMInt(sourceSize))
            return TRUE;
    }
    while (++retries < 3);

    return FALSE;
}

// Case-insensitive ASCII extension match. Used to route .gb/.gbc ROMs
// into the SGB subsystem instead of the 65816 SNES path.
static bool S9xFilenameHasExt(const char *name, const char *ext)
{
    if (!name || !ext) return false;
    const char *dot = strrchr(name, '.');
    if (!dot) return false;
    const char *a = dot;
    const char *b = ext;
    while (*a && *b)
    {
        char ca = *a++; if (ca >= 'A' && ca <= 'Z') ca = static_cast<char>(ca + 32);
        char cb = *b++; if (cb >= 'A' && cb <= 'Z') cb = static_cast<char>(cb + 32);
        if (ca != cb) return false;
    }
    return *a == 0 && *b == 0;
}

// Detect a Game Boy ROM by content. Every licensed GB cart carries the
// 48-byte Nintendo logo at 0x0104-0x0133 — if it's not present the real
// HW boot ROM refuses to run the cart. Matching lets us catch .gb/.gbc
// wrapped in .zip/.jma/.7z containers after FileLoader has unzipped.
//
// Sachen unlicensed multicarts (e.g. 4B-007) bury the logo inside
// 0x0104-0x01FF under an address bit permutation (A0/A1/A4/A6 swapped);
// the cart's MMC1 mapper unscrambles it on locked reads so the bootstrap
// still sees a valid logo. We try the strict match first, then redo the
// compare with the permutation so those carts route to the SGB.
static const uint8 kGbNintendoLogo[48] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

static inline uint16 SachenScrambleAddr(uint16 addr)
{
    return static_cast<uint16>((addr & ~0x53u)
                             | ((addr >> 6) & 0x01u)
                             | ((addr >> 3) & 0x02u)
                             | ((addr << 3) & 0x10u)
                             | ((addr << 6) & 0x40u));
}

static bool S9xRomBytesAreSachenScrambledGb(const uint8 *rom, int32 size)
{
    if (size < 0x0200) return false;
    for (int i = 0; i < 48; ++i)
    {
        const uint16 a = SachenScrambleAddr(static_cast<uint16>(0x0104 + i));
        if (rom[a] != kGbNintendoLogo[i]) return false;
    }
    return true;
}

// Logo-less Sachen single-game carts (e.g. Beast Fighter) carry no Nintendo
// logo at all — not even under the address swap — so the logo sniffs above miss
// them and they fall through to the 65816 parser ("LoROM: Corrupt"). But every
// Sachen cart presents a valid Game Boy header once its A0/A6 + A1/A4 address
// swap is undone: a NOP;JP (or JP) entry into ROM plus a header checksum that
// matches $014D (the standard $0134-$014C algorithm). That descrambled-but-valid
// header identifies the cart without any logo bytes, and a non-GB ROM has
// essentially no chance of satisfying both the entry shape and the checksum.
static bool S9xRomBytesAreSachenScrambledGbHeader(const uint8 *rom, int32 size)
{
    if (size < 0x0200) return false;
    uint16 entry;
    if (rom[SachenScrambleAddr(0x0100)] == 0x00 && rom[SachenScrambleAddr(0x0101)] == 0xC3)
        entry = static_cast<uint16>(rom[SachenScrambleAddr(0x0102)] | (rom[SachenScrambleAddr(0x0103)] << 8));
    else if (rom[SachenScrambleAddr(0x0100)] == 0xC3)
        entry = static_cast<uint16>(rom[SachenScrambleAddr(0x0101)] | (rom[SachenScrambleAddr(0x0102)] << 8));
    else
        return false;
    if (entry >= 0x8000) return false;
    uint8 sum = 0;
    for (uint16 a = 0x0134; a <= 0x014C; ++a)
        sum = static_cast<uint8>(sum - rom[SachenScrambleAddr(a)] - 1);
    return sum == rom[SachenScrambleAddr(0x014D)];
}

bool S9xRomBytesAreGb(const uint8 *rom, int32 size)
{
    if (size < 0x150 || !rom) return false;
    if (memcmp(rom + 0x0104, kGbNintendoLogo, 48) == 0) return true;
    if (S9xRomBytesAreSachenScrambledGb(rom, size)) return true;
    return S9xRomBytesAreSachenScrambledGbHeader(rom, size);
}

static std::string GBGameNameFromPath(const char *path)
{
    if (!path || !*path) return std::string();
    std::string s = path;
    const size_t sep = s.find_last_of("/\\");
    if (sep != std::string::npos) s = s.substr(sep + 1);
    const size_t dot = s.find_last_of('.');
    if (dot != std::string::npos && dot > 0) s = s.substr(0, dot);
    return s;
}

static void EmitSGBLoadBanner(const char *gb_path, uint8 bios_mode)
{
    const std::string name = GBGameNameFromPath(gb_path);
    const char *region = Settings.PAL ? "PAL" : "NTSC";
    char msg[1024];
    if (bios_mode == 2)
        snprintf(msg, sizeof msg, "\"%s\" (%s) via Super Game Boy 2", name.c_str(), region);
    else if (bios_mode == 1)
        snprintf(msg, sizeof msg, "\"%s\" (%s) via Super Game Boy", name.c_str(), region);
    else
        snprintf(msg, sizeof msg, "\"%s\" (%s)", name.c_str(), region);
    const uint32 saved = Settings.InitialInfoStringTimeout;
    Settings.InitialInfoStringTimeout = 60 * 5;
    S9xMessage(S9X_INFO, S9X_ROM_INFO, msg);
    Settings.InitialInfoStringTimeout = saved;
}

// Game Boy header $0143 CGB flag: $80 = CGB-enhanced, $C0 = CGB-only. Both
// honour the SGB-BIOS preference: under the SGB BIOS a CGB-only cart boots
// monochrome and shows its own "designed only for Game Boy Color" lockout,
// exactly as on real hardware. The BIOS-less fallback runs CGB carts in colour.
static uint8 GbBytesCgbFlag(const uint8 *rom, size_t size)
{
    return size > 0x143 ? rom[0x143] : 0;
}

static uint8 GbFileCgbFlag(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;
    uint8 flag = 0;
    if (fseek(f, 0x143, SEEK_SET) == 0)
    {
        int c = fgetc(f);
        if (c != EOF) flag = (uint8)c;
    }
    fclose(f);
    return flag;
}

// Detect and load a Game Boy ROM straight from a memory buffer. Mirrors the
// content-sniff branch in LoadROM so in-memory callers (LoadROMMem, and thus
// the libretro core) route .gb/.gbc carts into the SGB subsystem too instead
// of feeding them to the 65816 parser.
//   returns  1 : handled — loaded as a GB/SGB cart (caller should return TRUE)
//            0 : not a GB ROM — caller continues with the normal SNES path
//           -1 : GB ROM but the load failed (caller should return FALSE)
int CMemory::LoadGBFromBytes (const uint8 *rom, uint32 size, const char *filename)
{
    if (!S9xRomBytesAreGb(rom, static_cast<int32>(size)))
        return 0;

    if (filename && *filename)
    {
        strncpy(Settings.GBRomPath, filename, sizeof(Settings.GBRomPath) - 1);
        Settings.GBRomPath[sizeof(Settings.GBRomPath) - 1] = '\0';
    }
    else
        Settings.GBRomPath[0] = '\0';

    const uint8 gbFlag    = GbBytesCgbFlag(rom, (size_t)size);
    const bool  gbCgb     = (gbFlag & 0x80) != 0;

    std::string bios_path;
    uint8 bios_mode = 0;
    if (Settings.SGB_BIOSPreference >= 2 && FindSGB_BIOS(2, filename, bios_path))
    {
        bios_mode = 2;
    }
    else if (Settings.SGB_BIOSPreference >= 1)
    {
        bios_path.clear();
        if (FindSGB_BIOS(1, filename, bios_path)) bios_mode = 1;
        else bios_path.clear();
    }

    if (bios_mode &&
        LoadROMWithSGBBIOSBytes(rom, size, filename, bios_path.c_str()))
    {
        EmitSGBLoadBanner(filename, bios_mode);
        return 1;
    }

    // BIOS-less fallback — the legacy path that runs our GB core directly in
    // S9xMainLoop, gated on Settings.SuperGameBoy.
    S9xDeleteCheats();
    // Tear down for BIOS mode too (mirrors the BIOS path): a live BIOS ->
    // BIOS-less switch otherwise keeps the staged GB boot ROM.
    if (Settings.SuperGameBoy || Settings.SGB_BIOSModeActive) S9xSGBDeinit();
    if (Settings.SGB_BIOSModeActive) Settings.SGB_BIOSModeActive = FALSE;
    Settings.SuperGameBoy      = TRUE;
    Settings.GameBoyRunMode    = gbCgb ? 0 : 1;   // CGB carts run BIOS-less in CGB mode
    Settings.GBClockMultiplier = 1.0f;
    // InitROM never runs on this path, so set the video timing it would have.
    Settings.PAL               = FALSE;
    Settings.FrameTime         = Settings.FrameTimeNTSC;
    ROMFramesPerSecond         = 60;

    if (!S9xSGBInit() ||
        !S9xSGBLoadROMBytes(rom, static_cast<size_t>(size), filename))
    {
        Settings.SuperGameBoy = FALSE;
        Settings.GBRomPath[0] = '\0';
        return -1;
    }
    S9xSGBSetAudioRate(Settings.SoundPlaybackRate);
    S9xInitCheatData();
    if (filename && *filename)
    {
        ROMFilename = filename;
        S9xLoadCheatFile(S9xGetFilename(".cht", CHEAT_DIR).c_str());
    }
    EmitSGBLoadBanner(filename, 0);
    return 1;
}

bool8 CMemory::LoadROM (const char *filename)
{
    if(!filename || !*filename)
        return FALSE;

    // .gb / .gbc — hand off to the SGB subsystem. The 65816 path below
    // is bypassed entirely; S9xMainLoop gates on Settings.SuperGameBoy
    // and runs the GB core instead.
    if (S9xFilenameHasExt(filename, ".gb") || S9xFilenameHasExt(filename, ".gbc"))
    {
        // Remember the GB ROM path so the BIOS menu can reload with a
        // different preference.
        strncpy(Settings.GBRomPath, filename, sizeof(Settings.GBRomPath) - 1);
        Settings.GBRomPath[sizeof(Settings.GBRomPath) - 1] = '\0';

        const uint8 gbFlag    = GbFileCgbFlag(filename);
        const bool  gbCgb     = (gbFlag & 0x80) != 0;

        std::string bios_path;
        uint8 bios_mode = 0;
        if (Settings.SGB_BIOSPreference >= 2 && FindSGB_BIOS(2, filename, bios_path))
        {
            bios_mode = 2;
        }
        else if (Settings.SGB_BIOSPreference >= 1)
        {
            bios_path.clear();
            if (FindSGB_BIOS(1, filename, bios_path)) bios_mode = 1;
            else bios_path.clear();
        }

        if (bios_mode && LoadROMWithSGBBIOS(filename, bios_path.c_str()))
        {
            EmitSGBLoadBanner(filename, bios_mode);
            return TRUE;
        }

        // BIOS-less fallback — the legacy path that runs our GB core directly
        // in S9xMainLoop, gated on Settings.SuperGameBoy.
        S9xDeleteCheats();
        // Tear down for BIOS mode too (see the LoadGBFromBytes twin): a live
        // BIOS -> BIOS-less switch otherwise keeps the staged GB boot ROM.
        if (Settings.SuperGameBoy || Settings.SGB_BIOSModeActive) S9xSGBDeinit();
        if (Settings.SGB_BIOSModeActive) Settings.SGB_BIOSModeActive = FALSE;
        Settings.SuperGameBoy      = TRUE;
        Settings.GameBoyRunMode    = gbCgb ? 0 : 1;   // CGB carts run BIOS-less in CGB mode
        Settings.GBClockMultiplier = 1.0f;
        // InitROM never runs on this path, so set the video timing it would have.
        Settings.PAL               = FALSE;
        Settings.FrameTime         = Settings.FrameTimeNTSC;
        ROMFramesPerSecond         = 60;

        if (!S9xSGBInit() || !S9xSGBLoadROM(filename))
        {
            Settings.SuperGameBoy = FALSE;
            Settings.GBRomPath[0] = '\0';
            return FALSE;
        }
        S9xSGBSetAudioRate(Settings.SoundPlaybackRate);
        S9xInitCheatData();
        ROMFilename = filename;
        S9xLoadCheatFile(S9xGetFilename(".cht", CHEAT_DIR).c_str());
        EmitSGBLoadBanner(filename, 0);
        return TRUE;
    }

    // Loading a non-GB ROM — tear down any previous SGB state first.
    if (Settings.SuperGameBoy || Settings.SGB_BIOSModeActive)
    {
        S9xSGBDeinit();
        Settings.SuperGameBoy       = FALSE;
        Settings.SGB_BIOSModeActive = FALSE;
    }
    Settings.GBRomPath[0] = '\0';

    S9xResetSaveTimer(FALSE); // reset oops timer here so that .oops file has rom name of previous rom

    int32 totalFileSize;

    // Bounded for the same reason as LoadROMMem: one interleave retry is
    // legitimate, endless identical failures are not.
    int retries = 0;
    do
    {
        memset(ROM,0, MAX_ROM_SIZE);
        memset(&Multi, 0,sizeof(Multi));
        totalFileSize = FileLoader(ROM, filename, MAX_ROM_SIZE);

        if (!totalFileSize)
            return (FALSE);

        // Container-format sniff: .zip/.jma/.7z of a GB ROM land here
        // because FileLoader accepted the extension. If the unzipped
        // content carries the Nintendo logo it's a GB cart — route
        // into the SGB subsystem instead of the 65816 parser.
        {
            int gb = LoadGBFromBytes(ROM, (uint32)totalFileSize, filename);
            if (gb > 0) return TRUE;
            if (gb < 0) return FALSE;
        }

        CheckForAnyPatch(filename, HeaderCount != 0, totalFileSize);

        if (LoadROMInt(totalFileSize))
            return TRUE;
    }
    while (++retries < 3);

    return FALSE;
}

// P1 — BIOS-mode load. Runs the real SGB1/SGB2 SNES-side BIOS on the 65816
// and keeps our GB core loaded alongside as the cart's GB chip. The two
// CPUs don't yet talk to each other (P2 adds the cart I/O bridge); P1 just
// proves the dual-ROM load path works end-to-end.
//
// ORDER MATTERS: GB core init happens FIRST because LoadROMMem below
// memsets ROM[] — the GB bytes would be gone before our SGB::Cart could
// copy them out.
static bool8 LoadSGBBIOSBytes (const char *bios_path, std::vector<uint8> &out_bios, uint8 &out_mode)
{
    FILE *f = fopen(bios_path, "rb");
    if (!f) return FALSE;
    fseek(f, 0, SEEK_END);
    const long bios_size = ftell(f);
    if (bios_size <= 0 || bios_size > (long)CMemory::MAX_ROM_SIZE) { fclose(f); return FALSE; }
    fseek(f, 0, SEEK_SET);
    out_bios.assign((size_t)bios_size, 0);
    const bool ok = (fread(out_bios.data(), 1, bios_size, f) == (size_t)bios_size);
    fclose(f);
    if (!ok) return FALSE;
    out_mode = 1;
    if (!is_SGB_BIOS(out_bios.data(), (uint32)out_bios.size(), &out_mode)) return FALSE;
    return TRUE;
}

bool8 CMemory::LoadROMWithSGBBIOS (const char *gb_path, const char *bios_path)
{
    if (!gb_path || !bios_path) return FALSE;

    std::vector<uint8> bios;
    uint8 mode = 1;
    if (!LoadSGBBIOSBytes(bios_path, bios, mode)) return FALSE;

    // GB-side boot ROM. Publicly dumped sgb*.boot.rom files are almost
    // always plain DMG boot ROMs that don't send the 5-packet SGB
    // handshake the BIOS expects. Heuristic: a real SGB boot ROM contains
    // a `3E F1` sequence (LD A, $F1) somewhere in its body as the first
    // handshake command byte. If we don't find that, fall back to the
    // embedded LIJI32/SameBoy SGB boot ROM.
    std::string boot_path;
    std::vector<uint8> user_boot;
    const bool user_has_boot = FindSGB_BootROM(mode, gb_path, boot_path) &&
                               LoadSGBBootROM(boot_path.c_str(), user_boot);
    bool user_boot_is_sgb = false;
    if (user_has_boot)
    {
        for (size_t k = 0; k + 1 < user_boot.size(); ++k)
        {
            if (user_boot[k] == 0x3E && user_boot[k + 1] == 0xF1)
            { user_boot_is_sgb = true; break; }
        }
    }

    if (Settings.SuperGameBoy || Settings.SGB_BIOSModeActive)
    {
        S9xSGBDeinit();
        Settings.SuperGameBoy       = FALSE;
        Settings.SGB_BIOSModeActive = FALSE;
    }

    if (!S9xSGBInit()) return FALSE;
    if (user_boot_is_sgb)
        S9xSGBLoadBootROMBytes(user_boot.data(), user_boot.size());
    else
        S9xSGBLoadEmbeddedBootROM(mode);
    if (!S9xSGBLoadROM(gb_path)) return FALSE;
    S9xSGBSetAudioRate(Settings.SoundPlaybackRate);

    if (!LoadROMMem(bios.data(), (uint32)bios.size(), bios_path))
    {
        S9xSGBDeinit();
        return FALSE;
    }

    Settings.SGB_BIOSModeActive = TRUE;
    strncpy(Settings.SGB_BIOSPath, bios_path, sizeof Settings.SGB_BIOSPath - 1);
    Settings.SGB_BIOSPath[sizeof Settings.SGB_BIOSPath - 1] = 0;
    Settings.GameBoyRunMode     = mode;
    Settings.GBClockMultiplier  = 1.0f;

    if (gb_path && *gb_path)
    {
        strncpy(Settings.GBRomPath, gb_path, sizeof Settings.GBRomPath - 1);
        Settings.GBRomPath[sizeof Settings.GBRomPath - 1] = '\0';
    }

    ROMFilename = gb_path;
    S9xInitCheatData();
    return TRUE;
}

bool8 CMemory::LoadROMWithSGBBIOSBytes (const uint8 *gb_bytes, uint32 gb_size,
                                         const char *gb_path, const char *bios_path)
{
    if (!gb_bytes || !gb_size || !bios_path) return FALSE;

    std::vector<uint8> bios;
    uint8 mode = 1;
    if (!LoadSGBBIOSBytes(bios_path, bios, mode)) return FALSE;

    std::string boot_path;
    std::vector<uint8> user_boot;
    const bool user_has_boot = FindSGB_BootROM(mode, gb_path, boot_path) &&
                               LoadSGBBootROM(boot_path.c_str(), user_boot);
    bool user_boot_is_sgb = false;
    if (user_has_boot)
    {
        for (size_t k = 0; k + 1 < user_boot.size(); ++k)
        {
            if (user_boot[k] == 0x3E && user_boot[k + 1] == 0xF1)
            { user_boot_is_sgb = true; break; }
        }
    }

    // Snapshot the GB bytes before LoadROMMem clobbers ROM[].
    std::vector<uint8> gb_copy(gb_bytes, gb_bytes + gb_size);

    if (Settings.SuperGameBoy || Settings.SGB_BIOSModeActive)
    {
        S9xSGBDeinit();
        Settings.SuperGameBoy       = FALSE;
        Settings.SGB_BIOSModeActive = FALSE;
    }

    if (!S9xSGBInit()) return FALSE;
    if (user_boot_is_sgb)
        S9xSGBLoadBootROMBytes(user_boot.data(), user_boot.size());
    else
        S9xSGBLoadEmbeddedBootROM(mode);
    if (!S9xSGBLoadROMBytes(gb_copy.data(), gb_copy.size(), gb_path))
        return FALSE;
    S9xSGBSetAudioRate(Settings.SoundPlaybackRate);

    if (!LoadROMMem(bios.data(), (uint32)bios.size(), bios_path))
    {
        S9xSGBDeinit();
        return FALSE;
    }

    Settings.SGB_BIOSModeActive = TRUE;
    strncpy(Settings.SGB_BIOSPath, bios_path, sizeof Settings.SGB_BIOSPath - 1);
    Settings.SGB_BIOSPath[sizeof Settings.SGB_BIOSPath - 1] = 0;
    Settings.GameBoyRunMode     = mode;
    Settings.GBClockMultiplier  = 1.0f;

    if (gb_path && *gb_path)
    {
        strncpy(Settings.GBRomPath, gb_path, sizeof Settings.GBRomPath - 1);
        Settings.GBRomPath[sizeof Settings.GBRomPath - 1] = '\0';
    }

    ROMFilename = gb_path ? gb_path : "GameBoy ROM";
    S9xInitCheatData();
    return TRUE;
}

bool8 CMemory::LoadROMInt (int32 ROMfillSize)
{
	Settings.DisplayColor = BUILD_PIXEL(31, 31, 31);
	SET_UI_COLOR(255, 255, 255);

	CalculatedSize = 0;
	ExtendedFormat = NOPE;

	// Super Famicom Box cart images (GROM directory + ROMs) must divert
	// before scoring: a GROM's checksum bytes sit where the reset vector
	// would be, so the interleave heuristics would scramble the image.
	if (ROMfillSize >= 0x28000 &&
		ROM[0] >= 1 && ROM[0] <= 8 && ROM[1] == 0x05)
	{
		uint32	sum = 0;
		for (int32 i = 0; i < 0x7ffc; i++)
			sum += ROM[i];
		uint16	chk = ROM[0x7ffc] | (ROM[0x7ffd] << 8);
		uint16	cmp = ROM[0x7ffe] | (ROM[0x7fff] << 8);
		if ((uint16) sum == chk && (uint16) (chk ^ 0xffff) == cmp)
			return (LoadSFCBox(ROMfillSize));
	}

	int	hi_score, lo_score;
	int score_headered;
	int score_nonheadered;

	hi_score = ScoreHiROM(FALSE);
	lo_score = ScoreLoROM(FALSE);
	score_nonheadered = max(hi_score, lo_score);
	score_headered = max(ScoreHiROM(TRUE), ScoreLoROM(TRUE));

	bool size_is_likely_headered = ((ROMfillSize - 512) & 0xFFFF) == 0;
	if (size_is_likely_headered) { score_headered += 2; } else { score_headered -= 2; }
	if (First512BytesCountZeroes() >= 0x1E0) { score_headered += 2; } else { score_headered -= 2; }

	bool headered_score_highest = score_headered > score_nonheadered;

	if (HeaderCount == 0 && !Settings.ForceNoHeader && headered_score_highest)
	{
		memmove(ROM, ROM + 512, ROMfillSize - 512);
		ROMfillSize -= 512;
		S9xMessage(S9X_INFO, S9X_HEADER_WARNING, "Try 'force no-header' option if the game doesn't work");
		// modifying ROM, so we need to rescore
		hi_score = ScoreHiROM(FALSE);
		lo_score = ScoreLoROM(FALSE);
	}

	CalculatedSize = ((ROMfillSize + 0x1fff) / 0x2000) * 0x2000;

	if (CalculatedSize > 0x400000 &&
		(ROM[0x7fd5] + (ROM[0x7fd6] << 8)) != 0x1320 && // exclude SuperFX
		(ROM[0x7fd5] + (ROM[0x7fd6] << 8)) != 0x1420 &&
		(ROM[0x7fd5] + (ROM[0x7fd6] << 8)) != 0x1520 &&
		(ROM[0x7fd5] + (ROM[0x7fd6] << 8)) != 0x1A20 &&
		(ROM[0x7fd5] + (ROM[0x7fd6] << 8)) != 0x1730 && // exclude Super FX 3
		(ROM[0x7fd5] + (ROM[0x7fd6] << 8)) != 0x1830 &&
		(ROM[0x7fd5] + (ROM[0x7fd6] << 8)) != 0x3423 && // exclude SA-1
		(ROM[0x7fd5] + (ROM[0x7fd6] << 8)) != 0x3523 &&
		(ROM[0x7fd5] + (ROM[0x7fd6] << 8)) != 0x4332 && // exclude S-DD1
		(ROM[0x7fd5] + (ROM[0x7fd6] << 8)) != 0x4532 &&
		(ROM[0xffd5] + (ROM[0xffd6] << 8)) != 0xF93a && // exclude SPC7110
		(ROM[0xffd5] + (ROM[0xffd6] << 8)) != 0xF53a)
		ExtendedFormat = YEAH;

	// if both vectors are invalid, it's type 1 interleaved LoROM
	if (ExtendedFormat == NOPE &&
		((ROM[0x7ffc] + (ROM[0x7ffd] << 8)) < 0x8000) &&
		((ROM[0xfffc] + (ROM[0xfffd] << 8)) < 0x8000))
	{
		if (!Settings.ForceInterleaved && !Settings.ForceNotInterleaved)
			S9xDeinterleaveType1(ROMfillSize, ROM);
	}

	// CalculatedSize is now set, so rescore
	hi_score = ScoreHiROM(FALSE);
	lo_score = ScoreLoROM(FALSE);

	uint8	*RomHeader = ROM;

	if (ExtendedFormat != NOPE)
	{
		int	swappedhirom, swappedlorom;

		swappedhirom = ScoreHiROM(FALSE, 0x400000);
		swappedlorom = ScoreLoROM(FALSE, 0x400000);

		// set swapped here
		if (max(swappedlorom, swappedhirom) >= max(lo_score, hi_score))
		{
			ExtendedFormat = BIGFIRST;
			hi_score = swappedhirom;
			lo_score = swappedlorom;
			RomHeader += 0x400000;
		}
		else
			ExtendedFormat = SMALLFIRST;
	}

	bool8	interleaved, tales = FALSE;

    interleaved = Settings.ForceInterleaved || Settings.ForceInterleaved2 || Settings.ForceInterleaveGD24;

	if (Settings.ForceLoROM || (!Settings.ForceHiROM && lo_score >= hi_score))
	{
		LoROM = TRUE;
		HiROM = FALSE;

		// ignore map type byte if not 0x2x or 0x3x
		if ((RomHeader[0x7fd5] & 0xf0) == 0x20 || (RomHeader[0x7fd5] & 0xf0) == 0x30)
		{
			switch (RomHeader[0x7fd5] & 0xf)
			{
				case 1:
					interleaved = TRUE;
					break;

				case 5:
					interleaved = TRUE;
					tales = TRUE;
					break;
			}
		}
	}
	else
	{
		LoROM = FALSE;
		HiROM = TRUE;

		if ((RomHeader[0xffd5] & 0xf0) == 0x20 || (RomHeader[0xffd5] & 0xf0) == 0x30)
		{
			switch (RomHeader[0xffd5] & 0xf)
			{
				case 0:
				case 3:
					interleaved = TRUE;
					break;
			}
		}
	}

	// this two games fail to be detected
	if (!Settings.ForceHiROM && !Settings.ForceLoROM)
	{
		if (strncmp((char *) &ROM[0x7fc0], "YUYU NO QUIZ DE GO!GO!", 22) == 0 ||
		   (strncmp((char *) &ROM[0xffc0], "BATMAN--REVENGE JOKER",  21) == 0))
		{
			LoROM = TRUE;
			HiROM = FALSE;
			interleaved = FALSE;
			tales = FALSE;
		}
	}

	if (!Settings.ForceNotInterleaved && interleaved)
	{
		S9xMessage(S9X_INFO, S9X_ROM_INTERLEAVED_INFO, "ROM image is in interleaved format - converting...");

		if (tales)
		{
			if (ExtendedFormat == BIGFIRST)
			{
				S9xDeinterleaveType1(0x400000, ROM);
				S9xDeinterleaveType1(CalculatedSize - 0x400000, ROM + 0x400000);
			}
			else
			{
				S9xDeinterleaveType1(CalculatedSize - 0x400000, ROM);
				S9xDeinterleaveType1(0x400000, ROM + CalculatedSize - 0x400000);
			}

			LoROM = FALSE;
			HiROM = TRUE;
		}
		else if (Settings.ForceInterleaveGD24 && CalculatedSize == 0x300000)
		{
			bool8	t = LoROM;
			LoROM = HiROM;
			HiROM = t;
			S9xDeinterleaveGD24(CalculatedSize, ROM);
		}
		else if (Settings.ForceInterleaved2)
			S9xDeinterleaveType2(CalculatedSize, ROM);
		else
		{
			bool8	t = LoROM;
			LoROM = HiROM;
			HiROM = t;
			S9xDeinterleaveType1(CalculatedSize, ROM);
		}

		hi_score = ScoreHiROM(FALSE);
		lo_score = ScoreLoROM(FALSE);

		if ((HiROM && (lo_score >= hi_score || hi_score < 0)) ||
			(LoROM && (hi_score >  lo_score || lo_score < 0)))
		{
			S9xMessage(S9X_INFO, S9X_ROM_CONFUSING_FORMAT_INFO, "ROM lied about its type! Trying again.");
			Settings.ForceNotInterleaved = TRUE;
			Settings.ForceInterleaved = FALSE;
            return (FALSE);
		}
    }

	if (ExtendedFormat == SMALLFIRST)
		tales = TRUE;

	if (tales)
	{
		uint8	*tmp = (uint8 *) malloc(CalculatedSize - 0x400000);
		if (tmp)
		{
			S9xMessage(S9X_INFO, S9X_ROM_INTERLEAVED_INFO, "Fixing swapped ExHiROM...");
			memmove(tmp, ROM, CalculatedSize - 0x400000);
			memmove(ROM, ROM + CalculatedSize - 0x400000, 0x400000);
			memmove(ROM + 0x400000, tmp, CalculatedSize - 0x400000);
			free(tmp);
		}
	}

	memset(&SNESGameFixes, 0, sizeof(SNESGameFixes));
	SNESGameFixes.SRAMInitialValue = 0x60;
	SNESGameFixes.RAMInitialValue  = 0x55;

	InitROM();

	S9xReset();

	S9xDeleteCheats();
	S9xLoadCheatFile(S9xGetFilename(".cht", CHEAT_DIR).c_str());

    return (TRUE);
}

bool8 CMemory::LoadMultiCartMem (const uint8 *sourceA, uint32 sourceASize,
                                 const uint8 *sourceB, uint32 sourceBSize,
                                 const uint8 *bios, uint32 biosSize)
{
    uint32 offset = 0;
    memset(ROM, 0, MAX_ROM_SIZE);
	memset(&Multi, 0, sizeof(Multi));

    if(bios) {
        if(!is_SufamiTurbo_BIOS(bios,biosSize))
            return FALSE;

        memcpy(ROM,bios,biosSize);
        offset+=biosSize;
    }

    if(sourceA) {
        memcpy(ROM + offset,sourceA,sourceASize);
        Multi.cartOffsetA = offset;
        Multi.cartSizeA = sourceASize;
        offset += sourceASize;
        strcpy(Multi.fileNameA,"MemCartA");
    }

    if(sourceB) {
        memcpy(ROM + offset,sourceB,sourceBSize);
        Multi.cartOffsetB = offset;
        Multi.cartSizeB = sourceBSize;
        offset += sourceBSize;
        strcpy(Multi.fileNameB,"MemCartB");
    }

    return LoadMultiCartInt();
}

bool8 CMemory::LoadMultiCart (const char *cartA, const char *cartB)
{
    S9xResetSaveTimer(FALSE); // reset oops timer here so that .oops file has rom name of previous rom

    memset(ROM, 0, MAX_ROM_SIZE);
	memset(&Multi, 0, sizeof(Multi));

	Settings.DisplayColor = BUILD_PIXEL(31, 31, 31);
	SET_UI_COLOR(255, 255, 255);

    if (cartB && cartB[0])
		Multi.cartSizeB = FileLoader(ROM, cartB, MAX_ROM_SIZE);

    if (Multi.cartSizeB) {
        strcpy(Multi.fileNameB, cartB);

		CheckForAnyPatch(cartB, HeaderCount != 0, Multi.cartSizeB);

        Multi.cartOffsetB = 0x400000;
        memcpy(ROM + Multi.cartOffsetB,ROM,Multi.cartSizeB);
    }

	if (cartA && cartA[0])
		Multi.cartSizeA = FileLoader(ROM, cartA, MAX_ROM_SIZE);

    if (Multi.cartSizeA) {
        strcpy(Multi.fileNameA, cartA);

		CheckForAnyPatch(cartA, HeaderCount != 0, Multi.cartSizeA);
    }

    return LoadMultiCartInt();
}

bool8 CMemory::LoadMultiCartInt ()
{
	bool8	r = TRUE;

	CalculatedSize = 0;
	ExtendedFormat = NOPE;

	if (Multi.cartSizeA)
	{
		if (is_SufamiTurbo_Cart(ROM + Multi.cartOffsetA, Multi.cartSizeA))
			Multi.cartType = 4;
		else
		if (is_BSCartSA1_BIOS(ROM + Multi.cartOffsetA, Multi.cartSizeA))
			Multi.cartType = 5;
		else
		if (is_BSCart_BIOS(ROM + Multi.cartOffsetA, Multi.cartSizeA))
			Multi.cartType = 3;
	}
	else
	if (Multi.cartSizeB)
	{
        if (is_SufamiTurbo_Cart(ROM + Multi.cartOffsetB, Multi.cartSizeB))
			Multi.cartType = 4;
	}
	else
		Multi.cartType = 4; // assuming BIOS only


    if(Multi.cartType == 4 && Multi.cartOffsetA == 0) { // try to load bios from file
        Multi.cartOffsetA = 0x40000;
        if(Multi.cartSizeA)
            memmove(ROM + Multi.cartOffsetA, ROM, Multi.cartSizeA + Multi.cartSizeB);
        else if(Multi.cartOffsetB) // clear cart A so the bios can detect that it's not present
            memset(ROM, 0, Multi.cartOffsetB);

        FILE	*fp;
	    size_t	size;
		std::string path = S9xGetDirectory(BIOS_DIR) + SLASH_STR + "STBIOS.bin";

	    fp = fopen(path.c_str(), "rb");
	    if (fp)
	    {
		    size = fread((void *) ROM, 1, 0x40000, fp);
		    fclose(fp);
		    if (!is_SufamiTurbo_BIOS(ROM, size))
			    return (FALSE);
	    }
	    else
		    return (FALSE);

        ROMFilename = path;
    }

	switch (Multi.cartType)
	{
		case 4:
			r = LoadSufamiTurbo();
			break;

		case 3:
		case 5:
			r = LoadBSCart();
			break;

		default:
			r = FALSE;
	}

	if (!r)
	{
		memset(&Multi, 0, sizeof(Multi));
		return (FALSE);
	}

	if (Multi.cartSizeA)
		ROMFilename = Multi.fileNameA;
	else if (Multi.cartSizeB)
		ROMFilename = Multi.fileNameB;

	memset(&SNESGameFixes, 0, sizeof(SNESGameFixes));
	SNESGameFixes.SRAMInitialValue = 0x60;
	SNESGameFixes.RAMInitialValue  = 0x55;

	InitROM();

	S9xReset();

	S9xDeleteCheats();
	S9xLoadCheatFile(S9xGetFilename(".cht", CHEAT_DIR).c_str());

	return (TRUE);
}

bool8 CMemory::LoadSufamiTurbo ()
{
	Multi.sramA = SRAM;
	Multi.sramB = SRAM + 0x10000;

	if (Multi.cartSizeA)
	{
		Multi.sramSizeA = 4; // ROM[0x37]?
		Multi.sramMaskA = Multi.sramSizeA ? ((1 << (Multi.sramSizeA + 3)) * 128 - 1) : 0;
	}

	if (Multi.cartSizeB)
	{
        if (!is_SufamiTurbo_Cart(ROM + Multi.cartOffsetB, Multi.cartSizeB))
			Multi.cartSizeB = 0;
	}

	if (Multi.cartSizeB)
	{
		Multi.sramSizeB = 4; // ROM[0x37]?
		Multi.sramMaskB = Multi.sramSizeB ? ((1 << (Multi.sramSizeB + 3)) * 128 - 1) : 0;
	}

	LoROM = TRUE;
	HiROM = FALSE;
	CalculatedSize = 0x40000;

	return (TRUE);
}

bool8 CMemory::LoadBSCart ()
{
	Multi.sramA = SRAM;
	Multi.sramB = NULL;

	if (LoROM)
		Multi.sramSizeA = ROM[0x7fd8];
	else
		Multi.sramSizeA = ROM[0xffd8];

	Multi.sramMaskA = Multi.sramSizeA ? ((1 << (Multi.sramSizeA + 3)) * 128 - 1) : 0;
	Multi.sramSizeB = 0;
	Multi.sramMaskB = 0;

	CalculatedSize = Multi.cartSizeA;

	if (Multi.cartSizeB == 0 && Multi.cartSizeA <= (int32)(MAX_ROM_SIZE - 0x100000 - Multi.cartOffsetA))
	{
		//Initialize 1MB Empty Memory Pack only if cart B is cleared
		//It does not make a Memory Pack if game is loaded like a normal ROM
		Multi.cartOffsetB = Multi.cartOffsetA + CalculatedSize;
		Multi.cartSizeB = 0x100000;
		memset(Memory.ROM + Multi.cartOffsetB, 0xFF, 0x100000);
	}

	return (TRUE);
}

// ---------------------------------------------------------------------------
// Super Famicom Box (docs/sfcbox.md). Images use the fullsnes/no$sns merged
// format: 32K GROM directory, that cart's ROMs in directory order, then an
// 8K DSP-1 program dump if the cart carries the chip. The PSS-61 (slot 0)
// image may be followed by a PSS-62/63/64 (slot 1) image in the same file.

// Dedicated SuperFX ROM view for the box's GSU socket (Star Fox). fxemu
// wants the image linear at +0 (GSU banks 40h+) and the doubled-32K layout
// at +FX_MEMORY_32K_MIRRORS (GSU banks 00h-3Fh). The multi-game image can't
// be rearranged in place the way Map_SuperFXLoROMMap does — a PSS-61+63 file
// is 9.5MB and overlaps the mirror window — so the selected image is staged
// into this buffer when the KROM programs the GSU mapping.
static uint8	*SFCBoxFXRom = NULL;
static uint32	SFCBoxFXRomOffset = ~0u;

static bool8 SFCBoxStageGSU (uint32 off, uint32 size)
{
	if (!SFCBoxFXRom)
	{
		SFCBoxFXRom = (uint8 *) malloc(FX_MEMORY_32K_MIRRORS + 0x400000);
		if (!SFCBoxFXRom)
			return (FALSE);
	}

	if (SFCBoxFXRomOffset == off)
		return (TRUE);

	uint32	mask = size - 1;

	memset(SFCBoxFXRom, 0xff, 0x800000);
	memcpy(SFCBoxFXRom, Memory.ROM + off, size);

	// The doubled layout Map_SuperFXLoROMMap builds, but mirrored by the
	// image size instead of reading past it.
	for (uint32 c = 0; c < 64; c++)
	{
		const uint8	*src = Memory.ROM + off + ((c * 0x8000) & mask);
		memcpy(SFCBoxFXRom + FX_MEMORY_32K_MIRRORS + c * 0x10000,          src, 0x8000);
		memcpy(SFCBoxFXRom + FX_MEMORY_32K_MIRRORS + c * 0x10000 + 0x8000, src, 0x8000);
	}

	SFCBoxFXRomOffset = off;
	return (TRUE);
}

static bool8 SFCBoxValidGROM (const uint8 *grom, uint32 avail)
{
	if (avail < 0x8000 || grom[0] < 1 || grom[0] > 8 || grom[1] != 0x05)
		return (FALSE);

	uint32	sum = 0;
	for (uint32 i = 0; i < 0x7ffc; i++)
		sum += grom[i];

	uint16	chk = grom[0x7ffc] | (grom[0x7ffd] << 8);
	uint16	cmp = grom[0x7ffe] | (grom[0x7fff] << 8);
	return ((uint16) sum == chk && (uint16) (chk ^ 0xffff) == cmp);
}

// Walk one cart's GROM directory, filling the socket table; returns the
// image length in bytes (0 = malformed).
static uint32 SFCBoxParseSlot (int slot, uint32 base, uint32 avail)
{
	const uint8	*grom = Memory.ROM + base;
	uint32		nroms = grom[0];
	uint32		dir = grom[8] | (grom[9] << 8);

	if (dir + nroms * 3 > 0x8000)
		return (0);

	uint32	off = 0x8000;	// ROMs follow the 32K GROM

	for (uint32 i = 0; i < nroms; i++)
	{
		uint32	block = (uint32) (grom[dir + i * 2] | (grom[dir + i * 2 + 1] << 8)) * 0x1000;
		uint8	socket = grom[dir + nroms * 2 + i] & 3;

		if (block + 0x30 > 0x8000)
			return (0);

		uint32	p0 = grom[block] | (grom[block + 1] << 8);
		if (block + p0 + 0x2b > 0x8000)
			return (0);

		uint32	size = (uint32) grom[block + p0 + 0x19] * 0x20000;	// 128K units
		if (!size || base + off + size > avail)
			return (0);

		SFCBox.RomOffset[slot][socket] = base + off;
		SFCBox.RomSize[slot][socket] = size;
		off += size;
	}

	SFCBox.SlotChipset[slot] = grom[4];
	if (grom[4] & 0x02)		// trailing DSP-1 program dump
		off += 0x2000;

	SFCBox.GROM[slot] = Memory.ROM + base;
	SFCBox.SlotPresent[slot] = TRUE;
	return (off);
}

bool8 CMemory::LoadSFCBox (int32 ROMfillSize)
{
	memset(&SFCBox, 0, sizeof(SFCBox));
	SFCBoxFXRomOffset = ~0u;	// new image: invalidate the staged GSU view

	uint32	total = SFCBoxParseSlot(0, 0, (uint32) ROMfillSize);
	if (!total || !SFCBox.RomSize[0][0])
	{
		printf("SFC-Box: unrecognized GROM directory in slot 0 image.\n");
		return (FALSE);
	}

	// A second cart appended? (also probe +0x800 for images whose DSP dump
	// kept the 10K padded layout)
	if ((uint32) ROMfillSize >= total + 0x8000)
	{
		uint32	candidates[2] = { total, total + 0x800 };
		bool8	found = FALSE;

		for (int i = 0; i < 2 && !found; i++)
		{
			if (candidates[i] + 0x8000 <= (uint32) ROMfillSize &&
				SFCBoxValidGROM(ROM + candidates[i], ROMfillSize - candidates[i]))
				found = SFCBoxParseSlot(1, candidates[i], (uint32) ROMfillSize) != 0;
		}

		if (!found)
			printf("SFC-Box: trailing data after the slot 0 image is not a valid second cart; ignoring it.\n");
	}

	if (!S9xSFCBoxLoadKROM())
		return (FALSE);

	//// Identity
	LoROM = TRUE;
	HiROM = FALSE;
	ExtendedFormat = NOPE;
	strcpy(ROMName, "SUPER FAMICOM BOX");
	memset(ROMId, 0, 5);
	CompanyId = 0x01;
	ROMType = 0;
	ROMSpeed = 0x20;
	ROMRegion = 0;					// Japan, NTSC
	CalculatedSize = ((ROMfillSize + 0x1fff) / 0x2000) * 0x2000;
	ROMSize = 1;
	while (((uint32) 1024 << ROMSize) < CalculatedSize)
		ROMSize++;
	SRAMSize = 7;					// the PSS-61's shared 128K chip
	SRAMMask = 0x1ffff;

	//// Chip settings (the InitROM preamble we bypass)
	Settings.SuperFX = FALSE;
	SuperFX.isFx3 = FALSE;
	Settings.DSP = 0;
	Settings.SA1 = FALSE;
	Settings.C4 = FALSE;
	Settings.SDD1 = FALSE;
	Settings.SPC7110 = FALSE;
	Settings.SPC7110RTC = FALSE;
	Settings.OBC1 = FALSE;
	Settings.SETA = 0;
	Settings.SRTC = FALSE;
	Settings.MSU1 = FALSE;
	S9xInitBSX();					// clears Settings.BS
	Settings.SFCBox = TRUE;

	// DSP-1 (Mario Kart) is HLE'd; armed when a cart carries the chip and
	// windowed in/out by the mapping registers.
	if ((SFCBox.SlotChipset[0] | SFCBox.SlotChipset[1]) & 0x02)
	{
		Settings.DSP = 1;
		SetDSP = &DSP1SetByte;
		GetDSP = &DSP1GetByte;
	}

	Checksum_Calculate();
	ROMChecksum = CalculatedChecksum;
	ROMComplementChecksum = ROMChecksum ^ 0xffff;
	ROMCRC32 = caCRC32(ROM, CalculatedSize);
	sha256sum(ROM, CalculatedSize, ROMSHA256);

	//// NTSC-only timing
	Settings.PAL = FALSE;
	Settings.FrameTime = Settings.FrameTimeNTSC;
	ROMFramesPerSecond = 60;

	Timings.H_Max_Master = SNES_CYCLES_PER_SCANLINE;
	Timings.H_Max        = Timings.H_Max_Master;
	Timings.HBlankStart  = SNES_HBLANK_START_HC;
	Timings.HBlankEnd    = SNES_HBLANK_END_HC;
	Timings.HDMAInit     = SNES_HDMA_INIT_HC;
	Timings.HDMAStart    = SNES_HDMA_START_HC;
	Timings.RenderPos    = SNES_RENDER_START_HC;
	Timings.V_Max_Master = SNES_MAX_NTSC_VCOUNTER;
	Timings.V_Max        = Timings.V_Max_Master;
	Timings.DMACPUSync   = 18;
	Timings.NMIDMADelay  = 24;
	Timings.IRQTriggerCycles = 14;
	Timings.APUSpeedup = 0;
	S9xAPUTimingSetSpeedup(Timings.APUSpeedup);

	IPPU.TotalEmulatedFrames = 0;

	memset(&SNESGameFixes, 0, sizeof(SNESGameFixes));
	SNESGameFixes.SRAMInitialValue = 0x60;
	SNESGameFixes.RAMInitialValue  = 0x55;

	// Initial map; S9xReset() below powers the supervisor on, which remaps
	// again from the registers' reset state.
	Map_Initialize();
	S9xSFCBoxRemap();

	sprintf(String, "\"SUPER FAMICOM BOX\" slot0%s, %s, %s, CRC32:%08X",
			SFCBox.SlotPresent[1] ? " + slot1" : " only",
			Size(), Settings.DSP ? "DSP1" : "no DSP", ROMCRC32);
	S9xMessage(S9X_INFO, S9X_ROM_INFO, String);

	S9xReset();

	S9xDeleteCheats();
	S9xLoadCheatFile(S9xGetFilename(".cht", CHEAT_DIR).c_str());

	return (TRUE);
}

bool8 CMemory::LoadGNEXT ()
{
	Multi.sramA = SRAM;
	Multi.sramB = NULL;

	Multi.sramSizeA = ROM[0x7fd8];
	Multi.sramMaskA = Multi.sramSizeA ? ((1 << (Multi.sramSizeA + 3)) * 128 - 1) : 0;
	Multi.sramSizeB = 0;
	Multi.sramMaskB = 0;

	if (Multi.cartSizeB)
	{
		if (!is_GNEXT_Add_On(ROM + Multi.cartOffsetB, Multi.cartSizeB))
			Multi.cartSizeB = 0;
	}

	LoROM = TRUE;
	HiROM = FALSE;
	CalculatedSize = Multi.cartSizeA;

	return (TRUE);
}

bool8 CMemory::LoadSRTC (void)
{
	FILE	*fp;

	fp = fopen(S9xGetFilename(".rtc", SRAM_DIR).c_str(), "rb");
	if (!fp)
		return (FALSE);

	if (fread(RTCData.reg, 1, 20, fp) < 20)
		memset (RTCData.reg, 0, 20);
	fclose(fp);

	return (TRUE);
}

bool8 CMemory::SaveSRTC (void)
{
	FILE	*fp;

	fp = fopen(S9xGetFilename(".rtc", SRAM_DIR).c_str(), "wb");
	if (!fp)
		return (FALSE);

	if (fwrite(RTCData.reg, 1, 20, fp) < 20)
	{
		printf ("Failed to save clock data.\n");
	}
	fclose(fp);

	return (TRUE);
}

void CMemory::ClearSRAM (bool8 onlyNonSavedSRAM)
{
	if (onlyNonSavedSRAM)
		if (!(Settings.SuperFX && (ROMType < 0x15 || ROMType == 0x17)) && !(Settings.SA1 && ROMType == 0x34)) // can have SRAM
			return;
	// TODO: If SRAM size changes change this value as well
	memset(SRAM, SNESGameFixes.SRAMInitialValue, 0x80000);
}

bool8 CMemory::LoadSRAM (const char *filename)
{
	FILE	*file;
	int		size, len;

	if (S9xSGBIsActive() && S9xSGBHasBattery())
	{
		std::string sav(filename);
		size_t dot = sav.rfind('.');
		if (dot != std::string::npos) sav.replace(dot, std::string::npos, ".sav");
		else                          sav += ".sav";
		S9xSGBLoadBatteryFromPath(sav.c_str());
	}

	ClearSRAM();

	if (Multi.cartType && Multi.sramSizeB)
	{
		size = (1 << (Multi.sramSizeB + 3)) * 128;

		file = fopen(S9xGetFilename(Multi.fileNameB, ".srm", SRAM_DIR).c_str(), "rb");
		if (file)
		{
			len = fread((char *) Multi.sramB, 1, 0x10000, file);
			fclose(file);
			if (len - size == 512)
				memmove(Multi.sramB, Multi.sramB + 512, size);
		}
	}

	size = SRAMSize ? (1 << (SRAMSize + 3)) * 128 : 0;
	if (LoROM)
		size = size < 0x70000 ? size : 0x70000;
	else if (HiROM)
		size = size < 0x40000 ? size : 0x40000;

	if (size)
	{
		file = fopen(filename, "rb");
		if (file)
		{
			len = fread((char *) SRAM, 1, size, file);
			fclose(file);
			if (len - size == 512)
				memmove(SRAM, SRAM + 512, size);

			if (Settings.SRTC || Settings.SPC7110RTC)
				LoadSRTC();

			return (TRUE);
		}
		else if (Settings.BS && !Settings.BSXItself)
		{
			// The BS game's SRAM was not found
			// Try to read BS-X.srm instead
			std::string path = S9xGetDirectory(SRAM_DIR) + SLASH_STR + "BS-X.srm";

			file = fopen(path.c_str(), "rb");
			if (file)
			{
				len = fread((char *) SRAM, 1, size, file);
				fclose(file);
				if (len - size == 512)
					memmove(SRAM, SRAM + 512, size);

				S9xMessage(S9X_INFO, S9X_ROM_INFO, "The SRAM file wasn't found: BS-X.srm was read instead.");
				return (TRUE);
			}
			else
			{
				S9xMessage(S9X_INFO, S9X_ROM_INFO, "The SRAM file wasn't found, BS-X.srm wasn't found either.");
				return (FALSE);
			}
		}

		return (FALSE);
	}

	return (TRUE);
}

bool8 CMemory::SaveSRAM (const char *filename)
{
	if (S9xSGBIsActive() && S9xSGBHasBattery())
	{
		std::string sav(filename);
		size_t dot = sav.rfind('.');
		if (dot != std::string::npos) sav.replace(dot, std::string::npos, ".sav");
		else                          sav += ".sav";
		S9xSGBSaveBatteryToPath(sav.c_str());
	}

	if (Settings.SFCBox)
		S9xSFCBoxSaveNVRAM();	// KROM battery RAM rides along with the .srm

	if (Settings.SuperFX && (ROMType < 0x15 || ROMType == 0x17)) // doesn't have SRAM
		return (TRUE);

	if (Settings.SA1 && ROMType == 0x34)    // doesn't have SRAM
		return (TRUE);

	FILE	*file;
	int		size;

	if (Multi.cartType && Multi.sramSizeB)
	{
		std::string name = S9xGetFilename(Multi.fileNameB, ".srm", SRAM_DIR);
		size = (1 << (Multi.sramSizeB + 3)) * 128;

		file = fopen(name.c_str(), "wb");
		if (file)
		{
			if (!fwrite((char *) Multi.sramB, size, 1, file))
				printf ("Couldn't write to subcart SRAM file.\n");
			fclose(file);
		}
    }

    size = SRAMSize ? (1 << (SRAMSize + 3)) * 128 : 0;
	if (LoROM)
		size = size < 0x70000 ? size : 0x70000;
	else if (HiROM)
		size = size < 0x40000 ? size : 0x40000;

	if (size)
	{
		file = fopen(filename, "wb");
		if (file)
		{
			if (!fwrite((char *) SRAM, size, 1, file))
				printf ("Couldn't write to SRAM file.\n");
			fclose(file);

			if (Settings.SRTC || Settings.SPC7110RTC)
				SaveSRTC();

			return (TRUE);
		}
	}

	return (FALSE);
}

bool8 CMemory::SaveMPAK (const char *filename)
{
	if (Settings.BS || (Multi.cartSizeB && (Multi.cartType == 3)))
	{
		FILE	*file;
		int		size;

		size = 0x100000;
		if (size)
		{
			file = fopen(filename, "wb");
			if (file)
			{
				size_t	written;
				written = fwrite((char *)Memory.ROM + Multi.cartOffsetB, size, 1, file);
				fclose(file);

				return (written > 0);
			}
		}
	}
	return (FALSE);
}

// initialization

static uint32 caCRC32 (uint8 *array, uint32 size, uint32 crc32)
{
	for (uint32 i = 0; i < size; i++)
		crc32 = ((crc32 >> 8) & 0x00FFFFFF) ^ crc32Table[(crc32 ^ array[i]) & 0xFF];

	return (~crc32);
}

void CMemory::ParseSNESHeader (uint8 *RomHeader)
{
	bool8	bs = Settings.BS & !Settings.BSXItself;

	strncpy(ROMName, (char *) &RomHeader[0x10], ROM_NAME_LEN - 1);
	if (bs)
		memset(ROMName + 16, 0x20, ROM_NAME_LEN - 17);

	if (bs)
	{
		if (!(((RomHeader[0x29] & 0x20) && CalculatedSize <  0x100000) ||
			 (!(RomHeader[0x29] & 0x20) && CalculatedSize == 0x100000)))
			printf("BS: Size mismatch\n");

		// FIXME
		int	p = 0;
		while ((1 << p) < (int) CalculatedSize)
			p++;
		ROMSize = p - 10;
	}
	else
		ROMSize = RomHeader[0x27];

	SRAMSize  = bs ? 5 /* BS-X */    : RomHeader[0x28];
	ROMSpeed  = bs ? RomHeader[0x28] : RomHeader[0x25];
	ROMType   = bs ? 0xE5 /* BS-X */ : RomHeader[0x26];
	ROMRegion = bs ? 0               : RomHeader[0x29];

	ROMChecksum           = RomHeader[0x2E] + (RomHeader[0x2F] << 8);
	ROMComplementChecksum = RomHeader[0x2C] + (RomHeader[0x2D] << 8);

	memmove(ROMId, &RomHeader[0x02], 4);

	if (RomHeader[0x2A] != 0x33)
		CompanyId = ((RomHeader[0x2A] >> 4) & 0x0F) * 36 + (RomHeader[0x2A] & 0x0F);
	else
	if (isalnum(RomHeader[0x00]) && isalnum(RomHeader[0x01]))
	{
		int	l, r, l2, r2;
		l = toupper(RomHeader[0x00]);
		r = toupper(RomHeader[0x01]);
		l2 = (l > '9') ? l - '7' : l - '0';
		r2 = (r > '9') ? r - '7' : r - '0';
		CompanyId = l2 * 36 + r2;
	}
}

void CMemory::InitROM (void)
{
	Settings.SuperFX = FALSE;
	SuperFX.isFx3 = FALSE;
	Settings.DSP = 0;
	Settings.SA1 = FALSE;
	Settings.C4 = FALSE;
	Settings.SDD1 = FALSE;
	Settings.SPC7110 = FALSE;
	Settings.SPC7110RTC = FALSE;
	Settings.OBC1 = FALSE;
	Settings.SETA = 0;
	Settings.SRTC = FALSE;
	Settings.BS = FALSE;
	Settings.MSU1 = FALSE;
	S9xVoiceKunDetach();	// the hook PC is per-game; a new ROM needs a fresh attach

	SuperFX.nRomBanks = CalculatedSize >> 15;

	//// Parse ROM header and read ROM informatoin

	CompanyId = -1;
	memset(ROMId, 0, 5);

	uint8	*RomHeader = ROM + 0x7FB0;
	if (ExtendedFormat == BIGFIRST)
		RomHeader += 0x400000;
	if (HiROM)
		RomHeader += 0x8000;

	// A regular ROM load supersedes any active SFC-Box session (the box
	// path bypasses InitROM entirely, so this never undoes its own load).
	Settings.SFCBox = FALSE;
	S9xSFCBoxDeactivate();

	S9xInitBSX(); // Set BS header before parsing

	ParseSNESHeader(RomHeader);

	// NFL Football (Europe) (Proto) - Sculptured Software, board SHVC-4PV5B-01
	// (LoROM, 256Kbit battery SRAM, PAL-only). The proto ships a junk internal
	// header, so auto-detection maps it LoROM with a garbage SRAM size; the game
	// reads open bus / wrapped save RAM and runs off the rails - glitched menus,
	// black in-game, crashes. Pin the real board: 32KB SRAM + PAL region.
	static const uint8	nfl_boot[16] =
		{ 0x5C, 0x92, 0x81, 0x81, 0x78, 0x18, 0xFB, 0xD8, 0x5C, 0x0C, 0x80, 0x80, 0xC2, 0x30, 0x5C, 0x78 };
	if (CalculatedSize == 0x200000 &&
		memcmp(ROM, nfl_boot, sizeof(nfl_boot)) == 0 &&
		memcmp(ROM + 0x7FC0, "SDEBUG1", 7) == 0)
	{
		ROMType   = 0x00;
		ROMSpeed  = 0x20;
		ROMSize   = 0x0B;
		SRAMSize  = 5;
		ROMRegion = 2;
	}

	//// Detect and initialize chips
	//// detection codes are compatible with NSRT

	// DSP1/2/3/4
	if (ROMType == 0x03)
	{
		if (ROMSpeed == 0x30)
			Settings.DSP = 4; // DSP4
		else
			Settings.DSP = 1; // DSP1
	}
	else if (ROMType == 0x05)
	{
		if (ROMSpeed == 0x20)
			Settings.DSP = 2; // DSP2
		else if (ROMSpeed == 0x30 && RomHeader[0x2a] == 0xb2)
			Settings.DSP = 3; // DSP3
		else
			Settings.DSP = 1; // DSP1
	}

	switch (Settings.DSP)
	{
		case 1:	// DSP1
			if (HiROM)
			{
				DSP0.boundary = 0x7000;
				DSP0.maptype = M_DSP1_HIROM;
			}
			else if (CalculatedSize > 0x100000)
			{
				DSP0.boundary = 0x4000;
				DSP0.maptype = M_DSP1_LOROM_L;
			}
			else
			{
				DSP0.boundary = 0xc000;
				DSP0.maptype = M_DSP1_LOROM_S;
			}

			SetDSP = &DSP1SetByte;
			GetDSP = &DSP1GetByte;
			break;

		case 2: // DSP2
			DSP0.boundary = 0x10000;
			DSP0.maptype = M_DSP2_LOROM;
			SetDSP = &DSP2SetByte;
			GetDSP = &DSP2GetByte;
			break;

		case 3: // DSP3
			DSP0.boundary = 0xc000;
			DSP0.maptype = M_DSP3_LOROM;
			SetDSP = &DSP3SetByte;
			GetDSP = &DSP3GetByte;
			break;

		case 4: // DSP4
			DSP0.boundary = 0xc000;
			DSP0.maptype = M_DSP4_LOROM;
			SetDSP = &DSP4SetByte;
			GetDSP = &DSP4GetByte;
			break;

		default:
			SetDSP = NULL;
			GetDSP = NULL;
			break;
	}

	uint32	identifier = ((ROMType & 0xff) << 8) + (ROMSpeed & 0xff);

	switch (identifier)
	{
	    // SRTC
		case 0x5535:
			Settings.SRTC = TRUE;
			S9xInitSRTC();
			break;

		// SPC7110
		case 0xF93A:
			Settings.SPC7110RTC = TRUE;
			// Fall through
		case 0xF53A:
			Settings.SPC7110 = TRUE;
			S9xInitSPC7110();
			break;

		// OBC1
		case 0x2530:
			Settings.OBC1 = TRUE;
			break;

		// SA1
		case 0x3423:
		case 0x3523:
			Settings.SA1 = TRUE;
			break;

		// Super FX 3 (LRG, $18 = FX3+battery)
		case 0x1720:
		case 0x1730:
		case 0x1820:
		case 0x1830:
			SuperFX.isFx3 = TRUE;
			// Fall through
		// SuperFX
		case 0x1320:
		case 0x1420:
		case 0x1520:
		case 0x1A20:
		// SuperFX FastROM for ROM hacks
		case 0x1330:
		case 0x1430:
		case 0x1530:
		case 0x1A30:
			Settings.SuperFX = TRUE;
			S9xInitSuperFX();
			if (ROM[0x7FDA] == 0x33)
				SRAMSize = ROM[0x7FBD];
			else
				SRAMSize = 5;
			break;

		// SDD1
		case 0x4332:
		case 0x4532:
			Settings.SDD1 = TRUE;
			break;

		// ST018
		case 0xF530:
			Settings.SETA = ST_018;
			SetSETA = NULL;
			GetSETA = NULL;
			SRAMSize = 2;
			SNESGameFixes.SRAMInitialValue = 0x00;
			break;

		// ST010/011
		case 0xF630:
			if (ROM[0x7FD7] == 0x09)
			{
				Settings.SETA = ST_011;
				SetSETA = &S9xSetST011;
				GetSETA = &S9xGetST011;
			}
			else
			{
				Settings.SETA = ST_010;
				SetSETA = &S9xSetST010;
				GetSETA = &S9xGetST010;
			}

			SRAMSize = 2;
			SNESGameFixes.SRAMInitialValue = 0x00;
			break;

		// C4
		case 0xF320:
			Settings.C4 = TRUE;
			break;
	}

	// MSU1
	Settings.MSU1 = S9xMSU1ROMExists();

	//// Map memory and calculate checksum

	Map_Initialize();
	CalculatedChecksum = 0;

	const bool8	SDD1Decompressed = (CalculatedSize >= 0x800000) &&
			(Settings.SDD1 ||
			 strncmp(ROMName, "STREET FIGHTER ALPHA2", 21) == 0 ||
			 strncmp(ROMName, "STREET FIGHTER ZERO2", 20) == 0 ||
			 strncmp(ROMName, "Star Ocean", 10) == 0);

	if (SDD1Decompressed)
	{
		Settings.SDD1 = FALSE;
		Map_SDD1DecompressedMap();
	}
	else if (HiROM)
	{
		if (Settings.BS)
			/* Do nothing */;
		else if (Settings.SPC7110)
			Map_SPC7110HiROMMap();
		else if (ExtendedFormat != NOPE)
			Map_ExtendedHiROMMap();
		else if (Multi.cartType == 3)
			Map_BSCartHiROMMap();
		else
			Map_HiROMMap();
	}
	else
	{
		if (Settings.BS)
			/* Do nothing */;
		else if (Settings.SETA && Settings.SETA != ST_018)
			Map_SetaDSPLoROMMap();
		else if (Settings.SuperFX)
		{
			if (SuperFX.isFx3)
				Map_SuperFX3LoROMMap();
			else
				Map_SuperFXLoROMMap();
		}
		else if (Settings.SA1)
		{
			if (Multi.cartType == 5)
				Map_BSSA1LoROMMap();
			else
				Map_SA1LoROMMap();
		}
		else if (Settings.SDD1)
			Map_SDD1LoROMMap();
		else if (ExtendedFormat != NOPE)
			Map_JumboLoROMMap();
		else
		if (strncmp(ROMName, "WANDERERS FROM YS", 17) == 0)
			Map_NoMAD1LoROMMap();
		else if (Multi.cartType == 3)
			if (strncmp(ROMName, "SOUND NOVEL-TCOOL", 17) == 0 ||
				strncmp(ROMName, "DERBY STALLION 96", 17) == 0)
				Map_BSCartLoROMMap(1);
			else
				Map_BSCartLoROMMap(0);
		else if (strncmp(ROMName, "SOUND NOVEL-TCOOL", 17) == 0 ||
			strncmp(ROMName, "DERBY STALLION 96", 17) == 0)
			Map_ROM24MBSLoROMMap();
		else if (strncmp(ROMName, "THOROUGHBRED BREEDER3", 21) == 0 ||
			strncmp(ROMName, "RPG-TCOOL 2", 11) == 0)
			Map_SRAM512KLoROMMap();
		else if (strncmp(ROMName, "ADD-ON BASE CASSETE", 19) == 0)
		{
			if (Multi.cartType == 4)
			{
				SRAMSize = Multi.sramSizeA;
				Map_SufamiTurboLoROMMap();
			}
			else
			{
				SRAMSize = 5;
				Map_SufamiTurboPseudoLoROMMap();
			}
		}
		else if (strncmp(ROMName, "Super GAMEBOY", 13) == 0)
			Map_SGBLoROMMap();
		else
			Map_LoROMMap();
    }

	Checksum_Calculate();

	bool8 isChecksumOK = (ROMChecksum + ROMComplementChecksum == 0xffff) &
						 (ROMChecksum == CalculatedChecksum);

	//// Build more ROM information

	// CRC32
	if (!Settings.BS || Settings.BSXItself) // Not BS Dump
	{
		ROMCRC32 = caCRC32(ROM, CalculatedSize);
		sha256sum(ROM, CalculatedSize, ROMSHA256);
	}
	else // Convert to correct format before scan
	{
		int offset = HiROM ? 0xffc0 : 0x7fc0;
		// Backup
		uint8 BSMagic0 = ROM[offset + 22],
			  BSMagic1 = ROM[offset + 23];
		// uCONSRT standard
		ROM[offset + 22] = 0x42;
		ROM[offset + 23] = 0x00;
		// Calc
		ROMCRC32 = caCRC32(ROM, CalculatedSize);
		sha256sum(ROM, CalculatedSize, ROMSHA256);
		// Convert back
		ROM[offset + 22] = BSMagic0;
		ROM[offset + 23] = BSMagic1;
	}

	// NTSC/PAL
	if (Settings.ForceNTSC)
		Settings.PAL = FALSE;
	else if (Settings.ForcePAL)
		Settings.PAL = TRUE;
	else if (!Settings.BS && (((ROMRegion >= 2) && (ROMRegion <= 12)) || ROMRegion == 18)) // 18 is used by "Tintin in Tibet (Europe) (En,Es,Sv)"
		Settings.PAL = TRUE;
	else
		Settings.PAL = FALSE;

	if (Settings.PAL)
	{
		Settings.FrameTime = Settings.FrameTimePAL;
		ROMFramesPerSecond = 50;
	}
	else
	{
		Settings.FrameTime = Settings.FrameTimeNTSC;
		ROMFramesPerSecond = 60;
	}

	// truncate cart name
	ROMName[ROM_NAME_LEN - 1] = 0;
	if (strlen(ROMName))
	{
		char *p = ROMName + strlen(ROMName);
		if (p > ROMName + 21 && ROMName[20] == ' ')
			p = ROMName + 21;
		while (p > ROMName && *(p - 1) == ' ')
			p--;
		*p = 0;
	}

	// SRAM size
	SRAMMask = SRAMSize ? ((1 << (SRAMSize + 3)) * 128) - 1 : 0;

	// checksum
	if (!isChecksumOK || ((uint32) CalculatedSize > (uint32) (((1 << (ROMSize - 7)) * 128) * 1024)))
	{
		Settings.DisplayColor = BUILD_PIXEL(31, 31, 0);
		SET_UI_COLOR(255, 255, 0);
	}

	// Use slight blue tint to indicate ROM was patched.
	if (Settings.IsPatched)
	{
		Settings.DisplayColor = BUILD_PIXEL(26, 26, 31);
		SET_UI_COLOR(216, 216, 255);
	}

	if (Multi.cartType == 4)
	{
		Settings.DisplayColor = BUILD_PIXEL(0, 16, 31);
		SET_UI_COLOR(0, 128, 255);
	}

	//// Initialize emulation

	Timings.H_Max_Master = SNES_CYCLES_PER_SCANLINE;
	Timings.H_Max        = Timings.H_Max_Master;
	Timings.HBlankStart  = SNES_HBLANK_START_HC;
	Timings.HBlankEnd    = SNES_HBLANK_END_HC;
	Timings.HDMAInit     = SNES_HDMA_INIT_HC;
	Timings.HDMAStart    = SNES_HDMA_START_HC;
	Timings.RenderPos    = SNES_RENDER_START_HC;
	Timings.V_Max_Master = Settings.PAL ? SNES_MAX_PAL_VCOUNTER : SNES_MAX_NTSC_VCOUNTER;
	Timings.V_Max        = Timings.V_Max_Master;
	/* From byuu: The total delay time for both the initial (H)DMA sync (to the DMA clock),
	   and the end (H)DMA sync (back to the last CPU cycle's mcycle rate (6, 8, or 12)) always takes between 12-24 mcycles.
	   Possible delays: { 12, 14, 16, 18, 20, 22, 24 }
	   XXX: Snes9x can't emulate this timing :( so let's use the average value... */
	Timings.DMACPUSync   = 18;
	/* If the CPU is halted (i.e. for DMA) while /NMI goes low, the NMI will trigger
	   after the DMA completes (even if /NMI goes high again before the DMA
	   completes). In this case, there is a 24-30 cycle delay between the end of DMA
	   and the NMI handler, time enough for an instruction or two. */
	// Wild Guns, Mighty Morphin Power Rangers - The Fighting Edition
	Timings.NMIDMADelay  = 24;
	Timings.IRQTriggerCycles = 14;
	Timings.APUSpeedup = 0;
    Timings.GSUCelDelay = 0;
	S9xAPUTimingSetSpeedup(Timings.APUSpeedup);

	IPPU.TotalEmulatedFrames = 0;

	//// Hack games

	ApplyROMFixes();

	//// Show ROM information
	ROMId[4] = 0;
    strcpy(ROMId, SafeString(ROMId).c_str());


	sprintf(String, "\"%s\" [%s] %s, %s, %s, %s, SRAM:%s, ID:%s, CRC32:%08X",
		SafeString(ROMName).c_str(),
		 isChecksumOK ? "checksum ok"
		 : Settings.IsPatched == 3 ? "UPS Patched"
		 : Settings.IsPatched == 2 ? "BPS Patched"
		 : Settings.IsPatched == 1 ? "IPS Patched"
		 : ((Multi.cartType == 4) ? "no checksum"
		 : "bad checksum"),
		MapType(), Size(), KartContents(), Settings.PAL ? "PAL" : "NTSC", StaticRAMSize(), ROMId, ROMCRC32);

	if (!Settings.GBRomPath[0])
		S9xMessage(S9X_INFO, S9X_ROM_INFO, GetMultilineROMInfo().c_str());

	Settings.ForceLoROM = FALSE;
	Settings.ForceHiROM = FALSE;
	Settings.ForceHeader = FALSE;
	Settings.ForceNoHeader = FALSE;
	Settings.ForceInterleaved = FALSE;
	Settings.ForceInterleaved2 = FALSE;
	Settings.ForceInterleaveGD24 = FALSE;
	Settings.ForceNotInterleaved = FALSE;
	Settings.ForcePAL = FALSE;
	Settings.ForceNTSC = FALSE;

	Settings.TakeScreenshot = FALSE;

	if (stopMovie)
		S9xMovieStop(TRUE);

	if (PostRomInitFunc)
		PostRomInitFunc();

    S9xVerifyControllers();
}

// memory map

uint32 CMemory::map_mirror (uint32 size, uint32 pos)
{
	// from bsnes
	if (size == 0)
		return (0);
	if (pos < size)
		return (pos);

	uint32	mask = 1 << 31;
	while (!(pos & mask))
		mask >>= 1;

	if (size <= (pos & mask))
		return (map_mirror(size, pos - mask));
	else
		return (mask + map_mirror(size - mask, pos - mask));
}

void CMemory::map_lorom (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint32 size)
{
	uint32	c, i, p, addr;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			addr = (c & 0x7f) * 0x8000;
			Map[p] = ROM + map_mirror(size, addr) - (i & 0x8000);
			BlockIsROM[p] = TRUE;
			BlockIsRAM[p] = FALSE;
		}
	}
}

void CMemory::map_hirom (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint32 size)
{
	uint32	c, i, p, addr;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			addr = c << 16;
			Map[p] = ROM + map_mirror(size, addr);
			BlockIsROM[p] = TRUE;
			BlockIsRAM[p] = FALSE;
		}
	}
}

void CMemory::map_lorom_offset (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint32 size, uint32 offset)
{
	uint32	c, i, p, addr;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			addr = ((c - bank_s) & 0x7f) * 0x8000;
			Map[p] = ROM + offset + map_mirror(size, addr) - (i & 0x8000);
			BlockIsROM[p] = TRUE;
			BlockIsRAM[p] = FALSE;
		}
	}
}

void CMemory::map_hirom_offset (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint32 size, uint32 offset)
{
	uint32	c, i, p, addr;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			addr = (c - bank_s) << 16;
			Map[p] = ROM + offset + map_mirror(size, addr);
			BlockIsROM[p] = TRUE;
			BlockIsRAM[p] = FALSE;
		}
	}
}

void CMemory::map_space (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint8 *data)
{
	uint32	c, i, p;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			Map[p] = data;
			BlockIsROM[p] = FALSE;
			BlockIsRAM[p] = TRUE;
		}
	}
}

void CMemory::map_index (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, int index, int type)
{
	uint32	c, i, p;
	bool8	isROM, isRAM;

	isROM = ((type == MAP_TYPE_I_O) || (type == MAP_TYPE_RAM)) ? FALSE : TRUE;
	isRAM = ((type == MAP_TYPE_I_O) || (type == MAP_TYPE_ROM)) ? FALSE : TRUE;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			Map[p] = (uint8 *) (pint) index;
			BlockIsROM[p] = isROM;
			BlockIsRAM[p] = isRAM;
		}
	}
}

void CMemory::map_System (void)
{
	// will be overwritten
	map_space(0x00, 0x3f, 0x0000, 0x1fff, RAM);
	map_index(0x00, 0x3f, 0x2000, 0x3fff, MAP_PPU, MAP_TYPE_I_O);
	map_index(0x00, 0x3f, 0x4000, 0x5fff, MAP_CPU, MAP_TYPE_I_O);
	map_space(0x80, 0xbf, 0x0000, 0x1fff, RAM);
	map_index(0x80, 0xbf, 0x2000, 0x3fff, MAP_PPU, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x4000, 0x5fff, MAP_CPU, MAP_TYPE_I_O);
}

void CMemory::map_WRAM (void)
{
	// will overwrite others
	map_space(0x7e, 0x7e, 0x0000, 0xffff, RAM);
	map_space(0x7f, 0x7f, 0x0000, 0xffff, RAM + 0x10000);
}

void CMemory::map_LoROMSRAM (void)
{
        uint32 hi;

        if (ROMSize > 11 || SRAMSize > 5)
            hi = 0x7fff;
        else
            hi = 0xffff;

	map_index(0x70, 0x7d, 0x0000, hi, MAP_LOROM_SRAM, MAP_TYPE_RAM);
	if (SRAMSize > 0)
            map_index(0xf0, 0xff, 0x0000, hi, MAP_LOROM_SRAM, MAP_TYPE_RAM);
}

void CMemory::map_HiROMSRAM (void)
{
	map_index(0x20, 0x3f, 0x6000, 0x7fff, MAP_HIROM_SRAM, MAP_TYPE_RAM);
	map_index(0xa0, 0xbf, 0x6000, 0x7fff, MAP_HIROM_SRAM, MAP_TYPE_RAM);
}

void CMemory::map_DSP (void)
{
	switch (DSP0.maptype)
	{
		case M_DSP1_LOROM_S:
			map_index(0x20, 0x3f, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xa0, 0xbf, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			break;

		case M_DSP1_LOROM_L:
			map_index(0x60, 0x6f, 0x0000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xe0, 0xef, 0x0000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
			break;

		case M_DSP1_HIROM:
			map_index(0x00, 0x1f, 0x6000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0x80, 0x9f, 0x6000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
			break;

		case M_DSP2_LOROM:
			map_index(0x20, 0x3f, 0x6000, 0x6fff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0x20, 0x3f, 0x8000, 0xbfff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xa0, 0xbf, 0x6000, 0x6fff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xa0, 0xbf, 0x8000, 0xbfff, MAP_DSP, MAP_TYPE_I_O);
			break;

		case M_DSP3_LOROM:
			map_index(0x20, 0x3f, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xa0, 0xbf, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			break;

		case M_DSP4_LOROM:
			map_index(0x30, 0x3f, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xb0, 0xbf, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			break;
	}
}

void CMemory::map_C4 (void)
{
	map_index(0x00, 0x3f, 0x6000, 0x7fff, MAP_C4, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x6000, 0x7fff, MAP_C4, MAP_TYPE_I_O);
}

void CMemory::map_OBC1 (void)
{
	map_index(0x00, 0x3f, 0x6000, 0x7fff, MAP_OBC_RAM, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x6000, 0x7fff, MAP_OBC_RAM, MAP_TYPE_I_O);
}

void CMemory::map_SetaRISC (void)
{
	map_index(0x00, 0x3f, 0x3000, 0x3fff, MAP_SETA_RISC, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x3000, 0x3fff, MAP_SETA_RISC, MAP_TYPE_I_O);
}

void CMemory::map_SetaDSP (void)
{
	// where does the SETA chip access, anyway?
	// please confirm this?
	map_index(0x68, 0x6f, 0x0000, 0x7fff, MAP_SETA_DSP, MAP_TYPE_RAM);
	// and this!
	map_index(0x60, 0x67, 0x0000, 0x3fff, MAP_SETA_DSP, MAP_TYPE_I_O);

	// ST-0010:
	// map_index(0x68, 0x6f, 0x0000, 0x0fff, MAP_SETA_DSP, ?);
}

void CMemory::map_WriteProtectROM (void)
{
	memmove((void *) WriteMap, (void *) Map, sizeof(Map));

	for (int c = 0; c < 0x1000; c++)
	{
		if (BlockIsROM[c])
			WriteMap[c] = (uint8 *) MAP_NONE;
	}
}

void CMemory::Map_Initialize (void)
{
	for (int c = 0; c < 0x1000; c++)
	{
		Map[c]      = (uint8 *) MAP_NONE;
		WriteMap[c] = (uint8 *) MAP_NONE;
		BlockIsROM[c] = FALSE;
		BlockIsRAM[c] = FALSE;
	}
}

void CMemory::Map_LoROMMap (void)
{
	printf("Map_LoROMMap\n");
	map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize);

	if (Settings.DSP)
		map_DSP();
	else
	if (Settings.C4)
		map_C4();
	else
	if (Settings.OBC1)
		map_OBC1();
	else
	if (Settings.SETA == ST_018)
		map_SetaRISC();

    map_LoROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

// P2 — SGB cart map. Standard LoROM with the 0x6000-0x7FFF range in banks
// 0x00-0x3F and 0x80-0xBF routed to MAP_SGB_ICD2 so SNES accesses to the
// BIOS's cart-chip registers land in our bridge.
void CMemory::Map_SGBLoROMMap (void)
{
	printf("Map_SGBLoROMMap\n");
	map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize);

	map_index(0x00, 0x3f, 0x6000, 0x7fff, MAP_SGB_ICD2, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x6000, 0x7fff, MAP_SGB_ICD2, MAP_TYPE_I_O);

	map_WRAM();
	map_WriteProtectROM();
}

void CMemory::Map_NoMAD1LoROMMap (void)
{
	printf("Map_NoMAD1LoROMMap\n");
	map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize);

	map_index(0x70, 0x7f, 0x0000, 0xffff, MAP_LOROM_SRAM, MAP_TYPE_RAM);
	map_index(0xf0, 0xff, 0x0000, 0xffff, MAP_LOROM_SRAM, MAP_TYPE_RAM);

	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_JumboLoROMMap (void)
{
	// XXX: Which game uses this?
	printf("Map_JumboLoROMMap\n");
	map_System();

	map_lorom_offset(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize - 0x400000, 0x400000);
	map_lorom_offset(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize - 0x600000, 0x600000);
	map_lorom_offset(0x80, 0xbf, 0x8000, 0xffff, 0x400000, 0);
	map_lorom_offset(0xc0, 0xff, 0x0000, 0xffff, 0x400000, 0x200000);

	map_LoROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_ROM24MBSLoROMMap (void)
{
	// PCB: BSC-1A5M-01, BSC-1A7M-10
	printf("Map_ROM24MBSLoROMMap\n");
	map_System();

	map_lorom_offset(0x00, 0x1f, 0x8000, 0xffff, 0x100000, 0);
	map_lorom_offset(0x20, 0x3f, 0x8000, 0xffff, 0x100000, 0x100000);
	map_lorom_offset(0x80, 0x9f, 0x8000, 0xffff, 0x100000, 0x200000);
	map_lorom_offset(0xa0, 0xbf, 0x8000, 0xffff, 0x100000, 0x100000);

	map_LoROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SRAM512KLoROMMap (void)
{
	printf("Map_SRAM512KLoROMMap\n");
	map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize);

	map_space(0x70, 0x70, 0x0000, 0xffff, SRAM);
	map_space(0x71, 0x71, 0x0000, 0xffff, SRAM + 0x8000);
	map_space(0x72, 0x72, 0x0000, 0xffff, SRAM + 0x10000);
	map_space(0x73, 0x73, 0x0000, 0xffff, SRAM + 0x18000);

	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SufamiTurboLoROMMap (void)
{
	printf("Map_SufamiTurboLoROMMap\n");
	map_System();

	map_lorom_offset(0x00, 0x1f, 0x8000, 0xffff, 0x40000, 0);
	map_lorom_offset(0x20, 0x3f, 0x8000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);
	map_lorom_offset(0x40, 0x5f, 0x8000, 0xffff, Multi.cartSizeB, Multi.cartOffsetB);
	map_lorom_offset(0x80, 0x9f, 0x8000, 0xffff, 0x40000, 0);
	map_lorom_offset(0xa0, 0xbf, 0x8000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);
	map_lorom_offset(0xc0, 0xdf, 0x8000, 0xffff, Multi.cartSizeB, Multi.cartOffsetB);

	if (Multi.sramSizeA)
	{
		map_index(0x60, 0x63, 0x8000, 0xffff, MAP_LOROM_SRAM, MAP_TYPE_RAM);
		map_index(0xe0, 0xe3, 0x8000, 0xffff, MAP_LOROM_SRAM, MAP_TYPE_RAM);
	}

	if (Multi.sramSizeB)
	{
		map_index(0x70, 0x73, 0x8000, 0xffff, MAP_LOROM_SRAM_B, MAP_TYPE_RAM);
		map_index(0xf0, 0xf3, 0x8000, 0xffff, MAP_LOROM_SRAM_B, MAP_TYPE_RAM);
	}

	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SufamiTurboPseudoLoROMMap (void)
{
	// for combined images
	printf("Map_SufamiTurboPseudoLoROMMap\n");
	map_System();

	map_lorom_offset(0x00, 0x1f, 0x8000, 0xffff, 0x40000, 0);
	map_lorom_offset(0x20, 0x3f, 0x8000, 0xffff, 0x100000, 0x100000);
	map_lorom_offset(0x40, 0x5f, 0x8000, 0xffff, 0x100000, 0x200000);
	map_lorom_offset(0x80, 0x9f, 0x8000, 0xffff, 0x40000, 0);
	map_lorom_offset(0xa0, 0xbf, 0x8000, 0xffff, 0x100000, 0x100000);
	map_lorom_offset(0xc0, 0xdf, 0x8000, 0xffff, 0x100000, 0x200000);

	// I don't care :P
	map_space(0x60, 0x63, 0x8000, 0xffff, SRAM - 0x8000);
	map_space(0xe0, 0xe3, 0x8000, 0xffff, SRAM - 0x8000);
	map_space(0x70, 0x73, 0x8000, 0xffff, SRAM + 0x4000 - 0x8000);
	map_space(0xf0, 0xf3, 0x8000, 0xffff, SRAM + 0x4000 - 0x8000);

	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SuperFXLoROMMap (void)
{
	printf("Map_SuperFXLoROMMap\n");
	map_System();

	// Replicate the first 2Mb of the ROM at ROM + FX_MEMORY_32K_MIRRORS such that each 32K
	// block is repeated twice in each 64K block.
	for (int c = 0; c < 64; c++)
	{
		memmove(&ROM[FX_MEMORY_32K_MIRRORS + 0x0000 + c * 0x10000], &ROM[c * 0x8000], 0x8000);
		memmove(&ROM[FX_MEMORY_32K_MIRRORS + 0x8000 + c * 0x10000], &ROM[c * 0x8000], 0x8000);
	}

	// Support for ROMs up to 11MB by setting the ROM size to $0E, The GSU still cannot access more than 2 MB.
	if (ROM[0x7FD7] >= 0x0E) {
		map_lorom(0x00, 0x3f, 0x8000, 0xffff, 0x200000);
		map_lorom_offset(0x80, 0xbf, 0x8000, 0xffff, 0x200000, 0x200000);

		map_hirom_offset(0x40, 0x6f, 0x0000, 0xffff, 0x300000, 0x800000);
		map_hirom_offset(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize, 0x400000);

		map_space(0x00, 0x3f, 0x6000, 0x7fff, SRAM - 0x6000);
		map_space(0x80, 0xbf, 0x6000, 0x7fff, SRAM - 0x6000);
	}
	// Check GSU revision (not 100% accurate but it works)
	// GSU2
	else if (CalculatedSize > 0x200000)
	{
		map_lorom(0x00, 0x3f, 0x8000, 0xffff, 0x200000);
		map_lorom(0x80, 0xbf, 0x8000, 0xffff, 0x200000);

		map_hirom_offset(0x40, 0x5f, 0x0000, 0xffff, 0x200000, 0);
		map_hirom_offset(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize, 0);

		map_space(0x00, 0x3f, 0x6000, 0x7fff, SRAM - 0x6000);
		map_space(0x80, 0xbf, 0x6000, 0x7fff, SRAM - 0x6000);
	}
	// GSU1
	else
	{
		map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
		map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);

		map_hirom_offset(0x40, 0x5f, 0x0000, 0xffff, CalculatedSize, 0);
		map_hirom_offset(0xc0, 0xdf, 0x0000, 0xffff, CalculatedSize, 0);

		map_space(0x00, 0x3f, 0x6000, 0x7fff, SRAM - 0x6000);
		map_space(0x80, 0xbf, 0x6000, 0x7fff, SRAM - 0x6000);
		map_space(0xf0, 0xf0, 0x0000, 0xffff, SRAM);
		map_space(0xf1, 0xf1, 0x0000, 0xffff, SRAM + 0x10000);
	}

	// Respect SRAMSize
	map_space(0x70, 0x70, 0x0000, 0xffff, SRAM);
	map_space(0x71, 0x71, 0x0000, 0xffff, SRAM + 0x10000);
	if(SRAMSize > 7) {
		map_space(0x72, 0x72, 0x0000, 0xffff, SRAM + 0x20000);
		map_space(0x73, 0x73, 0x0000, 0xffff, SRAM + 0x30000);
	}
	if(SRAMSize > 8) {
		map_space(0x74, 0x74, 0x0000, 0xffff, SRAM + 0x40000);
		map_space(0x75, 0x75, 0x0000, 0xffff, SRAM + 0x50000);
		map_space(0x76, 0x76, 0x0000, 0xffff, SRAM + 0x60000);
		map_space(0x77, 0x77, 0x0000, 0xffff, SRAM + 0x70000);
	}
	if(SRAMSize > 9) {
		map_space(0x78, 0x78, 0x0000, 0xffff, SRAM + 0x80000);
		map_space(0x79, 0x79, 0x0000, 0xffff, SRAM + 0x90000);
		map_space(0x7A, 0x7A, 0x0000, 0xffff, SRAM + 0xA0000);
		map_space(0x7B, 0x7B, 0x0000, 0xffff, SRAM + 0xB0000);
		map_space(0x7C, 0x7C, 0x0000, 0xffff, SRAM + 0xC0000);
		map_space(0x7D, 0x7D, 0x0000, 0xffff, SRAM + 0xD0000);
	}

	map_WRAM();

	map_WriteProtectROM();
}

// Super FX 3 (LRG): GSU MMIO moves to $7000-$7FFF, ROM widens to banks
// $40-$6F/$C0-$FF, RAM sits only at $70-$71, no $6000-$7FFF RAM window.
void CMemory::Map_SuperFX3LoROMMap (void)
{
	printf("Map_SuperFX3LoROMMap\n");
	map_System();

	// Replicate the first 2Mb of the ROM at ROM + FX_MEMORY_32K_MIRRORS such that each 32K
	// block is repeated twice in each 64K block.
	for (int c = 0; c < 64; c++)
	{
		memmove(&ROM[FX_MEMORY_32K_MIRRORS + 0x0000 + c * 0x10000], &ROM[c * 0x8000], 0x8000);
		memmove(&ROM[FX_MEMORY_32K_MIRRORS + 0x8000 + c * 0x10000], &ROM[c * 0x8000], 0x8000);
	}

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);

	map_hirom_offset(0x40, 0x6f, 0x0000, 0xffff, CalculatedSize, 0);
	map_hirom_offset(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize, 0);

	// GSU registers, routed through S9xGetPPU/S9xSetPPU
	map_index(0x00, 0x3f, 0x7000, 0x7fff, MAP_PPU, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x7000, 0x7fff, MAP_PPU, MAP_TYPE_I_O);

	map_space(0x70, 0x70, 0x0000, 0xffff, SRAM);
	map_space(0x71, 0x71, 0x0000, 0xffff, SRAM + 0x10000);

	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SetaDSPLoROMMap (void)
{
	printf("Map_SetaDSPLoROMMap\n");
	map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x40, 0x7f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0xc0, 0xff, 0x8000, 0xffff, CalculatedSize);

	map_SetaDSP();

    map_LoROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SDD1LoROMMap (void)
{
	printf("Map_SDD1LoROMMap\n");
	map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);

	map_hirom_offset(0x60, 0x7f, 0x0000, 0xffff, CalculatedSize, 0);
	map_hirom_offset(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize, 0); // will be overwritten dynamically

	map_index(0x70, 0x7f, 0x0000, 0x7fff, MAP_LOROM_SRAM, MAP_TYPE_RAM);
	map_index(0xa0, 0xbf, 0x6000, 0x7fff, MAP_LOROM_SRAM, MAP_TYPE_RAM);

	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SA1LoROMMap (void)
{
	printf("Map_SA1LoROMMap\n");
	map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);

	map_hirom_offset(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize, 0);

	map_space(0x00, 0x3f, 0x3000, 0x37ff, FillRAM);
	map_space(0x80, 0xbf, 0x3000, 0x37ff, FillRAM);
	map_index(0x00, 0x3f, 0x6000, 0x7fff, MAP_BWRAM, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x6000, 0x7fff, MAP_BWRAM, MAP_TYPE_I_O);

	for (int c = 0x40; c < 0x4f; c++)
		map_space(c, c, 0x0000, 0xffff, SRAM + (c & 3) * 0x10000);

	map_WRAM();

	map_WriteProtectROM();

	// Now copy the map and correct it for the SA1 CPU.
	memmove((void *) SA1.Map, (void *) Map, sizeof(Map));
	memmove((void *) SA1.WriteMap, (void *) WriteMap, sizeof(WriteMap));

	// SA-1 Banks 00->3f and 80->bf
	for (int c = 0x000; c < 0x400; c += 0x10)
	{
		SA1.Map[c + 0] = SA1.Map[c + 0x800] = FillRAM + 0x3000;
		SA1.Map[c + 1] = SA1.Map[c + 0x801] = (uint8 *) MAP_NONE;
		SA1.WriteMap[c + 0] = SA1.WriteMap[c + 0x800] = FillRAM + 0x3000;
		SA1.WriteMap[c + 1] = SA1.WriteMap[c + 0x801] = (uint8 *) MAP_NONE;
	}

	// SA-1 Banks 40->4f
	for (int c = 0x400; c < 0x500; c++)
		SA1.Map[c] = SA1.WriteMap[c] = (uint8*) MAP_SA1RAM;

	// SA-1 Banks 60->6f
	for (int c = 0x600; c < 0x700; c++)
		SA1.Map[c] = SA1.WriteMap[c] = (uint8 *) MAP_BWRAM_BITMAP;

	// WRAM is inaccessable
	for (int c = 0x7e0; c < 0x800; c++)
		SA1.Map[c] = SA1.WriteMap[c] = (uint8 *) MAP_NONE;

	BWRAM = SRAM;
}

void CMemory::Map_BSSA1LoROMMap(void)
{
	printf("Map_BSSA1LoROMMap\n");
	map_System();

	map_lorom_offset(0x00, 0x3f, 0x8000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);
	map_lorom_offset(0x80, 0xbf, 0x8000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);

	map_hirom_offset(0xc0, 0xff, 0x0000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);

	map_space(0x00, 0x3f, 0x3000, 0x3fff, FillRAM);
	map_space(0x80, 0xbf, 0x3000, 0x3fff, FillRAM);
	map_index(0x00, 0x3f, 0x6000, 0x7fff, MAP_BWRAM, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x6000, 0x7fff, MAP_BWRAM, MAP_TYPE_I_O);

	for (int c = 0x40; c < 0x80; c++)
		map_space(c, c, 0x0000, 0xffff, SRAM + (c & 1) * 0x10000);

	map_WRAM();

	map_WriteProtectROM();

	// Now copy the map and correct it for the SA1 CPU.
	memmove((void *) SA1.Map, (void *) Map, sizeof(Map));
	memmove((void *) SA1.WriteMap, (void *) WriteMap, sizeof(WriteMap));

	// SA-1 Banks 00->3f and 80->bf
	for (int c = 0x000; c < 0x400; c += 0x10)
	{
		SA1.Map[c + 0] = SA1.Map[c + 0x800] = FillRAM + 0x3000;
		SA1.Map[c + 1] = SA1.Map[c + 0x801] = (uint8 *) MAP_NONE;
		SA1.WriteMap[c + 0] = SA1.WriteMap[c + 0x800] = FillRAM + 0x3000;
		SA1.WriteMap[c + 1] = SA1.WriteMap[c + 0x801] = (uint8 *) MAP_NONE;
	}

	// SA-1 Banks 60->6f
	for (int c = 0x600; c < 0x700; c++)
		SA1.Map[c] = SA1.WriteMap[c] = (uint8 *) MAP_BWRAM_BITMAP;

	// WRAM is inaccessable
	for (int c = 0x7e0; c < 0x800; c++)
		SA1.Map[c] = SA1.WriteMap[c] = (uint8 *) MAP_NONE;

	BWRAM = SRAM;
}

void CMemory::Map_HiROMMap (void)
{
	printf("Map_HiROMMap\n");
	map_System();

	map_hirom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_hirom(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize);
	map_hirom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	map_hirom(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize);

	if (Settings.DSP)
		map_DSP();

	map_HiROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_ExtendedHiROMMap (void)
{
	printf("Map_ExtendedHiROMMap\n");
	map_System();

	map_hirom_offset(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize - 0x400000, 0x400000);
	map_hirom_offset(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize - 0x400000, 0x400000);
	map_hirom_offset(0x80, 0xbf, 0x8000, 0xffff, 0x400000, 0);
	map_hirom_offset(0xc0, 0xff, 0x0000, 0xffff, 0x400000, 0);

	map_HiROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SPC7110HiROMMap (void)
{
	printf("Map_SPC7110HiROMMap\n");
	map_System();

	map_index(0x00, 0x00, 0x6000, 0x7fff, MAP_HIROM_SRAM, MAP_TYPE_RAM);
	map_hirom(0x00, 0x0f, 0x8000, 0xffff, CalculatedSize);
	map_index(0x30, 0x30, 0x6000, 0x7fff, MAP_HIROM_SRAM, MAP_TYPE_RAM);
	if(Memory.ROMSize >= 13)
		map_hirom_offset(0x40, 0x4f, 0x0000, 0xffff, CalculatedSize, 0x600000);
	map_index(0x50, 0x50, 0x0000, 0xffff, MAP_SPC7110_DRAM, MAP_TYPE_ROM);
	map_hirom(0x80, 0x8f, 0x8000, 0xffff, CalculatedSize);
	map_hirom_offset(0xc0, 0xcf, 0x0000, 0xffff, CalculatedSize, 0);
	map_index(0xd0, 0xff, 0x0000, 0xffff, MAP_SPC7110_ROM,  MAP_TYPE_ROM);

	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_BSCartLoROMMap(uint8 mapping)
{
	printf("Map_BSCartLoROMMap\n");

	BSX.MMC[0x02] = 0x00;
	BSX.MMC[0x0C] = 0x80;

	map_System();

	if (mapping)
	{
		map_lorom_offset(0x00, 0x1f, 0x8000, 0xffff, 0x100000, 0);
		map_lorom_offset(0x20, 0x3f, 0x8000, 0xffff, 0x100000, 0x100000);
		map_lorom_offset(0x80, 0x9f, 0x8000, 0xffff, 0x100000, 0x200000);
		map_lorom_offset(0xa0, 0xbf, 0x8000, 0xffff, 0x100000, 0x100000);
	}
	else
	{
		map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
		map_lorom(0x40, 0x7f, 0x0000, 0x7fff, CalculatedSize);
		map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
		map_lorom(0xc0, 0xff, 0x0000, 0x7fff, CalculatedSize);
	}

	map_LoROMSRAM();
	map_index(0xc0, 0xef, 0x0000, 0xffff, MAP_BSX, MAP_TYPE_RAM);
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_BSCartHiROMMap(void)
{
	printf("Map_BSCartHiROMMap\n");

	BSX.MMC[0x02] = 0x80;
	BSX.MMC[0x0C] = 0x80;

	map_System();
	map_hirom_offset(0x00, 0x1f, 0x8000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);
	map_hirom_offset(0x20, 0x3f, 0x8000, 0xffff, Multi.cartSizeB, Multi.cartOffsetB);
	map_hirom_offset(0x40, 0x5f, 0x0000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);
	map_hirom_offset(0x60, 0x7f, 0x0000, 0xffff, Multi.cartSizeB, Multi.cartOffsetB);
	map_hirom_offset(0x80, 0x9f, 0x8000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);
	map_hirom_offset(0xa0, 0xbf, 0x8000, 0xffff, Multi.cartSizeB, Multi.cartOffsetB);
	map_hirom_offset(0xc0, 0xdf, 0x0000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);

	if ((ROM[Multi.cartOffsetB + 0xFF00] == 0x4D)
		&& (ROM[Multi.cartOffsetB + 0xFF02] == 0x50)
		&& ((ROM[Multi.cartOffsetB + 0xFF06] & 0xF0) == 0x70))
	{
		//Type 7 Memory Pack detection - if detected, emulate it as Mask ROM
		map_hirom_offset(0xe0, 0xff, 0x0000, 0xffff, Multi.cartSizeB, Multi.cartOffsetB);
	}
	else
	{
		map_index(0xe0, 0xff, 0x0000, 0xffff, MAP_BSX, MAP_TYPE_RAM);
	}

	map_HiROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

// checksum

uint16 CMemory::checksum_calc_sum (uint8 *data, uint32 length)
{
	uint16	sum = 0;

	for (uint32 i = 0; i < length; i++)
		sum += data[i];

	return (sum);
}

uint16 CMemory::checksum_mirror_sum (uint8 *start, uint32 &length, uint32 mask)
{
	// from NSRT
	while (!(length & mask) && mask)
		mask >>= 1;

	uint16	part1 = checksum_calc_sum(start, mask);
	uint16	part2 = 0;

	uint32	next_length = length - mask;
	if (next_length)
	{
		part2 = checksum_mirror_sum(start + mask, next_length, mask >> 1);

		while (next_length < mask)
		{
			next_length += next_length;
			part2 += part2;
		}

		length = mask + mask;
	}

	return (part1 + part2);
}

void CMemory::Checksum_Calculate (void)
{
	// from NSRT
	uint16	sum = 0;

	if (Settings.BS && !Settings.BSXItself)
		sum = checksum_calc_sum(ROM, CalculatedSize) - checksum_calc_sum(ROM + (HiROM ? 0xffb0 : 0x7fb0), 48);
	else if (Settings.SPC7110)
	{
		sum = checksum_calc_sum(ROM, CalculatedSize);
		if (CalculatedSize == 0x300000)
			sum += sum;
	}
	else
	{
		if (CalculatedSize & 0x7fff)
			sum = checksum_calc_sum(ROM, CalculatedSize);
		else
		{
			uint32	length = CalculatedSize;
			sum = checksum_mirror_sum(ROM, length);
		}
	}

	CalculatedChecksum = sum;
}

// information

const char * CMemory::MapType (void)
{
	return (HiROM ? ((ExtendedFormat != NOPE) ? "ExHiROM": "HiROM") : "LoROM");
}

const char * CMemory::StaticRAMSize (void)
{
	static char	str[20];

	if (SRAMSize > 16)
		strcpy(str, "Corrupt");
	else
		sprintf(str, "%d Kbit", 8 * (SRAMMask + 1) / 1024);

	return (str);
}

const char * CMemory::Size (void)
{
	static char	str[20];

	if (Multi.cartType == 4)
		strcpy(str, "N/A");
	else if (ROMSize < 7 || ROMSize - 7 > 23)
		strcpy(str, "Corrupt");
	else
		sprintf(str, "%d Mbit", 1 << (ROMSize - 7));

	return (str);
}

const char * CMemory::Revision (void)
{
	static char	str[20];

	sprintf(str, "1.%d", HiROM ? ((ExtendedFormat != NOPE) ? ROM[0x40ffdb] : ROM[0xffdb]) : ROM[0x7fdb]);

	return (str);
}

const char * CMemory::KartContents (void)
{
	static char			str[64];
	static const char	*contents[3] = { "ROM", "ROM+RAM", "ROM+RAM+BAT" };

	char	chip[20];

	if (ROMType == 0 && !Settings.BS)
		return ("ROM");

	if (Settings.BS)
		strcpy(chip, "+BS");
	else if (Settings.SuperFX)
		strcpy(chip, SuperFX.isFx3 ? "+Super FX 3" : "+Super FX");
	else if (Settings.SDD1)
		strcpy(chip, "+S-DD1");
	else if (Settings.OBC1)
		strcpy(chip, "+OBC1");
	else if (Settings.SA1)
		strcpy(chip, "+SA-1");
	else if (Settings.SPC7110RTC)
		strcpy(chip, "+SPC7110+RTC");
	else if (Settings.SPC7110)
		strcpy(chip, "+SPC7110");
	else if (Settings.SRTC)
		strcpy(chip, "+S-RTC");
	else if (Settings.C4)
		strcpy(chip, "+C4");
	else if (Settings.SETA == ST_010)
		strcpy(chip, "+ST-010");
	else if (Settings.SETA == ST_011)
		strcpy(chip, "+ST-011");
	else if (Settings.SETA == ST_018)
		strcpy(chip, "+ST-018");
	else if (Settings.DSP)
		sprintf(chip, "+DSP-%d", Settings.DSP);
	else
		strcpy(chip, "");

	if (Settings.MSU1)
		sprintf(chip + strlen(chip), "+MSU-1");

	sprintf(str, "%s%s", contents[(ROMType & 0xf) % 3], chip);

	return (str);
}

const char * CMemory::Country (void)
{
	switch (ROMRegion)
	{
		case 0:		return("Japan");
		case 1:		return("USA and Canada");
		case 2:		return("Oceania, Europe and Asia");
		case 3:		return("Sweden");
		case 4:		return("Finland");
		case 5:		return("Denmark");
		case 6:		return("France");
		case 7:		return("Holland");
		case 8:		return("Spain");
		case 9:		return("Germany, Austria and Switzerland");
		case 10:	return("Italy");
		case 11:	return("Hong Kong and China");
		case 12:	return("Indonesia");
		case 13:	return("South Korea");
		default:	return("Unknown");
	}
}

const char * CMemory::PublishingCompany (void)
{
	if (CompanyId >= (int) (sizeof(nintendo_licensees) / sizeof(nintendo_licensees[0])) || CompanyId < 0)
		return ("Unknown");

	if (nintendo_licensees[CompanyId] == NULL)
		return ("Unknown");

	return (nintendo_licensees[CompanyId]);
}

std::string CMemory::GetMultilineROMInfo()
{
    bool8 isChecksumOK = (Memory.ROMChecksum + Memory.ROMComplementChecksum == 0xffff) &&
                         (Memory.ROMChecksum == Memory.CalculatedChecksum);
    std::string utf8_romname = Memory.ROMName;
    std::string tvstandard = Settings.PAL ? "PAL" : "NTSC";
	std::string romid = Memory.ROMId;
    std::string checksum = isChecksumOK              ? "Checksum OK"
                           : Settings.IsPatched == 3 ? "UPS patched"
                           : Settings.IsPatched == 2 ? "BPS patched"
                           : Settings.IsPatched == 1 ? "IPS patched"
                                                     : "Invalid Checksum";

    std::stringstream ss;
    ss << "\"" << utf8_romname << "\" (" + tvstandard + ") version " << Memory.Revision() << "\n";
    ss << Memory.KartContents() << ": " << Memory.MapType() << ": " << Memory.Size() << ", SRAM: " << Memory.StaticRAMSize() << "\n";
    ss << "ID: " << romid << ", CRC32: " << std::setfill('0') << std::setw(8) << std::setbase(16) << Memory.ROMCRC32 << ", " << checksum;

	return ss.str();
}

void CMemory::MakeRomInfoText (char *romtext)
{
	char	temp[256];

	romtext[0] = 0;

	sprintf(temp,   "            Cart Name: %s", ROMName);
	strcat(romtext, temp);
	sprintf(temp, "\n            Game Code: %s", ROMId);
	strcat(romtext, temp);
	sprintf(temp, "\n             Contents: %s", KartContents());
	strcat(romtext, temp);
	sprintf(temp, "\n                  Map: %s", MapType());
	strcat(romtext, temp);
	sprintf(temp, "\n                Speed: 0x%02X (%s)", ROMSpeed, (ROMSpeed & 0x10) ? "FastROM" : "SlowROM");
	strcat(romtext, temp);
	sprintf(temp, "\n                 Type: 0x%02X", ROMType);
	strcat(romtext, temp);
	sprintf(temp, "\n    Size (calculated): %dMbits", CalculatedSize / 0x20000);
	strcat(romtext, temp);
	sprintf(temp, "\n        Size (header): %s", Size());
	strcat(romtext, temp);
	sprintf(temp, "\n            SRAM size: %s", StaticRAMSize());
	strcat(romtext, temp);
	sprintf(temp, "\nChecksum (calculated): 0x%04X", CalculatedChecksum);
	strcat(romtext, temp);
	sprintf(temp, "\n    Checksum (header): 0x%04X", ROMChecksum);
	strcat(romtext, temp);
	sprintf(temp, "\n  Complement (header): 0x%04X", ROMComplementChecksum);
	strcat(romtext, temp);
	sprintf(temp, "\n         Video Output: %s", (ROMRegion > 12 || ROMRegion < 2) ? "NTSC 60Hz" : "PAL 50Hz");
	strcat(romtext, temp);
	sprintf(temp, "\n             Revision: %s", Revision());
	strcat(romtext, temp);
	sprintf(temp, "\n             Licensee: %s", PublishingCompany());
	strcat(romtext, temp);
	sprintf(temp, "\n               Region: %s", Country());
	strcat(romtext, temp);
	sprintf(temp, "\n                CRC32: 0x%08X", ROMCRC32);
	strcat(romtext, temp);
}

// hack

bool8 CMemory::match_na (const char *str)
{
	return (strcmp(ROMName, str) == 0);
}

bool8 CMemory::match_nn (const char *str)
{
	return (strncmp(ROMName, str, strlen(str)) == 0);
}

bool8 CMemory::match_nc (const char *str)
{
	return (strncasecmp(ROMName, str, strlen(str)) == 0);
}

bool8 CMemory::match_id (const char *str)
{
	return (strncmp(ROMId, str, strlen(str)) == 0);
}

SPF94 PF94;

// Campus Challenge '92 (EVENT-CC92). All four images are plain LoROM:
//   $00-$1F:8000-FFFF  selected game (0x09=SMW, 0x05=F-Zero, 0x03=Pilotwings)
//   $80-$9F:8000-FFFF  menu (Program), always mapped
//   $20-$3F,$A0-$BF:8000-FFFF  DSP-1 (uPD7725), for Pilotwings
//   $C0:0000 status read, $E0:0000 select write
// SRAM ($70-$7D,$F0-$FF:0000-7FFF) is mapped once at detection.
static void CC92MapGameWindow (void)
{
	int id = 0;
	if (PF94.select == 0x09)      id = 1;
	else if (PF94.select == 0x05) id = 2;
	else if (PF94.select == 0x03) id = 3;
	if (!PF94.romSize[id])
		id = 0;

	uint8	*game = Memory.ROM + PF94.romOff[id];
	uint32	gmask = PF94.romSize[id] - 1;
	uint8	*menu = Memory.ROM + PF94.romOff[0];
	uint32	mmask = PF94.romSize[0] - 1;

	for (uint32 bank = 0; bank <= 0x1f; bank++)
	{
		uint8 *gbase = game + ((bank << 15) & gmask) - 0x8000;
		uint8 *mbase = menu + ((bank << 15) & mmask) - 0x8000;
		for (uint32 blk = 8; blk <= 15; blk++)
		{
			uint32 lo = (bank << 4) | blk;   // $00-$1F: selected game
			uint32 hi = lo + 0x800;          // $80-$9F: menu
			Memory.Map[lo] = gbase;
			Memory.Map[hi] = mbase;
			Memory.WriteMap[lo] = Memory.WriteMap[hi] = (uint8 *) CMemory::MAP_NONE;
			Memory.BlockIsROM[lo] = Memory.BlockIsROM[hi] = TRUE;
			Memory.BlockIsRAM[lo] = Memory.BlockIsRAM[hi] = FALSE;
		}
	}

	// DSP-1 window $20-$3F,$A0-$BF:8000-FFFF (M_DSP1_LOROM_S layout).
	for (uint32 bank = 0x20; bank <= 0x3f; bank++)
	{
		for (uint32 blk = 8; blk <= 15; blk++)
		{
			uint32 lo = (bank << 4) | blk;   // $20-$3F
			uint32 hi = lo + 0x800;          // $A0-$BF
			Memory.Map[lo] = Memory.Map[hi] = (uint8 *) CMemory::MAP_DSP;
			Memory.WriteMap[lo] = Memory.WriteMap[hi] = (uint8 *) CMemory::MAP_DSP;
			Memory.BlockIsROM[lo] = Memory.BlockIsROM[hi] = FALSE;
			Memory.BlockIsRAM[lo] = Memory.BlockIsRAM[hi] = FALSE;
		}
	}

	// MCU registers: status read $C0:0000, select write $E0:0000.
	Memory.Map[(0xC0 << 4) | 0]      = (uint8 *) CMemory::MAP_EVENT;
	Memory.WriteMap[(0xC0 << 4) | 0] = (uint8 *) CMemory::MAP_NONE;
	Memory.BlockIsROM[(0xC0 << 4) | 0] = Memory.BlockIsRAM[(0xC0 << 4) | 0] = FALSE;
	Memory.Map[(0xE0 << 4) | 0]      = (uint8 *) CMemory::MAP_NONE;
	Memory.WriteMap[(0xE0 << 4) | 0] = (uint8 *) CMemory::MAP_EVENT;
	Memory.BlockIsROM[(0xE0 << 4) | 0] = Memory.BlockIsRAM[(0xE0 << 4) | 0] = FALSE;
}

static void PF94MapGameWindow (void)
{
	if (PF94.board == EVENT_BOARD_CC92)
	{
		CC92MapGameWindow();
		return;
	}

	int id = 0;
	if (PF94.select == 0x09)      id = 1;
	else if (PF94.select == 0x0c) id = 2;
	else if (PF94.select == 0x0a) id = 3;
	if (!PF94.romSize[id])
		id = 0;

	uint8	*img = Memory.ROM + PF94.romOff[id];
	uint32	mask = PF94.romSize[id] - 1;

	for (uint32 bank = 0; bank <= 0x1f; bank++)
	{
		uint8 *base;
		if (id == 2)
			base = img + ((bank << 16) & mask);
		else
			base = img + ((bank << 15) & mask) - 0x8000;

		for (uint32 blk = 8; blk <= 15; blk++)
		{
			uint32 i = (bank << 4) | blk;
			Memory.Map[i] = Memory.Map[i + 0x800] = base;
			Memory.WriteMap[i] = Memory.WriteMap[i + 0x800] = (uint8 *) CMemory::MAP_NONE;
			Memory.BlockIsROM[i] = Memory.BlockIsROM[i + 0x800] = TRUE;
			Memory.BlockIsRAM[i] = Memory.BlockIsRAM[i + 0x800] = FALSE;
		}

		// Mario Kart (HiROM build) carries the board's DSP-1 in the classic
		// HiROM window $00-$1F:6000-$7FFF; other games leave it unmapped.
		for (uint32 blk = 6; blk <= 7; blk++)
		{
			uint32 i = (bank << 4) | blk;
			uint8 *m = (uint8 *) (id == 2 ? CMemory::MAP_DSP : CMemory::MAP_NONE);
			Memory.Map[i] = Memory.Map[i + 0x800] = m;
			Memory.WriteMap[i] = Memory.WriteMap[i + 0x800] = m;
			Memory.BlockIsROM[i] = Memory.BlockIsROM[i + 0x800] = FALSE;
			Memory.BlockIsRAM[i] = Memory.BlockIsRAM[i + 0x800] = FALSE;
		}
	}

	Memory.Map[(0x10 << 4) | 6] = Memory.Map[(0x90 << 4) | 6] = (uint8 *) CMemory::MAP_EVENT;
	Memory.WriteMap[(0x10 << 4) | 6] = Memory.WriteMap[(0x90 << 4) | 6] = (uint8 *) CMemory::MAP_EVENT;

	for (uint32 bank = 0x40; bank <= 0x7d; bank++)
	{
		uint8 *base = img + (((bank - 0x40) << 16) & mask);
		for (uint32 blk = 0; blk <= 15; blk++)
		{
			uint32 i = (bank << 4) | blk;
			Memory.Map[i] = Memory.Map[i + 0x800] = base;
			Memory.WriteMap[i] = Memory.WriteMap[i + 0x800] = (uint8 *) CMemory::MAP_NONE;
			Memory.BlockIsROM[i] = Memory.BlockIsROM[i + 0x800] = TRUE;
			Memory.BlockIsRAM[i] = Memory.BlockIsRAM[i + 0x800] = FALSE;
		}
	}
}

uint8 S9xGetEvent (uint32 Address)
{
	// Campus Challenge '92: status register at $C0:0000 (only MAP_EVENT read).
	if (PF94.board == EVENT_BOARD_CC92)
	{
		if (PF94.timerOn && (IPPU.TotalEmulatedFrames - PF94.timerStart) >= PF94.timerFrames)
		{
			PF94.timerOn = FALSE;
			PF94.status |= 0x02;
		}
		return (PF94.status);
	}

	if ((Address & 0x7f0000) == 0x100000)
	{
		if (PF94.timerOn && (IPPU.TotalEmulatedFrames - PF94.timerStart) >= PF94.timerFrames)
		{
			PF94.timerOn = FALSE;
			PF94.status |= 0x02;
		}
		return (PF94.status);
	}
	return (0);
}

void S9xSetEvent (uint8 Byte, uint32 Address)
{
	// Campus Challenge '92: select register at $E0:0000 (only MAP_EVENT write).
	// 0x09=SMW (starts the session clock), 0x05=F-Zero, 0x03=Pilotwings.
	if (PF94.board == EVENT_BOARD_CC92)
	{
		PF94.select = Byte;
		if (Byte == 0x09 && !PF94.timerOn && !(PF94.status & 0x02))
		{
			PF94.timerOn = TRUE;
			PF94.timerStart = IPPU.TotalEmulatedFrames;
		}
		PF94MapGameWindow();
		return;
	}

	if ((Address & 0x7f0000) == 0x200000)
	{
		PF94.select = Byte;
		if (Byte == 0x09 && !PF94.timerOn && !(PF94.status & 0x02))
		{
			PF94.timerOn = TRUE;
			PF94.timerStart = IPPU.TotalEmulatedFrames;
		}
		PF94MapGameWindow();

		// Selecting a game whose ROM wasn't found leaves the program ROM in
		// the window, so the launch stub's JML would land mid-data. Skip it
		// and reboot the scoring program from its reset vector instead. That
		// returns to the start of the session, so clear the timer (disarm +
		// drop time-over) so the menu's next launch gets a fresh full clock.
		int id = 0;
		if (Byte == 0x09)      id = 1;
		else if (Byte == 0x0c) id = 2;
		else if (Byte == 0x0a) id = 3;
		if (id && !PF94.romSize[id])
		{
			PF94.select = 0;
			PF94.status = 0;
			PF94.timerOn = FALSE;
			PF94MapGameWindow();
			S9xSetPCBase(0x008000);
		}
	}
}

void S9xPF94Reset (void)
{
	if (!PF94.active)
		return;
	PF94.select = 0;
	PF94.status = 0;
	PF94.timerOn = FALSE;
	PF94MapGameWindow();
}

void S9xPF94PostLoadState (void)
{
	if (!PF94.active)
		return;
	// The frame counter isn't in the snapshot; a cross-session load can leave
	// timerStart in the future (instant time-over). Rebase to the live counter.
	if (PF94.timerOn && PF94.timerStart > IPPU.TotalEmulatedFrames)
		PF94.timerStart = IPPU.TotalEmulatedFrames;
	PF94MapGameWindow();
}

int S9xPF94TimeRemaining (void)
{
	if (!PF94.active)
		return (-1);

	int fps = Settings.PAL ? 50 : 60;

	if (PF94.status & 0x02)
		return (0);
	if (!PF94.timerOn)
		return (PF94.timerFrames / fps);

	uint32 elapsed = IPPU.TotalEmulatedFrames - PF94.timerStart;
	if (elapsed >= PF94.timerFrames)
		return (0);
	return ((PF94.timerFrames - elapsed + fps - 1) / fps);
}

// The two event carts keep independent timer settings; these return the loaded
// board's value (PowerFest '94 vs Campus Challenge '92), clamped to valid ranges.
int S9xEventTimerMinutes (void)
{
	int m = (PF94.board == EVENT_BOARD_CC92) ? Settings.CC92TimerMinutes : Settings.PF94TimerMinutes;
	return (m >= 3 && m <= 18) ? m : 6;
}

int S9xEventTimerDisplay (void)
{
	int d = (PF94.board == EVENT_BOARD_CC92) ? Settings.CC92TimerDisplay : Settings.PF94TimerDisplay;
	return (d >= 0 && d <= 2) ? d : 0;
}

static uint32 PF94LoadAuxROM (const std::string &dir, const char *const *names, size_t count, uint8 *dst, uint32 maxSize);

void S9xPF94LoadGames (void)
{
	if (!PF94.active)
		return;

	// Campus Challenge '92 is a single combined image (menu + 3 games already
	// resident); there are no sibling ROMs to load. Just (re)arm the DSP-1 that
	// Pilotwings uses, in the LoROM window the board maps it into.
	if (PF94.board == EVENT_BOARD_CC92)
	{
		Settings.DSP = 1;
		DSP0.boundary = 0xc000;
		DSP0.maptype = M_DSP1_LOROM_S;
		SetDSP = &DSP1SetByte;
		GetDSP = &DSP1GetByte;
		return;
	}

	std::string dir = Memory.ROMFilename;
	size_t slash = dir.find_last_of("/\\");
	dir = (slash == std::string::npos) ? "" : dir.substr(0, slash + 1);

	static const char *ll_names[] =
	{
		"PowerFest 94 - Super Mario Bros. - The Lost Levels (USA).sfc",
		"PowerFest 94 - Super Mario Bros. - The Lost Levels (USA).smc",
		"PowerFest 94 - Super Mario Bros. - The Lost Levels (USA).zip",
		"lost-levels.bin",
	};
	static const char *kart_names[] =
	{
		"PowerFest 94 - Super Mario Kart (USA).sfc",
		"PowerFest 94 - Super Mario Kart (USA).smc",
		"PowerFest 94 - Super Mario Kart (USA).zip",
		"mario-kart.bin",
	};
	static const char *griffey_names[] =
	{
		"PowerFest 94 - Ken Griffey Jr Presents Major League Baseball (USA).sfc",
		"PowerFest 94 - Ken Griffey Jr Presents Major League Baseball (USA).smc",
		"PowerFest 94 - Ken Griffey Jr Presents Major League Baseball (USA).zip",
		"PowerFest 94 - Ken Griffey Jr. Presents Major League Baseball (USA).sfc",
		"PowerFest 94 - Ken Griffey Jr. Presents Major League Baseball (USA).zip",
		"ken-griffey.bin",
	};

	PF94.romOff[1] = 0x400000;
	PF94.romSize[1] = PF94LoadAuxROM(dir, ll_names, sizeof(ll_names) / sizeof(*ll_names), Memory.ROM + 0x400000, 0x100000);
	PF94.romOff[2] = 0x600000;
	PF94.romSize[2] = PF94LoadAuxROM(dir, kart_names, sizeof(kart_names) / sizeof(*kart_names), Memory.ROM + 0x600000, 0x100000);
	PF94.romOff[3] = 0x700000;
	PF94.romSize[3] = PF94LoadAuxROM(dir, griffey_names, sizeof(griffey_names) / sizeof(*griffey_names), Memory.ROM + 0x700000, 0x100000);

	if (PF94.romSize[2])
	{
		Settings.DSP = 1;
		DSP0.boundary = 0x7000;
		DSP0.maptype = M_DSP1_HIROM;
		SetDSP = &DSP1SetByte;
		GetDSP = &DSP1GetByte;
	}

	printf("PowerFest '94 board active: Lost Levels %s, Mario Kart %s, Ken Griffey %s\n",
		PF94.romSize[1] ? "ok" : "missing",
		PF94.romSize[2] ? "ok" : "missing",
		PF94.romSize[3] ? "ok" : "missing");
}

static uint32 PF94LoadAuxROM (const std::string &dir, const char *const *names, size_t count, uint8 *dst, uint32 maxSize)
{
	for (size_t i = 0; i < count; i++)
	{
		std::string path = dir + names[i];

		if (splitpath(path.c_str()).ext_is(".zip"))
		{
		#ifdef UNZIP_SUPPORT
			uint32 size = 0;
			if (LoadZip(path.c_str(), &size, dst, maxSize + 512))
			{
				if ((size % 1024) == 512)
				{
					memmove(dst, dst + 512, size - 512);
					size -= 512;
				}
				if (size >= 0x8000 && size <= maxSize && (size & (size - 1)) == 0)
					return (size);
			}
		#endif
			continue;
		}

		FILE *fp = fopen(path.c_str(), "rb");
		if (!fp)
			continue;

		fseek(fp, 0, SEEK_END);
		long size = ftell(fp);
		fseek(fp, (size % 1024) == 512 ? 512 : 0, SEEK_SET);
		size -= (size % 1024) == 512 ? 512 : 0;

		if (size >= 0x8000 && size <= (long) maxSize && (size & (size - 1)) == 0 &&
			fread(dst, 1, size, fp) == (size_t) size)
		{
			fclose(fp);
			return ((uint32) size);
		}
		fclose(fp);
	}
	return (0);
}

// Street Fighter EX Plus Alpha (unlicensed): a cart chip answers queries the
// game pushes into ROM space at $80:8000-$87FF, read back from $80:8100. We
// return ROM there, so the SPC bank base resolves to $08 instead of $05 and the
// uploader reads a garbage header, overruns APU RAM and hangs the boot.
static void S9xSimulateSFEXProtection (uint8 *rom, uint32 size, uint32 crc32)
{
	// Headerless pirate, so no name to match: identify by dump CRC32.
	if (size != 0x200000 || crc32 != 0xDAD59B9F)
		return;

	// The exact instructions we overwrite, at all three query sites
	if (rom[0x0463] != 0x20 || rom[0x0464] != 0xFC || rom[0x0465] != 0x88 ||
	    rom[0x0477] != 0x20 || rom[0x0478] != 0xFC || rom[0x0479] != 0x88 ||
	    rom[0x05B6] != 0x20 || rom[0x05B7] != 0xFC || rom[0x05B8] != 0x88 ||
	    rom[0x08FC] != 0x08 || rom[0x08FD] != 0xC2 || rom[0x08FE] != 0x30 ||
	    rom[0x00A2] != 0xA5 || rom[0x00A3] != 0x8C ||
	    rom[0x047A] != 0xAD || rom[0x047B] != 0x04 || rom[0x047C] != 0x01 ||
	    rom[0x05B9] != 0xAD || rom[0x05BA] != 0x04 || rom[0x05BB] != 0x01)
		return;

	rom[0x00A2] = 0xA9; rom[0x00A3] = 0x03;		// LDA #$03   -> sound bank $05
	rom[0x00A4] = 0xEA; rom[0x00A5] = 0xEA;
	rom[0x047A] = 0xA9; rom[0x047B] = 0x0A;		// LDA #$000A -> gfx bank / NMI phase base
	rom[0x047C] = 0x00;
	// $85C3 jump table: wrong entries derail into a bad P/PC, leaking
	// 2 bytes of stack per frame until it walks into direct page.
	rom[0x05B9] = 0xA9; rom[0x05BA] = 0x02;		// LDA #$0002 -> round-init $8694
	rom[0x05BB] = 0x00;

	printf("Street Fighter EX Plus Alpha: cart protection simulated\n");
}

// Titles that need invalid VRAM access allowed to render correctly. Keyed on
// ROMCRC32; the effective setting is per-game, so the user's own preference in
// the Hacks dialog is left untouched for every other cart.
static const struct
{
	uint32		crc32;
	const char	*title;
} allow_invalid_vram[] =
{
	// DMAs the ASK Kodansha logo into VRAM with the screen still on: blocked,
	// the title screen is a blank white BG1 and a soft reset comes back
	// garbled. Verified both fixed with the transfer allowed.
	{ 0xB56EC084, "Rin Kaihou Kudan no Igo Taidou (J)" },
};

void CMemory::ApplyROMFixes (void)
{
	Settings.BlockInvalidVRAMAccess = Settings.BlockInvalidVRAMAccessMaster;
	Settings.BlockInvalidVRAMAccessOverride = FALSE;

	// Not gated on DisableGameSpecificHacks: this stands in for cart hardware
	// we don't emulate, without which the game never boots at all.
	S9xSimulateSFEXProtection(ROM, CalculatedSize, ROMCRC32);

	// Astro Hawk (PD): enables NMI before clearing WRAM, so the NMI handler
	// runs on power-on garbage; only boots if WRAM starts zeroed. CRC match —
	// its internal header is blank.
	if (ROMCRC32 == 0x41D59381)
	{
		SNESGameFixes.RAMInitialValue = 0x00;
		printf("Applied zeroed-WRAM hack.\n");
	}

	// Games that upload graphics to VRAM with the screen still on. Blocking
	// those writes is right in general, so it stays on for everything else
	// and the Hacks dialog checkbox keeps whatever the user chose; these
	// titles just turn it off for themselves.
	for (unsigned i = 0; i < sizeof(allow_invalid_vram) / sizeof(allow_invalid_vram[0]); i++)
	{
		if (ROMCRC32 == allow_invalid_vram[i].crc32)
		{
			Settings.BlockInvalidVRAMAccess = FALSE;
			Settings.BlockInvalidVRAMAccessOverride = TRUE;
			printf("Allowing invalid VRAM access for %s.\n", allow_invalid_vram[i].title);
			break;
		}
	}

	PF94.active = FALSE;
	PF94.board  = EVENT_BOARD_PF94;

	if (Settings.DisableGameSpecificHacks)
		return;

	// APU timing hacks
	// 3 breaks the menu (snes9xgit/snes9x#563): the CGRAM upload races the
	// game's own HDMA-disable and the armed CGADD HDMA folds the palette.
	if (match_na("CIRCUIT USA"))
		Timings.APUSpeedup = 2;

	S9xAPUTimingSetSpeedup(Timings.APUSpeedup);

	// Other timing hacks
	// The delay to sync CPU and DMA which Snes9x does not emulate.
	// Some games need really severe delay timing...
	if (match_na("BATTLE GRANDPRIX")) // Battle Grandprix
		Timings.DMACPUSync = 20;
	else if (match_na("KORYU NO MIMI ENG")) // Koryu no Mimi translation by rpgone)
	{
		// An infinite loop reads $4210 and checks NMI flag. This only works if LDA instruction executes before the NMI triggers,
		// which doesn't work very well with s9x's default DMA timing.
		Timings.DMACPUSync = 20;
	}

	if (Timings.DMACPUSync != 18)
		printf("DMA sync: %d\n", Timings.DMACPUSync);

	// SRAM initial value
	if (match_na("HITOMI3"))
	{
		SRAMSize = 1;
		SRAMMask = ((1 << (SRAMSize + 3)) * 128) - 1;
	}

	// SRAM value fixes
	if (match_na("SUPER DRIFT OUT")      || // Super Drift Out
		match_na("SATAN IS OUR FATHER!") ||
		match_na("S.F.S.95 della SerieA") ||
		match_id("AACJ") || // Nichibutsu Arcade Classics
		match_na("goemon 4"))               // Ganbare Goemon Kirakira Douchuu
		SNESGameFixes.SRAMInitialValue = 0x00;

	// Additional game fixes by sanmaiwashi ...
	// XXX: unnecessary?
	if (match_na("SFX \xC5\xB2\xC4\xB6\xDE\xDD\xC0\xDE\xD1\xD3\xC9\xB6\xDE\xC0\xD8 1")) // SD Gundam Gaiden - Knight Gundam Monogatari
		SNESGameFixes.SRAMInitialValue = 0x6b;

	// others: BS and ST-01x games are 0x00.

	// OAM hacks :(
	// OAM hacks because we don't fully understand the behavior of the SNES.
	// Totally wacky display in 2P mode...
	// seems to need a disproven behavior, so we're definitely overlooking some other bug?
	if (match_nn("UNIRACERS")) // Uniracers
	{
		SNESGameFixes.Uniracers = TRUE;
		printf("Applied Uniracers hack.\n");
	}

	// PowerFest '94 - Scoring (the event board's master program ROM), identified
	// by its board work-RAM clear loop (STA/LDA $30:6000,X). Activates the
	// MX15001 board: work RAM at $30-$3F:6000-$7FFF, status register $10:6000,
	// select register $20:6000, session timer, and the game window switched
	// between the sibling game ROMs found next to this one.
	static const uint8 pf94_ramclear[13] = { 0xA9, 0x00, 0x00, 0x9F, 0x00, 0x60, 0x30, 0xBF, 0x00, 0x60, 0x30, 0xD0, 0xF3 };
	if (CalculatedSize == 0x40000 && memcmp(ROM + 0x01F8, pf94_ramclear, 13) == 0)
	{
		SRAMSize = 3;
		SRAMMask = ((1 << (SRAMSize + 3)) * 128) - 1;
		for (uint32 bank = 0x30; bank <= 0x3f; bank++)
		{
			for (uint32 blk = 6; blk <= 7; blk++)
			{
				uint32 i = (bank << 4) | blk;
				Map[i] = Map[i + 0x800] = (uint8 *) MAP_LOROM_SRAM;
				WriteMap[i] = WriteMap[i + 0x800] = (uint8 *) MAP_LOROM_SRAM;
				BlockIsRAM[i] = BlockIsRAM[i + 0x800] = TRUE;
				BlockIsROM[i] = BlockIsROM[i + 0x800] = FALSE;
			}
		}

		Map[(0x10 << 4) | 6] = Map[(0x90 << 4) | 6] = (uint8 *) MAP_EVENT;
		WriteMap[(0x10 << 4) | 6] = WriteMap[(0x90 << 4) | 6] = (uint8 *) MAP_EVENT;
		Map[(0x20 << 4) | 6] = Map[(0xA0 << 4) | 6] = (uint8 *) MAP_EVENT;
		WriteMap[(0x20 << 4) | 6] = WriteMap[(0xA0 << 4) | 6] = (uint8 *) MAP_EVENT;

		PF94.active = TRUE;
		PF94.select = 0;
		PF94.status = 0;
		PF94.timerOn = FALSE;
		int pf94min = (Settings.PF94TimerMinutes >= 3 && Settings.PF94TimerMinutes <= 18) ? Settings.PF94TimerMinutes : 6;
		PF94.timerFrames = pf94min * 60 * (Settings.PAL ? 50 : 60);
		PF94.romOff[0] = 0;
		PF94.romSize[0] = CalculatedSize;

		S9xPF94LoadGames();
	}

	// Nintendo Campus Challenge '92 (SNES-EVENT board) — the canonical combined
	// cart image: 256KB menu (Program) + Super Mario World + F-Zero + Pilotwings
	// (512KB each) = 0x1C0000. Identified by the menu's boot code at offset 0, its
	// EVENT select write (STA $E00000 @ menu 0x6002), and the SMW segment header at
	// 0x47FC0. Activates the EVENT-CC92 map: status $C0:0000, select $E0:0000, LoROM
	// game window $00-$1F, menu forced $80-$9F, DSP-1 $20-$3F/$A0-$BF, board work
	// RAM $70-$7D,$F0-$FF:0000-7FFF. select 0x09=SMW (starts the clock), 0x05=F-Zero,
	// 0x03=Pilotwings.
	static const uint8 cc92_boot[16] = { 0xA9, 0x00, 0x8F, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static const uint8 cc92_reset[16] = { 0x78, 0x18, 0xFB, 0xC2, 0x30, 0xA2, 0xFF, 0x1F, 0x9A, 0xA0, 0x00, 0x00, 0x5A, 0x2B, 0xCA, 0x74 };
	if (memcmp(ROM, cc92_reset, 16) == 0 &&
		memcmp(ROM + 0x6000, cc92_boot, 6) == 0 &&
		memcmp(ROM + 0x47FC0, "SUPER MARIOWORLD", 16) == 0)
	{
		// Board work RAM (LoROM SRAM) at $70-$7D,$F0-$FD:0000-7FFF.
		SRAMSize = 3;
		SRAMMask = ((1 << (SRAMSize + 3)) * 128) - 1;
		for (uint32 bank = 0x70; bank <= 0x7d; bank++)
		{
			for (uint32 blk = 0; blk <= 7; blk++)
			{
				uint32 i = (bank << 4) | blk;
				Map[i] = Map[i + 0x800] = (uint8 *) MAP_LOROM_SRAM;
				WriteMap[i] = WriteMap[i + 0x800] = (uint8 *) MAP_LOROM_SRAM;
				BlockIsRAM[i] = BlockIsRAM[i + 0x800] = TRUE;
				BlockIsROM[i] = BlockIsROM[i + 0x800] = FALSE;
			}
		}

		PF94.active = TRUE;
		PF94.board  = EVENT_BOARD_CC92;
		PF94.select = 0;
		PF94.status = 0;
		PF94.timerOn = FALSE;
		int cc92min = (Settings.CC92TimerMinutes >= 3 && Settings.CC92TimerMinutes <= 18) ? Settings.CC92TimerMinutes : 6;
		PF94.timerFrames = cc92min * 60 * (Settings.PAL ? 50 : 60);
		PF94.romOff[0] = 0x000000; PF94.romSize[0] = 0x40000;   // menu (Program)
		PF94.romOff[1] = 0x040000; PF94.romSize[1] = 0x80000;   // Super Mario World
		PF94.romOff[2] = 0x0C0000; PF94.romSize[2] = 0x80000;   // F-Zero
		PF94.romOff[3] = 0x140000; PF94.romSize[3] = 0x80000;   // Pilotwings

		S9xPF94LoadGames();   // CC92 branch arms the DSP-1
		PF94MapGameWindow();  // CC92 branch builds the window / DSP / registers

		printf("Nintendo Campus Challenge '92 board active (menu + SMW + F-Zero + Pilotwings).\n");
	}

	// PowerFest '94 - Super Mario Bros. - The Lost Levels (event cart sub-ROM),
	// identified by its per-frame MX15001 status poll (LDA $10:6000 / AND #$02 /
	// BEQ +4 / JML) so both the 512KB underdump and a complete dump match.
	// Course clear jumps into the event board's menu/scoring ROM (banks $20+),
	// which re-enters at the next instruction. Skip the jump.
	static const uint8 pf94_poll[9] = { 0xAF, 0x00, 0x60, 0x10, 0x29, 0x02, 0xF0, 0x04, 0x5C };
	if ((CalculatedSize == 0x80000 || CalculatedSize == 0x100000) &&
		memcmp(ROM + 0x8073, pf94_poll, 9) == 0 &&
		ROM[0xB094] == 0x5C && ROM[0xB095] == 0x00 && ROM[0xB096] == 0xE0 && ROM[0xB097] == 0x20)
	{
		ROM[0xB094] = ROM[0xB095] = ROM[0xB096] = ROM[0xB097] = 0xEA;
		printf("Applied PowerFest '94 Lost Levels course-clear patch.\n");

		// If the scoring ROM is next to the game ROM, map it into the board's
		// menu window (banks $20-$3F/$A0-$BF, LoROM layout) like the MX15001
		// does, so menu-window reads return real chip data.
		const char *candidates[] =
		{
			"PowerFest 94 - Scoring (USA).sfc",
			"PowerFest 94 - Scoring (USA).smc",
			"PowerFest '94 - Scoring (USA).sfc",
			"PowerFest 94 - Scoring.sfc",
			"scoring.sfc", "scoring.smc", "scoring.bin",
		};

		std::string dir = ROMFilename;
		size_t slash = dir.find_last_of("/\\");
		dir = (slash == std::string::npos) ? "" : dir.substr(0, slash + 1);

		for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++)
		{
			std::string path = dir + candidates[i];
			FILE *fp = fopen(path.c_str(), "rb");
			if (!fp)
				continue;

			fseek(fp, 0, SEEK_END);
			long size = ftell(fp);
			fseek(fp, (size % 1024) == 512 ? 512 : 0, SEEK_SET);
			size -= (size % 1024) == 512 ? 512 : 0;

			uint8 *menu = ROM + 0x200000;
			if (size >= 0x8000 && size <= 0x80000 && (size & (size - 1)) == 0 &&
				fread(menu, 1, size, fp) == (size_t) size)
			{
				for (uint32 bank = 0x20; bank <= 0x3f; bank++)
				{
					uint8 *base = menu + (((bank - 0x20) << 15) & (size - 1)) - 0x8000;
					for (uint32 blk = 8; blk <= 15; blk++)
					{
						Map[(bank << 4) | blk] = base;
						Map[((bank + 0x80) << 4) | blk] = base;
					}
				}
				printf("Mapped PowerFest '94 scoring ROM (%s) into banks $20-$3F.\n", candidates[i]);
				fclose(fp);
				break;
			}
			fclose(fp);
		}
	}

	// Render Position
	if (match_na("Sugoro Quest++"))
		Timings.RenderPos = 128;
	else if (match_na("FIREPOWER 2000") || match_na("SUPER SWIV"))
		Timings.RenderPos = 32;
	else if (match_na("DERBY STALLION 98"))
		Timings.RenderPos = 128;
	else if (match_na("FULL THROTTLE RACING"))
		Timings.RenderPos = 128;
	// From bsnes
	else if (match_na("NHL '94") || match_na("NHL PROHOCKEY'94"))
		Timings.RenderPos = 32;
	else if (match_na("ADVENTURES OF FRANKEN") && Settings.PAL)
		Timings.RenderPos = 32;

    if (match_na("FX SKIING NINTENDO 96"))
        Timings.GSUCelDelay = 312;
}

std::string CMemory::SafeString(std::string s, bool allow_jis /*=false*/)
{
    std::string safe;
    for (size_t i = 0; i < s.length(); i++)
    {
        if (s[i] >= 32 && s[i] < 127) // ASCII
            safe += s[i];
        else
            if (allow_jis && ROMRegion == 0 && ((uint8)s[i] >= 0xa0 && (uint8)s[i] < 0xe0)) // JIS X 201 - Katakana
                safe += s[i];
            else
                safe += '_';
    }

    return safe;
}

// BPS % UPS % IPS

// number decoding used for both BPS and UPS
static uint32 XPSdecode (const uint8 *data, unsigned &addr, unsigned size)
{
	uint32 offset = 0, shift = 1;
	while(addr < size) {
		uint8 x = data[addr++];
		offset += (x & 0x7f) * shift;
		if(x & 0x80) break;
		shift <<= 7;
		offset += shift;
	}
	return offset;
}

static std::vector<uint8_t> ReadStreamUntilEOF(Stream *r)
{
    const size_t max_buffer_size = 4096;
    std::vector<uint8_t> data;
    uint8_t buffer[max_buffer_size];
    size_t total_size = 0;
    size_t buffer_size = 0;

    int value = 0;
    while (value != EOF)
    {
        value = r->get_char();
        if (value != EOF)
            buffer[buffer_size++] = value;

        if (buffer_size == max_buffer_size || (value == EOF && buffer_size > 0))
        {
            data.resize(data.size() + buffer_size);
            memcpy(&data[total_size], buffer, buffer_size);
            total_size += buffer_size;
            buffer_size = 0;
        }
    }

    return data;
}

//NOTE: UPS patches are *never* created against a headered ROM!
//this is per the UPS file specification. however, do note that it is
//technically possible for a non-compliant patcher to ignore this requirement.
//therefore, it is *imperative* that no emulator support such patches.
//thusly, we ignore the "long offset" parameter below. failure to do so would
//completely invalidate the purpose of UPS; which is to avoid header vs
//no-header patching errors that result in IPS patches having a 50/50 chance of
//being applied correctly.

static bool8 ReadUPSPatch (Stream *r, long, int32 &rom_size)
{
	//Reader lacks size() and rewind(), so we need to read in the file to get its size
	auto data_vector = ReadStreamUntilEOF(r);
	uint8 *data = &data_vector[0];
	uint32 size = data_vector.size();

	//4-byte header + 1-byte input size + 1-byte output size + 4-byte patch CRC32 + 4-byte unpatched CRC32 + 4-byte patched CRC32
	if(size < 18) return false;  //patch is too small

	uint32 addr = 4;
	if (memcmp(data, "UPS1", 4) != 0) return false; //patch has an invalid header

	uint32 patch_crc32 = caCRC32(data, size - 4);  //don't include patch CRC32 itself in CRC32 calculation
	uint32 rom_crc32 = caCRC32(Memory.ROM, rom_size);
	uint32 px_crc32 = (data[size - 12] << 0) + (data[size - 11] << 8) + (data[size - 10] << 16) + (data[size -  9] << 24);
	uint32 py_crc32 = (data[size -  8] << 0) + (data[size -  7] << 8) + (data[size -  6] << 16) + (data[size -  5] << 24);
	uint32 pp_crc32 = (data[size -  4] << 0) + (data[size -  3] << 8) + (data[size -  2] << 16) + (data[size -  1] << 24);
	if(patch_crc32 != pp_crc32) { return false; }  //patch is corrupted
	if(!Settings.IgnorePatchChecksum && (rom_crc32 != px_crc32) && (rom_crc32 != py_crc32)) return false; //patch is for a different ROM

	uint32 px_size = XPSdecode(data, addr, size);
	uint32 py_size = XPSdecode(data, addr, size);
	uint32 out_size = ((uint32) rom_size == px_size) ? py_size : px_size;
	if(out_size > CMemory::MAX_ROM_SIZE) { return false; }  //applying this patch will overflow Memory.ROM buffer

	//fill expanded area with 0x00s; so that XORing works as expected below.
	//note that this is needed (and works) whether output ROM is larger or smaller than pre-patched ROM
	for(unsigned i = min((uint32) rom_size, out_size); i < max((uint32) rom_size, out_size); i++) {
		Memory.ROM[i] = 0x00;
	}

	uint32 relative = 0;
	while(addr < size - 12) {
		relative += XPSdecode(data, addr, size);
		while(addr < size - 12 && relative < CMemory::MAX_ROM_SIZE) {
			uint8 x = data[addr++];
			Memory.ROM[relative++] ^= x;
			if(!x) break;
		}
	}

	rom_size = out_size;

	uint32 out_crc32 = caCRC32(Memory.ROM, rom_size);
	if(Settings.IgnorePatchChecksum
	|| ((rom_crc32 == px_crc32) && (out_crc32 == py_crc32))
	|| ((rom_crc32 == py_crc32) && (out_crc32 == px_crc32))
	) {
		Settings.IsPatched = 3;
		return true;
	} else {
		//technically, reaching here means that patching has failed.
		//we should return false, but unfortunately Memory.ROM has already
		//been modified above and cannot be undone. to do this properly, we
		//would need to make a copy of Memory.ROM, apply the patch, and then
		//copy that back to Memory.ROM.
		//
		//however, the only way for this case to happen is if the UPS patch file
		//itself is corrupted, which should be detected by the patch CRC32 check
		//above anyway. errors due to the wrong ROM or patch file being used are
		//already caught above.
		fprintf(stderr, "WARNING: UPS patching appears to have failed.\nGame may not be playable.\n");
		return true;
	}
}

// header notes for UPS patches also apply to BPS
//
// logic taken from http://byuu.org/programming/bps and the accompanying source
//
static bool8 ReadBPSPatch (Stream *r, long, int32 &rom_size)
{
	auto data_vector = ReadStreamUntilEOF(r);
	uint8 *data = &data_vector[0];
	uint32 size = data_vector.size();

	/* 4-byte header + 1-byte input size + 1-byte output size + 1-byte metadata size
	   + 4-byte unpatched CRC32 + 4-byte patched CRC32 + 4-byte patch CRC32 */
	if(size < 19) return false; //patch is too small

	uint32 addr = 4;
	if (memcmp(data, "BPS1", 4) != 0) return false; //patch has an invalid header

	uint32 patch_crc32 = caCRC32(data, size - 4);  //don't include patch CRC32 itself in CRC32 calculation
	uint32 rom_crc32 = caCRC32(Memory.ROM, rom_size);
	uint32 source_crc32 = (data[size - 12] << 0) + (data[size - 11] << 8) + (data[size - 10] << 16) + (data[size -  9] << 24);
	uint32 target_crc32 = (data[size -  8] << 0) + (data[size -  7] << 8) + (data[size -  6] << 16) + (data[size -  5] << 24);
	uint32 pp_crc32 = (data[size -  4] << 0) + (data[size -  3] << 8) + (data[size -  2] << 16) + (data[size -  1] << 24);
	if(patch_crc32 != pp_crc32) return false;  //patch is corrupted
	if(!Settings.IgnorePatchChecksum && rom_crc32 != source_crc32) return false;  //patch is for a different ROM

	XPSdecode(data, addr, size);
	uint32 target_size = XPSdecode(data, addr, size);
	uint32 metadata_size = XPSdecode(data, addr, size);
	addr += metadata_size;

	if(target_size > CMemory::MAX_ROM_SIZE) return false;  //applying this patch will overflow Memory.ROM buffer

	enum { SourceRead, TargetRead, SourceCopy, TargetCopy };
	uint32 outputOffset = 0, sourceRelativeOffset = 0, targetRelativeOffset = 0;

	std::vector<uint8_t> patched_rom_vector(target_size);
	uint8 *patched_rom = &patched_rom_vector[0];
	memset(patched_rom, 0, target_size);

	while(addr < size - 12) {
		uint32 length = XPSdecode(data, addr, size);
		uint32 mode = length & 3;
		length = (length >> 2) + 1;

		switch((int)mode) {
			case SourceRead:
				while(length--) {
					patched_rom[outputOffset] = Memory.ROM[outputOffset];
					outputOffset++;
				}
				break;
			case TargetRead:
				while(length--) patched_rom[outputOffset++] = data[addr++];
				break;
			case SourceCopy:
			case TargetCopy:
				int32 offset = XPSdecode(data, addr, size);
				bool negative = offset & 1;
				offset >>= 1;
				if(negative) offset = -offset;

				if(mode == SourceCopy) {
					sourceRelativeOffset += offset;
					while(length--) patched_rom[outputOffset++] = Memory.ROM[sourceRelativeOffset++];
				} else {
					targetRelativeOffset += offset;
					while(length--) patched_rom[outputOffset++] = patched_rom[targetRelativeOffset++];
				}
				break;
		}
	}

	uint32 out_crc32 = caCRC32(patched_rom, target_size);
	if(Settings.IgnorePatchChecksum || out_crc32 == target_crc32) {
		memcpy(Memory.ROM, patched_rom, target_size);
		rom_size = target_size;
		Settings.IsPatched = 2;
		return true;
	} else {
		fprintf(stderr, "WARNING: BPS patching failed.\nROM has not been altered.\n");
		return false;
	}
}

static long ReadInt (Stream *r, unsigned nbytes)
{
	long	v = 0;

	while (nbytes--)
	{
		int	c = r->get_char();
		if (c == EOF)
			return (-1);
		v = (v << 8) | (c & 0xFF);
	}

	return (v);
}

static bool8 ReadIPSPatch (Stream *r, long offset, int32 &rom_size)
{
	const int32	IPS_EOF = 0x00454F46l;
	int32		ofs;
	char		fname[6];

	fname[5] = 0;
	for (int i = 0; i < 5; i++)
	{
		int	c = r->get_char();
		if (c == EOF)
			return (0);
		fname[i] = (char) c;
	}

	if (strncmp(fname, "PATCH", 5))
		return (0);

	for (;;)
	{
		long	len, rlen;
		int		rchar;

		ofs = ReadInt(r, 3);
		if (ofs == -1)
			return (0);

		if (ofs == IPS_EOF)
			break;

		ofs -= offset;

		len = ReadInt(r, 2);
		if (len == -1)
			return (0);

		if (len)
		{
			if (ofs + len > CMemory::MAX_ROM_SIZE)
				return (0);

			while (len--)
			{
				rchar = r->get_char();
				if (rchar == EOF)
					return (0);
				Memory.ROM[ofs++] = (uint8) rchar;
			}

			if (ofs > rom_size)
				rom_size = ofs;
		}
		else
		{
			rlen = ReadInt(r, 2);
			if (rlen == -1)
				return (0);

			rchar = r->get_char();
			if (rchar == EOF)
				return (0);

			if (ofs + rlen > CMemory::MAX_ROM_SIZE)
				return (0);

			while (rlen--)
				Memory.ROM[ofs++] = (uint8) rchar;

			if (ofs > rom_size)
				rom_size = ofs;
		}
	}

	ofs = ReadInt(r, 3);
	if (ofs != -1 && ofs - offset < rom_size)
		rom_size = ofs - offset;

	Settings.IsPatched = 1;
	return (1);
}

#ifdef UNZIP_SUPPORT
static int unzFindExtension (unzFile &file, const char *ext, bool restart, bool print, bool allowExact)
{
	unz_file_info	info;
	int				port, l = strlen(ext), e = allowExact ? 0 : 1;

	if (restart)
		port = unzGoToFirstFile(file);
	else
		port = unzGoToNextFile(file);

	while (port == UNZ_OK)
	{
		int		len;
		char	name[132];

		unzGetCurrentFileInfo(file, &info, name, 128, NULL, 0, NULL, 0);
		len = strlen(name);

		if (len >= l + e && name[len - l - 1] == '.' && strcasecmp(name + len - l, ext) == 0 && unzOpenCurrentFile(file) == UNZ_OK)
		{
			if (print)
				printf("Using patch %s", name);

			return (port);
		}

		port = unzGoToNextFile(file);
	}

	return (port);
}
#endif

void CMemory::CheckForAnyPatch(const char *rom_filename, bool8 header, int32 &rom_size)
{
    Settings.IsPatched = false;

    if (Settings.NoPatch)
        return;

    FSTREAM patch_file = NULL;
    long offset = header ? 512 : 0;
    int ret;
    bool flag = false;

    auto path = splitpath(rom_filename);

    auto try_patch = [&](const char *type, std::string filename, bool8(*read_patch_func)(Stream * r, long offset, int32 &rom_size)) -> bool {
        if ((patch_file = OPEN_FSTREAM(filename.c_str(), "rb")) != NULL)
        {
            printf("Using %s patch %s", type, filename.c_str());

            Stream *s = new fStream(patch_file);
            ret = read_patch_func(s, offset, rom_size);
            s->closeStream();

            if (ret)
            {
                printf("!\n");
                flag = true;
                return true;
            }
            else
                printf(" failed!\n");
        }
        return false;
    };

    auto try_ips_sequence = [&](const char *pattern, enum s9x_getdirtype dirtype) -> bool {
        for (int i = 0; i < 1000; i++)
        {
            char ips[9];
            snprintf(ips, 9, pattern, i);
            if (!try_patch("IPS", S9xGetFilename(ips, dirtype), ReadIPSPatch))
                break;
        }
        return flag;
    };

    auto try_patch_type_sequence = [&](enum s9x_getdirtype dirtype) -> bool {
        if (try_patch("BPS", S9xGetFilename(".bps", dirtype), ReadBPSPatch))
            return true;
        if (try_patch("UPS", S9xGetFilename(".ups", dirtype), ReadUPSPatch))
            return true;
        if (try_patch("IPS", S9xGetFilename(".ips", dirtype), ReadIPSPatch))
            return true;
        if (try_ips_sequence(".%03d.ips", dirtype))
            return true;
        if (try_ips_sequence(".ips%d", dirtype))
            return true;
        if (try_ips_sequence(".ip%d", dirtype))
            return true;

        return false;
    };

    if (try_patch_type_sequence(ROMFILENAME_DIR))
        return;

#ifdef UNZIP_SUPPORT
    if (path.ext_is(".zip"))
    {
        unzFile file = unzOpen(rom_filename);
        if (file)
        {
            auto try_zip_patch = [&](const char *ext, bool8 (*read_patch_func)(Stream * r, long offset, int32 &rom_size)) -> bool {
                if (unzFindExtension(file, ext) == UNZ_OK)
                {
                    printf(" in %s", rom_filename);

                    Stream *s = new unzStream(file);
                    ret = read_patch_func(s, offset, rom_size);
                    delete s;

                    if (ret)
                    {
                        printf("!\n");
                        flag = true;
                        return true;
                    }

                    printf(" failed!\n");
                }
                return false;
            };

            auto try_zip_ips_sequence = [&](const char *pattern) {
                for (int i = 0; i < 1000; i++)
                {
                    char ips[8];
                    snprintf(ips, 8, pattern, i);
                    if (!try_zip_patch(ips, ReadIPSPatch))
                        break;
                }
            };

            if (!flag)
                try_zip_patch("bps", ReadBPSPatch);
            if (!flag)
                try_zip_patch("ups", ReadUPSPatch);
            if (!flag)
                try_zip_patch("ips", ReadIPSPatch);
            if (!flag)
                try_zip_ips_sequence("%03d.ips");
            if (!flag)
                try_zip_ips_sequence("ips%d");
            if (!flag)
                try_zip_ips_sequence("ip%d");

            int close_ret = unzClose(file);
            assert(close_ret == UNZ_OK);

            if (flag)
                return;
        }
    }

    // Mercurial Magic (MSU-1 distribution pack)
    if (path.ext_is(".msu1")) // ROM was *NOT* loaded from a .msu1 pack
    {
        Stream *s = S9xMSU1OpenFile("patch.bps", TRUE);
        if (s)
        {
            printf("Using BPS patch from msu1");
            ret = ReadBPSPatch(s, offset, rom_size);
            s->closeStream();

            if (ret)
                printf("!\n");
            else
                printf(" failed!\n");
        }
    }
#endif

    if (try_patch_type_sequence(PATCH_DIR))
        return;
}

// ---------------------------------------------------------------------------
// Super Famicom Box: rebuild the SNES-visible map from the supervisor's
// mapping registers ([C0h]/[C1h] on the KROM bus). Runs at board power-on
// and on every register write — the KROM remaps live, SNES running or not
// (real hardware does the same; the SNES executes garbage until the KROM
// pulses its reset line).

void S9xSFCBoxRemap (void)
{
	if (!Settings.SFCBox)
		return;

	uint8	r0 = SFCBox.MapReg0, r1 = SFCBox.MapReg1;
	int		slot = (r0 >> 2) & 1;
	int		socket = r0 & 3;
	int		mapmode = r1 & 3;	// 0=reserved, 1=GSU, 2=LoROM, 3=HiROM

	uint32	off = SFCBox.RomOffset[slot][socket];
	uint32	size = SFCBox.RomSize[slot][socket];

	// C1's map-mode field is what the GROM helper programs; C0 bit7 covers
	// the reset state before the helper ever ran.
	bool8	hirom = (mapmode == 3) || (mapmode == 0 && (r0 & 0x80));

	bool8	gsu = FALSE;
	if (mapmode == 1)
	{
		if (size && SFCBoxStageGSU(off, size))
			gsu = TRUE;
		else
			printf("SFC-Box: GSU socket empty (or staging failed); mapping plain LoROM.\n");
	}

	if (!size)
	{
		// Unpopulated socket: point the window at the menu ROM so fetches
		// see something coherent instead of open bus.
		off = SFCBox.RomOffset[0][0];
		size = SFCBox.RomSize[0][0];
		hirom = FALSE;
		if (!size)
			return;
	}

	Memory.Map_Initialize();
	Memory.map_System();

	if (gsu)
	{
		// GSU-1 board layout (cf. Map_SuperFXLoROMMap), pointed at the
		// staged view. The 32K work RAM is the cart's own chip (IC21 on
		// the PSS-61) — kept beyond the 128K shared save SRAM so Star Fox
		// can't scribble over other games' saves.
		uint8	*gsuram = Memory.SRAM + 0x20000;
		uint32	mask = size - 1;

		for (uint32 bank = 0; bank < 0x40; bank++)
		{
			uint8	*base = SFCBoxFXRom + ((bank << 15) & mask) - 0x8000;
			for (uint32 blk = 8; blk <= 15; blk++)
			{
				uint32	p = (bank << 4) | blk;
				Memory.Map[p] = Memory.Map[p + 0x800] = base;
				Memory.BlockIsROM[p] = Memory.BlockIsROM[p + 0x800] = TRUE;
				Memory.BlockIsRAM[p] = Memory.BlockIsRAM[p + 0x800] = FALSE;
			}
		}

		for (uint32 bank = 0x40; bank <= 0x5f; bank++)
		{
			uint8	*base = SFCBoxFXRom + (((bank - 0x40) << 16) & mask);
			for (uint32 blk = 0; blk <= 15; blk++)
			{
				uint32	p = (bank << 4) | blk;
				Memory.Map[p] = Memory.Map[p + 0x800] = base;
				Memory.BlockIsROM[p] = Memory.BlockIsROM[p + 0x800] = TRUE;
				Memory.BlockIsRAM[p] = Memory.BlockIsRAM[p + 0x800] = FALSE;
			}
		}

		Memory.map_space(0x00, 0x3f, 0x6000, 0x7fff, gsuram - 0x6000);
		Memory.map_space(0x80, 0xbf, 0x6000, 0x7fff, gsuram - 0x6000);
		Memory.map_space(0x70, 0x70, 0x0000, 0xffff, gsuram);
		Memory.map_space(0x71, 0x71, 0x0000, 0xffff, gsuram + 0x10000);
		Memory.map_space(0xf0, 0xf0, 0x0000, 0xffff, gsuram);
		Memory.map_space(0xf1, 0xf1, 0x0000, 0xffff, gsuram + 0x10000);

		SuperFX.pvRom = SFCBoxFXRom;
		SuperFX.nRomBanks = size >> 15;
		SuperFX.pvRam = gsuram;
		SuperFX.nRamBanks = 1;
		if (!Settings.SuperFX)
			S9xInitSuperFX();
		Settings.SuperFX = TRUE;
		S9xResetSuperFX();		// rebuilds the GSU bank tables from pvRom
	}
	else if (Settings.SuperFX)
	{
		// Leaving the GSU socket: disarm and restore the Init() defaults.
		Settings.SuperFX = FALSE;
		CPU.IRQExternal = FALSE;
		SuperFX.pvRom = Memory.ROM;
		SuperFX.nRomBanks = (2 * 1024 * 1024) / (32 * 1024);
		SuperFX.pvRam = Memory.SRAM;
		SuperFX.nRamBanks = 2;
	}

	if (gsu)
	{
		// ROM/RAM windows are all placed; skip the LoROM/HiROM/DSP/SRAM
		// branches below (the KROM never combines them with GSU mode).
	}
	else if (hirom)
	{
		Memory.map_hirom_offset(0x00, 0x3f, 0x8000, 0xffff, size, off);
		Memory.map_hirom_offset(0x40, 0x7d, 0x0000, 0xffff, size, off);
		Memory.map_hirom_offset(0x80, 0xbf, 0x8000, 0xffff, size, off);
		Memory.map_hirom_offset(0xc0, 0xff, 0x0000, 0xffff, size, off);
	}
	else
	{
		Memory.map_lorom_offset(0x00, 0x3f, 0x8000, 0xffff, size, off);
		Memory.map_lorom_offset(0x40, 0x7f, 0x0000, 0xffff, size, off);
		Memory.map_lorom_offset(0x80, 0xbf, 0x8000, 0xffff, size, off);
		Memory.map_lorom_offset(0xc0, 0xff, 0x0000, 0xffff, size, off);
	}

	// DSP-1 window (Mario Kart: HiROM layout on real carts). The KROM's
	// socket probe can leave the DSP bit set alongside GSU mode — the
	// window would clobber the GSU program banks, so gate it out.
	if (!gsu && (r0 & 0x20) && Settings.DSP)
	{
		if (hirom)
		{
			DSP0.boundary = 0x7000;
			DSP0.maptype = M_DSP1_HIROM;
		}
		else
		{
			DSP0.boundary = 0xc000;
			DSP0.maptype = M_DSP1_LOROM_S;
		}
		Memory.map_DSP();
	}

	// Shared 128K SRAM: [C1h] picks a 32K-aligned base and a 2K/8K/32K
	// window size; the MAP_SFCBOX_SRAM handler applies both. In GSU mode
	// banks 70/71 belong to the cart's own work RAM instead.
	if (!gsu && (r0 & 0x08))
	{
		static const uint32	window[4] = { 0x800, 0x2000, 0x2000, 0x8000 };

		SFCBox.SRAMWindowMask = window[(r1 >> 6) & 3] - 1;
		SFCBox.SRAMWindowBase = (uint32) ((r1 >> 2) & 3) * 0x8000;
		SFCBox.SRAMHiROM = hirom;

		if (hirom)
		{
			Memory.map_index(0x20, 0x3f, 0x6000, 0x7fff, CMemory::MAP_SFCBOX_SRAM, CMemory::MAP_TYPE_RAM);
			Memory.map_index(0xa0, 0xbf, 0x6000, 0x7fff, CMemory::MAP_SFCBOX_SRAM, CMemory::MAP_TYPE_RAM);
		}
		else
		{
			Memory.map_index(0x70, 0x7d, 0x0000, 0x7fff, CMemory::MAP_SFCBOX_SRAM, CMemory::MAP_TYPE_RAM);
			Memory.map_index(0xf0, 0xff, 0x0000, 0x7fff, CMemory::MAP_SFCBOX_SRAM, CMemory::MAP_TYPE_RAM);
		}
	}

	Memory.map_WRAM();
	Memory.map_WriteProtectROM();
}

// Street Fighter Alpha 2 and Star Ocean are the only two S-DD1 cartridges. Both
// have circulating conversions whose graphics were decompressed ahead of time so
// that the chip is no longer needed, which is what lets them run from flash
// cartridges. The decompressed data does not fit the original address space, so
// the conversions grow the image and address it in two halves: the upper half of
// each bank sits where LoROM would put it, and the lower half sits one whole
// image further into the file. Banks $C0 and above are a window composed from the
// lower halves of two other banks.
//
// No real S-DD1 cartridge is larger than Star Ocean's 48 Mbit, so an S-DD1 image
// at or above 64 Mbit is one of these conversions.

void CMemory::Map_SDD1DecompressedMap (void)
{
	const int	banks = (int) (CalculatedSize >> 16);

	map_System();

	for (int bank = 0; bank < 256; bank++)
	{
		if (bank == 0x7e || bank == 0x7f)
			continue;

		uint8	*low, *high;

		if (bank >= 0xc0)
		{
			const int	offset = bank - 0xc0;

			if (0x80 + offset >= banks || offset >= banks)
				continue;

			low  = ROM + (size_t) (0x80 + offset + banks) * 0x8000;
			high = ROM + (size_t) (offset + banks) * 0x8000 - 0x8000;
		}
		else
		{
			if (bank >= banks)
				continue;

			low  = ROM + (size_t) (bank + banks) * 0x8000;
			high = ROM + (size_t) bank * 0x8000 - 0x8000;
		}

		for (int block = 0; block < 16; block++)
		{
			const bool8	upper_half_only = (bank < 0x40) || (bank >= 0x80 && bank < 0xc0);

			if (block < 8 && upper_half_only)
				continue;

			const int	slot = (bank << 4) | block;

			Map[slot] = (block < 8) ? low : high;
			BlockIsROM[slot] = TRUE;
			BlockIsRAM[slot] = FALSE;
		}
	}

	map_WRAM();
	map_WriteProtectROM();
}
