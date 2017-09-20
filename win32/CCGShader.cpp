/***********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2010  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),

  (c) Copyright 2002 - 2011  zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja

  (c) Copyright 2009 - 2016  BearOso,
                             OV2

  (c) Copyright 2011 - 2016  Hans-Kristian Arntzen,
                             Daniel De Matteis
                             (Under no circumstances will commercial rights be given)


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com),
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti

  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code used in 1.39-1.51
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  SPC7110 and RTC C++ emulator code used in 1.52+
  (c) Copyright 2009         byuu,
                             neviksti

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001 - 2006  byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound emulator code used in 1.5-1.51
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  Sound emulator code used in 1.52+
  (c) Copyright 2004 - 2007  Shay Green (gblargg@gmail.com)

  S-SMP emulator code used in 1.54+
  (c) Copyright 2016         byuu

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  NTSC filter
  (c) Copyright 2006 - 2007  Shay Green

  GTK+ GUI code
  (c) Copyright 2004 - 2016  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2016  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2011  zones

  Libretro port
  (c) Copyright 2011 - 2016  Hans-Kristian Arntzen,
                             Daniel De Matteis
                             (Under no circumstances will commercial rights be given)


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com/

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
 ***********************************************************************************/
#include "CCGShader.h"
#include "../conffile.h"

CCGShader::CCGShader(void)
{

}

CCGShader::~CCGShader(void)
{
}

cgScaleType CCGShader::scaleStringToEnum(const char *scale)
{
	if(!strcasecmp(scale,"source")) {
		return CG_SCALE_SOURCE;
	} else if(!strcasecmp(scale,"viewport")) {
		return CG_SCALE_VIEWPORT;
	} else if(!strcasecmp(scale,"absolute")) {
		return CG_SCALE_ABSOLUTE;
	} else {
		return CG_SCALE_NONE;
	}
}

bool CCGShader::LoadShader(const char *path)
{
	ConfigFile conf;
	int shaderCount;
	char keyName[100];

	shaderPasses.clear();
	lookupTextures.clear();

	if(strlen(path)<4 || strcasecmp(&path[strlen(path)-4],".cgp")) {
		shaderPass pass;
		pass.scaleParams.scaleTypeX = CG_SCALE_NONE;
		pass.scaleParams.scaleTypeY = CG_SCALE_NONE;
		pass.linearFilter = false;
		pass.filterSet = false;
		strcpy(pass.cgShaderFile,path);
		shaderPasses.push_back(pass);
		return true;
	} else {
		conf.LoadFile(path);
	}	

	shaderCount = conf.GetInt("::shaders",0);

	if(shaderCount<1)
		return false;

	for(int i=0;i<shaderCount;i++) {
		shaderPass pass;
		sprintf(keyName,"::filter_linear%u",i);
		pass.linearFilter = conf.GetBool(keyName);
		pass.filterSet = conf.Exists(keyName);

		sprintf(keyName,"::scale_type%u",i);
		const char *scaleType = conf.GetString(keyName,"");
		if(!strcasecmp(scaleType,"")) {
			sprintf(keyName,"::scale_type_x%u",i);
			const char *scaleTypeX = conf.GetString(keyName,"");
			if(*scaleTypeX=='\0' && (i!=(shaderCount-1)))
				scaleTypeX = "source";
			pass.scaleParams.scaleTypeX = scaleStringToEnum(scaleTypeX);
			sprintf(keyName,"::scale_type_y%u",i);
			const char *scaleTypeY = conf.GetString(keyName,"");
			if(*scaleTypeY=='\0' && (i!=(shaderCount-1)))
				scaleTypeY = "source";
			pass.scaleParams.scaleTypeY = scaleStringToEnum(scaleTypeY);
		} else {
			cgScaleType sType = scaleStringToEnum(scaleType);
			pass.scaleParams.scaleTypeX = sType;
			pass.scaleParams.scaleTypeY = sType;
		}

		sprintf(keyName,"::scale%u",i);
		const char *scaleFloat = conf.GetString(keyName,"");
		int scaleInt = conf.GetInt(keyName,0);
		if(!strcasecmp(scaleFloat,"")) {
			sprintf(keyName,"::scale_x%u",i);
			const char *scaleFloatX = conf.GetString(keyName,"1.0");
			pass.scaleParams.scaleX = atof(scaleFloatX);
			pass.scaleParams.absX = conf.GetInt(keyName,1);
			sprintf(keyName,"::scale_y%u",i);
			const char *scaleFloatY = conf.GetString(keyName,"1.0");
			pass.scaleParams.scaleY = atof(scaleFloatY);
			pass.scaleParams.absY = conf.GetInt(keyName,1);

		} else {
			float floatval = atof(scaleFloat);
			pass.scaleParams.scaleX = floatval;
			pass.scaleParams.absX = scaleInt;
			pass.scaleParams.scaleY = floatval;
			pass.scaleParams.absY = scaleInt;
		}

		sprintf(keyName,"::shader%u",i);
		strcpy(pass.cgShaderFile,conf.GetString(keyName,""));

        sprintf(keyName,"::frame_count_mod%u",i);
        pass.frameCounterMod = conf.GetInt(keyName,0);

        sprintf(keyName,"::float_framebuffer%u",i);
        pass.floatFbo = conf.GetBool(keyName);

		shaderPasses.push_back(pass);
	}

	char *shaderIds = conf.GetStringDup("::textures","");

	char *id = strtok(shaderIds,";");
	while(id!=NULL) {
		lookupTexture tex;
		sprintf(keyName,"::%s",id);
		strcpy(tex.id,id);
		strcpy(tex.texturePath,conf.GetString(keyName,""));
		sprintf(keyName,"::%s_linear",id);
		tex.linearfilter = conf.GetBool(keyName,true);
		lookupTextures.push_back(tex);
		id = strtok(NULL,";");
	}

	free(shaderIds);

	return true;
}
