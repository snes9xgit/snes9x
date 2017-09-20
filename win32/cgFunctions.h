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



#ifndef CGFUNCTIONS_H
#define CGFUNCTIONS_H

/* uncomment this if you have the cg toolkit installed and want its headers to be used
   http://developer.nvidia.com/object/cg_download.html
*/
//#define HAVE_CG_INCLUDES

#ifdef HAVE_CG_INCLUDES

#define CG_EXPLICIT
#define CGD3D9_EXPLICIT
#define CGGL_EXPLICIT

#include <cg/cg.h>
#include <cg/cgd3d9.h>
#include <cg/cggl.h>

#else

#include "cgMini.h"

#endif

//cg.dll
typedef CG_API CGcontext (CGENTRY *CGCC)(void);
extern CGCC cgCreateContext;
typedef CG_API void (CGENTRY *CGDC)(CGcontext context);
extern CGDC cgDestroyContext;
typedef CG_API CGparameter (CGENTRY *CGGNP)(CGprogram program, const char *name);
extern CGGNP cgGetNamedParameter;
typedef CG_API CGerror (CGENTRY *CGGE)(void);
extern CGGE cgGetError;
typedef CG_API const char * (CGENTRY *CGGES)(CGerror error);
extern CGGES cgGetErrorString;
typedef CG_API const char * (CGENTRY *CGGLL)(CGcontext context);
extern CGGLL cgGetLastListing;
typedef CG_API CGprogram (CGENTRY *CGCP)(CGcontext context, CGenum program_type, const char *program, CGprofile profile, const char *entry, const char **args);
extern CGCP cgCreateProgram;
typedef CG_API void (CGENTRY *CGDP)(CGprogram program);
extern CGDP cgDestroyProgram;
typedef CG_API unsigned long (CGENTRY *CGGPRI)(CGparameter param);
extern CGGPRI cgGetParameterResourceIndex;
typedef CG_API CGparameter (CGENTRY *CGGFP)(CGprogram program, CGenum name_space);
extern CGGFP cgGetFirstParameter;
typedef CG_API CGparameter (CGENTRY *CGGNEP)(CGparameter current);
extern CGGNEP cgGetNextParameter;
typedef CG_API CGenum (CGENTRY *CGGPD)(CGparameter param);
extern CGGPD cgGetParameterDirection;
typedef CG_API const char * (CGENTRY *CGGPS)(CGparameter param);
extern CGGPS cgGetParameterSemantic;
typedef CG_API const char * (CGENTRY *CGGRS)(CGresource resource);
extern CGGRS cgGetResourceString;
typedef CG_API CGenum (CGENTRY *CGGPV)(CGparameter param);
extern CGGPV cgGetParameterVariability;
typedef CG_API CGtype (CGENTRY *CGGPT)(CGparameter param);
extern CGGPT cgGetParameterType;
typedef CG_API CGparameter (CGENTRY *CGGFSP)(CGparameter param);
extern CGGFSP cgGetFirstStructParameter;
typedef CG_API const char * (CGENTRY *CGGPN)(CGparameter param);
extern CGGPN cgGetParameterName;

