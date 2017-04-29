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

#ifndef CGMINI_H
#define CGMINI_H

#include <d3d9.h>
#include <gl/GL.h>

#define CGENTRY __cdecl
#define CGD3D9ENTRY __cdecl
#define CGGLENTRY __cdecl
#define CGD3D9DLL_API __declspec(dllimport)
#define CGGL_API

#define CG_API
typedef struct _CGprogram *CGprogram;
typedef struct _CGcontext *CGcontext;
typedef struct _CGparameter *CGparameter;
typedef enum
{
    CG_NO_ERROR = 0,
    CG_COMPILER_ERROR = 1,
    CG_INVALID_PARAMETER_ERROR = 2,
    CG_INVALID_PROFILE_ERROR = 3,
    CG_PROGRAM_LOAD_ERROR = 4,
    CG_PROGRAM_BIND_ERROR = 5,
    CG_PROGRAM_NOT_LOADED_ERROR = 6,
    CG_UNSUPPORTED_GL_EXTENSION_ERROR = 7,
    CG_INVALID_VALUE_TYPE_ERROR = 8,
    CG_NOT_MATRIX_PARAM_ERROR = 9,
    CG_INVALID_ENUMERANT_ERROR = 10,
    CG_NOT_4x4_MATRIX_ERROR = 11,
    CG_FILE_READ_ERROR = 12,
    CG_FILE_WRITE_ERROR = 13,
    CG_NVPARSE_ERROR = 14,
    CG_MEMORY_ALLOC_ERROR = 15,
    CG_INVALID_CONTEXT_HANDLE_ERROR = 16,
    CG_INVALID_PROGRAM_HANDLE_ERROR = 17,
    CG_INVALID_PARAM_HANDLE_ERROR = 18,
    CG_UNKNOWN_PROFILE_ERROR = 19,
    CG_VAR_ARG_ERROR = 20,
    CG_INVALID_DIMENSION_ERROR = 21,
    CG_ARRAY_PARAM_ERROR = 22,
    CG_OUT_OF_ARRAY_BOUNDS_ERROR = 23,
    CG_CONFLICTING_TYPES_ERROR = 24,
    CG_CONFLICTING_PARAMETER_TYPES_ERROR = 25,
    CG_PARAMETER_IS_NOT_SHARED_ERROR = 26,
    CG_INVALID_PARAMETER_VARIABILITY_ERROR = 27,
    CG_CANNOT_DESTROY_PARAMETER_ERROR = 28,
    CG_NOT_ROOT_PARAMETER_ERROR = 29,
    CG_PARAMETERS_DO_NOT_MATCH_ERROR = 30,
    CG_IS_NOT_PROGRAM_PARAMETER_ERROR = 31,
    CG_INVALID_PARAMETER_TYPE_ERROR = 32,
    CG_PARAMETER_IS_NOT_RESIZABLE_ARRAY_ERROR = 33,
    CG_INVALID_SIZE_ERROR = 34,
    CG_BIND_CREATES_CYCLE_ERROR = 35,
    CG_ARRAY_TYPES_DO_NOT_MATCH_ERROR = 36,
    CG_ARRAY_DIMENSIONS_DO_NOT_MATCH_ERROR = 37,
    CG_ARRAY_HAS_WRONG_DIMENSION_ERROR = 38,
    CG_TYPE_IS_NOT_DEFINED_IN_PROGRAM_ERROR = 39,
    CG_INVALID_EFFECT_HANDLE_ERROR = 40,
    CG_INVALID_STATE_HANDLE_ERROR = 41,
    CG_INVALID_STATE_ASSIGNMENT_HANDLE_ERROR = 42,
    CG_INVALID_PASS_HANDLE_ERROR = 43,
    CG_INVALID_ANNOTATION_HANDLE_ERROR = 44,
    CG_INVALID_TECHNIQUE_HANDLE_ERROR = 45,
    CG_INVALID_PARAMETER_HANDLE_ERROR = 46,
    CG_STATE_ASSIGNMENT_TYPE_MISMATCH_ERROR = 47,
    CG_INVALID_FUNCTION_HANDLE_ERROR = 48,
    CG_INVALID_TECHNIQUE_ERROR = 49,
    CG_INVALID_POINTER_ERROR = 50,
    CG_NOT_ENOUGH_DATA_ERROR = 51,
    CG_NON_NUMERIC_PARAMETER_ERROR = 52,
    CG_ARRAY_SIZE_MISMATCH_ERROR = 53,
    CG_CANNOT_SET_NON_UNIFORM_PARAMETER_ERROR = 54,
    CG_DUPLICATE_NAME_ERROR = 55,
    CG_INVALID_OBJ_HANDLE_ERROR = 56,
    CG_INVALID_BUFFER_HANDLE_ERROR = 57,
    CG_BUFFER_INDEX_OUT_OF_RANGE_ERROR = 58,
    CG_BUFFER_ALREADY_MAPPED_ERROR = 59,
    CG_BUFFER_UPDATE_NOT_ALLOWED_ERROR = 60,
    CG_GLSLG_UNCOMBINED_LOAD_ERROR = 61
} CGerror;
typedef enum
{
    CG_UNKNOWN = 4096,
    CG_IN = 4097,
    CG_OUT = 4098,
    CG_INOUT = 4099,
    CG_MIXED = 4100,
    CG_VARYING = 4101,
    CG_UNIFORM = 4102,
    CG_CONSTANT = 4103,
    CG_PROGRAM_SOURCE = 4104, /* cgGetProgramString                       */
    CG_PROGRAM_ENTRY = 4105, /* cgGetProgramString                       */
    CG_COMPILED_PROGRAM = 4106, /* cgGetProgramString                       */
    CG_PROGRAM_PROFILE = 4107, /* cgGetProgramString                       */
    CG_GLOBAL = 4108,
    CG_PROGRAM = 4109,
    CG_DEFAULT = 4110,
    CG_ERROR = 4111,
    CG_SOURCE = 4112,
    CG_OBJECT = 4113,
    CG_COMPILE_MANUAL = 4114,
    CG_COMPILE_IMMEDIATE = 4115,
    CG_COMPILE_LAZY = 4116,
    CG_CURRENT = 4117,
    CG_LITERAL = 4118,
    CG_VERSION = 4119, /* cgGetString                              */
    CG_ROW_MAJOR = 4120,
    CG_COLUMN_MAJOR = 4121,
    CG_FRAGMENT = 4122, /* cgGetProgramInput and cgGetProgramOutput */
    CG_VERTEX = 4123, /* cgGetProgramInput and cgGetProgramOutput */
    CG_POINT = 4124, /* Geometry program cgGetProgramInput       */
    CG_LINE = 4125, /* Geometry program cgGetProgramInput       */
    CG_LINE_ADJ = 4126, /* Geometry program cgGetProgramInput       */
    CG_TRIANGLE = 4127, /* Geometry program cgGetProgramInput       */
    CG_TRIANGLE_ADJ = 4128, /* Geometry program cgGetProgramInput       */
    CG_POINT_OUT = 4129, /* Geometry program cgGetProgramOutput      */
    CG_LINE_OUT = 4130, /* Geometry program cgGetProgramOutput      */
    CG_TRIANGLE_OUT = 4131, /* Geometry program cgGetProgramOutput      */
    CG_IMMEDIATE_PARAMETER_SETTING = 4132,
    CG_DEFERRED_PARAMETER_SETTING = 4133,
    CG_NO_LOCKS_POLICY = 4134,
    CG_THREAD_SAFE_POLICY = 4135,
    CG_FORCE_UPPER_CASE_POLICY = 4136,
    CG_UNCHANGED_CASE_POLICY = 4137,
    CG_IS_OPENGL_PROFILE = 4138,
    CG_IS_DIRECT3D_PROFILE = 4139,
    CG_IS_DIRECT3D_8_PROFILE = 4140,
    CG_IS_DIRECT3D_9_PROFILE = 4141,
    CG_IS_DIRECT3D_10_PROFILE = 4142,
    CG_IS_VERTEX_PROFILE = 4143,
    CG_IS_FRAGMENT_PROFILE = 4144,
    CG_IS_GEOMETRY_PROFILE = 4145,
    CG_IS_TRANSLATION_PROFILE = 4146,
    CG_IS_HLSL_PROFILE = 4147,
    CG_IS_GLSL_PROFILE = 4148,
    CG_IS_TESSELLATION_CONTROL_PROFILE = 4149,
    CG_IS_TESSELLATION_EVALUATION_PROFILE = 4150,
    CG_PATCH = 4152, /* cgGetProgramInput and cgGetProgramOutput */
    CG_IS_DIRECT3D_11_PROFILE = 4153
} CGenum;
typedef enum {} CGprofile;
typedef enum {} CGresource;
typedef enum
{
    CG_UNKNOWN_TYPE = 0,
    CG_ARRAY = 2,
    CG_STRING = 1135,
    CG_STRUCT = 1,
    CG_TYPELESS_STRUCT = 3,
    CG_TEXTURE = 1137,
    CG_BUFFER = 1319,
    CG_UNIFORMBUFFER = 1320,
    CG_ADDRESS = 1321,
    CG_PIXELSHADER_TYPE = 1142,
    CG_PROGRAM_TYPE = 1136,
    CG_VERTEXSHADER_TYPE = 1141,
    CG_SAMPLER = 1143,
    CG_SAMPLER1D = 1065,
    CG_SAMPLER1DARRAY = 1138,
    CG_SAMPLER1DSHADOW = 1313,
    CG_SAMPLER2D = 1066,
    CG_SAMPLER2DARRAY = 1139,
    CG_SAMPLER2DMS = 1317, /* ARB_texture_multisample */
    CG_SAMPLER2DMSARRAY = 1318, /* ARB_texture_multisample */
    CG_SAMPLER2DSHADOW = 1314,
    CG_SAMPLER3D = 1067,
    CG_SAMPLERBUF = 1144,
    CG_SAMPLERCUBE = 1069,
    CG_SAMPLERCUBEARRAY = 1140,
    CG_SAMPLERRBUF = 1316, /* NV_explicit_multisample */
    CG_SAMPLERRECT = 1068,
    CG_SAMPLERRECTSHADOW = 1315,
    CG_TYPE_START_ENUM = 1024,
    CG_BOOL = 1114,
    CG_BOOL1 = 1115,
    CG_BOOL2 = 1116,
    CG_BOOL3 = 1117,
    CG_BOOL4 = 1118,
    CG_BOOL1x1 = 1119,
    CG_BOOL1x2 = 1120,
    CG_BOOL1x3 = 1121,
    CG_BOOL1x4 = 1122,
    CG_BOOL2x1 = 1123,
    CG_BOOL2x2 = 1124,
    CG_BOOL2x3 = 1125,
    CG_BOOL2x4 = 1126,
    CG_BOOL3x1 = 1127,
    CG_BOOL3x2 = 1128,
    CG_BOOL3x3 = 1129,
    CG_BOOL3x4 = 1130,
    CG_BOOL4x1 = 1131,
    CG_BOOL4x2 = 1132,
    CG_BOOL4x3 = 1133,
    CG_BOOL4x4 = 1134,
    CG_CHAR = 1166,
    CG_CHAR1 = 1167,
    CG_CHAR2 = 1168,
    CG_CHAR3 = 1169,
    CG_CHAR4 = 1170,
    CG_CHAR1x1 = 1171,
    CG_CHAR1x2 = 1172,
    CG_CHAR1x3 = 1173,
    CG_CHAR1x4 = 1174,
    CG_CHAR2x1 = 1175,
    CG_CHAR2x2 = 1176,
    CG_CHAR2x3 = 1177,
    CG_CHAR2x4 = 1178,
    CG_CHAR3x1 = 1179,
    CG_CHAR3x2 = 1180,
    CG_CHAR3x3 = 1181,
    CG_CHAR3x4 = 1182,
    CG_CHAR4x1 = 1183,
    CG_CHAR4x2 = 1184,
    CG_CHAR4x3 = 1185,
    CG_CHAR4x4 = 1186,
    CG_DOUBLE = 1145,
    CG_DOUBLE1 = 1146,
    CG_DOUBLE2 = 1147,
    CG_DOUBLE3 = 1148,
    CG_DOUBLE4 = 1149,
    CG_DOUBLE1x1 = 1150,
    CG_DOUBLE1x2 = 1151,
    CG_DOUBLE1x3 = 1152,
    CG_DOUBLE1x4 = 1153,
    CG_DOUBLE2x1 = 1154,
    CG_DOUBLE2x2 = 1155,
    CG_DOUBLE2x3 = 1156,
    CG_DOUBLE2x4 = 1157,
    CG_DOUBLE3x1 = 1158,
    CG_DOUBLE3x2 = 1159,
    CG_DOUBLE3x3 = 1160,
    CG_DOUBLE3x4 = 1161,
    CG_DOUBLE4x1 = 1162,
    CG_DOUBLE4x2 = 1163,
    CG_DOUBLE4x3 = 1164,
    CG_DOUBLE4x4 = 1165,
    CG_FIXED = 1070,
    CG_FIXED1 = 1092,
    CG_FIXED2 = 1071,
    CG_FIXED3 = 1072,
    CG_FIXED4 = 1073,
    CG_FIXED1x1 = 1074,
    CG_FIXED1x2 = 1075,
    CG_FIXED1x3 = 1076,
    CG_FIXED1x4 = 1077,
    CG_FIXED2x1 = 1078,
    CG_FIXED2x2 = 1079,
    CG_FIXED2x3 = 1080,
    CG_FIXED2x4 = 1081,
    CG_FIXED3x1 = 1082,
    CG_FIXED3x2 = 1083,
    CG_FIXED3x3 = 1084,
    CG_FIXED3x4 = 1085,
    CG_FIXED4x1 = 1086,
    CG_FIXED4x2 = 1087,
    CG_FIXED4x3 = 1088,
    CG_FIXED4x4 = 1089,
    CG_FLOAT = 1045,
    CG_FLOAT1 = 1091,
    CG_FLOAT2 = 1046,
    CG_FLOAT3 = 1047,
    CG_FLOAT4 = 1048,
    CG_FLOAT1x1 = 1049,
    CG_FLOAT1x2 = 1050,
    CG_FLOAT1x3 = 1051,
    CG_FLOAT1x4 = 1052,
    CG_FLOAT2x1 = 1053,
    CG_FLOAT2x2 = 1054,
    CG_FLOAT2x3 = 1055,
    CG_FLOAT2x4 = 1056,
    CG_FLOAT3x1 = 1057,
    CG_FLOAT3x2 = 1058,
    CG_FLOAT3x3 = 1059,
    CG_FLOAT3x4 = 1060,
    CG_FLOAT4x1 = 1061,
    CG_FLOAT4x2 = 1062,
    CG_FLOAT4x3 = 1063,
    CG_FLOAT4x4 = 1064,
    CG_HALF = 1025,
    CG_HALF1 = 1090,
    CG_HALF2 = 1026,
    CG_HALF3 = 1027,
    CG_HALF4 = 1028,
    CG_HALF1x1 = 1029,
    CG_HALF1x2 = 1030,
    CG_HALF1x3 = 1031,
    CG_HALF1x4 = 1032,
    CG_HALF2x1 = 1033,
    CG_HALF2x2 = 1034,
    CG_HALF2x3 = 1035,
    CG_HALF2x4 = 1036,
    CG_HALF3x1 = 1037,
    CG_HALF3x2 = 1038,
    CG_HALF3x3 = 1039,
    CG_HALF3x4 = 1040,
    CG_HALF4x1 = 1041,
    CG_HALF4x2 = 1042,
    CG_HALF4x3 = 1043,
    CG_HALF4x4 = 1044,
    CG_INT = 1093,
    CG_INT1 = 1094,
    CG_INT2 = 1095,
    CG_INT3 = 1096,
    CG_INT4 = 1097,
    CG_INT1x1 = 1098,
    CG_INT1x2 = 1099,
    CG_INT1x3 = 1100,
    CG_INT1x4 = 1101,
    CG_INT2x1 = 1102,
    CG_INT2x2 = 1103,
    CG_INT2x3 = 1104,
    CG_INT2x4 = 1105,
    CG_INT3x1 = 1106,
    CG_INT3x2 = 1107,
    CG_INT3x3 = 1108,
    CG_INT3x4 = 1109,
    CG_INT4x1 = 1110,
    CG_INT4x2 = 1111,
    CG_INT4x3 = 1112,
    CG_INT4x4 = 1113,
    CG_LONG = 1271,
    CG_LONG1 = 1272,
    CG_LONG2 = 1273,
    CG_LONG3 = 1274,
    CG_LONG4 = 1275,
    CG_LONG1x1 = 1276,
    CG_LONG1x2 = 1277,
    CG_LONG1x3 = 1278,
    CG_LONG1x4 = 1279,
    CG_LONG2x1 = 1280,
    CG_LONG2x2 = 1281,
    CG_LONG2x3 = 1282,
    CG_LONG2x4 = 1283,
    CG_LONG3x1 = 1284,
    CG_LONG3x2 = 1285,
    CG_LONG3x3 = 1286,
    CG_LONG3x4 = 1287,
    CG_LONG4x1 = 1288,
    CG_LONG4x2 = 1289,
    CG_LONG4x3 = 1290,
    CG_LONG4x4 = 1291,
    CG_SHORT = 1208,
    CG_SHORT1 = 1209,
    CG_SHORT2 = 1210,
    CG_SHORT3 = 1211,
    CG_SHORT4 = 1212,
    CG_SHORT1x1 = 1213,
    CG_SHORT1x2 = 1214,
    CG_SHORT1x3 = 1215,
    CG_SHORT1x4 = 1216,
    CG_SHORT2x1 = 1217,
    CG_SHORT2x2 = 1218,
    CG_SHORT2x3 = 1219,
    CG_SHORT2x4 = 1220,
    CG_SHORT3x1 = 1221,
    CG_SHORT3x2 = 1222,
    CG_SHORT3x3 = 1223,
    CG_SHORT3x4 = 1224,
    CG_SHORT4x1 = 1225,
    CG_SHORT4x2 = 1226,
    CG_SHORT4x3 = 1227,
    CG_SHORT4x4 = 1228,
    CG_UCHAR = 1187,
    CG_UCHAR1 = 1188,
    CG_UCHAR2 = 1189,
    CG_UCHAR3 = 1190,
    CG_UCHAR4 = 1191,
    CG_UCHAR1x1 = 1192,
    CG_UCHAR1x2 = 1193,
    CG_UCHAR1x3 = 1194,
    CG_UCHAR1x4 = 1195,
    CG_UCHAR2x1 = 1196,
    CG_UCHAR2x2 = 1197,
    CG_UCHAR2x3 = 1198,
    CG_UCHAR2x4 = 1199,
    CG_UCHAR3x1 = 1200,
    CG_UCHAR3x2 = 1201,
    CG_UCHAR3x3 = 1202,
    CG_UCHAR3x4 = 1203,
    CG_UCHAR4x1 = 1204,
    CG_UCHAR4x2 = 1205,
    CG_UCHAR4x3 = 1206,
    CG_UCHAR4x4 = 1207,
    CG_UINT = 1250,
    CG_UINT1 = 1251,
    CG_UINT2 = 1252,
    CG_UINT3 = 1253,
    CG_UINT4 = 1254,
    CG_UINT1x1 = 1255,
    CG_UINT1x2 = 1256,
    CG_UINT1x3 = 1257,
    CG_UINT1x4 = 1258,
    CG_UINT2x1 = 1259,
    CG_UINT2x2 = 1260,
    CG_UINT2x3 = 1261,
    CG_UINT2x4 = 1262,
    CG_UINT3x1 = 1263,
    CG_UINT3x2 = 1264,
    CG_UINT3x3 = 1265,
    CG_UINT3x4 = 1266,
    CG_UINT4x1 = 1267,
    CG_UINT4x2 = 1268,
    CG_UINT4x3 = 1269,
    CG_UINT4x4 = 1270,
    CG_ULONG = 1292,
    CG_ULONG1 = 1293,
    CG_ULONG2 = 1294,
    CG_ULONG3 = 1295,
    CG_ULONG4 = 1296,
    CG_ULONG1x1 = 1297,
    CG_ULONG1x2 = 1298,
    CG_ULONG1x3 = 1299,
    CG_ULONG1x4 = 1300,
    CG_ULONG2x1 = 1301,
    CG_ULONG2x2 = 1302,
    CG_ULONG2x3 = 1303,
    CG_ULONG2x4 = 1304,
    CG_ULONG3x1 = 1305,
    CG_ULONG3x2 = 1306,
    CG_ULONG3x3 = 1307,
    CG_ULONG3x4 = 1308,
    CG_ULONG4x1 = 1309,
    CG_ULONG4x2 = 1310,
    CG_ULONG4x3 = 1311,
    CG_ULONG4x4 = 1312,
    CG_USHORT = 1229,
    CG_USHORT1 = 1230,
    CG_USHORT2 = 1231,
    CG_USHORT3 = 1232,
    CG_USHORT4 = 1233,
    CG_USHORT1x1 = 1234,
    CG_USHORT1x2 = 1235,
    CG_USHORT1x3 = 1236,
    CG_USHORT1x4 = 1237,
    CG_USHORT2x1 = 1238,
    CG_USHORT2x2 = 1239,
    CG_USHORT2x3 = 1240,
    CG_USHORT2x4 = 1241,
    CG_USHORT3x1 = 1242,
    CG_USHORT3x2 = 1243,
    CG_USHORT3x3 = 1244,
    CG_USHORT3x4 = 1245,
    CG_USHORT4x1 = 1246,
    CG_USHORT4x2 = 1247,
    CG_USHORT4x3 = 1248,
    CG_USHORT4x4 = 1249
} CGtype;
typedef int CGbool;
typedef enum
{
    CG_GL_MATRIX_IDENTITY = 0,
    CG_GL_MATRIX_TRANSPOSE = 1,
    CG_GL_MATRIX_INVERSE = 2,
    CG_GL_MATRIX_INVERSE_TRANSPOSE = 3,
    CG_GL_MODELVIEW_MATRIX = 4,
    CG_GL_PROJECTION_MATRIX = 5,
    CG_GL_TEXTURE_MATRIX = 6,
    CG_GL_MODELVIEW_PROJECTION_MATRIX = 7,
    CG_GL_VERTEX = 8,
    CG_GL_FRAGMENT = 9,
    CG_GL_GEOMETRY = 10,
    CG_GL_TESSELLATION_CONTROL = 11,
    CG_GL_TESSELLATION_EVALUATION = 12
} CGGLenum;

#endif