//cgD3D9.dll
typedef CGD3D9DLL_API HRESULT (CGD3D9ENTRY *CGD3DSD)(IDirect3DDevice9 *pDevice);
extern CGD3DSD cgD3D9SetDevice;
typedef CGD3D9DLL_API HRESULT (CGD3D9ENTRY *CGD3DBP)(CGprogram prog);
extern CGD3DBP cgD3D9BindProgram;
typedef CGD3D9DLL_API CGprofile (CGD3D9ENTRY *CGD3DGLVP)(void);
extern CGD3DGLVP cgD3D9GetLatestVertexProfile;
typedef CGD3D9DLL_API CGprofile (CGD3D9ENTRY *CGD3DGLPP)(void);
extern CGD3DGLPP cgD3D9GetLatestPixelProfile;
typedef CGD3D9DLL_API const char ** (CGD3D9ENTRY *CGD3DGOO)(CGprofile profile);
extern CGD3DGOO cgD3D9GetOptimalOptions;
typedef CGD3D9DLL_API HRESULT (CGD3D9ENTRY *CGD3DLP)(CGprogram prog, CGbool paramShadowing, DWORD assemFlags);
extern CGD3DLP cgD3D9LoadProgram;
typedef CGD3D9DLL_API HRESULT (CGD3D9ENTRY *CGD3DSUM)(CGparameter param, const D3DMATRIX *matrix);
extern CGD3DSUM cgD3D9SetUniformMatrix;
typedef CGD3D9DLL_API HRESULT (CGD3D9ENTRY *CGD3DSU)(CGparameter param, const void *floats);
extern CGD3DSU cgD3D9SetUniform;
typedef CGD3D9DLL_API HRESULT (CGD3D9ENTRY *CGD3DST)(CGparameter param, IDirect3DBaseTexture9 *tex);
extern CGD3DST cgD3D9SetTexture;
typedef CGD3D9DLL_API CGbool (CGD3D9ENTRY *CGD3DGVD)(CGprogram prog, D3DVERTEXELEMENT9 decl[MAXD3DDECLLENGTH]);
extern CGD3DGVD cgD3D9GetVertexDeclaration;
typedef CGD3D9DLL_API HRESULT (CGD3D9ENTRY *CGD3DSSS)(CGparameter param, D3DSAMPLERSTATETYPE type, DWORD value);
extern CGD3DSSS cgD3D9SetSamplerState;

//cggl.dll
typedef CGGL_API void (CGGLENTRY *CGGLSSMP)(CGparameter param, CGGLenum matrix, CGGLenum transform);
extern CGGLSSMP cgGLSetStateMatrixParameter;
typedef CGGL_API void (CGGLENTRY *CGGLSP1F)(CGparameter param, float x);
extern CGGLSP1F cgGLSetParameter1f;
typedef CGGL_API void (CGGLENTRY *CGGLSP2FV)(CGparameter param, const float *v);
extern CGGLSP2FV cgGLSetParameter2fv;
typedef CGGL_API CGprofile (CGGLENTRY *CGGLGLP)(CGGLenum profile_type);
extern CGGLGLP cgGLGetLatestProfile;
typedef CGGL_API void (CGGLENTRY *CGGLEP)(CGprofile profile);
extern CGGLEP cgGLEnableProfile;
typedef CGGL_API void (CGGLENTRY *CGGLDP)(CGprofile profile);
extern CGGLDP cgGLDisableProfile;
typedef CGGL_API void (CGGLENTRY *CGGLSOO)(CGprofile profile);
extern CGGLSOO cgGLSetOptimalOptions;
typedef CGGL_API void (CGGLENTRY *CGGLLP)(CGprogram program);
extern CGGLLP cgGLLoadProgram;
typedef CGGL_API void (CGGLENTRY *CGGLBP)(CGprogram program);
extern CGGLBP cgGLBindProgram;
typedef CGGL_API void (CGGLENTRY *CGGLSTP)(CGparameter param, GLuint texobj);
extern CGGLSTP cgGLSetTextureParameter;
typedef CGGL_API void (CGGLENTRY *CGGLETP)(CGparameter param);
extern CGGLETP cgGLEnableTextureParameter;
typedef CGGL_API void (CGGLENTRY *CGGLSPP)(CGparameter param, GLint fsize, GLenum type, GLsizei stride, const GLvoid *pointer);
extern CGGLSPP cgGLSetParameterPointer;
typedef CGGL_API void (CGGLENTRY *CGGLECS)(CGparameter param);
extern CGGLECS cgGLEnableClientState;
typedef CGGL_API void (CGGLENTRY *CGGLDCS)(CGparameter param);
extern CGGLDCS cgGLDisableClientState;

//cgfunctions.cpp
bool loadCgFunctions();
void unloadCgLibrary();

#endif		//CGFUNCTIONS_H
