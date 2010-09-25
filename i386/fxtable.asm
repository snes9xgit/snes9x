; Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
;
; Super FX assembler emulator code
; (c) Copyright 1998, 1999 zsKnight and _Demo_.
;
; Permission to use, copy, modify and distribute Snes9x in both binary and
; source form, for non-commercial purposes, is hereby granted without fee,
; providing that this license information and copyright notice appear with
; all copies and any derived work.
;
; This software is provided 'as-is', without any express or implied
; warranty. In no event shall the authors be held liable for any damages
; arising from the use of this software.
;
; Snes9x is freeware for PERSONAL USE only. Commercial users should
; seek permission of the copyright holders first. Commercial use includes
; charging money for Snes9x or software derived from Snes9x.
;
; The copyright holders request that bug fixes and improvements to the code
; should be forwarded to them so everyone can benefit from the modifications
; in future versions.
;
; Super NES and Super Nintendo Entertainment System are trademarks of
; Nintendo Co., Limited and its subsidiary companies.
;
%include "macros.mac"
EXTSYM FxTable,FxTableA1,FxTableA2,FxTableA3,FxTableb,FxTablebA1,FxTablebA2
EXTSYM FxTablebA3,FxTablec,FxTablecA1,FxTablecA2,FxTablecA3,FxTabled
EXTSYM FxTabledA1,FxTabledA2,FxTabledA3,SfxMemTable,romdata,sfxramdata
EXTSYM sfxplottable

EXTSYM FxOp00,FxOp01,FxOp02,FxOp03,FxOp04,FxOp05,FxOp06,FxOp07,FxOp08
EXTSYM FxOp09,FxOp0A,FxOp0B,FxOp0C,FxOp0D,FxOp0E,FxOp0F,FxOp10,FxOp11
EXTSYM FxOp12,FxOp13,FxOp14,FxOp15,FxOp16,FxOp17,FxOp18,FxOp19,FxOp1A
EXTSYM FxOp1B,FxOp1C,FxOp1D,FxOp1E,FxOp1F,FxOp20,FxOp21,FxOp22,FxOp23
EXTSYM FxOp24,FxOp25,FxOp26,FxOp27,FxOp28,FxOp29,FxOp2A,FxOp2B,FxOp2C
EXTSYM FxOp2D,FxOp2E,FxOp2F,FxOp30,FxOp30A1,FxOp31,FxOp31A1,FxOp32
EXTSYM FxOp32A1,FxOp33,FxOp33A1,FxOp34,FxOp34A1,FxOp35,FxOp35A1,FxOp36
EXTSYM FxOp36A1,FxOp37,FxOp37A1,FxOp38,FxOp38A1,FxOp39,FxOp39A1,FxOp3A
EXTSYM FxOp3AA1,FxOp3B,FxOp3BA1,FxOp3C,FxOp3D,FxOp3E,FxOp3F,FxOp40
EXTSYM FxOp40A1,FxOp41,FxOp41A1,FxOp42,FxOp42A1,FxOp43,FxOp43A1,FxOp44
EXTSYM FxOp44A1,FxOp45,FxOp45A1,FxOp46,FxOp46A1,FxOp47,FxOp47A1,FxOp48
EXTSYM FxOp48A1,FxOp49,FxOp49A1,FxOp4A,FxOp4AA1,FxOp4B,FxOp4BA1,FxOp4C
EXTSYM FxOp4CA1,FxOp4D,FxOp4E,FxOp4EA1,FxOp4F,FxOp50,FxOp50A1,FxOp50A2
EXTSYM FxOp50A3,FxOp51,FxOp51A1,FxOp51A2,FxOp51A3,FxOp52,FxOp52A1,FxOp52A2
EXTSYM FxOp52A3,FxOp53,FxOp53A1,FxOp53A2,FxOp53A3,FxOp54,FxOp54A1,FxOp54A2
EXTSYM FxOp54A3,FxOp55,FxOp55A1,FxOp55A2,FxOp55A3,FxOp56,FxOp56A1,FxOp56A2
EXTSYM FxOp56A3,FxOp57,FxOp57A1,FxOp57A2,FxOp57A3,FxOp58,FxOp58A1,FxOp58A2
EXTSYM FxOp58A3,FxOp59,FxOp59A1,FxOp59A2,FxOp59A3,FxOp5A,FxOp5AA1,FxOp5AA2
EXTSYM FxOp5AA3,FxOp5B,FxOp5BA1,FxOp5BA2,FxOp5BA3,FxOp5C,FxOp5CA1,FxOp5CA2
EXTSYM FxOp5CA3,FxOp5D,FxOp5DA1,FxOp5DA2,FxOp5DA3,FxOp5E,FxOp5EA1,FxOp5EA2
EXTSYM FxOp5EA3,FxOp5F,FxOp5FA1,FxOp5FA2,FxOp5FA3,FxOp60,FxOp60A1,FxOp60A2
EXTSYM FxOp60A3,FxOp61,FxOp61A1,FxOp61A2,FxOp61A3,FxOp62,FxOp62A1,FxOp62A2
EXTSYM FxOp62A3,FxOp63,FxOp63A1,FxOp63A2,FxOp63A3,FxOp64,FxOp64A1,FxOp64A2
EXTSYM FxOp64A3,FxOp65,FxOp65A1,FxOp65A2,FxOp65A3,FxOp66,FxOp66A1,FxOp66A2
EXTSYM FxOp66A3,FxOp67,FxOp67A1,FxOp67A2,FxOp67A3,FxOp68,FxOp68A1,FxOp68A2
EXTSYM FxOp68A3,FxOp69,FxOp69A1,FxOp69A2,FxOp69A3,FxOp6A,FxOp6AA1,FxOp6AA2
EXTSYM FxOp6AA3,FxOp6B,FxOp6BA1,FxOp6BA2,FxOp6BA3,FxOp6C,FxOp6CA1,FxOp6CA2
EXTSYM FxOp6CA3,FxOp6D,FxOp6DA1,FxOp6DA2,FxOp6DA3,FxOp6E,FxOp6EA1,FxOp6EA2
EXTSYM FxOp6EA3,FxOp6F,FxOp6FA1,FxOp6FA2,FxOp6FA3,FxOp70,FxOp71,FxOp71A1
EXTSYM FxOp71A2,FxOp71A3,FxOp72,FxOp72A1,FxOp72A2,FxOp72A3,FxOp73,FxOp73A1
EXTSYM FxOp73A2,FxOp73A3,FxOp74,FxOp74A1,FxOp74A2,FxOp74A3,FxOp75,FxOp75A1
EXTSYM FxOp75A2,FxOp75A3,FxOp76,FxOp76A1,FxOp76A2,FxOp76A3,FxOp77,FxOp77A1
EXTSYM FxOp77A2,FxOp77A3,FxOp78,FxOp78A1,FxOp78A2,FxOp78A3,FxOp79,FxOp79A1
EXTSYM FxOp79A2,FxOp79A3,FxOp7A,FxOp7AA1,FxOp7AA2,FxOp7AA3,FxOp7B,FxOp7BA1
EXTSYM FxOp7BA2,FxOp7BA3,FxOp7C,FxOp7CA1,FxOp7CA2,FxOp7CA3,FxOp7D,FxOp7DA1
EXTSYM FxOp7DA2,FxOp7DA3,FxOp7E,FxOp7EA1,FxOp7EA2,FxOp7EA3,FxOp7F,FxOp7FA1
EXTSYM FxOp7FA2,FxOp7FA3,FxOp80,FxOp80A1,FxOp80A2,FxOp80A3,FxOp81,FxOp81A1
EXTSYM FxOp81A2,FxOp81A3,FxOp82,FxOp82A1,FxOp82A2,FxOp82A3,FxOp83,FxOp83A1
EXTSYM FxOp83A2,FxOp83A3,FxOp84,FxOp84A1,FxOp84A2,FxOp84A3,FxOp85,FxOp85A1
EXTSYM FxOp85A2,FxOp85A3,FxOp86,FxOp86A1,FxOp86A2,FxOp86A3,FxOp87,FxOp87A1
EXTSYM FxOp87A2,FxOp87A3,FxOp88,FxOp88A1,FxOp88A2,FxOp88A3,FxOp89,FxOp89A1
EXTSYM FxOp89A2,FxOp89A3,FxOp8A,FxOp8AA1,FxOp8AA2,FxOp8AA3,FxOp8B,FxOp8BA1
EXTSYM FxOp8BA2,FxOp8BA3,FxOp8C,FxOp8CA1,FxOp8CA2,FxOp8CA3,FxOp8D,FxOp8DA1
EXTSYM FxOp8DA2,FxOp8DA3,FxOp8E,FxOp8EA1,FxOp8EA2,FxOp8EA3,FxOp8F,FxOp8FA1
EXTSYM FxOp8FA2,FxOp8FA3,FxOp90,FxOp91,FxOp92,FxOp93,FxOp94,FxOp95,FxOp96
EXTSYM FxOp96A1,FxOp97,FxOp98,FxOp98A1,FxOp99,FxOp99A1,FxOp9A,FxOp9AA1,FxOp9B
EXTSYM FxOp9BA1,FxOp9C,FxOp9CA1,FxOp9D,FxOp9DA1,FxOp9E,FxOp9F,FxOp9FA1,FxOpA0
EXTSYM FxOpA0A1,FxOpA0A2,FxOpA1,FxOpA1A1,FxOpA1A2,FxOpA2,FxOpA2A1,FxOpA2A2
EXTSYM FxOpA3,FxOpA3A1,FxOpA3A2,FxOpA4,FxOpA4A1,FxOpA4A2,FxOpA5,FxOpA5A1
EXTSYM FxOpA5A2,FxOpA6,FxOpA6A1,FxOpA6A2,FxOpA7,FxOpA7A1,FxOpA7A2,FxOpA8
EXTSYM FxOpA8A1,FxOpA8A2,FxOpA9,FxOpA9A1,FxOpA9A2,FxOpAA,FxOpAAA1,FxOpAAA2
EXTSYM FxOpAB,FxOpABA1,FxOpABA2,FxOpAC,FxOpACA1,FxOpACA2,FxOpAD,FxOpADA1
EXTSYM FxOpADA2,FxOpAE,FxOpAEA1,FxOpAEA2,FxOpAF,FxOpAFA1,FxOpAFA2,FxOpB0
EXTSYM FxOpB1,FxOpB2,FxOpB3,FxOpB4,FxOpB5,FxOpB6,FxOpB7,FxOpB8,FxOpB9,FxOpBA
EXTSYM FxOpBB,FxOpBC,FxOpBD,FxOpBE,FxOpBF,FxOpC0,FxOpC1,FxOpC1A1,FxOpC1A2
EXTSYM FxOpC1A3,FxOpC2,FxOpC2A1,FxOpC2A2,FxOpC2A3,FxOpC3,FxOpC3A1,FxOpC3A2
EXTSYM FxOpC3A3,FxOpC4,FxOpC4A1,FxOpC4A2,FxOpC4A3,FxOpC5,FxOpC5A1,FxOpC5A2
EXTSYM FxOpC5A3,FxOpC6,FxOpC6A1,FxOpC6A2,FxOpC6A3,FxOpC7,FxOpC7A1,FxOpC7A2
EXTSYM FxOpC7A3,FxOpC8,FxOpC8A1,FxOpC8A2,FxOpC8A3,FxOpC9,FxOpC9A1,FxOpC9A2
EXTSYM FxOpC9A3,FxOpCA,FxOpCAA1,FxOpCAA2,FxOpCAA3,FxOpCB,FxOpCBA1,FxOpCBA2
EXTSYM FxOpCBA3,FxOpCC,FxOpCCA1,FxOpCCA2,FxOpCCA3,FxOpCD,FxOpCDA1,FxOpCDA2
EXTSYM FxOpCDA3,FxOpCE,FxOpCEA1,FxOpCEA2,FxOpCEA3,FxOpCF,FxOpCFA1,FxOpCFA2
EXTSYM FxOpCFA3,FxOpD0,FxOpD1,FxOpD2,FxOpD3,FxOpD4,FxOpD5,FxOpD6,FxOpD7
EXTSYM FxOpD8,FxOpD9,FxOpDA,FxOpDB,FxOpDC,FxOpDD,FxOpDE,FxOpDF,FxOpDFA2
EXTSYM FxOpDFA3,FxOpE0,FxOpE1,FxOpE2,FxOpE3,FxOpE4,FxOpE5,FxOpE6,FxOpE7
EXTSYM FxOpE8,FxOpE9,FxOpEA,FxOpEB,FxOpEC,FxOpED,FxOpEE,FxOpEF,FxOpEFA1
EXTSYM FxOpEFA2,FxOpEFA3,FxOpF0,FxOpF0A1,FxOpF0A2,FxOpF1,FxOpF1A1,FxOpF1A2
EXTSYM FxOpF2,FxOpF2A1,FxOpF2A2,FxOpF3,FxOpF3A1,FxOpF3A2,FxOpF4,FxOpF4A1
EXTSYM FxOpF4A2,FxOpF5,FxOpF5A1,FxOpF5A2,FxOpF6,FxOpF6A1,FxOpF6A2,FxOpF7
EXTSYM FxOpF7A1,FxOpF7A2,FxOpF8,FxOpF8A1,FxOpF8A2,FxOpF9,FxOpF9A1,FxOpF9A2
EXTSYM FxOpFA,FxOpFAA1,FxOpFAA2,FxOpFB,FxOpFBA1,FxOpFBA2,FxOpFC,FxOpFCA1
EXTSYM FxOpFCA2,FxOpFD,FxOpFDA1,FxOpFDA2,FxOpFE,FxOpFEA1,FxOpFEA2,FxOpFF
EXTSYM FxOpFFA1,FxOpFFA2,FxOpb05,FxOpb06,FxOpb07,FxOpb08,FxOpb09,FxOpb0A
EXTSYM FxOpb0B,FxOpb0C,FxOpb0D,FxOpb0E,FxOpb0F,FxOpb10,FxOpb11,FxOpb12
EXTSYM FxOpb13,FxOpb14,FxOpb15,FxOpb16,FxOpb17,FxOpb18,FxOpb19,FxOpb1A,FxOpb1B
EXTSYM FxOpb1C,FxOpb1D,FxOpb1E,FxOpb1F,FxOpb3D,FxOpb3E,FxOpb3F,FxOpbB0,FxOpbB1
EXTSYM FxOpbB2,FxOpbB3,FxOpbB4,FxOpbB5,FxOpbB6,FxOpbB7,FxOpbB8,FxOpbB9,FxOpbBA
EXTSYM FxOpbBB,FxOpbBC,FxOpbBD,FxOpbBE,FxOpbBF,FxOpc05,FxOpc06,FxOpc07,FxOpc08
EXTSYM FxOpc09,FxOpc0A,FxOpc0B,FxOpc0C,FxOpc0D,FxOpc0E,FxOpc0F,FxOpc10,FxOpc11
EXTSYM FxOpc12,FxOpc13,FxOpc14,FxOpc15,FxOpc16,FxOpc17,FxOpc18,FxOpc19,FxOpc1A
EXTSYM FxOpc1B,FxOpc1C,FxOpc1D,FxOpc1E,FxOpc1F,FxOpc3D,FxOpc3E,FxOpc3F,FxOpcB0
EXTSYM FxOpcB1,FxOpcB2,FxOpcB3,FxOpcB4,FxOpcB5,FxOpcB6,FxOpcB7,FxOpcB8,FxOpcB9
EXTSYM FxOpcBA,FxOpcBB,FxOpcBC,FxOpcBD,FxOpcBE,FxOpcBF,FxOpd00,FxOpd01,FxOpd02
EXTSYM FxOpd03,FxOpd04,FxOpd05,FxOpd06,FxOpd07,FxOpd08,FxOpd09,FxOpd0A,FxOpd0B
EXTSYM FxOpd0C,FxOpd0D,FxOpd0E,FxOpd0F,FxOpd10,FxOpd11,FxOpd12,FxOpd13,FxOpd14
EXTSYM FxOpd15,FxOpd16,FxOpd17,FxOpd18,FxOpd19,FxOpd1A,FxOpd1B,FxOpd1C,FxOpd1D
EXTSYM FxOpd1E,FxOpd1F,FxOpd20,FxOpd21,FxOpd22,FxOpd23,FxOpd24,FxOpd25,FxOpd26
EXTSYM FxOpd27,FxOpd28,FxOpd29,FxOpd2A,FxOpd2B,FxOpd2C,FxOpd2D,FxOpd2E,FxOpd2F
EXTSYM FxOpd30,FxOpd30A1,FxOpd31,FxOpd31A1,FxOpd32,FxOpd32A1,FxOpd33,FxOpd33A1
EXTSYM FxOpd34,FxOpd34A1,FxOpd35,FxOpd35A1,FxOpd36,FxOpd36A1,FxOpd37,FxOpd37A1
EXTSYM FxOpd38,FxOpd38A1,FxOpd39,FxOpd39A1,FxOpd3A,FxOpd3AA1,FxOpd3B,FxOpd3BA1
EXTSYM FxOpd3C,FxOpd3D,FxOpd3E,FxOpd3F,FxOpd40,FxOpd40A1,FxOpd41,FxOpd41A1
EXTSYM FxOpd42,FxOpd42A1,FxOpd43,FxOpd43A1,FxOpd44,FxOpd44A1,FxOpd45,FxOpd45A1
EXTSYM FxOpd46,FxOpd46A1,FxOpd47,FxOpd47A1,FxOpd48,FxOpd48A1,FxOpd49,FxOpd49A1
EXTSYM FxOpd4A,FxOpd4AA1,FxOpd4B,FxOpd4BA1,FxOpd4C,FxOpd4CA1,FxOpd4D,FxOpd4E
EXTSYM FxOpd4EA1,FxOpd4F,FxOpd50,FxOpd50A1,FxOpd50A2,FxOpd50A3,FxOpd51,FxOpd51A1
EXTSYM FxOpd51A2,FxOpd51A3,FxOpd52,FxOpd52A1,FxOpd52A2,FxOpd52A3,FxOpd53
EXTSYM FxOpd53A1,FxOpd53A2,FxOpd53A3,FxOpd54,FxOpd54A1,FxOpd54A2,FxOpd54A3
EXTSYM FxOpd55,FxOpd55A1,FxOpd55A2,FxOpd55A3,FxOpd56,FxOpd56A1,FxOpd56A2
EXTSYM FxOpd56A3,FxOpd57,FxOpd57A1,FxOpd57A2,FxOpd57A3,FxOpd58,FxOpd58A1
EXTSYM FxOpd58A2,FxOpd58A3,FxOpd59,FxOpd59A1,FxOpd59A2,FxOpd59A3,FxOpd5A
EXTSYM FxOpd5AA1,FxOpd5AA2,FxOpd5AA3,FxOpd5B,FxOpd5BA1,FxOpd5BA2,FxOpd5BA3
EXTSYM FxOpd5C,FxOpd5CA1,FxOpd5CA2,FxOpd5CA3,FxOpd5D,FxOpd5DA1,FxOpd5DA2
EXTSYM FxOpd5DA3,FxOpd5E,FxOpd5EA1,FxOpd5EA2,FxOpd5EA3,FxOpd5F,FxOpd5FA1
EXTSYM FxOpd5FA2,FxOpd5FA3,FxOpd60,FxOpd60A1,FxOpd60A2,FxOpd60A3,FxOpd61
EXTSYM FxOpd61A1,FxOpd61A2,FxOpd61A3,FxOpd62,FxOpd62A1,FxOpd62A2,FxOpd62A3
EXTSYM FxOpd63,FxOpd63A1,FxOpd63A2,FxOpd63A3,FxOpd64,FxOpd64A1,FxOpd64A2
EXTSYM FxOpd64A3,FxOpd65,FxOpd65A1,FxOpd65A2,FxOpd65A3,FxOpd66,FxOpd66A1
EXTSYM FxOpd66A2,FxOpd66A3,FxOpd67,FxOpd67A1,FxOpd67A2,FxOpd67A3,FxOpd68
EXTSYM FxOpd68A1,FxOpd68A2,FxOpd68A3,FxOpd69,FxOpd69A1,FxOpd69A2,FxOpd69A3
EXTSYM FxOpd6A,FxOpd6AA1,FxOpd6AA2,FxOpd6AA3,FxOpd6B,FxOpd6BA1,FxOpd6BA2
EXTSYM FxOpd6BA3,FxOpd6C,FxOpd6CA1,FxOpd6CA2,FxOpd6CA3,FxOpd6D,FxOpd6DA1
EXTSYM FxOpd6DA2,FxOpd6DA3,FxOpd6E,FxOpd6EA1,FxOpd6EA2,FxOpd6EA3,FxOpd6F
EXTSYM FxOpd6FA1,FxOpd6FA2,FxOpd6FA3,FxOpd70,FxOpd71,FxOpd71A1,FxOpd71A2
EXTSYM FxOpd71A3,FxOpd72,FxOpd72A1,FxOpd72A2,FxOpd72A3,FxOpd73,FxOpd73A1
EXTSYM FxOpd73A2,FxOpd73A3,FxOpd74,FxOpd74A1,FxOpd74A2,FxOpd74A3,FxOpd75
EXTSYM FxOpd75A1,FxOpd75A2,FxOpd75A3,FxOpd76,FxOpd76A1,FxOpd76A2,FxOpd76A3
EXTSYM FxOpd77,FxOpd77A1,FxOpd77A2,FxOpd77A3,FxOpd78,FxOpd78A1,FxOpd78A2
EXTSYM FxOpd78A3,FxOpd79,FxOpd79A1,FxOpd79A2,FxOpd79A3,FxOpd7A,FxOpd7AA1
EXTSYM FxOpd7AA2,FxOpd7AA3,FxOpd7B,FxOpd7BA1,FxOpd7BA2,FxOpd7BA3,FxOpd7C
EXTSYM FxOpd7CA1,FxOpd7CA2,FxOpd7CA3,FxOpd7D,FxOpd7DA1,FxOpd7DA2,FxOpd7DA3
EXTSYM FxOpd7E,FxOpd7EA1,FxOpd7EA2,FxOpd7EA3,FxOpd7F,FxOpd7FA1,FxOpd7FA2
EXTSYM FxOpd7FA3,FxOpd80,FxOpd80A1,FxOpd80A2,FxOpd80A3,FxOpd81,FxOpd81A1
EXTSYM FxOpd81A2,FxOpd81A3,FxOpd82,FxOpd82A1,FxOpd82A2,FxOpd82A3,FxOpd83
EXTSYM FxOpd83A1,FxOpd83A2,FxOpd83A3,FxOpd84,FxOpd84A1,FxOpd84A2,FxOpd84A3
EXTSYM FxOpd85,FxOpd85A1,FxOpd85A2,FxOpd85A3,FxOpd86,FxOpd86A1,FxOpd86A2
EXTSYM FxOpd86A3,FxOpd87,FxOpd87A1,FxOpd87A2,FxOpd87A3,FxOpd88,FxOpd88A1
EXTSYM FxOpd88A2,FxOpd88A3,FxOpd89,FxOpd89A1,FxOpd89A2,FxOpd89A3,FxOpd8A
EXTSYM FxOpd8AA1,FxOpd8AA2,FxOpd8AA3,FxOpd8B,FxOpd8BA1,FxOpd8BA2,FxOpd8BA3
EXTSYM FxOpd8C,FxOpd8CA1,FxOpd8CA2,FxOpd8CA3,FxOpd8D,FxOpd8DA1,FxOpd8DA2
EXTSYM FxOpd8DA3,FxOpd8E,FxOpd8EA1,FxOpd8EA2,FxOpd8EA3,FxOpd8F,FxOpd8FA1
EXTSYM FxOpd8FA2,FxOpd8FA3,FxOpd90,FxOpd91,FxOpd92,FxOpd93,FxOpd94,FxOpd95
EXTSYM FxOpd96,FxOpd96A1,FxOpd97,FxOpd98,FxOpd98A1,FxOpd99,FxOpd99A1,FxOpd9A
EXTSYM FxOpd9AA1,FxOpd9B,FxOpd9BA1,FxOpd9C,FxOpd9CA1,FxOpd9D,FxOpd9DA1,FxOpd9E
EXTSYM FxOpd9F,FxOpd9FA1,FxOpdA0,FxOpdA0A1,FxOpdA0A2,FxOpdA1,FxOpdA1A1,FxOpdA1A2
EXTSYM FxOpdA2,FxOpdA2A1,FxOpdA2A2,FxOpdA3,FxOpdA3A1,FxOpdA3A2,FxOpdA4,FxOpdA4A1
EXTSYM FxOpdA4A2,FxOpdA5,FxOpdA5A1,FxOpdA5A2,FxOpdA6,FxOpdA6A1,FxOpdA6A2,FxOpdA7
EXTSYM FxOpdA7A1,FxOpdA7A2,FxOpdA8,FxOpdA8A1,FxOpdA8A2,FxOpdA9,FxOpdA9A1
EXTSYM FxOpdA9A2,FxOpdAA,FxOpdAAA1,FxOpdAAA2,FxOpdAB,FxOpdABA1,FxOpdABA2
EXTSYM FxOpdAC,FxOpdACA1,FxOpdACA2,FxOpdAD,FxOpdADA1,FxOpdADA2,FxOpdAE
EXTSYM FxOpdAEA1,FxOpdAEA2,FxOpdAF,FxOpdAFA1,FxOpdAFA2,FxOpdB0,FxOpdB1,FxOpdB2
EXTSYM FxOpdB3,FxOpdB4,FxOpdB5,FxOpdB6,FxOpdB7,FxOpdB8,FxOpdB9,FxOpdBA,FxOpdBB
EXTSYM FxOpdBC,FxOpdBD,FxOpdBE,FxOpdBF,FxOpdC0,FxOpdC1,FxOpdC1A1,FxOpdC1A2
EXTSYM FxOpdC1A3,FxOpdC2,FxOpdC2A1,FxOpdC2A2,FxOpdC2A3,FxOpdC3,FxOpdC3A1
EXTSYM FxOpdC3A2,FxOpdC3A3,FxOpdC4,FxOpdC4A1,FxOpdC4A2,FxOpdC4A3,FxOpdC5
EXTSYM FxOpdC5A1,FxOpdC5A2,FxOpdC5A3,FxOpdC6,FxOpdC6A1,FxOpdC6A2,FxOpdC6A3
EXTSYM FxOpdC7,FxOpdC7A1,FxOpdC7A2,FxOpdC7A3,FxOpdC8,FxOpdC8A1,FxOpdC8A2
EXTSYM FxOpdC8A3,FxOpdC9,FxOpdC9A1,FxOpdC9A2,FxOpdC9A3,FxOpdCA,FxOpdCAA1
EXTSYM FxOpdCAA2,FxOpdCAA3,FxOpdCB,FxOpdCBA1,FxOpdCBA2,FxOpdCBA3,FxOpdCC
EXTSYM FxOpdCCA1,FxOpdCCA2,FxOpdCCA3,FxOpdCD,FxOpdCDA1,FxOpdCDA2,FxOpdCDA3
EXTSYM FxOpdCE,FxOpdCEA1,FxOpdCEA2,FxOpdCEA3,FxOpdCF,FxOpdCFA1,FxOpdCFA2
EXTSYM FxOpdCFA3,FxOpdD0,FxOpdD1,FxOpdD2,FxOpdD3,FxOpdD4,FxOpdD5,FxOpdD6
EXTSYM FxOpdD7,FxOpdD8,FxOpdD9,FxOpdDA,FxOpdDB,FxOpdDC,FxOpdDD,FxOpdDE,FxOpdDF
EXTSYM FxOpdDFA2,FxOpdDFA3,FxOpdE0,FxOpdE1,FxOpdE2,FxOpdE3,FxOpdE4,FxOpdE5
EXTSYM FxOpdE6,FxOpdE7,FxOpdE8,FxOpdE9,FxOpdEA,FxOpdEB,FxOpdEC,FxOpdED,FxOpdEE
EXTSYM FxOpdEF,FxOpdEFA1,FxOpdEFA2,FxOpdEFA3,FxOpdF0,FxOpdF0A1,FxOpdF0A2,FxOpdF1
EXTSYM FxOpdF1A1,FxOpdF1A2,FxOpdF2,FxOpdF2A1,FxOpdF2A2,FxOpdF3,FxOpdF3A1,FxOpdF3A2
EXTSYM FxOpdF4,FxOpdF4A1,FxOpdF4A2,FxOpdF5,FxOpdF5A1,FxOpdF5A2,FxOpdF6,FxOpdF6A1
EXTSYM FxOpdF6A2,FxOpdF7,FxOpdF7A1,FxOpdF7A2,FxOpdF8,FxOpdF8A1,FxOpdF8A2,FxOpdF9
EXTSYM FxOpdF9A1,FxOpdF9A2,FxOpdFA,FxOpdFAA1,FxOpdFAA2,FxOpdFB,FxOpdFBA1,FxOpdFBA2
EXTSYM FxOpdFC,FxOpdFCA1,FxOpdFCA2,FxOpdFD,FxOpdFDA1,FxOpdFDA2,FxOpdFE,FxOpdFEA1
EXTSYM FxOpdFEA2,FxOpdFF,FxOpdFFA1,FxOpdFFA2,PLOTJmpa,PLOTJmpb

EXTSYM FxOp4C1284b,FxOp4C1284bz,FxOp4C1284bd,FxOp4C1284bzd
EXTSYM FxOp4C1282b,FxOp4C1282bz,FxOp4C1282bd,FxOp4C1282bzd
EXTSYM FxOp4C1288b,FxOp4C1288bz,FxOp4C1288bd,FxOp4C1288bzd
EXTSYM FxOp4C1288bl,FxOp4C1288bzl,FxOp4C1288bdl,FxOp4C1288bzdl
EXTSYM FxOpd4C1284b,FxOpd4C1284bz,FxOpd4C1284bd,FxOpd4C1284bzd
EXTSYM FxOpd4C1282b,FxOpd4C1282bz,FxOpd4C1282bd,FxOpd4C1282bzd
EXTSYM FxOpd4C1288b,FxOpd4C1288bz,FxOpd4C1288bd,FxOpd4C1288bzd
EXTSYM FxOpd4C1288bl,FxOpd4C1288bzl,FxOpd4C1288bdl,FxOpd4C1288bzdl


EXTSYM fxxand,fxbit01,fxbit23,fxbit45,fxbit67

NEWSYM FxTableAsmStart

SECTION .bss ;ALIGN=32
NEWSYM sfx128lineloc, resd 1
NEWSYM sfx160lineloc, resd 1
NEWSYM sfx192lineloc, resd 1
NEWSYM sfxobjlineloc, resd 1

SECTION .text

NEWSYM InitFxTables

;   cmp al,0
;   je near .colors4
;   cmp al,3
;   je near .colors256
;   test byte[SfxPOR],01h
;   jz .zerocheck
;   test byte[SfxPOR],02h
;   jz .nodither4b
   ; Initiate PLOT jump tables
   xor ebx,ebx
   mov ecx,64
   mov eax,PLOTJmpa
.ploop
   mov edx,ebx
   push ebx
   and edx,03h
   test bl,04h
   jz near .zerocheck
   test bl,08h
   jnz .dither
   cmp dl,0
   je .colors4
   cmp dl,3
   je .colors256
   mov ebx,FxOp4C1284b
   jmp .end
.colors4
   mov ebx,FxOp4C1282b
   jmp .end
.colors256
   test bl,20h
   jnz .lowercheck
   mov ebx,FxOp4C1288b
   jmp .end
.lowercheck
   mov ebx,FxOp4C1288bl
   jmp .end
.dither
   cmp dl,0
   je .colors4d
   cmp dl,3
   je .colors256d
   mov ebx,FxOp4C1284bd
   jmp .end
.colors4d
   mov ebx,FxOp4C1282bd
   jmp .end
.colors256d
   test bl,20h
   jnz .lowercheckd
   mov ebx,FxOp4C1288bd
   jmp .end
.lowercheckd
   mov ebx,FxOp4C1288bdl
   jmp .end
.zerocheck
   test bl,08h
   jnz .ditherz
   cmp dl,0
   je .colors4z
   cmp dl,3
   je .colors256z
   mov ebx,FxOp4C1284bz
   jmp .end
.colors4z
   mov ebx,FxOp4C1282bz
   jmp .end
.colors256z
   test bl,20h
   jnz .lowercheckz
   mov ebx,FxOp4C1288bz
   jmp .end
.lowercheckz
   mov ebx,FxOp4C1288bzl
   jmp .end
.ditherz
   cmp dl,0
   je .colors4dz
   cmp dl,3
   je .colors256dz
   mov ebx,FxOp4C1284bzd
   jmp .end
.colors4dz
   mov ebx,FxOp4C1282bzd
   jmp .end
.colors256dz
   test bl,20h
   jnz .lowercheckdz
   mov ebx,FxOp4C1288bzd
   jmp .end
.lowercheckdz
   mov ebx,FxOp4C1288bzdl
.end
   mov [eax],ebx
   pop ebx
   inc ebx
   add eax,4
   dec ecx
   jnz near .ploop

   xor ebx,ebx
   mov ecx,64
   mov eax,PLOTJmpb
.ploop2
   mov edx,ebx
   push ebx
   and edx,03h
   test bl,04h
   jz near .zerocheck2
   test bl,08h
   jnz .dither2
   cmp dl,0
   je .colors42
   cmp dl,3
   je .colors2562
   mov ebx,FxOpd4C1284b
   jmp .end2
.colors42
   mov ebx,FxOpd4C1282b
   jmp .end2
.colors2562
   test bl,20h
   jnz .lowercheck2
   mov ebx,FxOpd4C1288b
   jmp .end2
.lowercheck2
   mov ebx,FxOpd4C1288bl
   jmp .end2
.dither2
   cmp dl,0
   je .colors4d2
   cmp dl,3
   je .colors256d2
   mov ebx,FxOpd4C1284bd
   jmp .end2
.colors4d2
   mov ebx,FxOpd4C1282bd
   jmp .end2
.colors256d2
   test bl,20h
   jnz .lowercheckd2
   mov ebx,FxOpd4C1288bd
   jmp .end2
.lowercheckd2
   mov ebx,FxOpd4C1288bdl
   jmp .end2
.zerocheck2
   test bl,08h
   jnz .ditherz2
   cmp dl,0
   je .colors4z2
   cmp dl,3
   je .colors256z2
   mov ebx,FxOpd4C1284bz
   jmp .end2
.colors4z2
   mov ebx,FxOpd4C1282bz
   jmp .end2
.colors256z2
   test bl,20h
   jnz .lowercheckz2
   mov ebx,FxOpd4C1288bz
   jmp .end2
.lowercheckz2
   mov ebx,FxOpd4C1288bzl
   jmp .end2
.ditherz2
   cmp dl,0
   je .colors4dz2
   cmp dl,3
   je .colors256dz2
   mov ebx,FxOpd4C1284bzd
   jmp .end2
.colors4dz2
   mov ebx,FxOpd4C1282bzd
   jmp .end2
.colors256dz2
   test bl,20h
   jnz .lowercheckdz2
   mov ebx,FxOpd4C1288bzd
   jmp .end2
.lowercheckdz2
   mov ebx,FxOpd4C1288bzdl
.end2
   mov [eax],ebx
   pop ebx
   inc ebx
   add eax,4
   dec ecx
   jnz near .ploop2

   ; Initiate AND and bit tables
   mov eax,fxxand
   mov ecx,256
   xor ebx,ebx
.loopat
   push ecx
   mov ecx,ebx
   and ecx,07h
   xor ecx,07h
   mov edx,0101h
   shl edx,cl
   xor edx,0FFFFFFFFh
   pop ecx
   mov [eax],edx
   inc ebx
   add eax,4
   dec ecx
   jnz .loopat
   ; bit tables
   mov eax,fxbit01
   mov ecx,256
   xor ebx,ebx
.loopb01
   xor edx,edx
   test ebx,01h
   jz .nob0
   or edx,0FFh
.nob0
   test ebx,02h
   jz .nob1
   or edx,0FF00h
.nob1
   mov [eax],edx
   xor edx,edx
   test ebx,04h
   jz .nob2
   or edx,0FFh
.nob2
   test ebx,08h
   jz .nob3
   or edx,0FF00h
.nob3
   mov [eax+256*4],edx
   xor edx,edx
   test ebx,10h
   jz .nob4
   or edx,0FFh
.nob4
   test ebx,20h
   jz .nob5
   or edx,0FF00h
.nob5
   mov [eax+256*8],edx
   xor edx,edx
   test ebx,40h
   jz .nob6
   or edx,0FFh
.nob6
   test ebx,80h
   jz .nob7
   or edx,0FF00h
.nob7
   mov [eax+256*12],edx
   add eax,4
   inc ebx
   dec ecx
   jnz near .loopb01

   ; Initialize PLOT tables
;   mov esi,[sfxramdata]
   mov esi,[sfxplottable]
;   add esi,1024*1024
   ; Create 4 * 256k plot tables
   ; 128 line mode
   mov [sfx128lineloc],esi
   xor eax,eax
   xor ebx,ebx
.nexty
.nextx
   cmp eax,128
   jae .over
   push eax
   push ebx
   shr eax,3
   shr ebx,3
   shl ebx,4
   add ebx,eax
   mov [esi],ebx
   add esi,4
   pop ebx
   pop eax
   jmp .notover
.over
   mov dword[esi],0FFFFFFFFh
   add esi,4
.notover
   inc al
   jnz .nextx
   inc bl
   jnz .nexty
   ; 160 line mode
   mov [sfx160lineloc],esi
.nexty2
.nextx2
   cmp eax,160
   jae .over2
   push eax
   push ebx
   shr eax,3
   shr ebx,3
   mov edx,ebx
   shl ebx,4
   shl edx,2
   add ebx,eax
   add ebx,edx
   mov [esi],ebx
   add esi,4
   pop ebx
   pop eax
   jmp .notover2
.over2
   mov dword[esi],0FFFFFFFFh
   add esi,4
.notover2
   inc al
   jnz .nextx2
   inc bl
   jnz .nexty2
   ; 192 line mode
   mov [sfx192lineloc],esi
.nexty3
.nextx3
   cmp eax,192
   jae .over3
   push eax
   push ebx
   shr eax,3
   shr ebx,3
   mov edx,ebx
   shl ebx,4
   shl edx,3
   add ebx,eax
   add ebx,edx
   mov [esi],ebx
   add esi,4
   pop ebx
   pop eax
   jmp .notover3
.over3
   mov dword[esi],0FFFFFFFFh
   add esi,4
.notover3
   inc al
   jnz .nextx3
   inc bl
   jnz .nexty3
   ; obj mode
   mov [sfxobjlineloc],esi
.nexty4
.nextx4
   push eax
   push ebx
   and ebx,80h
   and eax,80h
   shl ebx,1
   shl eax,2
   add ebx,eax
   mov edx,ebx
   pop ebx
   pop eax
   push eax
   push ebx
   and ebx,78h
   and eax,78h
   shr ebx,3
   shl eax,1
   add ebx,eax
   add edx,ebx
   pop ebx
   pop eax
   mov [esi],edx
   add esi,4
   inc al
   jnz .nextx4
   inc bl
   jnz .nexty4

   mov eax,[romdata]
   xor ebx,ebx
   mov ecx,256
.loopc
   mov [SfxMemTable+ebx*4],eax
   inc ebx
   dec ecx
   jnz .loopc

   mov eax,[romdata]
   add eax,200000h
   xor ebx,ebx
   mov ecx,64
.loop
   mov [SfxMemTable+ebx*4],eax
   add eax,65536
   inc ebx
   dec ecx
   jnz .loop

   mov eax,[romdata]
   mov ecx,64
.loop2
   mov [SfxMemTable+ebx*4],eax
   add eax,65536
   inc ebx
   dec ecx
   jnz .loop2

   mov eax,[sfxramdata]
   mov [SfxMemTable+70h*4],eax
   add eax,65536
   mov [SfxMemTable+71h*4],eax
   add eax,65536
   mov [SfxMemTable+72h*4],eax
   add eax,65536
   mov [SfxMemTable+73h*4],eax

   mov eax,[romdata]
   add eax,200000h

   xor ebx,ebx
   mov ebx,80h
   mov ecx,64
.loops
   mov [SfxMemTable+ebx*4],eax
   add eax,65536
   inc ebx
   dec ecx
   jnz .loops

   mov eax,[romdata]
   mov ecx,64
.loop2s
   mov [SfxMemTable+ebx*4],eax
   add eax,65536
   inc ebx
   dec ecx
   jnz .loop2s
   mov eax,[sfxramdata]
   mov [SfxMemTable+0F0h*4],eax
   add eax,65536
   mov [SfxMemTable+0F1h*4],eax
   add eax,65536
   mov [SfxMemTable+0F2h*4],eax
   add eax,65536
   mov [SfxMemTable+0F3h*4],eax

   mov dword [FxTable+00h*4],FxOp00
   mov dword [FxTable+01h*4],FxOp01
   mov dword [FxTable+02h*4],FxOp02
   mov dword [FxTable+03h*4],FxOp03
   mov dword [FxTable+04h*4],FxOp04
   mov dword [FxTable+05h*4],FxOp05
   mov dword [FxTable+06h*4],FxOp06
   mov dword [FxTable+07h*4],FxOp07
   mov dword [FxTable+08h*4],FxOp08
   mov dword [FxTable+09h*4],FxOp09
   mov dword [FxTable+0Ah*4],FxOp0A
   mov dword [FxTable+0Bh*4],FxOp0B
   mov dword [FxTable+0Ch*4],FxOp0C
   mov dword [FxTable+0Dh*4],FxOp0D
   mov dword [FxTable+0Eh*4],FxOp0E
   mov dword [FxTable+0Fh*4],FxOp0F

   mov dword [FxTable+10h*4],FxOp10
   mov dword [FxTable+11h*4],FxOp11
   mov dword [FxTable+12h*4],FxOp12
   mov dword [FxTable+13h*4],FxOp13
   mov dword [FxTable+14h*4],FxOp14
   mov dword [FxTable+15h*4],FxOp15
   mov dword [FxTable+16h*4],FxOp16
   mov dword [FxTable+17h*4],FxOp17
   mov dword [FxTable+18h*4],FxOp18
   mov dword [FxTable+19h*4],FxOp19
   mov dword [FxTable+1Ah*4],FxOp1A
   mov dword [FxTable+1Bh*4],FxOp1B
   mov dword [FxTable+1Ch*4],FxOp1C
   mov dword [FxTable+1Dh*4],FxOp1D
   mov dword [FxTable+1Eh*4],FxOp1E
   mov dword [FxTable+1Fh*4],FxOp1F

   mov dword [FxTable+20h*4],FxOp20
   mov dword [FxTable+21h*4],FxOp21
   mov dword [FxTable+22h*4],FxOp22
   mov dword [FxTable+23h*4],FxOp23
   mov dword [FxTable+24h*4],FxOp24
   mov dword [FxTable+25h*4],FxOp25
   mov dword [FxTable+26h*4],FxOp26
   mov dword [FxTable+27h*4],FxOp27
   mov dword [FxTable+28h*4],FxOp28
   mov dword [FxTable+29h*4],FxOp29
   mov dword [FxTable+2Ah*4],FxOp2A
   mov dword [FxTable+2Bh*4],FxOp2B
   mov dword [FxTable+2Ch*4],FxOp2C
   mov dword [FxTable+2Dh*4],FxOp2D
   mov dword [FxTable+2Eh*4],FxOp2E
   mov dword [FxTable+2Fh*4],FxOp2F

   mov dword [FxTable+30h*4],FxOp30
   mov dword [FxTable+31h*4],FxOp31
   mov dword [FxTable+32h*4],FxOp32
   mov dword [FxTable+33h*4],FxOp33
   mov dword [FxTable+34h*4],FxOp34
   mov dword [FxTable+35h*4],FxOp35
   mov dword [FxTable+36h*4],FxOp36
   mov dword [FxTable+37h*4],FxOp37
   mov dword [FxTable+38h*4],FxOp38
   mov dword [FxTable+39h*4],FxOp39
   mov dword [FxTable+3Ah*4],FxOp3A
   mov dword [FxTable+3Bh*4],FxOp3B
   mov dword [FxTable+3Ch*4],FxOp3C
   mov dword [FxTable+3Dh*4],FxOp3D
   mov dword [FxTable+3Eh*4],FxOp3E
   mov dword [FxTable+3Fh*4],FxOp3F

   mov dword [FxTable+40h*4],FxOp40
   mov dword [FxTable+41h*4],FxOp41
   mov dword [FxTable+42h*4],FxOp42
   mov dword [FxTable+43h*4],FxOp43
   mov dword [FxTable+44h*4],FxOp44
   mov dword [FxTable+45h*4],FxOp45
   mov dword [FxTable+46h*4],FxOp46
   mov dword [FxTable+47h*4],FxOp47
   mov dword [FxTable+48h*4],FxOp48
   mov dword [FxTable+49h*4],FxOp49
   mov dword [FxTable+4Ah*4],FxOp4A
   mov dword [FxTable+4Bh*4],FxOp4B
   mov dword [FxTable+4Ch*4],FxOp4C
   mov dword [FxTable+4Dh*4],FxOp4D
   mov dword [FxTable+4Eh*4],FxOp4E
   mov dword [FxTable+4Fh*4],FxOp4F

   mov dword [FxTable+50h*4],FxOp50
   mov dword [FxTable+51h*4],FxOp51
   mov dword [FxTable+52h*4],FxOp52
   mov dword [FxTable+53h*4],FxOp53
   mov dword [FxTable+54h*4],FxOp54
   mov dword [FxTable+55h*4],FxOp55
   mov dword [FxTable+56h*4],FxOp56
   mov dword [FxTable+57h*4],FxOp57
   mov dword [FxTable+58h*4],FxOp58
   mov dword [FxTable+59h*4],FxOp59
   mov dword [FxTable+5Ah*4],FxOp5A
   mov dword [FxTable+5Bh*4],FxOp5B
   mov dword [FxTable+5Ch*4],FxOp5C
   mov dword [FxTable+5Dh*4],FxOp5D
   mov dword [FxTable+5Eh*4],FxOp5E
   mov dword [FxTable+5Fh*4],FxOp5F

   mov dword [FxTable+60h*4],FxOp60
   mov dword [FxTable+61h*4],FxOp61
   mov dword [FxTable+62h*4],FxOp62
   mov dword [FxTable+63h*4],FxOp63
   mov dword [FxTable+64h*4],FxOp64
   mov dword [FxTable+65h*4],FxOp65
   mov dword [FxTable+66h*4],FxOp66
   mov dword [FxTable+67h*4],FxOp67
   mov dword [FxTable+68h*4],FxOp68
   mov dword [FxTable+69h*4],FxOp69
   mov dword [FxTable+6Ah*4],FxOp6A
   mov dword [FxTable+6Bh*4],FxOp6B
   mov dword [FxTable+6Ch*4],FxOp6C
   mov dword [FxTable+6Dh*4],FxOp6D
   mov dword [FxTable+6Eh*4],FxOp6E
   mov dword [FxTable+6Fh*4],FxOp6F

   mov dword [FxTable+70h*4],FxOp70
   mov dword [FxTable+71h*4],FxOp71
   mov dword [FxTable+72h*4],FxOp72
   mov dword [FxTable+73h*4],FxOp73
   mov dword [FxTable+74h*4],FxOp74
   mov dword [FxTable+75h*4],FxOp75
   mov dword [FxTable+76h*4],FxOp76
   mov dword [FxTable+77h*4],FxOp77
   mov dword [FxTable+78h*4],FxOp78
   mov dword [FxTable+79h*4],FxOp79
   mov dword [FxTable+7Ah*4],FxOp7A
   mov dword [FxTable+7Bh*4],FxOp7B
   mov dword [FxTable+7Ch*4],FxOp7C
   mov dword [FxTable+7Dh*4],FxOp7D
   mov dword [FxTable+7Eh*4],FxOp7E
   mov dword [FxTable+7Fh*4],FxOp7F

   mov dword [FxTable+80h*4],FxOp80
   mov dword [FxTable+81h*4],FxOp81
   mov dword [FxTable+82h*4],FxOp82
   mov dword [FxTable+83h*4],FxOp83
   mov dword [FxTable+84h*4],FxOp84
   mov dword [FxTable+85h*4],FxOp85
   mov dword [FxTable+86h*4],FxOp86
   mov dword [FxTable+87h*4],FxOp87
   mov dword [FxTable+88h*4],FxOp88
   mov dword [FxTable+89h*4],FxOp89
   mov dword [FxTable+8Ah*4],FxOp8A
   mov dword [FxTable+8Bh*4],FxOp8B
   mov dword [FxTable+8Ch*4],FxOp8C
   mov dword [FxTable+8Dh*4],FxOp8D
   mov dword [FxTable+8Eh*4],FxOp8E
   mov dword [FxTable+8Fh*4],FxOp8F

   mov dword [FxTable+90h*4],FxOp90
   mov dword [FxTable+91h*4],FxOp91
   mov dword [FxTable+92h*4],FxOp92
   mov dword [FxTable+93h*4],FxOp93
   mov dword [FxTable+94h*4],FxOp94
   mov dword [FxTable+95h*4],FxOp95
   mov dword [FxTable+96h*4],FxOp96
   mov dword [FxTable+97h*4],FxOp97
   mov dword [FxTable+98h*4],FxOp98
   mov dword [FxTable+99h*4],FxOp99
   mov dword [FxTable+9Ah*4],FxOp9A
   mov dword [FxTable+9Bh*4],FxOp9B
   mov dword [FxTable+9Ch*4],FxOp9C
   mov dword [FxTable+9Dh*4],FxOp9D
   mov dword [FxTable+9Eh*4],FxOp9E
   mov dword [FxTable+9Fh*4],FxOp9F

   mov dword [FxTable+0A0h*4],FxOpA0
   mov dword [FxTable+0A1h*4],FxOpA1
   mov dword [FxTable+0A2h*4],FxOpA2
   mov dword [FxTable+0A3h*4],FxOpA3
   mov dword [FxTable+0A4h*4],FxOpA4
   mov dword [FxTable+0A5h*4],FxOpA5
   mov dword [FxTable+0A6h*4],FxOpA6
   mov dword [FxTable+0A7h*4],FxOpA7
   mov dword [FxTable+0A8h*4],FxOpA8
   mov dword [FxTable+0A9h*4],FxOpA9
   mov dword [FxTable+0AAh*4],FxOpAA
   mov dword [FxTable+0ABh*4],FxOpAB
   mov dword [FxTable+0ACh*4],FxOpAC
   mov dword [FxTable+0ADh*4],FxOpAD
   mov dword [FxTable+0AEh*4],FxOpAE
   mov dword [FxTable+0AFh*4],FxOpAF

   mov dword [FxTable+0B0h*4],FxOpB0
   mov dword [FxTable+0B1h*4],FxOpB1
   mov dword [FxTable+0B2h*4],FxOpB2
   mov dword [FxTable+0B3h*4],FxOpB3
   mov dword [FxTable+0B4h*4],FxOpB4
   mov dword [FxTable+0B5h*4],FxOpB5
   mov dword [FxTable+0B6h*4],FxOpB6
   mov dword [FxTable+0B7h*4],FxOpB7
   mov dword [FxTable+0B8h*4],FxOpB8
   mov dword [FxTable+0B9h*4],FxOpB9
   mov dword [FxTable+0BAh*4],FxOpBA
   mov dword [FxTable+0BBh*4],FxOpBB
   mov dword [FxTable+0BCh*4],FxOpBC
   mov dword [FxTable+0BDh*4],FxOpBD
   mov dword [FxTable+0BEh*4],FxOpBE
   mov dword [FxTable+0BFh*4],FxOpBF

   mov dword [FxTable+0C0h*4],FxOpC0
   mov dword [FxTable+0C1h*4],FxOpC1
   mov dword [FxTable+0C2h*4],FxOpC2
   mov dword [FxTable+0C3h*4],FxOpC3
   mov dword [FxTable+0C4h*4],FxOpC4
   mov dword [FxTable+0C5h*4],FxOpC5
   mov dword [FxTable+0C6h*4],FxOpC6
   mov dword [FxTable+0C7h*4],FxOpC7
   mov dword [FxTable+0C8h*4],FxOpC8
   mov dword [FxTable+0C9h*4],FxOpC9
   mov dword [FxTable+0CAh*4],FxOpCA
   mov dword [FxTable+0CBh*4],FxOpCB
   mov dword [FxTable+0CCh*4],FxOpCC
   mov dword [FxTable+0CDh*4],FxOpCD
   mov dword [FxTable+0CEh*4],FxOpCE
   mov dword [FxTable+0CFh*4],FxOpCF

   mov dword [FxTable+0D0h*4],FxOpD0
   mov dword [FxTable+0D1h*4],FxOpD1
   mov dword [FxTable+0D2h*4],FxOpD2
   mov dword [FxTable+0D3h*4],FxOpD3
   mov dword [FxTable+0D4h*4],FxOpD4
   mov dword [FxTable+0D5h*4],FxOpD5
   mov dword [FxTable+0D6h*4],FxOpD6
   mov dword [FxTable+0D7h*4],FxOpD7
   mov dword [FxTable+0D8h*4],FxOpD8
   mov dword [FxTable+0D9h*4],FxOpD9
   mov dword [FxTable+0DAh*4],FxOpDA
   mov dword [FxTable+0DBh*4],FxOpDB
   mov dword [FxTable+0DCh*4],FxOpDC
   mov dword [FxTable+0DDh*4],FxOpDD
   mov dword [FxTable+0DEh*4],FxOpDE
   mov dword [FxTable+0DFh*4],FxOpDF

   mov dword [FxTable+0E0h*4],FxOpE0
   mov dword [FxTable+0E1h*4],FxOpE1
   mov dword [FxTable+0E2h*4],FxOpE2
   mov dword [FxTable+0E3h*4],FxOpE3
   mov dword [FxTable+0E4h*4],FxOpE4
   mov dword [FxTable+0E5h*4],FxOpE5
   mov dword [FxTable+0E6h*4],FxOpE6
   mov dword [FxTable+0E7h*4],FxOpE7
   mov dword [FxTable+0E8h*4],FxOpE8
   mov dword [FxTable+0E9h*4],FxOpE9
   mov dword [FxTable+0EAh*4],FxOpEA
   mov dword [FxTable+0EBh*4],FxOpEB
   mov dword [FxTable+0ECh*4],FxOpEC
   mov dword [FxTable+0EDh*4],FxOpED
   mov dword [FxTable+0EEh*4],FxOpEE
   mov dword [FxTable+0EFh*4],FxOpEF

   mov dword [FxTable+0F0h*4],FxOpF0
   mov dword [FxTable+0F1h*4],FxOpF1
   mov dword [FxTable+0F2h*4],FxOpF2
   mov dword [FxTable+0F3h*4],FxOpF3
   mov dword [FxTable+0F4h*4],FxOpF4
   mov dword [FxTable+0F5h*4],FxOpF5
   mov dword [FxTable+0F6h*4],FxOpF6
   mov dword [FxTable+0F7h*4],FxOpF7
   mov dword [FxTable+0F8h*4],FxOpF8
   mov dword [FxTable+0F9h*4],FxOpF9
   mov dword [FxTable+0FAh*4],FxOpFA
   mov dword [FxTable+0FBh*4],FxOpFB
   mov dword [FxTable+0FCh*4],FxOpFC
   mov dword [FxTable+0FDh*4],FxOpFD
   mov dword [FxTable+0FEh*4],FxOpFE
   mov dword [FxTable+0FFh*4],FxOpFF


   mov dword [FxTableA1+00h*4],FxOp00
   mov dword [FxTableA1+01h*4],FxOp01
   mov dword [FxTableA1+02h*4],FxOp02
   mov dword [FxTableA1+03h*4],FxOp03
   mov dword [FxTableA1+04h*4],FxOp04
   mov dword [FxTableA1+05h*4],FxOp05
   mov dword [FxTableA1+06h*4],FxOp06
   mov dword [FxTableA1+07h*4],FxOp07
   mov dword [FxTableA1+08h*4],FxOp08
   mov dword [FxTableA1+09h*4],FxOp09
   mov dword [FxTableA1+0Ah*4],FxOp0A
   mov dword [FxTableA1+0Bh*4],FxOp0B
   mov dword [FxTableA1+0Ch*4],FxOp0C
   mov dword [FxTableA1+0Dh*4],FxOp0D
   mov dword [FxTableA1+0Eh*4],FxOp0E
   mov dword [FxTableA1+0Fh*4],FxOp0F

   mov dword [FxTableA1+10h*4],FxOp10
   mov dword [FxTableA1+11h*4],FxOp11
   mov dword [FxTableA1+12h*4],FxOp12
   mov dword [FxTableA1+13h*4],FxOp13
   mov dword [FxTableA1+14h*4],FxOp14
   mov dword [FxTableA1+15h*4],FxOp15
   mov dword [FxTableA1+16h*4],FxOp16
   mov dword [FxTableA1+17h*4],FxOp17
   mov dword [FxTableA1+18h*4],FxOp18
   mov dword [FxTableA1+19h*4],FxOp19
   mov dword [FxTableA1+1Ah*4],FxOp1A
   mov dword [FxTableA1+1Bh*4],FxOp1B
   mov dword [FxTableA1+1Ch*4],FxOp1C
   mov dword [FxTableA1+1Dh*4],FxOp1D
   mov dword [FxTableA1+1Eh*4],FxOp1E
   mov dword [FxTableA1+1Fh*4],FxOp1F

   mov dword [FxTableA1+20h*4],FxOp20
   mov dword [FxTableA1+21h*4],FxOp21
   mov dword [FxTableA1+22h*4],FxOp22
   mov dword [FxTableA1+23h*4],FxOp23
   mov dword [FxTableA1+24h*4],FxOp24
   mov dword [FxTableA1+25h*4],FxOp25
   mov dword [FxTableA1+26h*4],FxOp26
   mov dword [FxTableA1+27h*4],FxOp27
   mov dword [FxTableA1+28h*4],FxOp28
   mov dword [FxTableA1+29h*4],FxOp29
   mov dword [FxTableA1+2Ah*4],FxOp2A
   mov dword [FxTableA1+2Bh*4],FxOp2B
   mov dword [FxTableA1+2Ch*4],FxOp2C
   mov dword [FxTableA1+2Dh*4],FxOp2D
   mov dword [FxTableA1+2Eh*4],FxOp2E
   mov dword [FxTableA1+2Fh*4],FxOp2F

   mov dword [FxTableA1+30h*4],FxOp30A1
   mov dword [FxTableA1+31h*4],FxOp31A1
   mov dword [FxTableA1+32h*4],FxOp32A1
   mov dword [FxTableA1+33h*4],FxOp33A1
   mov dword [FxTableA1+34h*4],FxOp34A1
   mov dword [FxTableA1+35h*4],FxOp35A1
   mov dword [FxTableA1+36h*4],FxOp36A1
   mov dword [FxTableA1+37h*4],FxOp37A1
   mov dword [FxTableA1+38h*4],FxOp38A1
   mov dword [FxTableA1+39h*4],FxOp39A1
   mov dword [FxTableA1+3Ah*4],FxOp3AA1
   mov dword [FxTableA1+3Bh*4],FxOp3BA1
   mov dword [FxTableA1+3Ch*4],FxOp3C
   mov dword [FxTableA1+3Dh*4],FxOp3D
   mov dword [FxTableA1+3Eh*4],FxOp3E
   mov dword [FxTableA1+3Fh*4],FxOp3F

   mov dword [FxTableA1+40h*4],FxOp40A1
   mov dword [FxTableA1+41h*4],FxOp41A1
   mov dword [FxTableA1+42h*4],FxOp42A1
   mov dword [FxTableA1+43h*4],FxOp43A1
   mov dword [FxTableA1+44h*4],FxOp44A1
   mov dword [FxTableA1+45h*4],FxOp45A1
   mov dword [FxTableA1+46h*4],FxOp46A1
   mov dword [FxTableA1+47h*4],FxOp47A1
   mov dword [FxTableA1+48h*4],FxOp48A1
   mov dword [FxTableA1+49h*4],FxOp49A1
   mov dword [FxTableA1+4Ah*4],FxOp4AA1
   mov dword [FxTableA1+4Bh*4],FxOp4BA1
   mov dword [FxTableA1+4Ch*4],FxOp4CA1
   mov dword [FxTableA1+4Dh*4],FxOp4D
   mov dword [FxTableA1+4Eh*4],FxOp4EA1
   mov dword [FxTableA1+4Fh*4],FxOp4F

   mov dword [FxTableA1+50h*4],FxOp50A1
   mov dword [FxTableA1+51h*4],FxOp51A1
   mov dword [FxTableA1+52h*4],FxOp52A1
   mov dword [FxTableA1+53h*4],FxOp53A1
   mov dword [FxTableA1+54h*4],FxOp54A1
   mov dword [FxTableA1+55h*4],FxOp55A1
   mov dword [FxTableA1+56h*4],FxOp56A1
   mov dword [FxTableA1+57h*4],FxOp57A1
   mov dword [FxTableA1+58h*4],FxOp58A1
   mov dword [FxTableA1+59h*4],FxOp59A1
   mov dword [FxTableA1+5Ah*4],FxOp5AA1
   mov dword [FxTableA1+5Bh*4],FxOp5BA1
   mov dword [FxTableA1+5Ch*4],FxOp5CA1
   mov dword [FxTableA1+5Dh*4],FxOp5DA1
   mov dword [FxTableA1+5Eh*4],FxOp5EA1
   mov dword [FxTableA1+5Fh*4],FxOp5FA1

   mov dword [FxTableA1+60h*4],FxOp60A1
   mov dword [FxTableA1+61h*4],FxOp61A1
   mov dword [FxTableA1+62h*4],FxOp62A1
   mov dword [FxTableA1+63h*4],FxOp63A1
   mov dword [FxTableA1+64h*4],FxOp64A1
   mov dword [FxTableA1+65h*4],FxOp65A1
   mov dword [FxTableA1+66h*4],FxOp66A1
   mov dword [FxTableA1+67h*4],FxOp67A1
   mov dword [FxTableA1+68h*4],FxOp68A1
   mov dword [FxTableA1+69h*4],FxOp69A1
   mov dword [FxTableA1+6Ah*4],FxOp6AA1
   mov dword [FxTableA1+6Bh*4],FxOp6BA1
   mov dword [FxTableA1+6Ch*4],FxOp6CA1
   mov dword [FxTableA1+6Dh*4],FxOp6DA1
   mov dword [FxTableA1+6Eh*4],FxOp6EA1
   mov dword [FxTableA1+6Fh*4],FxOp6FA1

   mov dword [FxTableA1+70h*4],FxOp70
   mov dword [FxTableA1+71h*4],FxOp71A1
   mov dword [FxTableA1+72h*4],FxOp72A1
   mov dword [FxTableA1+73h*4],FxOp73A1
   mov dword [FxTableA1+74h*4],FxOp74A1
   mov dword [FxTableA1+75h*4],FxOp75A1
   mov dword [FxTableA1+76h*4],FxOp76A1
   mov dword [FxTableA1+77h*4],FxOp77A1
   mov dword [FxTableA1+78h*4],FxOp78A1
   mov dword [FxTableA1+79h*4],FxOp79A1
   mov dword [FxTableA1+7Ah*4],FxOp7AA1
   mov dword [FxTableA1+7Bh*4],FxOp7BA1
   mov dword [FxTableA1+7Ch*4],FxOp7CA1
   mov dword [FxTableA1+7Dh*4],FxOp7DA1
   mov dword [FxTableA1+7Eh*4],FxOp7EA1
   mov dword [FxTableA1+7Fh*4],FxOp7FA1

   mov dword [FxTableA1+80h*4],FxOp80A1
   mov dword [FxTableA1+81h*4],FxOp81A1
   mov dword [FxTableA1+82h*4],FxOp82A1
   mov dword [FxTableA1+83h*4],FxOp83A1
   mov dword [FxTableA1+84h*4],FxOp84A1
   mov dword [FxTableA1+85h*4],FxOp85A1
   mov dword [FxTableA1+86h*4],FxOp86A1
   mov dword [FxTableA1+87h*4],FxOp87A1
   mov dword [FxTableA1+88h*4],FxOp88A1
   mov dword [FxTableA1+89h*4],FxOp89A1
   mov dword [FxTableA1+8Ah*4],FxOp8AA1
   mov dword [FxTableA1+8Bh*4],FxOp8BA1
   mov dword [FxTableA1+8Ch*4],FxOp8CA1
   mov dword [FxTableA1+8Dh*4],FxOp8DA1
   mov dword [FxTableA1+8Eh*4],FxOp8EA1
   mov dword [FxTableA1+8Fh*4],FxOp8FA1

   mov dword [FxTableA1+90h*4],FxOp90
   mov dword [FxTableA1+91h*4],FxOp91
   mov dword [FxTableA1+92h*4],FxOp92
   mov dword [FxTableA1+93h*4],FxOp93
   mov dword [FxTableA1+94h*4],FxOp94
   mov dword [FxTableA1+95h*4],FxOp95
   mov dword [FxTableA1+96h*4],FxOp96A1
   mov dword [FxTableA1+97h*4],FxOp97
   mov dword [FxTableA1+98h*4],FxOp98A1
   mov dword [FxTableA1+99h*4],FxOp99A1
   mov dword [FxTableA1+9Ah*4],FxOp9AA1
   mov dword [FxTableA1+9Bh*4],FxOp9BA1
   mov dword [FxTableA1+9Ch*4],FxOp9CA1
   mov dword [FxTableA1+9Dh*4],FxOp9DA1
   mov dword [FxTableA1+9Eh*4],FxOp9E
   mov dword [FxTableA1+9Fh*4],FxOp9FA1

   mov dword [FxTableA1+0A0h*4],FxOpA0A1
   mov dword [FxTableA1+0A1h*4],FxOpA1A1
   mov dword [FxTableA1+0A2h*4],FxOpA2A1
   mov dword [FxTableA1+0A3h*4],FxOpA3A1
   mov dword [FxTableA1+0A4h*4],FxOpA4A1
   mov dword [FxTableA1+0A5h*4],FxOpA5A1
   mov dword [FxTableA1+0A6h*4],FxOpA6A1
   mov dword [FxTableA1+0A7h*4],FxOpA7A1
   mov dword [FxTableA1+0A8h*4],FxOpA8A1
   mov dword [FxTableA1+0A9h*4],FxOpA9A1
   mov dword [FxTableA1+0AAh*4],FxOpAAA1
   mov dword [FxTableA1+0ABh*4],FxOpABA1
   mov dword [FxTableA1+0ACh*4],FxOpACA1
   mov dword [FxTableA1+0ADh*4],FxOpADA1
   mov dword [FxTableA1+0AEh*4],FxOpAEA1
   mov dword [FxTableA1+0AFh*4],FxOpAFA1

   mov dword [FxTableA1+0B0h*4],FxOpB0
   mov dword [FxTableA1+0B1h*4],FxOpB1
   mov dword [FxTableA1+0B2h*4],FxOpB2
   mov dword [FxTableA1+0B3h*4],FxOpB3
   mov dword [FxTableA1+0B4h*4],FxOpB4
   mov dword [FxTableA1+0B5h*4],FxOpB5
   mov dword [FxTableA1+0B6h*4],FxOpB6
   mov dword [FxTableA1+0B7h*4],FxOpB7
   mov dword [FxTableA1+0B8h*4],FxOpB8
   mov dword [FxTableA1+0B9h*4],FxOpB9
   mov dword [FxTableA1+0BAh*4],FxOpBA
   mov dword [FxTableA1+0BBh*4],FxOpBB
   mov dword [FxTableA1+0BCh*4],FxOpBC
   mov dword [FxTableA1+0BDh*4],FxOpBD
   mov dword [FxTableA1+0BEh*4],FxOpBE
   mov dword [FxTableA1+0BFh*4],FxOpBF

   mov dword [FxTableA1+0C0h*4],FxOpC0
   mov dword [FxTableA1+0C1h*4],FxOpC1A1
   mov dword [FxTableA1+0C2h*4],FxOpC2A1
   mov dword [FxTableA1+0C3h*4],FxOpC3A1
   mov dword [FxTableA1+0C4h*4],FxOpC4A1
   mov dword [FxTableA1+0C5h*4],FxOpC5A1
   mov dword [FxTableA1+0C6h*4],FxOpC6A1
   mov dword [FxTableA1+0C7h*4],FxOpC7A1
   mov dword [FxTableA1+0C8h*4],FxOpC8A1
   mov dword [FxTableA1+0C9h*4],FxOpC9A1
   mov dword [FxTableA1+0CAh*4],FxOpCAA1
   mov dword [FxTableA1+0CBh*4],FxOpCBA1
   mov dword [FxTableA1+0CCh*4],FxOpCCA1
   mov dword [FxTableA1+0CDh*4],FxOpCDA1
   mov dword [FxTableA1+0CEh*4],FxOpCEA1
   mov dword [FxTableA1+0CFh*4],FxOpCFA1

   mov dword [FxTableA1+0D0h*4],FxOpD0
   mov dword [FxTableA1+0D1h*4],FxOpD1
   mov dword [FxTableA1+0D2h*4],FxOpD2
   mov dword [FxTableA1+0D3h*4],FxOpD3
   mov dword [FxTableA1+0D4h*4],FxOpD4
   mov dword [FxTableA1+0D5h*4],FxOpD5
   mov dword [FxTableA1+0D6h*4],FxOpD6
   mov dword [FxTableA1+0D7h*4],FxOpD7
   mov dword [FxTableA1+0D8h*4],FxOpD8
   mov dword [FxTableA1+0D9h*4],FxOpD9
   mov dword [FxTableA1+0DAh*4],FxOpDA
   mov dword [FxTableA1+0DBh*4],FxOpDB
   mov dword [FxTableA1+0DCh*4],FxOpDC
   mov dword [FxTableA1+0DDh*4],FxOpDD
   mov dword [FxTableA1+0DEh*4],FxOpDE
   mov dword [FxTableA1+0DFh*4],FxOpDF

   mov dword [FxTableA1+0E0h*4],FxOpE0
   mov dword [FxTableA1+0E1h*4],FxOpE1
   mov dword [FxTableA1+0E2h*4],FxOpE2
   mov dword [FxTableA1+0E3h*4],FxOpE3
   mov dword [FxTableA1+0E4h*4],FxOpE4
   mov dword [FxTableA1+0E5h*4],FxOpE5
   mov dword [FxTableA1+0E6h*4],FxOpE6
   mov dword [FxTableA1+0E7h*4],FxOpE7
   mov dword [FxTableA1+0E8h*4],FxOpE8
   mov dword [FxTableA1+0E9h*4],FxOpE9
   mov dword [FxTableA1+0EAh*4],FxOpEA
   mov dword [FxTableA1+0EBh*4],FxOpEB
   mov dword [FxTableA1+0ECh*4],FxOpEC
   mov dword [FxTableA1+0EDh*4],FxOpED
   mov dword [FxTableA1+0EEh*4],FxOpEE
   mov dword [FxTableA1+0EFh*4],FxOpEFA1

   mov dword [FxTableA1+0F0h*4],FxOpF0A1
   mov dword [FxTableA1+0F1h*4],FxOpF1A1
   mov dword [FxTableA1+0F2h*4],FxOpF2A1
   mov dword [FxTableA1+0F3h*4],FxOpF3A1
   mov dword [FxTableA1+0F4h*4],FxOpF4A1
   mov dword [FxTableA1+0F5h*4],FxOpF5A1
   mov dword [FxTableA1+0F6h*4],FxOpF6A1
   mov dword [FxTableA1+0F7h*4],FxOpF7A1
   mov dword [FxTableA1+0F8h*4],FxOpF8A1
   mov dword [FxTableA1+0F9h*4],FxOpF9A1
   mov dword [FxTableA1+0FAh*4],FxOpFAA1
   mov dword [FxTableA1+0FBh*4],FxOpFBA1
   mov dword [FxTableA1+0FCh*4],FxOpFCA1
   mov dword [FxTableA1+0FDh*4],FxOpFDA1
   mov dword [FxTableA1+0FEh*4],FxOpFEA1
   mov dword [FxTableA1+0FFh*4],FxOpFFA1


   mov dword [FxTableA2+00h*4],FxOp00
   mov dword [FxTableA2+01h*4],FxOp01
   mov dword [FxTableA2+02h*4],FxOp02
   mov dword [FxTableA2+03h*4],FxOp03
   mov dword [FxTableA2+04h*4],FxOp04
   mov dword [FxTableA2+05h*4],FxOp05
   mov dword [FxTableA2+06h*4],FxOp06
   mov dword [FxTableA2+07h*4],FxOp07
   mov dword [FxTableA2+08h*4],FxOp08
   mov dword [FxTableA2+09h*4],FxOp09
   mov dword [FxTableA2+0Ah*4],FxOp0A
   mov dword [FxTableA2+0Bh*4],FxOp0B
   mov dword [FxTableA2+0Ch*4],FxOp0C
   mov dword [FxTableA2+0Dh*4],FxOp0D
   mov dword [FxTableA2+0Eh*4],FxOp0E
   mov dword [FxTableA2+0Fh*4],FxOp0F

   mov dword [FxTableA2+10h*4],FxOp10
   mov dword [FxTableA2+11h*4],FxOp11
   mov dword [FxTableA2+12h*4],FxOp12
   mov dword [FxTableA2+13h*4],FxOp13
   mov dword [FxTableA2+14h*4],FxOp14
   mov dword [FxTableA2+15h*4],FxOp15
   mov dword [FxTableA2+16h*4],FxOp16
   mov dword [FxTableA2+17h*4],FxOp17
   mov dword [FxTableA2+18h*4],FxOp18
   mov dword [FxTableA2+19h*4],FxOp19
   mov dword [FxTableA2+1Ah*4],FxOp1A
   mov dword [FxTableA2+1Bh*4],FxOp1B
   mov dword [FxTableA2+1Ch*4],FxOp1C
   mov dword [FxTableA2+1Dh*4],FxOp1D
   mov dword [FxTableA2+1Eh*4],FxOp1E
   mov dword [FxTableA2+1Fh*4],FxOp1F

   mov dword [FxTableA2+20h*4],FxOp20
   mov dword [FxTableA2+21h*4],FxOp21
   mov dword [FxTableA2+22h*4],FxOp22
   mov dword [FxTableA2+23h*4],FxOp23
   mov dword [FxTableA2+24h*4],FxOp24
   mov dword [FxTableA2+25h*4],FxOp25
   mov dword [FxTableA2+26h*4],FxOp26
   mov dword [FxTableA2+27h*4],FxOp27
   mov dword [FxTableA2+28h*4],FxOp28
   mov dword [FxTableA2+29h*4],FxOp29
   mov dword [FxTableA2+2Ah*4],FxOp2A
   mov dword [FxTableA2+2Bh*4],FxOp2B
   mov dword [FxTableA2+2Ch*4],FxOp2C
   mov dword [FxTableA2+2Dh*4],FxOp2D
   mov dword [FxTableA2+2Eh*4],FxOp2E
   mov dword [FxTableA2+2Fh*4],FxOp2F

   mov dword [FxTableA2+30h*4],FxOp30
   mov dword [FxTableA2+31h*4],FxOp31
   mov dword [FxTableA2+32h*4],FxOp32
   mov dword [FxTableA2+33h*4],FxOp33
   mov dword [FxTableA2+34h*4],FxOp34
   mov dword [FxTableA2+35h*4],FxOp35
   mov dword [FxTableA2+36h*4],FxOp36
   mov dword [FxTableA2+37h*4],FxOp37
   mov dword [FxTableA2+38h*4],FxOp38
   mov dword [FxTableA2+39h*4],FxOp39
   mov dword [FxTableA2+3Ah*4],FxOp3A
   mov dword [FxTableA2+3Bh*4],FxOp3B
   mov dword [FxTableA2+3Ch*4],FxOp3C
   mov dword [FxTableA2+3Dh*4],FxOp3D
   mov dword [FxTableA2+3Eh*4],FxOp3E
   mov dword [FxTableA2+3Fh*4],FxOp3F

   mov dword [FxTableA2+40h*4],FxOp40
   mov dword [FxTableA2+41h*4],FxOp41
   mov dword [FxTableA2+42h*4],FxOp42
   mov dword [FxTableA2+43h*4],FxOp43
   mov dword [FxTableA2+44h*4],FxOp44
   mov dword [FxTableA2+45h*4],FxOp45
   mov dword [FxTableA2+46h*4],FxOp46
   mov dword [FxTableA2+47h*4],FxOp47
   mov dword [FxTableA2+48h*4],FxOp48
   mov dword [FxTableA2+49h*4],FxOp49
   mov dword [FxTableA2+4Ah*4],FxOp4A
   mov dword [FxTableA2+4Bh*4],FxOp4B
   mov dword [FxTableA2+4Ch*4],FxOp4C
   mov dword [FxTableA2+4Dh*4],FxOp4D
   mov dword [FxTableA2+4Eh*4],FxOp4E
   mov dword [FxTableA2+4Fh*4],FxOp4F

   mov dword [FxTableA2+50h*4],FxOp50A2
   mov dword [FxTableA2+51h*4],FxOp51A2
   mov dword [FxTableA2+52h*4],FxOp52A2
   mov dword [FxTableA2+53h*4],FxOp53A2
   mov dword [FxTableA2+54h*4],FxOp54A2
   mov dword [FxTableA2+55h*4],FxOp55A2
   mov dword [FxTableA2+56h*4],FxOp56A2
   mov dword [FxTableA2+57h*4],FxOp57A2
   mov dword [FxTableA2+58h*4],FxOp58A2
   mov dword [FxTableA2+59h*4],FxOp59A2
   mov dword [FxTableA2+5Ah*4],FxOp5AA2
   mov dword [FxTableA2+5Bh*4],FxOp5BA2
   mov dword [FxTableA2+5Ch*4],FxOp5CA2
   mov dword [FxTableA2+5Dh*4],FxOp5DA2
   mov dword [FxTableA2+5Eh*4],FxOp5EA2
   mov dword [FxTableA2+5Fh*4],FxOp5FA2

   mov dword [FxTableA2+60h*4],FxOp60A2
   mov dword [FxTableA2+61h*4],FxOp61A2
   mov dword [FxTableA2+62h*4],FxOp62A2
   mov dword [FxTableA2+63h*4],FxOp63A2
   mov dword [FxTableA2+64h*4],FxOp64A2
   mov dword [FxTableA2+65h*4],FxOp65A2
   mov dword [FxTableA2+66h*4],FxOp66A2
   mov dword [FxTableA2+67h*4],FxOp67A2
   mov dword [FxTableA2+68h*4],FxOp68A2
   mov dword [FxTableA2+69h*4],FxOp69A2
   mov dword [FxTableA2+6Ah*4],FxOp6AA2
   mov dword [FxTableA2+6Bh*4],FxOp6BA2
   mov dword [FxTableA2+6Ch*4],FxOp6CA2
   mov dword [FxTableA2+6Dh*4],FxOp6DA2
   mov dword [FxTableA2+6Eh*4],FxOp6EA2
   mov dword [FxTableA2+6Fh*4],FxOp6FA2

   mov dword [FxTableA2+70h*4],FxOp70
   mov dword [FxTableA2+71h*4],FxOp71A2
   mov dword [FxTableA2+72h*4],FxOp72A2
   mov dword [FxTableA2+73h*4],FxOp73A2
   mov dword [FxTableA2+74h*4],FxOp74A2
   mov dword [FxTableA2+75h*4],FxOp75A2
   mov dword [FxTableA2+76h*4],FxOp76A2
   mov dword [FxTableA2+77h*4],FxOp77A2
   mov dword [FxTableA2+78h*4],FxOp78A2
   mov dword [FxTableA2+79h*4],FxOp79A2
   mov dword [FxTableA2+7Ah*4],FxOp7AA2
   mov dword [FxTableA2+7Bh*4],FxOp7BA2
   mov dword [FxTableA2+7Ch*4],FxOp7CA2
   mov dword [FxTableA2+7Dh*4],FxOp7DA2
   mov dword [FxTableA2+7Eh*4],FxOp7EA2
   mov dword [FxTableA2+7Fh*4],FxOp7FA2

   mov dword [FxTableA2+80h*4],FxOp80A2
   mov dword [FxTableA2+81h*4],FxOp81A2
   mov dword [FxTableA2+82h*4],FxOp82A2
   mov dword [FxTableA2+83h*4],FxOp83A2
   mov dword [FxTableA2+84h*4],FxOp84A2
   mov dword [FxTableA2+85h*4],FxOp85A2
   mov dword [FxTableA2+86h*4],FxOp86A2
   mov dword [FxTableA2+87h*4],FxOp87A2
   mov dword [FxTableA2+88h*4],FxOp88A2
   mov dword [FxTableA2+89h*4],FxOp89A2
   mov dword [FxTableA2+8Ah*4],FxOp8AA2
   mov dword [FxTableA2+8Bh*4],FxOp8BA2
   mov dword [FxTableA2+8Ch*4],FxOp8CA2
   mov dword [FxTableA2+8Dh*4],FxOp8DA2
   mov dword [FxTableA2+8Eh*4],FxOp8EA2
   mov dword [FxTableA2+8Fh*4],FxOp8FA2

   mov dword [FxTableA2+90h*4],FxOp90
   mov dword [FxTableA2+91h*4],FxOp91
   mov dword [FxTableA2+92h*4],FxOp92
   mov dword [FxTableA2+93h*4],FxOp93
   mov dword [FxTableA2+94h*4],FxOp94
   mov dword [FxTableA2+95h*4],FxOp95
   mov dword [FxTableA2+96h*4],FxOp96
   mov dword [FxTableA2+97h*4],FxOp97
   mov dword [FxTableA2+98h*4],FxOp98
   mov dword [FxTableA2+99h*4],FxOp99
   mov dword [FxTableA2+9Ah*4],FxOp9A
   mov dword [FxTableA2+9Bh*4],FxOp9B
   mov dword [FxTableA2+9Ch*4],FxOp9C
   mov dword [FxTableA2+9Dh*4],FxOp9D
   mov dword [FxTableA2+9Eh*4],FxOp9E
   mov dword [FxTableA2+9Fh*4],FxOp9F

   mov dword [FxTableA2+0A0h*4],FxOpA0A2
   mov dword [FxTableA2+0A1h*4],FxOpA1A2
   mov dword [FxTableA2+0A2h*4],FxOpA2A2
   mov dword [FxTableA2+0A3h*4],FxOpA3A2
   mov dword [FxTableA2+0A4h*4],FxOpA4A2
   mov dword [FxTableA2+0A5h*4],FxOpA5A2
   mov dword [FxTableA2+0A6h*4],FxOpA6A2
   mov dword [FxTableA2+0A7h*4],FxOpA7A2
   mov dword [FxTableA2+0A8h*4],FxOpA8A2
   mov dword [FxTableA2+0A9h*4],FxOpA9A2
   mov dword [FxTableA2+0AAh*4],FxOpAAA2
   mov dword [FxTableA2+0ABh*4],FxOpABA2
   mov dword [FxTableA2+0ACh*4],FxOpACA2
   mov dword [FxTableA2+0ADh*4],FxOpADA2
   mov dword [FxTableA2+0AEh*4],FxOpAEA2
   mov dword [FxTableA2+0AFh*4],FxOpAFA2

   mov dword [FxTableA2+0B0h*4],FxOpB0
   mov dword [FxTableA2+0B1h*4],FxOpB1
   mov dword [FxTableA2+0B2h*4],FxOpB2
   mov dword [FxTableA2+0B3h*4],FxOpB3
   mov dword [FxTableA2+0B4h*4],FxOpB4
   mov dword [FxTableA2+0B5h*4],FxOpB5
   mov dword [FxTableA2+0B6h*4],FxOpB6
   mov dword [FxTableA2+0B7h*4],FxOpB7
   mov dword [FxTableA2+0B8h*4],FxOpB8
   mov dword [FxTableA2+0B9h*4],FxOpB9
   mov dword [FxTableA2+0BAh*4],FxOpBA
   mov dword [FxTableA2+0BBh*4],FxOpBB
   mov dword [FxTableA2+0BCh*4],FxOpBC
   mov dword [FxTableA2+0BDh*4],FxOpBD
   mov dword [FxTableA2+0BEh*4],FxOpBE
   mov dword [FxTableA2+0BFh*4],FxOpBF

   mov dword [FxTableA2+0C0h*4],FxOpC0
   mov dword [FxTableA2+0C1h*4],FxOpC1A2
   mov dword [FxTableA2+0C2h*4],FxOpC2A2
   mov dword [FxTableA2+0C3h*4],FxOpC3A2
   mov dword [FxTableA2+0C4h*4],FxOpC4A2
   mov dword [FxTableA2+0C5h*4],FxOpC5A2
   mov dword [FxTableA2+0C6h*4],FxOpC6A2
   mov dword [FxTableA2+0C7h*4],FxOpC7A2
   mov dword [FxTableA2+0C8h*4],FxOpC8A2
   mov dword [FxTableA2+0C9h*4],FxOpC9A2
   mov dword [FxTableA2+0CAh*4],FxOpCAA2
   mov dword [FxTableA2+0CBh*4],FxOpCBA2
   mov dword [FxTableA2+0CCh*4],FxOpCCA2
   mov dword [FxTableA2+0CDh*4],FxOpCDA2
   mov dword [FxTableA2+0CEh*4],FxOpCEA2
   mov dword [FxTableA2+0CFh*4],FxOpCFA2

   mov dword [FxTableA2+0D0h*4],FxOpD0
   mov dword [FxTableA2+0D1h*4],FxOpD1
   mov dword [FxTableA2+0D2h*4],FxOpD2
   mov dword [FxTableA2+0D3h*4],FxOpD3
   mov dword [FxTableA2+0D4h*4],FxOpD4
   mov dword [FxTableA2+0D5h*4],FxOpD5
   mov dword [FxTableA2+0D6h*4],FxOpD6
   mov dword [FxTableA2+0D7h*4],FxOpD7
   mov dword [FxTableA2+0D8h*4],FxOpD8
   mov dword [FxTableA2+0D9h*4],FxOpD9
   mov dword [FxTableA2+0DAh*4],FxOpDA
   mov dword [FxTableA2+0DBh*4],FxOpDB
   mov dword [FxTableA2+0DCh*4],FxOpDC
   mov dword [FxTableA2+0DDh*4],FxOpDD
   mov dword [FxTableA2+0DEh*4],FxOpDE
   mov dword [FxTableA2+0DFh*4],FxOpDFA2

   mov dword [FxTableA2+0E0h*4],FxOpE0
   mov dword [FxTableA2+0E1h*4],FxOpE1
   mov dword [FxTableA2+0E2h*4],FxOpE2
   mov dword [FxTableA2+0E3h*4],FxOpE3
   mov dword [FxTableA2+0E4h*4],FxOpE4
   mov dword [FxTableA2+0E5h*4],FxOpE5
   mov dword [FxTableA2+0E6h*4],FxOpE6
   mov dword [FxTableA2+0E7h*4],FxOpE7
   mov dword [FxTableA2+0E8h*4],FxOpE8
   mov dword [FxTableA2+0E9h*4],FxOpE9
   mov dword [FxTableA2+0EAh*4],FxOpEA
   mov dword [FxTableA2+0EBh*4],FxOpEB
   mov dword [FxTableA2+0ECh*4],FxOpEC
   mov dword [FxTableA2+0EDh*4],FxOpED
   mov dword [FxTableA2+0EEh*4],FxOpEE
   mov dword [FxTableA2+0EFh*4],FxOpEFA2

   mov dword [FxTableA2+0F0h*4],FxOpF0A2
   mov dword [FxTableA2+0F1h*4],FxOpF1A2
   mov dword [FxTableA2+0F2h*4],FxOpF2A2
   mov dword [FxTableA2+0F3h*4],FxOpF3A2
   mov dword [FxTableA2+0F4h*4],FxOpF4A2
   mov dword [FxTableA2+0F5h*4],FxOpF5A2
   mov dword [FxTableA2+0F6h*4],FxOpF6A2
   mov dword [FxTableA2+0F7h*4],FxOpF7A2
   mov dword [FxTableA2+0F8h*4],FxOpF8A2
   mov dword [FxTableA2+0F9h*4],FxOpF9A2
   mov dword [FxTableA2+0FAh*4],FxOpFAA2
   mov dword [FxTableA2+0FBh*4],FxOpFBA2
   mov dword [FxTableA2+0FCh*4],FxOpFCA2
   mov dword [FxTableA2+0FDh*4],FxOpFDA2
   mov dword [FxTableA2+0FEh*4],FxOpFEA2
   mov dword [FxTableA2+0FFh*4],FxOpFFA2

   mov dword [FxTableA3+00h*4],FxOp00
   mov dword [FxTableA3+01h*4],FxOp01
   mov dword [FxTableA3+02h*4],FxOp02
   mov dword [FxTableA3+03h*4],FxOp03
   mov dword [FxTableA3+04h*4],FxOp04
   mov dword [FxTableA3+05h*4],FxOp05
   mov dword [FxTableA3+06h*4],FxOp06
   mov dword [FxTableA3+07h*4],FxOp07
   mov dword [FxTableA3+08h*4],FxOp08
   mov dword [FxTableA3+09h*4],FxOp09
   mov dword [FxTableA3+0Ah*4],FxOp0A
   mov dword [FxTableA3+0Bh*4],FxOp0B
   mov dword [FxTableA3+0Ch*4],FxOp0C
   mov dword [FxTableA3+0Dh*4],FxOp0D
   mov dword [FxTableA3+0Eh*4],FxOp0E
   mov dword [FxTableA3+0Fh*4],FxOp0F

   mov dword [FxTableA3+10h*4],FxOp10
   mov dword [FxTableA3+11h*4],FxOp11
   mov dword [FxTableA3+12h*4],FxOp12
   mov dword [FxTableA3+13h*4],FxOp13
   mov dword [FxTableA3+14h*4],FxOp14
   mov dword [FxTableA3+15h*4],FxOp15
   mov dword [FxTableA3+16h*4],FxOp16
   mov dword [FxTableA3+17h*4],FxOp17
   mov dword [FxTableA3+18h*4],FxOp18
   mov dword [FxTableA3+19h*4],FxOp19
   mov dword [FxTableA3+1Ah*4],FxOp1A
   mov dword [FxTableA3+1Bh*4],FxOp1B
   mov dword [FxTableA3+1Ch*4],FxOp1C
   mov dword [FxTableA3+1Dh*4],FxOp1D
   mov dword [FxTableA3+1Eh*4],FxOp1E
   mov dword [FxTableA3+1Fh*4],FxOp1F

   mov dword [FxTableA3+20h*4],FxOp20
   mov dword [FxTableA3+21h*4],FxOp21
   mov dword [FxTableA3+22h*4],FxOp22
   mov dword [FxTableA3+23h*4],FxOp23
   mov dword [FxTableA3+24h*4],FxOp24
   mov dword [FxTableA3+25h*4],FxOp25
   mov dword [FxTableA3+26h*4],FxOp26
   mov dword [FxTableA3+27h*4],FxOp27
   mov dword [FxTableA3+28h*4],FxOp28
   mov dword [FxTableA3+29h*4],FxOp29
   mov dword [FxTableA3+2Ah*4],FxOp2A
   mov dword [FxTableA3+2Bh*4],FxOp2B
   mov dword [FxTableA3+2Ch*4],FxOp2C
   mov dword [FxTableA3+2Dh*4],FxOp2D
   mov dword [FxTableA3+2Eh*4],FxOp2E
   mov dword [FxTableA3+2Fh*4],FxOp2F

   mov dword [FxTableA3+30h*4],FxOp30
   mov dword [FxTableA3+31h*4],FxOp31
   mov dword [FxTableA3+32h*4],FxOp32
   mov dword [FxTableA3+33h*4],FxOp33
   mov dword [FxTableA3+34h*4],FxOp34
   mov dword [FxTableA3+35h*4],FxOp35
   mov dword [FxTableA3+36h*4],FxOp36
   mov dword [FxTableA3+37h*4],FxOp37
   mov dword [FxTableA3+38h*4],FxOp38
   mov dword [FxTableA3+39h*4],FxOp39
   mov dword [FxTableA3+3Ah*4],FxOp3A
   mov dword [FxTableA3+3Bh*4],FxOp3B
   mov dword [FxTableA3+3Ch*4],FxOp3C
   mov dword [FxTableA3+3Dh*4],FxOp3D
   mov dword [FxTableA3+3Eh*4],FxOp3E
   mov dword [FxTableA3+3Fh*4],FxOp3F

   mov dword [FxTableA3+40h*4],FxOp40
   mov dword [FxTableA3+41h*4],FxOp41
   mov dword [FxTableA3+42h*4],FxOp42
   mov dword [FxTableA3+43h*4],FxOp43
   mov dword [FxTableA3+44h*4],FxOp44
   mov dword [FxTableA3+45h*4],FxOp45
   mov dword [FxTableA3+46h*4],FxOp46
   mov dword [FxTableA3+47h*4],FxOp47
   mov dword [FxTableA3+48h*4],FxOp48
   mov dword [FxTableA3+49h*4],FxOp49
   mov dword [FxTableA3+4Ah*4],FxOp4A
   mov dword [FxTableA3+4Bh*4],FxOp4B
   mov dword [FxTableA3+4Ch*4],FxOp4C
   mov dword [FxTableA3+4Dh*4],FxOp4D
   mov dword [FxTableA3+4Eh*4],FxOp4E
   mov dword [FxTableA3+4Fh*4],FxOp4F

   mov dword [FxTableA3+50h*4],FxOp50A3
   mov dword [FxTableA3+51h*4],FxOp51A3
   mov dword [FxTableA3+52h*4],FxOp52A3
   mov dword [FxTableA3+53h*4],FxOp53A3
   mov dword [FxTableA3+54h*4],FxOp54A3
   mov dword [FxTableA3+55h*4],FxOp55A3
   mov dword [FxTableA3+56h*4],FxOp56A3
   mov dword [FxTableA3+57h*4],FxOp57A3
   mov dword [FxTableA3+58h*4],FxOp58A3
   mov dword [FxTableA3+59h*4],FxOp59A3
   mov dword [FxTableA3+5Ah*4],FxOp5AA3
   mov dword [FxTableA3+5Bh*4],FxOp5BA3
   mov dword [FxTableA3+5Ch*4],FxOp5CA3
   mov dword [FxTableA3+5Dh*4],FxOp5DA3
   mov dword [FxTableA3+5Eh*4],FxOp5EA3
   mov dword [FxTableA3+5Fh*4],FxOp5FA3

   mov dword [FxTableA3+60h*4],FxOp60A3
   mov dword [FxTableA3+61h*4],FxOp61A3
   mov dword [FxTableA3+62h*4],FxOp62A3
   mov dword [FxTableA3+63h*4],FxOp63A3
   mov dword [FxTableA3+64h*4],FxOp64A3
   mov dword [FxTableA3+65h*4],FxOp65A3
   mov dword [FxTableA3+66h*4],FxOp66A3
   mov dword [FxTableA3+67h*4],FxOp67A3
   mov dword [FxTableA3+68h*4],FxOp68A3
   mov dword [FxTableA3+69h*4],FxOp69A3
   mov dword [FxTableA3+6Ah*4],FxOp6AA3
   mov dword [FxTableA3+6Bh*4],FxOp6BA3
   mov dword [FxTableA3+6Ch*4],FxOp6CA3
   mov dword [FxTableA3+6Dh*4],FxOp6DA3
   mov dword [FxTableA3+6Eh*4],FxOp6EA3
   mov dword [FxTableA3+6Fh*4],FxOp6FA3

   mov dword [FxTableA3+70h*4],FxOp70
   mov dword [FxTableA3+71h*4],FxOp71A3
   mov dword [FxTableA3+72h*4],FxOp72A3
   mov dword [FxTableA3+73h*4],FxOp73A3
   mov dword [FxTableA3+74h*4],FxOp74A3
   mov dword [FxTableA3+75h*4],FxOp75A3
   mov dword [FxTableA3+76h*4],FxOp76A3
   mov dword [FxTableA3+77h*4],FxOp77A3
   mov dword [FxTableA3+78h*4],FxOp78A3
   mov dword [FxTableA3+79h*4],FxOp79A3
   mov dword [FxTableA3+7Ah*4],FxOp7AA3
   mov dword [FxTableA3+7Bh*4],FxOp7BA3
   mov dword [FxTableA3+7Ch*4],FxOp7CA3
   mov dword [FxTableA3+7Dh*4],FxOp7DA3
   mov dword [FxTableA3+7Eh*4],FxOp7EA3
   mov dword [FxTableA3+7Fh*4],FxOp7FA3

   mov dword [FxTableA3+80h*4],FxOp80A3
   mov dword [FxTableA3+81h*4],FxOp81A3
   mov dword [FxTableA3+82h*4],FxOp82A3
   mov dword [FxTableA3+83h*4],FxOp83A3
   mov dword [FxTableA3+84h*4],FxOp84A3
   mov dword [FxTableA3+85h*4],FxOp85A3
   mov dword [FxTableA3+86h*4],FxOp86A3
   mov dword [FxTableA3+87h*4],FxOp87A3
   mov dword [FxTableA3+88h*4],FxOp88A3
   mov dword [FxTableA3+89h*4],FxOp89A3
   mov dword [FxTableA3+8Ah*4],FxOp8AA3
   mov dword [FxTableA3+8Bh*4],FxOp8BA3
   mov dword [FxTableA3+8Ch*4],FxOp8CA3
   mov dword [FxTableA3+8Dh*4],FxOp8DA3
   mov dword [FxTableA3+8Eh*4],FxOp8EA3
   mov dword [FxTableA3+8Fh*4],FxOp8FA3

   mov dword [FxTableA3+90h*4],FxOp90
   mov dword [FxTableA3+91h*4],FxOp91
   mov dword [FxTableA3+92h*4],FxOp92
   mov dword [FxTableA3+93h*4],FxOp93
   mov dword [FxTableA3+94h*4],FxOp94
   mov dword [FxTableA3+95h*4],FxOp95
   mov dword [FxTableA3+96h*4],FxOp96
   mov dword [FxTableA3+97h*4],FxOp97
   mov dword [FxTableA3+98h*4],FxOp98
   mov dword [FxTableA3+99h*4],FxOp99
   mov dword [FxTableA3+9Ah*4],FxOp9A
   mov dword [FxTableA3+9Bh*4],FxOp9B
   mov dword [FxTableA3+9Ch*4],FxOp9C
   mov dword [FxTableA3+9Dh*4],FxOp9D
   mov dword [FxTableA3+9Eh*4],FxOp9E
   mov dword [FxTableA3+9Fh*4],FxOp9F

   mov dword [FxTableA3+0A0h*4],FxOpA0
   mov dword [FxTableA3+0A1h*4],FxOpA1
   mov dword [FxTableA3+0A2h*4],FxOpA2
   mov dword [FxTableA3+0A3h*4],FxOpA3
   mov dword [FxTableA3+0A4h*4],FxOpA4
   mov dword [FxTableA3+0A5h*4],FxOpA5
   mov dword [FxTableA3+0A6h*4],FxOpA6
   mov dword [FxTableA3+0A7h*4],FxOpA7
   mov dword [FxTableA3+0A8h*4],FxOpA8
   mov dword [FxTableA3+0A9h*4],FxOpA9
   mov dword [FxTableA3+0AAh*4],FxOpAA
   mov dword [FxTableA3+0ABh*4],FxOpAB
   mov dword [FxTableA3+0ACh*4],FxOpAC
   mov dword [FxTableA3+0ADh*4],FxOpAD
   mov dword [FxTableA3+0AEh*4],FxOpAE
   mov dword [FxTableA3+0AFh*4],FxOpAF

   mov dword [FxTableA3+0B0h*4],FxOpB0
   mov dword [FxTableA3+0B1h*4],FxOpB1
   mov dword [FxTableA3+0B2h*4],FxOpB2
   mov dword [FxTableA3+0B3h*4],FxOpB3
   mov dword [FxTableA3+0B4h*4],FxOpB4
   mov dword [FxTableA3+0B5h*4],FxOpB5
   mov dword [FxTableA3+0B6h*4],FxOpB6
   mov dword [FxTableA3+0B7h*4],FxOpB7
   mov dword [FxTableA3+0B8h*4],FxOpB8
   mov dword [FxTableA3+0B9h*4],FxOpB9
   mov dword [FxTableA3+0BAh*4],FxOpBA
   mov dword [FxTableA3+0BBh*4],FxOpBB
   mov dword [FxTableA3+0BCh*4],FxOpBC
   mov dword [FxTableA3+0BDh*4],FxOpBD
   mov dword [FxTableA3+0BEh*4],FxOpBE
   mov dword [FxTableA3+0BFh*4],FxOpBF

   mov dword [FxTableA3+0C0h*4],FxOpC0
   mov dword [FxTableA3+0C1h*4],FxOpC1A3
   mov dword [FxTableA3+0C2h*4],FxOpC2A3
   mov dword [FxTableA3+0C3h*4],FxOpC3A3
   mov dword [FxTableA3+0C4h*4],FxOpC4A3
   mov dword [FxTableA3+0C5h*4],FxOpC5A3
   mov dword [FxTableA3+0C6h*4],FxOpC6A3
   mov dword [FxTableA3+0C7h*4],FxOpC7A3
   mov dword [FxTableA3+0C8h*4],FxOpC8A3
   mov dword [FxTableA3+0C9h*4],FxOpC9A3
   mov dword [FxTableA3+0CAh*4],FxOpCAA3
   mov dword [FxTableA3+0CBh*4],FxOpCBA3
   mov dword [FxTableA3+0CCh*4],FxOpCCA3
   mov dword [FxTableA3+0CDh*4],FxOpCDA3
   mov dword [FxTableA3+0CEh*4],FxOpCEA3
   mov dword [FxTableA3+0CFh*4],FxOpCFA3

   mov dword [FxTableA3+0D0h*4],FxOpD0
   mov dword [FxTableA3+0D1h*4],FxOpD1
   mov dword [FxTableA3+0D2h*4],FxOpD2
   mov dword [FxTableA3+0D3h*4],FxOpD3
   mov dword [FxTableA3+0D4h*4],FxOpD4
   mov dword [FxTableA3+0D5h*4],FxOpD5
   mov dword [FxTableA3+0D6h*4],FxOpD6
   mov dword [FxTableA3+0D7h*4],FxOpD7
   mov dword [FxTableA3+0D8h*4],FxOpD8
   mov dword [FxTableA3+0D9h*4],FxOpD9
   mov dword [FxTableA3+0DAh*4],FxOpDA
   mov dword [FxTableA3+0DBh*4],FxOpDB
   mov dword [FxTableA3+0DCh*4],FxOpDC
   mov dword [FxTableA3+0DDh*4],FxOpDD
   mov dword [FxTableA3+0DEh*4],FxOpDE
   mov dword [FxTableA3+0DFh*4],FxOpDFA3

   mov dword [FxTableA3+0E0h*4],FxOpE0
   mov dword [FxTableA3+0E1h*4],FxOpE1
   mov dword [FxTableA3+0E2h*4],FxOpE2
   mov dword [FxTableA3+0E3h*4],FxOpE3
   mov dword [FxTableA3+0E4h*4],FxOpE4
   mov dword [FxTableA3+0E5h*4],FxOpE5
   mov dword [FxTableA3+0E6h*4],FxOpE6
   mov dword [FxTableA3+0E7h*4],FxOpE7
   mov dword [FxTableA3+0E8h*4],FxOpE8
   mov dword [FxTableA3+0E9h*4],FxOpE9
   mov dword [FxTableA3+0EAh*4],FxOpEA
   mov dword [FxTableA3+0EBh*4],FxOpEB
   mov dword [FxTableA3+0ECh*4],FxOpEC
   mov dword [FxTableA3+0EDh*4],FxOpED
   mov dword [FxTableA3+0EEh*4],FxOpEE
   mov dword [FxTableA3+0EFh*4],FxOpEFA3

   mov dword [FxTableA3+0F0h*4],FxOpF0
   mov dword [FxTableA3+0F1h*4],FxOpF1
   mov dword [FxTableA3+0F2h*4],FxOpF2
   mov dword [FxTableA3+0F3h*4],FxOpF3
   mov dword [FxTableA3+0F4h*4],FxOpF4
   mov dword [FxTableA3+0F5h*4],FxOpF5
   mov dword [FxTableA3+0F6h*4],FxOpF6
   mov dword [FxTableA3+0F7h*4],FxOpF7
   mov dword [FxTableA3+0F8h*4],FxOpF8
   mov dword [FxTableA3+0F9h*4],FxOpF9
   mov dword [FxTableA3+0FAh*4],FxOpFA
   mov dword [FxTableA3+0FBh*4],FxOpFB
   mov dword [FxTableA3+0FCh*4],FxOpFC
   mov dword [FxTableA3+0FDh*4],FxOpFD
   mov dword [FxTableA3+0FEh*4],FxOpFE
   mov dword [FxTableA3+0FFh*4],FxOpFF

   ; Transfer FxTable to FxTableb and FxTablec
   mov esi,FxTable
   mov edi,FxTableb
   mov ecx,256*4
   rep movsd
   mov esi,FxTable
   mov edi,FxTablec
   mov ecx,256*4
   rep movsd

   mov dword [FxTableb+05h*4],FxOpb05
   mov dword [FxTableb+06h*4],FxOpb06
   mov dword [FxTableb+07h*4],FxOpb07
   mov dword [FxTableb+08h*4],FxOpb08
   mov dword [FxTableb+09h*4],FxOpb09
   mov dword [FxTableb+0Ah*4],FxOpb0A
   mov dword [FxTableb+0Bh*4],FxOpb0B
   mov dword [FxTableb+0Ch*4],FxOpb0C
   mov dword [FxTableb+0Dh*4],FxOpb0D
   mov dword [FxTableb+0Eh*4],FxOpb0E
   mov dword [FxTableb+0Fh*4],FxOpb0F
   mov dword [FxTableb+10h*4],FxOpb10
   mov dword [FxTableb+11h*4],FxOpb11
   mov dword [FxTableb+12h*4],FxOpb12
   mov dword [FxTableb+13h*4],FxOpb13
   mov dword [FxTableb+14h*4],FxOpb14
   mov dword [FxTableb+15h*4],FxOpb15
   mov dword [FxTableb+16h*4],FxOpb16
   mov dword [FxTableb+17h*4],FxOpb17
   mov dword [FxTableb+18h*4],FxOpb18
   mov dword [FxTableb+19h*4],FxOpb19
   mov dword [FxTableb+1Ah*4],FxOpb1A
   mov dword [FxTableb+1Bh*4],FxOpb1B
   mov dword [FxTableb+1Ch*4],FxOpb1C
   mov dword [FxTableb+1Dh*4],FxOpb1D
   mov dword [FxTableb+1Eh*4],FxOpb1E
   mov dword [FxTableb+1Fh*4],FxOpb1F
   mov dword [FxTableb+3Dh*4],FxOpb3D
   mov dword [FxTableb+3Eh*4],FxOpb3E
   mov dword [FxTableb+3Fh*4],FxOpb3F
   mov dword [FxTableb+0B0h*4],FxOpbB0
   mov dword [FxTableb+0B1h*4],FxOpbB1
   mov dword [FxTableb+0B2h*4],FxOpbB2
   mov dword [FxTableb+0B3h*4],FxOpbB3
   mov dword [FxTableb+0B4h*4],FxOpbB4
   mov dword [FxTableb+0B5h*4],FxOpbB5
   mov dword [FxTableb+0B6h*4],FxOpbB6
   mov dword [FxTableb+0B7h*4],FxOpbB7
   mov dword [FxTableb+0B8h*4],FxOpbB8
   mov dword [FxTableb+0B9h*4],FxOpbB9
   mov dword [FxTableb+0BAh*4],FxOpbBA
   mov dword [FxTableb+0BBh*4],FxOpbBB
   mov dword [FxTableb+0BCh*4],FxOpbBC
   mov dword [FxTableb+0BDh*4],FxOpbBD
   mov dword [FxTableb+0BEh*4],FxOpbBE
   mov dword [FxTableb+0BFh*4],FxOpbBF

   mov dword [FxTablebA1+05h*4],FxOpb05
   mov dword [FxTablebA1+06h*4],FxOpb06
   mov dword [FxTablebA1+07h*4],FxOpb07
   mov dword [FxTablebA1+08h*4],FxOpb08
   mov dword [FxTablebA1+09h*4],FxOpb09
   mov dword [FxTablebA1+0Ah*4],FxOpb0A
   mov dword [FxTablebA1+0Bh*4],FxOpb0B
   mov dword [FxTablebA1+0Ch*4],FxOpb0C
   mov dword [FxTablebA1+0Dh*4],FxOpb0D
   mov dword [FxTablebA1+0Eh*4],FxOpb0E
   mov dword [FxTablebA1+0Fh*4],FxOpb0F
   mov dword [FxTablebA1+10h*4],FxOpb10
   mov dword [FxTablebA1+11h*4],FxOpb11
   mov dword [FxTablebA1+12h*4],FxOpb12
   mov dword [FxTablebA1+13h*4],FxOpb13
   mov dword [FxTablebA1+14h*4],FxOpb14
   mov dword [FxTablebA1+15h*4],FxOpb15
   mov dword [FxTablebA1+16h*4],FxOpb16
   mov dword [FxTablebA1+17h*4],FxOpb17
   mov dword [FxTablebA1+18h*4],FxOpb18
   mov dword [FxTablebA1+19h*4],FxOpb19
   mov dword [FxTablebA1+1Ah*4],FxOpb1A
   mov dword [FxTablebA1+1Bh*4],FxOpb1B
   mov dword [FxTablebA1+1Ch*4],FxOpb1C
   mov dword [FxTablebA1+1Dh*4],FxOpb1D
   mov dword [FxTablebA1+1Eh*4],FxOpb1E
   mov dword [FxTablebA1+1Fh*4],FxOpb1F
   mov dword [FxTablebA1+3Dh*4],FxOpb3D
   mov dword [FxTablebA1+3Eh*4],FxOpb3E
   mov dword [FxTablebA1+3Fh*4],FxOpb3F
   mov dword [FxTablebA1+0B0h*4],FxOpbB0
   mov dword [FxTablebA1+0B1h*4],FxOpbB1
   mov dword [FxTablebA1+0B2h*4],FxOpbB2
   mov dword [FxTablebA1+0B3h*4],FxOpbB3
   mov dword [FxTablebA1+0B4h*4],FxOpbB4
   mov dword [FxTablebA1+0B5h*4],FxOpbB5
   mov dword [FxTablebA1+0B6h*4],FxOpbB6
   mov dword [FxTablebA1+0B7h*4],FxOpbB7
   mov dword [FxTablebA1+0B8h*4],FxOpbB8
   mov dword [FxTablebA1+0B9h*4],FxOpbB9
   mov dword [FxTablebA1+0BAh*4],FxOpbBA
   mov dword [FxTablebA1+0BBh*4],FxOpbBB
   mov dword [FxTablebA1+0BCh*4],FxOpbBC
   mov dword [FxTablebA1+0BDh*4],FxOpbBD
   mov dword [FxTablebA1+0BEh*4],FxOpbBE
   mov dword [FxTablebA1+0BFh*4],FxOpbBF

   mov dword [FxTablebA2+05h*4],FxOpb05
   mov dword [FxTablebA2+06h*4],FxOpb06
   mov dword [FxTablebA2+07h*4],FxOpb07
   mov dword [FxTablebA2+08h*4],FxOpb08
   mov dword [FxTablebA2+09h*4],FxOpb09
   mov dword [FxTablebA2+0Ah*4],FxOpb0A
   mov dword [FxTablebA2+0Bh*4],FxOpb0B
   mov dword [FxTablebA2+0Ch*4],FxOpb0C
   mov dword [FxTablebA2+0Dh*4],FxOpb0D
   mov dword [FxTablebA2+0Eh*4],FxOpb0E
   mov dword [FxTablebA2+0Fh*4],FxOpb0F
   mov dword [FxTablebA2+10h*4],FxOpb10
   mov dword [FxTablebA2+11h*4],FxOpb11
   mov dword [FxTablebA2+12h*4],FxOpb12
   mov dword [FxTablebA2+13h*4],FxOpb13
   mov dword [FxTablebA2+14h*4],FxOpb14
   mov dword [FxTablebA2+15h*4],FxOpb15
   mov dword [FxTablebA2+16h*4],FxOpb16
   mov dword [FxTablebA2+17h*4],FxOpb17
   mov dword [FxTablebA2+18h*4],FxOpb18
   mov dword [FxTablebA2+19h*4],FxOpb19
   mov dword [FxTablebA2+1Ah*4],FxOpb1A
   mov dword [FxTablebA2+1Bh*4],FxOpb1B
   mov dword [FxTablebA2+1Ch*4],FxOpb1C
   mov dword [FxTablebA2+1Dh*4],FxOpb1D
   mov dword [FxTablebA2+1Eh*4],FxOpb1E
   mov dword [FxTablebA2+1Fh*4],FxOpb1F
   mov dword [FxTablebA2+3Dh*4],FxOpb3D
   mov dword [FxTablebA2+3Eh*4],FxOpb3E
   mov dword [FxTablebA2+3Fh*4],FxOpb3F
   mov dword [FxTablebA2+0B0h*4],FxOpbB0
   mov dword [FxTablebA2+0B1h*4],FxOpbB1
   mov dword [FxTablebA2+0B2h*4],FxOpbB2
   mov dword [FxTablebA2+0B3h*4],FxOpbB3
   mov dword [FxTablebA2+0B4h*4],FxOpbB4
   mov dword [FxTablebA2+0B5h*4],FxOpbB5
   mov dword [FxTablebA2+0B6h*4],FxOpbB6
   mov dword [FxTablebA2+0B7h*4],FxOpbB7
   mov dword [FxTablebA2+0B8h*4],FxOpbB8
   mov dword [FxTablebA2+0B9h*4],FxOpbB9
   mov dword [FxTablebA2+0BAh*4],FxOpbBA
   mov dword [FxTablebA2+0BBh*4],FxOpbBB
   mov dword [FxTablebA2+0BCh*4],FxOpbBC
   mov dword [FxTablebA2+0BDh*4],FxOpbBD
   mov dword [FxTablebA2+0BEh*4],FxOpbBE
   mov dword [FxTablebA2+0BFh*4],FxOpbBF

   mov dword [FxTablebA3+05h*4],FxOpb05
   mov dword [FxTablebA3+06h*4],FxOpb06
   mov dword [FxTablebA3+07h*4],FxOpb07
   mov dword [FxTablebA3+08h*4],FxOpb08
   mov dword [FxTablebA3+09h*4],FxOpb09
   mov dword [FxTablebA3+0Ah*4],FxOpb0A
   mov dword [FxTablebA3+0Bh*4],FxOpb0B
   mov dword [FxTablebA3+0Ch*4],FxOpb0C
   mov dword [FxTablebA3+0Dh*4],FxOpb0D
   mov dword [FxTablebA3+0Eh*4],FxOpb0E
   mov dword [FxTablebA3+0Fh*4],FxOpb0F
   mov dword [FxTablebA3+10h*4],FxOpb10
   mov dword [FxTablebA3+11h*4],FxOpb11
   mov dword [FxTablebA3+12h*4],FxOpb12
   mov dword [FxTablebA3+13h*4],FxOpb13
   mov dword [FxTablebA3+14h*4],FxOpb14
   mov dword [FxTablebA3+15h*4],FxOpb15
   mov dword [FxTablebA3+16h*4],FxOpb16
   mov dword [FxTablebA3+17h*4],FxOpb17
   mov dword [FxTablebA3+18h*4],FxOpb18
   mov dword [FxTablebA3+19h*4],FxOpb19
   mov dword [FxTablebA3+1Ah*4],FxOpb1A
   mov dword [FxTablebA3+1Bh*4],FxOpb1B
   mov dword [FxTablebA3+1Ch*4],FxOpb1C
   mov dword [FxTablebA3+1Dh*4],FxOpb1D
   mov dword [FxTablebA3+1Eh*4],FxOpb1E
   mov dword [FxTablebA3+1Fh*4],FxOpb1F
   mov dword [FxTablebA3+3Dh*4],FxOpb3D
   mov dword [FxTablebA3+3Eh*4],FxOpb3E
   mov dword [FxTablebA3+3Fh*4],FxOpb3F
   mov dword [FxTablebA3+0B0h*4],FxOpbB0
   mov dword [FxTablebA3+0B1h*4],FxOpbB1
   mov dword [FxTablebA3+0B2h*4],FxOpbB2
   mov dword [FxTablebA3+0B3h*4],FxOpbB3
   mov dword [FxTablebA3+0B4h*4],FxOpbB4
   mov dword [FxTablebA3+0B5h*4],FxOpbB5
   mov dword [FxTablebA3+0B6h*4],FxOpbB6
   mov dword [FxTablebA3+0B7h*4],FxOpbB7
   mov dword [FxTablebA3+0B8h*4],FxOpbB8
   mov dword [FxTablebA3+0B9h*4],FxOpbB9
   mov dword [FxTablebA3+0BAh*4],FxOpbBA
   mov dword [FxTablebA3+0BBh*4],FxOpbBB
   mov dword [FxTablebA3+0BCh*4],FxOpbBC
   mov dword [FxTablebA3+0BDh*4],FxOpbBD
   mov dword [FxTablebA3+0BEh*4],FxOpbBE
   mov dword [FxTablebA3+0BFh*4],FxOpbBF

   mov dword [FxTablec+05h*4],FxOpc05
   mov dword [FxTablec+06h*4],FxOpc06
   mov dword [FxTablec+07h*4],FxOpc07
   mov dword [FxTablec+08h*4],FxOpc08
   mov dword [FxTablec+09h*4],FxOpc09
   mov dword [FxTablec+0Ah*4],FxOpc0A
   mov dword [FxTablec+0Bh*4],FxOpc0B
   mov dword [FxTablec+0Ch*4],FxOpc0C
   mov dword [FxTablec+0Dh*4],FxOpc0D
   mov dword [FxTablec+0Eh*4],FxOpc0E
   mov dword [FxTablec+0Fh*4],FxOpc0F
   mov dword [FxTablec+10h*4],FxOpc10
   mov dword [FxTablec+11h*4],FxOpc11
   mov dword [FxTablec+12h*4],FxOpc12
   mov dword [FxTablec+13h*4],FxOpc13
   mov dword [FxTablec+14h*4],FxOpc14
   mov dword [FxTablec+15h*4],FxOpc15
   mov dword [FxTablec+16h*4],FxOpc16
   mov dword [FxTablec+17h*4],FxOpc17
   mov dword [FxTablec+18h*4],FxOpc18
   mov dword [FxTablec+19h*4],FxOpc19
   mov dword [FxTablec+1Ah*4],FxOpc1A
   mov dword [FxTablec+1Bh*4],FxOpc1B
   mov dword [FxTablec+1Ch*4],FxOpc1C
   mov dword [FxTablec+1Dh*4],FxOpc1D
   mov dword [FxTablec+1Eh*4],FxOpc1E
   mov dword [FxTablec+1Fh*4],FxOpc1F
   mov dword [FxTablec+3Dh*4],FxOpc3D
   mov dword [FxTablec+3Eh*4],FxOpc3E
   mov dword [FxTablec+3Fh*4],FxOpc3F
   mov dword [FxTablec+0B0h*4],FxOpcB0
   mov dword [FxTablec+0B1h*4],FxOpcB1
   mov dword [FxTablec+0B2h*4],FxOpcB2
   mov dword [FxTablec+0B3h*4],FxOpcB3
   mov dword [FxTablec+0B4h*4],FxOpcB4
   mov dword [FxTablec+0B5h*4],FxOpcB5
   mov dword [FxTablec+0B6h*4],FxOpcB6
   mov dword [FxTablec+0B7h*4],FxOpcB7
   mov dword [FxTablec+0B8h*4],FxOpcB8
   mov dword [FxTablec+0B9h*4],FxOpcB9
   mov dword [FxTablec+0BAh*4],FxOpcBA
   mov dword [FxTablec+0BBh*4],FxOpcBB
   mov dword [FxTablec+0BCh*4],FxOpcBC
   mov dword [FxTablec+0BDh*4],FxOpcBD
   mov dword [FxTablec+0BEh*4],FxOpcBE
   mov dword [FxTablec+0BFh*4],FxOpcBF

   mov dword [FxTablecA1+05h*4],FxOpc05
   mov dword [FxTablecA1+06h*4],FxOpc06
   mov dword [FxTablecA1+07h*4],FxOpc07
   mov dword [FxTablecA1+08h*4],FxOpc08
   mov dword [FxTablecA1+09h*4],FxOpc09
   mov dword [FxTablecA1+0Ah*4],FxOpc0A
   mov dword [FxTablecA1+0Bh*4],FxOpc0B
   mov dword [FxTablecA1+0Ch*4],FxOpc0C
   mov dword [FxTablecA1+0Dh*4],FxOpc0D
   mov dword [FxTablecA1+0Eh*4],FxOpc0E
   mov dword [FxTablecA1+0Fh*4],FxOpc0F
   mov dword [FxTablecA1+10h*4],FxOpc10
   mov dword [FxTablecA1+11h*4],FxOpc11
   mov dword [FxTablecA1+12h*4],FxOpc12
   mov dword [FxTablecA1+13h*4],FxOpc13
   mov dword [FxTablecA1+14h*4],FxOpc14
   mov dword [FxTablecA1+15h*4],FxOpc15
   mov dword [FxTablecA1+16h*4],FxOpc16
   mov dword [FxTablecA1+17h*4],FxOpc17
   mov dword [FxTablecA1+18h*4],FxOpc18
   mov dword [FxTablecA1+19h*4],FxOpc19
   mov dword [FxTablecA1+1Ah*4],FxOpc1A
   mov dword [FxTablecA1+1Bh*4],FxOpc1B
   mov dword [FxTablecA1+1Ch*4],FxOpc1C
   mov dword [FxTablecA1+1Dh*4],FxOpc1D
   mov dword [FxTablecA1+1Eh*4],FxOpc1E
   mov dword [FxTablecA1+1Fh*4],FxOpc1F
   mov dword [FxTablecA1+3Dh*4],FxOpc3D
   mov dword [FxTablecA1+3Eh*4],FxOpc3E
   mov dword [FxTablecA1+3Fh*4],FxOpc3F
   mov dword [FxTablecA1+0B0h*4],FxOpcB0
   mov dword [FxTablecA1+0B1h*4],FxOpcB1
   mov dword [FxTablecA1+0B2h*4],FxOpcB2
   mov dword [FxTablecA1+0B3h*4],FxOpcB3
   mov dword [FxTablecA1+0B4h*4],FxOpcB4
   mov dword [FxTablecA1+0B5h*4],FxOpcB5
   mov dword [FxTablecA1+0B6h*4],FxOpcB6
   mov dword [FxTablecA1+0B7h*4],FxOpcB7
   mov dword [FxTablecA1+0B8h*4],FxOpcB8
   mov dword [FxTablecA1+0B9h*4],FxOpcB9
   mov dword [FxTablecA1+0BAh*4],FxOpcBA
   mov dword [FxTablecA1+0BBh*4],FxOpcBB
   mov dword [FxTablecA1+0BCh*4],FxOpcBC
   mov dword [FxTablecA1+0BDh*4],FxOpcBD
   mov dword [FxTablecA1+0BEh*4],FxOpcBE
   mov dword [FxTablecA1+0BFh*4],FxOpcBF

   mov dword [FxTablecA2+05h*4],FxOpc05
   mov dword [FxTablecA2+06h*4],FxOpc06
   mov dword [FxTablecA2+07h*4],FxOpc07
   mov dword [FxTablecA2+08h*4],FxOpc08
   mov dword [FxTablecA2+09h*4],FxOpc09
   mov dword [FxTablecA2+0Ah*4],FxOpc0A
   mov dword [FxTablecA2+0Bh*4],FxOpc0B
   mov dword [FxTablecA2+0Ch*4],FxOpc0C
   mov dword [FxTablecA2+0Dh*4],FxOpc0D
   mov dword [FxTablecA2+0Eh*4],FxOpc0E
   mov dword [FxTablecA2+0Fh*4],FxOpc0F
   mov dword [FxTablecA2+10h*4],FxOpc10
   mov dword [FxTablecA2+11h*4],FxOpc11
   mov dword [FxTablecA2+12h*4],FxOpc12
   mov dword [FxTablecA2+13h*4],FxOpc13
   mov dword [FxTablecA2+14h*4],FxOpc14
   mov dword [FxTablecA2+15h*4],FxOpc15
   mov dword [FxTablecA2+16h*4],FxOpc16
   mov dword [FxTablecA2+17h*4],FxOpc17
   mov dword [FxTablecA2+18h*4],FxOpc18
   mov dword [FxTablecA2+19h*4],FxOpc19
   mov dword [FxTablecA2+1Ah*4],FxOpc1A
   mov dword [FxTablecA2+1Bh*4],FxOpc1B
   mov dword [FxTablecA2+1Ch*4],FxOpc1C
   mov dword [FxTablecA2+1Dh*4],FxOpc1D
   mov dword [FxTablecA2+1Eh*4],FxOpc1E
   mov dword [FxTablecA2+1Fh*4],FxOpc1F
   mov dword [FxTablecA2+3Dh*4],FxOpc3D
   mov dword [FxTablecA2+3Eh*4],FxOpc3E
   mov dword [FxTablecA2+3Fh*4],FxOpc3F
   mov dword [FxTablecA2+0B0h*4],FxOpcB0
   mov dword [FxTablecA2+0B1h*4],FxOpcB1
   mov dword [FxTablecA2+0B2h*4],FxOpcB2
   mov dword [FxTablecA2+0B3h*4],FxOpcB3
   mov dword [FxTablecA2+0B4h*4],FxOpcB4
   mov dword [FxTablecA2+0B5h*4],FxOpcB5
   mov dword [FxTablecA2+0B6h*4],FxOpcB6
   mov dword [FxTablecA2+0B7h*4],FxOpcB7
   mov dword [FxTablecA2+0B8h*4],FxOpcB8
   mov dword [FxTablecA2+0B9h*4],FxOpcB9
   mov dword [FxTablecA2+0BAh*4],FxOpcBA
   mov dword [FxTablecA2+0BBh*4],FxOpcBB
   mov dword [FxTablecA2+0BCh*4],FxOpcBC
   mov dword [FxTablecA2+0BDh*4],FxOpcBD
   mov dword [FxTablecA2+0BEh*4],FxOpcBE
   mov dword [FxTablecA2+0BFh*4],FxOpcBF

   mov dword [FxTablecA3+05h*4],FxOpc05
   mov dword [FxTablecA3+06h*4],FxOpc06
   mov dword [FxTablecA3+07h*4],FxOpc07
   mov dword [FxTablecA3+08h*4],FxOpc08
   mov dword [FxTablecA3+09h*4],FxOpc09
   mov dword [FxTablecA3+0Ah*4],FxOpc0A
   mov dword [FxTablecA3+0Bh*4],FxOpc0B
   mov dword [FxTablecA3+0Ch*4],FxOpc0C
   mov dword [FxTablecA3+0Dh*4],FxOpc0D
   mov dword [FxTablecA3+0Eh*4],FxOpc0E
   mov dword [FxTablecA3+0Fh*4],FxOpc0F
   mov dword [FxTablecA3+10h*4],FxOpc10
   mov dword [FxTablecA3+11h*4],FxOpc11
   mov dword [FxTablecA3+12h*4],FxOpc12
   mov dword [FxTablecA3+13h*4],FxOpc13
   mov dword [FxTablecA3+14h*4],FxOpc14
   mov dword [FxTablecA3+15h*4],FxOpc15
   mov dword [FxTablecA3+16h*4],FxOpc16
   mov dword [FxTablecA3+17h*4],FxOpc17
   mov dword [FxTablecA3+18h*4],FxOpc18
   mov dword [FxTablecA3+19h*4],FxOpc19
   mov dword [FxTablecA3+1Ah*4],FxOpc1A
   mov dword [FxTablecA3+1Bh*4],FxOpc1B
   mov dword [FxTablecA3+1Ch*4],FxOpc1C
   mov dword [FxTablecA3+1Dh*4],FxOpc1D
   mov dword [FxTablecA3+1Eh*4],FxOpc1E
   mov dword [FxTablecA3+1Fh*4],FxOpc1F
   mov dword [FxTablecA3+3Dh*4],FxOpc3D
   mov dword [FxTablecA3+3Eh*4],FxOpc3E
   mov dword [FxTablecA3+3Fh*4],FxOpc3F
   mov dword [FxTablecA3+0B0h*4],FxOpcB0
   mov dword [FxTablecA3+0B1h*4],FxOpcB1
   mov dword [FxTablecA3+0B2h*4],FxOpcB2
   mov dword [FxTablecA3+0B3h*4],FxOpcB3
   mov dword [FxTablecA3+0B4h*4],FxOpcB4
   mov dword [FxTablecA3+0B5h*4],FxOpcB5
   mov dword [FxTablecA3+0B6h*4],FxOpcB6
   mov dword [FxTablecA3+0B7h*4],FxOpcB7
   mov dword [FxTablecA3+0B8h*4],FxOpcB8
   mov dword [FxTablecA3+0B9h*4],FxOpcB9
   mov dword [FxTablecA3+0BAh*4],FxOpcBA
   mov dword [FxTablecA3+0BBh*4],FxOpcBB
   mov dword [FxTablecA3+0BCh*4],FxOpcBC
   mov dword [FxTablecA3+0BDh*4],FxOpcBD
   mov dword [FxTablecA3+0BEh*4],FxOpcBE
   mov dword [FxTablecA3+0BFh*4],FxOpcBF

   mov dword [FxTabled+00h*4],FxOpd00
   mov dword [FxTabled+01h*4],FxOpd01
   mov dword [FxTabled+02h*4],FxOpd02
   mov dword [FxTabled+03h*4],FxOpd03
   mov dword [FxTabled+04h*4],FxOpd04
   mov dword [FxTabled+05h*4],FxOpd05
   mov dword [FxTabled+06h*4],FxOpd06
   mov dword [FxTabled+07h*4],FxOpd07
   mov dword [FxTabled+08h*4],FxOpd08
   mov dword [FxTabled+09h*4],FxOpd09
   mov dword [FxTabled+0Ah*4],FxOpd0A
   mov dword [FxTabled+0Bh*4],FxOpd0B
   mov dword [FxTabled+0Ch*4],FxOpd0C
   mov dword [FxTabled+0Dh*4],FxOpd0D
   mov dword [FxTabled+0Eh*4],FxOpd0E
   mov dword [FxTabled+0Fh*4],FxOpd0F

   mov dword [FxTabled+10h*4],FxOpd10
   mov dword [FxTabled+11h*4],FxOpd11
   mov dword [FxTabled+12h*4],FxOpd12
   mov dword [FxTabled+13h*4],FxOpd13
   mov dword [FxTabled+14h*4],FxOpd14
   mov dword [FxTabled+15h*4],FxOpd15
   mov dword [FxTabled+16h*4],FxOpd16
   mov dword [FxTabled+17h*4],FxOpd17
   mov dword [FxTabled+18h*4],FxOpd18
   mov dword [FxTabled+19h*4],FxOpd19
   mov dword [FxTabled+1Ah*4],FxOpd1A
   mov dword [FxTabled+1Bh*4],FxOpd1B
   mov dword [FxTabled+1Ch*4],FxOpd1C
   mov dword [FxTabled+1Dh*4],FxOpd1D
   mov dword [FxTabled+1Eh*4],FxOpd1E
   mov dword [FxTabled+1Fh*4],FxOpd1F

   mov dword [FxTabled+20h*4],FxOpd20
   mov dword [FxTabled+21h*4],FxOpd21
   mov dword [FxTabled+22h*4],FxOpd22
   mov dword [FxTabled+23h*4],FxOpd23
   mov dword [FxTabled+24h*4],FxOpd24
   mov dword [FxTabled+25h*4],FxOpd25
   mov dword [FxTabled+26h*4],FxOpd26
   mov dword [FxTabled+27h*4],FxOpd27
   mov dword [FxTabled+28h*4],FxOpd28
   mov dword [FxTabled+29h*4],FxOpd29
   mov dword [FxTabled+2Ah*4],FxOpd2A
   mov dword [FxTabled+2Bh*4],FxOpd2B
   mov dword [FxTabled+2Ch*4],FxOpd2C
   mov dword [FxTabled+2Dh*4],FxOpd2D
   mov dword [FxTabled+2Eh*4],FxOpd2E
   mov dword [FxTabled+2Fh*4],FxOpd2F

   mov dword [FxTabled+30h*4],FxOpd30
   mov dword [FxTabled+31h*4],FxOpd31
   mov dword [FxTabled+32h*4],FxOpd32
   mov dword [FxTabled+33h*4],FxOpd33
   mov dword [FxTabled+34h*4],FxOpd34
   mov dword [FxTabled+35h*4],FxOpd35
   mov dword [FxTabled+36h*4],FxOpd36
   mov dword [FxTabled+37h*4],FxOpd37
   mov dword [FxTabled+38h*4],FxOpd38
   mov dword [FxTabled+39h*4],FxOpd39
   mov dword [FxTabled+3Ah*4],FxOpd3A
   mov dword [FxTabled+3Bh*4],FxOpd3B
   mov dword [FxTabled+3Ch*4],FxOpd3C
   mov dword [FxTabled+3Dh*4],FxOpd3D
   mov dword [FxTabled+3Eh*4],FxOpd3E
   mov dword [FxTabled+3Fh*4],FxOpd3F

   mov dword [FxTabled+40h*4],FxOpd40
   mov dword [FxTabled+41h*4],FxOpd41
   mov dword [FxTabled+42h*4],FxOpd42
   mov dword [FxTabled+43h*4],FxOpd43
   mov dword [FxTabled+44h*4],FxOpd44
   mov dword [FxTabled+45h*4],FxOpd45
   mov dword [FxTabled+46h*4],FxOpd46
   mov dword [FxTabled+47h*4],FxOpd47
   mov dword [FxTabled+48h*4],FxOpd48
   mov dword [FxTabled+49h*4],FxOpd49
   mov dword [FxTabled+4Ah*4],FxOpd4A
   mov dword [FxTabled+4Bh*4],FxOpd4B
   mov dword [FxTabled+4Ch*4],FxOpd4C
   mov dword [FxTabled+4Dh*4],FxOpd4D
   mov dword [FxTabled+4Eh*4],FxOpd4E
   mov dword [FxTabled+4Fh*4],FxOpd4F

   mov dword [FxTabled+50h*4],FxOpd50
   mov dword [FxTabled+51h*4],FxOpd51
   mov dword [FxTabled+52h*4],FxOpd52
   mov dword [FxTabled+53h*4],FxOpd53
   mov dword [FxTabled+54h*4],FxOpd54
   mov dword [FxTabled+55h*4],FxOpd55
   mov dword [FxTabled+56h*4],FxOpd56
   mov dword [FxTabled+57h*4],FxOpd57
   mov dword [FxTabled+58h*4],FxOpd58
   mov dword [FxTabled+59h*4],FxOpd59
   mov dword [FxTabled+5Ah*4],FxOpd5A
   mov dword [FxTabled+5Bh*4],FxOpd5B
   mov dword [FxTabled+5Ch*4],FxOpd5C
   mov dword [FxTabled+5Dh*4],FxOpd5D
   mov dword [FxTabled+5Eh*4],FxOpd5E
   mov dword [FxTabled+5Fh*4],FxOpd5F

   mov dword [FxTabled+60h*4],FxOpd60
   mov dword [FxTabled+61h*4],FxOpd61
   mov dword [FxTabled+62h*4],FxOpd62
   mov dword [FxTabled+63h*4],FxOpd63
   mov dword [FxTabled+64h*4],FxOpd64
   mov dword [FxTabled+65h*4],FxOpd65
   mov dword [FxTabled+66h*4],FxOpd66
   mov dword [FxTabled+67h*4],FxOpd67
   mov dword [FxTabled+68h*4],FxOpd68
   mov dword [FxTabled+69h*4],FxOpd69
   mov dword [FxTabled+6Ah*4],FxOpd6A
   mov dword [FxTabled+6Bh*4],FxOpd6B
   mov dword [FxTabled+6Ch*4],FxOpd6C
   mov dword [FxTabled+6Dh*4],FxOpd6D
   mov dword [FxTabled+6Eh*4],FxOpd6E
   mov dword [FxTabled+6Fh*4],FxOpd6F

   mov dword [FxTabled+70h*4],FxOpd70
   mov dword [FxTabled+71h*4],FxOpd71
   mov dword [FxTabled+72h*4],FxOpd72
   mov dword [FxTabled+73h*4],FxOpd73
   mov dword [FxTabled+74h*4],FxOpd74
   mov dword [FxTabled+75h*4],FxOpd75
   mov dword [FxTabled+76h*4],FxOpd76
   mov dword [FxTabled+77h*4],FxOpd77
   mov dword [FxTabled+78h*4],FxOpd78
   mov dword [FxTabled+79h*4],FxOpd79
   mov dword [FxTabled+7Ah*4],FxOpd7A
   mov dword [FxTabled+7Bh*4],FxOpd7B
   mov dword [FxTabled+7Ch*4],FxOpd7C
   mov dword [FxTabled+7Dh*4],FxOpd7D
   mov dword [FxTabled+7Eh*4],FxOpd7E
   mov dword [FxTabled+7Fh*4],FxOpd7F

   mov dword [FxTabled+80h*4],FxOpd80
   mov dword [FxTabled+81h*4],FxOpd81
   mov dword [FxTabled+82h*4],FxOpd82
   mov dword [FxTabled+83h*4],FxOpd83
   mov dword [FxTabled+84h*4],FxOpd84
   mov dword [FxTabled+85h*4],FxOpd85
   mov dword [FxTabled+86h*4],FxOpd86
   mov dword [FxTabled+87h*4],FxOpd87
   mov dword [FxTabled+88h*4],FxOpd88
   mov dword [FxTabled+89h*4],FxOpd89
   mov dword [FxTabled+8Ah*4],FxOpd8A
   mov dword [FxTabled+8Bh*4],FxOpd8B
   mov dword [FxTabled+8Ch*4],FxOpd8C
   mov dword [FxTabled+8Dh*4],FxOpd8D
   mov dword [FxTabled+8Eh*4],FxOpd8E
   mov dword [FxTabled+8Fh*4],FxOpd8F

   mov dword [FxTabled+90h*4],FxOpd90
   mov dword [FxTabled+91h*4],FxOpd91
   mov dword [FxTabled+92h*4],FxOpd92
   mov dword [FxTabled+93h*4],FxOpd93
   mov dword [FxTabled+94h*4],FxOpd94
   mov dword [FxTabled+95h*4],FxOpd95
   mov dword [FxTabled+96h*4],FxOpd96
   mov dword [FxTabled+97h*4],FxOpd97
   mov dword [FxTabled+98h*4],FxOpd98
   mov dword [FxTabled+99h*4],FxOpd99
   mov dword [FxTabled+9Ah*4],FxOpd9A
   mov dword [FxTabled+9Bh*4],FxOpd9B
   mov dword [FxTabled+9Ch*4],FxOpd9C
   mov dword [FxTabled+9Dh*4],FxOpd9D
   mov dword [FxTabled+9Eh*4],FxOpd9E
   mov dword [FxTabled+9Fh*4],FxOpd9F

   mov dword [FxTabled+0A0h*4],FxOpdA0
   mov dword [FxTabled+0A1h*4],FxOpdA1
   mov dword [FxTabled+0A2h*4],FxOpdA2
   mov dword [FxTabled+0A3h*4],FxOpdA3
   mov dword [FxTabled+0A4h*4],FxOpdA4
   mov dword [FxTabled+0A5h*4],FxOpdA5
   mov dword [FxTabled+0A6h*4],FxOpdA6
   mov dword [FxTabled+0A7h*4],FxOpdA7
   mov dword [FxTabled+0A8h*4],FxOpdA8
   mov dword [FxTabled+0A9h*4],FxOpdA9
   mov dword [FxTabled+0AAh*4],FxOpdAA
   mov dword [FxTabled+0ABh*4],FxOpdAB
   mov dword [FxTabled+0ACh*4],FxOpdAC
   mov dword [FxTabled+0ADh*4],FxOpdAD
   mov dword [FxTabled+0AEh*4],FxOpdAE
   mov dword [FxTabled+0AFh*4],FxOpdAF

   mov dword [FxTabled+0B0h*4],FxOpdB0
   mov dword [FxTabled+0B1h*4],FxOpdB1
   mov dword [FxTabled+0B2h*4],FxOpdB2
   mov dword [FxTabled+0B3h*4],FxOpdB3
   mov dword [FxTabled+0B4h*4],FxOpdB4
   mov dword [FxTabled+0B5h*4],FxOpdB5
   mov dword [FxTabled+0B6h*4],FxOpdB6
   mov dword [FxTabled+0B7h*4],FxOpdB7
   mov dword [FxTabled+0B8h*4],FxOpdB8
   mov dword [FxTabled+0B9h*4],FxOpdB9
   mov dword [FxTabled+0BAh*4],FxOpdBA
   mov dword [FxTabled+0BBh*4],FxOpdBB
   mov dword [FxTabled+0BCh*4],FxOpdBC
   mov dword [FxTabled+0BDh*4],FxOpdBD
   mov dword [FxTabled+0BEh*4],FxOpdBE
   mov dword [FxTabled+0BFh*4],FxOpdBF

   mov dword [FxTabled+0C0h*4],FxOpdC0
   mov dword [FxTabled+0C1h*4],FxOpdC1
   mov dword [FxTabled+0C2h*4],FxOpdC2
   mov dword [FxTabled+0C3h*4],FxOpdC3
   mov dword [FxTabled+0C4h*4],FxOpdC4
   mov dword [FxTabled+0C5h*4],FxOpdC5
   mov dword [FxTabled+0C6h*4],FxOpdC6
   mov dword [FxTabled+0C7h*4],FxOpdC7
   mov dword [FxTabled+0C8h*4],FxOpdC8
   mov dword [FxTabled+0C9h*4],FxOpdC9
   mov dword [FxTabled+0CAh*4],FxOpdCA
   mov dword [FxTabled+0CBh*4],FxOpdCB
   mov dword [FxTabled+0CCh*4],FxOpdCC
   mov dword [FxTabled+0CDh*4],FxOpdCD
   mov dword [FxTabled+0CEh*4],FxOpdCE
   mov dword [FxTabled+0CFh*4],FxOpdCF

   mov dword [FxTabled+0D0h*4],FxOpdD0
   mov dword [FxTabled+0D1h*4],FxOpdD1
   mov dword [FxTabled+0D2h*4],FxOpdD2
   mov dword [FxTabled+0D3h*4],FxOpdD3
   mov dword [FxTabled+0D4h*4],FxOpdD4
   mov dword [FxTabled+0D5h*4],FxOpdD5
   mov dword [FxTabled+0D6h*4],FxOpdD6
   mov dword [FxTabled+0D7h*4],FxOpdD7
   mov dword [FxTabled+0D8h*4],FxOpdD8
   mov dword [FxTabled+0D9h*4],FxOpdD9
   mov dword [FxTabled+0DAh*4],FxOpdDA
   mov dword [FxTabled+0DBh*4],FxOpdDB
   mov dword [FxTabled+0DCh*4],FxOpdDC
   mov dword [FxTabled+0DDh*4],FxOpdDD
   mov dword [FxTabled+0DEh*4],FxOpdDE
   mov dword [FxTabled+0DFh*4],FxOpdDF

   mov dword [FxTabled+0E0h*4],FxOpdE0
   mov dword [FxTabled+0E1h*4],FxOpdE1
   mov dword [FxTabled+0E2h*4],FxOpdE2
   mov dword [FxTabled+0E3h*4],FxOpdE3
   mov dword [FxTabled+0E4h*4],FxOpdE4
   mov dword [FxTabled+0E5h*4],FxOpdE5
   mov dword [FxTabled+0E6h*4],FxOpdE6
   mov dword [FxTabled+0E7h*4],FxOpdE7
   mov dword [FxTabled+0E8h*4],FxOpdE8
   mov dword [FxTabled+0E9h*4],FxOpdE9
   mov dword [FxTabled+0EAh*4],FxOpdEA
   mov dword [FxTabled+0EBh*4],FxOpdEB
   mov dword [FxTabled+0ECh*4],FxOpdEC
   mov dword [FxTabled+0EDh*4],FxOpdED
   mov dword [FxTabled+0EEh*4],FxOpdEE
   mov dword [FxTabled+0EFh*4],FxOpdEF

   mov dword [FxTabled+0F0h*4],FxOpdF0
   mov dword [FxTabled+0F1h*4],FxOpdF1
   mov dword [FxTabled+0F2h*4],FxOpdF2
   mov dword [FxTabled+0F3h*4],FxOpdF3
   mov dword [FxTabled+0F4h*4],FxOpdF4
   mov dword [FxTabled+0F5h*4],FxOpdF5
   mov dword [FxTabled+0F6h*4],FxOpdF6
   mov dword [FxTabled+0F7h*4],FxOpdF7
   mov dword [FxTabled+0F8h*4],FxOpdF8
   mov dword [FxTabled+0F9h*4],FxOpdF9
   mov dword [FxTabled+0FAh*4],FxOpdFA
   mov dword [FxTabled+0FBh*4],FxOpdFB
   mov dword [FxTabled+0FCh*4],FxOpdFC
   mov dword [FxTabled+0FDh*4],FxOpdFD
   mov dword [FxTabled+0FEh*4],FxOpdFE
   mov dword [FxTabled+0FFh*4],FxOpdFF


   mov dword [FxTabledA1+00h*4],FxOpd00
   mov dword [FxTabledA1+01h*4],FxOpd01
   mov dword [FxTabledA1+02h*4],FxOpd02
   mov dword [FxTabledA1+03h*4],FxOpd03
   mov dword [FxTabledA1+04h*4],FxOpd04
   mov dword [FxTabledA1+05h*4],FxOpd05
   mov dword [FxTabledA1+06h*4],FxOpd06
   mov dword [FxTabledA1+07h*4],FxOpd07
   mov dword [FxTabledA1+08h*4],FxOpd08
   mov dword [FxTabledA1+09h*4],FxOpd09
   mov dword [FxTabledA1+0Ah*4],FxOpd0A
   mov dword [FxTabledA1+0Bh*4],FxOpd0B
   mov dword [FxTabledA1+0Ch*4],FxOpd0C
   mov dword [FxTabledA1+0Dh*4],FxOpd0D
   mov dword [FxTabledA1+0Eh*4],FxOpd0E
   mov dword [FxTabledA1+0Fh*4],FxOpd0F

   mov dword [FxTabledA1+10h*4],FxOpd10
   mov dword [FxTabledA1+11h*4],FxOpd11
   mov dword [FxTabledA1+12h*4],FxOpd12
   mov dword [FxTabledA1+13h*4],FxOpd13
   mov dword [FxTabledA1+14h*4],FxOpd14
   mov dword [FxTabledA1+15h*4],FxOpd15
   mov dword [FxTabledA1+16h*4],FxOpd16
   mov dword [FxTabledA1+17h*4],FxOpd17
   mov dword [FxTabledA1+18h*4],FxOpd18
   mov dword [FxTabledA1+19h*4],FxOpd19
   mov dword [FxTabledA1+1Ah*4],FxOpd1A
   mov dword [FxTabledA1+1Bh*4],FxOpd1B
   mov dword [FxTabledA1+1Ch*4],FxOpd1C
   mov dword [FxTabledA1+1Dh*4],FxOpd1D
   mov dword [FxTabledA1+1Eh*4],FxOpd1E
   mov dword [FxTabledA1+1Fh*4],FxOpd1F

   mov dword [FxTabledA1+20h*4],FxOpd20
   mov dword [FxTabledA1+21h*4],FxOpd21
   mov dword [FxTabledA1+22h*4],FxOpd22
   mov dword [FxTabledA1+23h*4],FxOpd23
   mov dword [FxTabledA1+24h*4],FxOpd24
   mov dword [FxTabledA1+25h*4],FxOpd25
   mov dword [FxTabledA1+26h*4],FxOpd26
   mov dword [FxTabledA1+27h*4],FxOpd27
   mov dword [FxTabledA1+28h*4],FxOpd28
   mov dword [FxTabledA1+29h*4],FxOpd29
   mov dword [FxTabledA1+2Ah*4],FxOpd2A
   mov dword [FxTabledA1+2Bh*4],FxOpd2B
   mov dword [FxTabledA1+2Ch*4],FxOpd2C
   mov dword [FxTabledA1+2Dh*4],FxOpd2D
   mov dword [FxTabledA1+2Eh*4],FxOpd2E
   mov dword [FxTabledA1+2Fh*4],FxOpd2F

   mov dword [FxTabledA1+30h*4],FxOpd30A1
   mov dword [FxTabledA1+31h*4],FxOpd31A1
   mov dword [FxTabledA1+32h*4],FxOpd32A1
   mov dword [FxTabledA1+33h*4],FxOpd33A1
   mov dword [FxTabledA1+34h*4],FxOpd34A1
   mov dword [FxTabledA1+35h*4],FxOpd35A1
   mov dword [FxTabledA1+36h*4],FxOpd36A1
   mov dword [FxTabledA1+37h*4],FxOpd37A1
   mov dword [FxTabledA1+38h*4],FxOpd38A1
   mov dword [FxTabledA1+39h*4],FxOpd39A1
   mov dword [FxTabledA1+3Ah*4],FxOpd3AA1
   mov dword [FxTabledA1+3Bh*4],FxOpd3BA1
   mov dword [FxTabledA1+3Ch*4],FxOpd3C
   mov dword [FxTabledA1+3Dh*4],FxOpd3D
   mov dword [FxTabledA1+3Eh*4],FxOpd3E
   mov dword [FxTabledA1+3Fh*4],FxOpd3F

   mov dword [FxTabledA1+40h*4],FxOpd40A1
   mov dword [FxTabledA1+41h*4],FxOpd41A1
   mov dword [FxTabledA1+42h*4],FxOpd42A1
   mov dword [FxTabledA1+43h*4],FxOpd43A1
   mov dword [FxTabledA1+44h*4],FxOpd44A1
   mov dword [FxTabledA1+45h*4],FxOpd45A1
   mov dword [FxTabledA1+46h*4],FxOpd46A1
   mov dword [FxTabledA1+47h*4],FxOpd47A1
   mov dword [FxTabledA1+48h*4],FxOpd48A1
   mov dword [FxTabledA1+49h*4],FxOpd49A1
   mov dword [FxTabledA1+4Ah*4],FxOpd4AA1
   mov dword [FxTabledA1+4Bh*4],FxOpd4BA1
   mov dword [FxTabledA1+4Ch*4],FxOpd4CA1
   mov dword [FxTabledA1+4Dh*4],FxOpd4D
   mov dword [FxTabledA1+4Eh*4],FxOpd4EA1
   mov dword [FxTabledA1+4Fh*4],FxOpd4F

   mov dword [FxTabledA1+50h*4],FxOpd50A1
   mov dword [FxTabledA1+51h*4],FxOpd51A1
   mov dword [FxTabledA1+52h*4],FxOpd52A1
   mov dword [FxTabledA1+53h*4],FxOpd53A1
   mov dword [FxTabledA1+54h*4],FxOpd54A1
   mov dword [FxTabledA1+55h*4],FxOpd55A1
   mov dword [FxTabledA1+56h*4],FxOpd56A1
   mov dword [FxTabledA1+57h*4],FxOpd57A1
   mov dword [FxTabledA1+58h*4],FxOpd58A1
   mov dword [FxTabledA1+59h*4],FxOpd59A1
   mov dword [FxTabledA1+5Ah*4],FxOpd5AA1
   mov dword [FxTabledA1+5Bh*4],FxOpd5BA1
   mov dword [FxTabledA1+5Ch*4],FxOpd5CA1
   mov dword [FxTabledA1+5Dh*4],FxOpd5DA1
   mov dword [FxTabledA1+5Eh*4],FxOpd5EA1
   mov dword [FxTabledA1+5Fh*4],FxOpd5FA1

   mov dword [FxTabledA1+60h*4],FxOpd60A1
   mov dword [FxTabledA1+61h*4],FxOpd61A1
   mov dword [FxTabledA1+62h*4],FxOpd62A1
   mov dword [FxTabledA1+63h*4],FxOpd63A1
   mov dword [FxTabledA1+64h*4],FxOpd64A1
   mov dword [FxTabledA1+65h*4],FxOpd65A1
   mov dword [FxTabledA1+66h*4],FxOpd66A1
   mov dword [FxTabledA1+67h*4],FxOpd67A1
   mov dword [FxTabledA1+68h*4],FxOpd68A1
   mov dword [FxTabledA1+69h*4],FxOpd69A1
   mov dword [FxTabledA1+6Ah*4],FxOpd6AA1
   mov dword [FxTabledA1+6Bh*4],FxOpd6BA1
   mov dword [FxTabledA1+6Ch*4],FxOpd6CA1
   mov dword [FxTabledA1+6Dh*4],FxOpd6DA1
   mov dword [FxTabledA1+6Eh*4],FxOpd6EA1
   mov dword [FxTabledA1+6Fh*4],FxOpd6FA1

   mov dword [FxTabledA1+70h*4],FxOpd70
   mov dword [FxTabledA1+71h*4],FxOpd71A1
   mov dword [FxTabledA1+72h*4],FxOpd72A1
   mov dword [FxTabledA1+73h*4],FxOpd73A1
   mov dword [FxTabledA1+74h*4],FxOpd74A1
   mov dword [FxTabledA1+75h*4],FxOpd75A1
   mov dword [FxTabledA1+76h*4],FxOpd76A1
   mov dword [FxTabledA1+77h*4],FxOpd77A1
   mov dword [FxTabledA1+78h*4],FxOpd78A1
   mov dword [FxTabledA1+79h*4],FxOpd79A1
   mov dword [FxTabledA1+7Ah*4],FxOpd7AA1
   mov dword [FxTabledA1+7Bh*4],FxOpd7BA1
   mov dword [FxTabledA1+7Ch*4],FxOpd7CA1
   mov dword [FxTabledA1+7Dh*4],FxOpd7DA1
   mov dword [FxTabledA1+7Eh*4],FxOpd7EA1
   mov dword [FxTabledA1+7Fh*4],FxOpd7FA1

   mov dword [FxTabledA1+80h*4],FxOpd80A1
   mov dword [FxTabledA1+81h*4],FxOpd81A1
   mov dword [FxTabledA1+82h*4],FxOpd82A1
   mov dword [FxTabledA1+83h*4],FxOpd83A1
   mov dword [FxTabledA1+84h*4],FxOpd84A1
   mov dword [FxTabledA1+85h*4],FxOpd85A1
   mov dword [FxTabledA1+86h*4],FxOpd86A1
   mov dword [FxTabledA1+87h*4],FxOpd87A1
   mov dword [FxTabledA1+88h*4],FxOpd88A1
   mov dword [FxTabledA1+89h*4],FxOpd89A1
   mov dword [FxTabledA1+8Ah*4],FxOpd8AA1
   mov dword [FxTabledA1+8Bh*4],FxOpd8BA1
   mov dword [FxTabledA1+8Ch*4],FxOpd8CA1
   mov dword [FxTabledA1+8Dh*4],FxOpd8DA1
   mov dword [FxTabledA1+8Eh*4],FxOpd8EA1
   mov dword [FxTabledA1+8Fh*4],FxOpd8FA1

   mov dword [FxTabledA1+90h*4],FxOpd90
   mov dword [FxTabledA1+91h*4],FxOpd91
   mov dword [FxTabledA1+92h*4],FxOpd92
   mov dword [FxTabledA1+93h*4],FxOpd93
   mov dword [FxTabledA1+94h*4],FxOpd94
   mov dword [FxTabledA1+95h*4],FxOpd95
   mov dword [FxTabledA1+96h*4],FxOpd96A1
   mov dword [FxTabledA1+97h*4],FxOpd97
   mov dword [FxTabledA1+98h*4],FxOpd98A1
   mov dword [FxTabledA1+99h*4],FxOpd99A1
   mov dword [FxTabledA1+9Ah*4],FxOpd9AA1
   mov dword [FxTabledA1+9Bh*4],FxOpd9BA1
   mov dword [FxTabledA1+9Ch*4],FxOpd9CA1
   mov dword [FxTabledA1+9Dh*4],FxOpd9DA1
   mov dword [FxTabledA1+9Eh*4],FxOpd9E
   mov dword [FxTabledA1+9Fh*4],FxOpd9FA1

   mov dword [FxTabledA1+0A0h*4],FxOpdA0A1
   mov dword [FxTabledA1+0A1h*4],FxOpdA1A1
   mov dword [FxTabledA1+0A2h*4],FxOpdA2A1
   mov dword [FxTabledA1+0A3h*4],FxOpdA3A1
   mov dword [FxTabledA1+0A4h*4],FxOpdA4A1
   mov dword [FxTabledA1+0A5h*4],FxOpdA5A1
   mov dword [FxTabledA1+0A6h*4],FxOpdA6A1
   mov dword [FxTabledA1+0A7h*4],FxOpdA7A1
   mov dword [FxTabledA1+0A8h*4],FxOpdA8A1
   mov dword [FxTabledA1+0A9h*4],FxOpdA9A1
   mov dword [FxTabledA1+0AAh*4],FxOpdAAA1
   mov dword [FxTabledA1+0ABh*4],FxOpdABA1
   mov dword [FxTabledA1+0ACh*4],FxOpdACA1
   mov dword [FxTabledA1+0ADh*4],FxOpdADA1
   mov dword [FxTabledA1+0AEh*4],FxOpdAEA1
   mov dword [FxTabledA1+0AFh*4],FxOpdAFA1

   mov dword [FxTabledA1+0B0h*4],FxOpdB0
   mov dword [FxTabledA1+0B1h*4],FxOpdB1
   mov dword [FxTabledA1+0B2h*4],FxOpdB2
   mov dword [FxTabledA1+0B3h*4],FxOpdB3
   mov dword [FxTabledA1+0B4h*4],FxOpdB4
   mov dword [FxTabledA1+0B5h*4],FxOpdB5
   mov dword [FxTabledA1+0B6h*4],FxOpdB6
   mov dword [FxTabledA1+0B7h*4],FxOpdB7
   mov dword [FxTabledA1+0B8h*4],FxOpdB8
   mov dword [FxTabledA1+0B9h*4],FxOpdB9
   mov dword [FxTabledA1+0BAh*4],FxOpdBA
   mov dword [FxTabledA1+0BBh*4],FxOpdBB
   mov dword [FxTabledA1+0BCh*4],FxOpdBC
   mov dword [FxTabledA1+0BDh*4],FxOpdBD
   mov dword [FxTabledA1+0BEh*4],FxOpdBE
   mov dword [FxTabledA1+0BFh*4],FxOpdBF

   mov dword [FxTabledA1+0C0h*4],FxOpdC0
   mov dword [FxTabledA1+0C1h*4],FxOpdC1A1
   mov dword [FxTabledA1+0C2h*4],FxOpdC2A1
   mov dword [FxTabledA1+0C3h*4],FxOpdC3A1
   mov dword [FxTabledA1+0C4h*4],FxOpdC4A1
   mov dword [FxTabledA1+0C5h*4],FxOpdC5A1
   mov dword [FxTabledA1+0C6h*4],FxOpdC6A1
   mov dword [FxTabledA1+0C7h*4],FxOpdC7A1
   mov dword [FxTabledA1+0C8h*4],FxOpdC8A1
   mov dword [FxTabledA1+0C9h*4],FxOpdC9A1
   mov dword [FxTabledA1+0CAh*4],FxOpdCAA1
   mov dword [FxTabledA1+0CBh*4],FxOpdCBA1
   mov dword [FxTabledA1+0CCh*4],FxOpdCCA1
   mov dword [FxTabledA1+0CDh*4],FxOpdCDA1
   mov dword [FxTabledA1+0CEh*4],FxOpdCEA1
   mov dword [FxTabledA1+0CFh*4],FxOpdCFA1

   mov dword [FxTabledA1+0D0h*4],FxOpdD0
   mov dword [FxTabledA1+0D1h*4],FxOpdD1
   mov dword [FxTabledA1+0D2h*4],FxOpdD2
   mov dword [FxTabledA1+0D3h*4],FxOpdD3
   mov dword [FxTabledA1+0D4h*4],FxOpdD4
   mov dword [FxTabledA1+0D5h*4],FxOpdD5
   mov dword [FxTabledA1+0D6h*4],FxOpdD6
   mov dword [FxTabledA1+0D7h*4],FxOpdD7
   mov dword [FxTabledA1+0D8h*4],FxOpdD8
   mov dword [FxTabledA1+0D9h*4],FxOpdD9
   mov dword [FxTabledA1+0DAh*4],FxOpdDA
   mov dword [FxTabledA1+0DBh*4],FxOpdDB
   mov dword [FxTabledA1+0DCh*4],FxOpdDC
   mov dword [FxTabledA1+0DDh*4],FxOpdDD
   mov dword [FxTabledA1+0DEh*4],FxOpdDE
   mov dword [FxTabledA1+0DFh*4],FxOpdDF

   mov dword [FxTabledA1+0E0h*4],FxOpdE0
   mov dword [FxTabledA1+0E1h*4],FxOpdE1
   mov dword [FxTabledA1+0E2h*4],FxOpdE2
   mov dword [FxTabledA1+0E3h*4],FxOpdE3
   mov dword [FxTabledA1+0E4h*4],FxOpdE4
   mov dword [FxTabledA1+0E5h*4],FxOpdE5
   mov dword [FxTabledA1+0E6h*4],FxOpdE6
   mov dword [FxTabledA1+0E7h*4],FxOpdE7
   mov dword [FxTabledA1+0E8h*4],FxOpdE8
   mov dword [FxTabledA1+0E9h*4],FxOpdE9
   mov dword [FxTabledA1+0EAh*4],FxOpdEA
   mov dword [FxTabledA1+0EBh*4],FxOpdEB
   mov dword [FxTabledA1+0ECh*4],FxOpdEC
   mov dword [FxTabledA1+0EDh*4],FxOpdED
   mov dword [FxTabledA1+0EEh*4],FxOpdEE
   mov dword [FxTabledA1+0EFh*4],FxOpdEFA1

   mov dword [FxTabledA1+0F0h*4],FxOpdF0A1
   mov dword [FxTabledA1+0F1h*4],FxOpdF1A1
   mov dword [FxTabledA1+0F2h*4],FxOpdF2A1
   mov dword [FxTabledA1+0F3h*4],FxOpdF3A1
   mov dword [FxTabledA1+0F4h*4],FxOpdF4A1
   mov dword [FxTabledA1+0F5h*4],FxOpdF5A1
   mov dword [FxTabledA1+0F6h*4],FxOpdF6A1
   mov dword [FxTabledA1+0F7h*4],FxOpdF7A1
   mov dword [FxTabledA1+0F8h*4],FxOpdF8A1
   mov dword [FxTabledA1+0F9h*4],FxOpdF9A1
   mov dword [FxTabledA1+0FAh*4],FxOpdFAA1
   mov dword [FxTabledA1+0FBh*4],FxOpdFBA1
   mov dword [FxTabledA1+0FCh*4],FxOpdFCA1
   mov dword [FxTabledA1+0FDh*4],FxOpdFDA1
   mov dword [FxTabledA1+0FEh*4],FxOpdFEA1
   mov dword [FxTabledA1+0FFh*4],FxOpdFFA1


   mov dword [FxTabledA2+00h*4],FxOpd00
   mov dword [FxTabledA2+01h*4],FxOpd01
   mov dword [FxTabledA2+02h*4],FxOpd02
   mov dword [FxTabledA2+03h*4],FxOpd03
   mov dword [FxTabledA2+04h*4],FxOpd04
   mov dword [FxTabledA2+05h*4],FxOpd05
   mov dword [FxTabledA2+06h*4],FxOpd06
   mov dword [FxTabledA2+07h*4],FxOpd07
   mov dword [FxTabledA2+08h*4],FxOpd08
   mov dword [FxTabledA2+09h*4],FxOpd09
   mov dword [FxTabledA2+0Ah*4],FxOpd0A
   mov dword [FxTabledA2+0Bh*4],FxOpd0B
   mov dword [FxTabledA2+0Ch*4],FxOpd0C
   mov dword [FxTabledA2+0Dh*4],FxOpd0D
   mov dword [FxTabledA2+0Eh*4],FxOpd0E
   mov dword [FxTabledA2+0Fh*4],FxOpd0F

   mov dword [FxTabledA2+10h*4],FxOpd10
   mov dword [FxTabledA2+11h*4],FxOpd11
   mov dword [FxTabledA2+12h*4],FxOpd12
   mov dword [FxTabledA2+13h*4],FxOpd13
   mov dword [FxTabledA2+14h*4],FxOpd14
   mov dword [FxTabledA2+15h*4],FxOpd15
   mov dword [FxTabledA2+16h*4],FxOpd16
   mov dword [FxTabledA2+17h*4],FxOpd17
   mov dword [FxTabledA2+18h*4],FxOpd18
   mov dword [FxTabledA2+19h*4],FxOpd19
   mov dword [FxTabledA2+1Ah*4],FxOpd1A
   mov dword [FxTabledA2+1Bh*4],FxOpd1B
   mov dword [FxTabledA2+1Ch*4],FxOpd1C
   mov dword [FxTabledA2+1Dh*4],FxOpd1D
   mov dword [FxTabledA2+1Eh*4],FxOpd1E
   mov dword [FxTabledA2+1Fh*4],FxOpd1F

   mov dword [FxTabledA2+20h*4],FxOpd20
   mov dword [FxTabledA2+21h*4],FxOpd21
   mov dword [FxTabledA2+22h*4],FxOpd22
   mov dword [FxTabledA2+23h*4],FxOpd23
   mov dword [FxTabledA2+24h*4],FxOpd24
   mov dword [FxTabledA2+25h*4],FxOpd25
   mov dword [FxTabledA2+26h*4],FxOpd26
   mov dword [FxTabledA2+27h*4],FxOpd27
   mov dword [FxTabledA2+28h*4],FxOpd28
   mov dword [FxTabledA2+29h*4],FxOpd29
   mov dword [FxTabledA2+2Ah*4],FxOpd2A
   mov dword [FxTabledA2+2Bh*4],FxOpd2B
   mov dword [FxTabledA2+2Ch*4],FxOpd2C
   mov dword [FxTabledA2+2Dh*4],FxOpd2D
   mov dword [FxTabledA2+2Eh*4],FxOpd2E
   mov dword [FxTabledA2+2Fh*4],FxOpd2F

   mov dword [FxTabledA2+30h*4],FxOpd30
   mov dword [FxTabledA2+31h*4],FxOpd31
   mov dword [FxTabledA2+32h*4],FxOpd32
   mov dword [FxTabledA2+33h*4],FxOpd33
   mov dword [FxTabledA2+34h*4],FxOpd34
   mov dword [FxTabledA2+35h*4],FxOpd35
   mov dword [FxTabledA2+36h*4],FxOpd36
   mov dword [FxTabledA2+37h*4],FxOpd37
   mov dword [FxTabledA2+38h*4],FxOpd38
   mov dword [FxTabledA2+39h*4],FxOpd39
   mov dword [FxTabledA2+3Ah*4],FxOpd3A
   mov dword [FxTabledA2+3Bh*4],FxOpd3B
   mov dword [FxTabledA2+3Ch*4],FxOpd3C
   mov dword [FxTabledA2+3Dh*4],FxOpd3D
   mov dword [FxTabledA2+3Eh*4],FxOpd3E
   mov dword [FxTabledA2+3Fh*4],FxOpd3F

   mov dword [FxTabledA2+40h*4],FxOpd40
   mov dword [FxTabledA2+41h*4],FxOpd41
   mov dword [FxTabledA2+42h*4],FxOpd42
   mov dword [FxTabledA2+43h*4],FxOpd43
   mov dword [FxTabledA2+44h*4],FxOpd44
   mov dword [FxTabledA2+45h*4],FxOpd45
   mov dword [FxTabledA2+46h*4],FxOpd46
   mov dword [FxTabledA2+47h*4],FxOpd47
   mov dword [FxTabledA2+48h*4],FxOpd48
   mov dword [FxTabledA2+49h*4],FxOpd49
   mov dword [FxTabledA2+4Ah*4],FxOpd4A
   mov dword [FxTabledA2+4Bh*4],FxOpd4B
   mov dword [FxTabledA2+4Ch*4],FxOpd4C
   mov dword [FxTabledA2+4Dh*4],FxOpd4D
   mov dword [FxTabledA2+4Eh*4],FxOpd4E
   mov dword [FxTabledA2+4Fh*4],FxOpd4F

   mov dword [FxTabledA2+50h*4],FxOpd50A2
   mov dword [FxTabledA2+51h*4],FxOpd51A2
   mov dword [FxTabledA2+52h*4],FxOpd52A2
   mov dword [FxTabledA2+53h*4],FxOpd53A2
   mov dword [FxTabledA2+54h*4],FxOpd54A2
   mov dword [FxTabledA2+55h*4],FxOpd55A2
   mov dword [FxTabledA2+56h*4],FxOpd56A2
   mov dword [FxTabledA2+57h*4],FxOpd57A2
   mov dword [FxTabledA2+58h*4],FxOpd58A2
   mov dword [FxTabledA2+59h*4],FxOpd59A2
   mov dword [FxTabledA2+5Ah*4],FxOpd5AA2
   mov dword [FxTabledA2+5Bh*4],FxOpd5BA2
   mov dword [FxTabledA2+5Ch*4],FxOpd5CA2
   mov dword [FxTabledA2+5Dh*4],FxOpd5DA2
   mov dword [FxTabledA2+5Eh*4],FxOpd5EA2
   mov dword [FxTabledA2+5Fh*4],FxOpd5FA2

   mov dword [FxTabledA2+60h*4],FxOpd60A2
   mov dword [FxTabledA2+61h*4],FxOpd61A2
   mov dword [FxTabledA2+62h*4],FxOpd62A2
   mov dword [FxTabledA2+63h*4],FxOpd63A2
   mov dword [FxTabledA2+64h*4],FxOpd64A2
   mov dword [FxTabledA2+65h*4],FxOpd65A2
   mov dword [FxTabledA2+66h*4],FxOpd66A2
   mov dword [FxTabledA2+67h*4],FxOpd67A2
   mov dword [FxTabledA2+68h*4],FxOpd68A2
   mov dword [FxTabledA2+69h*4],FxOpd69A2
   mov dword [FxTabledA2+6Ah*4],FxOpd6AA2
   mov dword [FxTabledA2+6Bh*4],FxOpd6BA2
   mov dword [FxTabledA2+6Ch*4],FxOpd6CA2
   mov dword [FxTabledA2+6Dh*4],FxOpd6DA2
   mov dword [FxTabledA2+6Eh*4],FxOpd6EA2
   mov dword [FxTabledA2+6Fh*4],FxOpd6FA2

   mov dword [FxTabledA2+70h*4],FxOpd70
   mov dword [FxTabledA2+71h*4],FxOpd71A2
   mov dword [FxTabledA2+72h*4],FxOpd72A2
   mov dword [FxTabledA2+73h*4],FxOpd73A2
   mov dword [FxTabledA2+74h*4],FxOpd74A2
   mov dword [FxTabledA2+75h*4],FxOpd75A2
   mov dword [FxTabledA2+76h*4],FxOpd76A2
   mov dword [FxTabledA2+77h*4],FxOpd77A2
   mov dword [FxTabledA2+78h*4],FxOpd78A2
   mov dword [FxTabledA2+79h*4],FxOpd79A2
   mov dword [FxTabledA2+7Ah*4],FxOpd7AA2
   mov dword [FxTabledA2+7Bh*4],FxOpd7BA2
   mov dword [FxTabledA2+7Ch*4],FxOpd7CA2
   mov dword [FxTabledA2+7Dh*4],FxOpd7DA2
   mov dword [FxTabledA2+7Eh*4],FxOpd7EA2
   mov dword [FxTabledA2+7Fh*4],FxOpd7FA2

   mov dword [FxTabledA2+80h*4],FxOpd80A2
   mov dword [FxTabledA2+81h*4],FxOpd81A2
   mov dword [FxTabledA2+82h*4],FxOpd82A2
   mov dword [FxTabledA2+83h*4],FxOpd83A2
   mov dword [FxTabledA2+84h*4],FxOpd84A2
   mov dword [FxTabledA2+85h*4],FxOpd85A2
   mov dword [FxTabledA2+86h*4],FxOpd86A2
   mov dword [FxTabledA2+87h*4],FxOpd87A2
   mov dword [FxTabledA2+88h*4],FxOpd88A2
   mov dword [FxTabledA2+89h*4],FxOpd89A2
   mov dword [FxTabledA2+8Ah*4],FxOpd8AA2
   mov dword [FxTabledA2+8Bh*4],FxOpd8BA2
   mov dword [FxTabledA2+8Ch*4],FxOpd8CA2
   mov dword [FxTabledA2+8Dh*4],FxOpd8DA2
   mov dword [FxTabledA2+8Eh*4],FxOpd8EA2
   mov dword [FxTabledA2+8Fh*4],FxOpd8FA2

   mov dword [FxTabledA2+90h*4],FxOpd90
   mov dword [FxTabledA2+91h*4],FxOpd91
   mov dword [FxTabledA2+92h*4],FxOpd92
   mov dword [FxTabledA2+93h*4],FxOpd93
   mov dword [FxTabledA2+94h*4],FxOpd94
   mov dword [FxTabledA2+95h*4],FxOpd95
   mov dword [FxTabledA2+96h*4],FxOpd96
   mov dword [FxTabledA2+97h*4],FxOpd97
   mov dword [FxTabledA2+98h*4],FxOpd98
   mov dword [FxTabledA2+99h*4],FxOpd99
   mov dword [FxTabledA2+9Ah*4],FxOpd9A
   mov dword [FxTabledA2+9Bh*4],FxOpd9B
   mov dword [FxTabledA2+9Ch*4],FxOpd9C
   mov dword [FxTabledA2+9Dh*4],FxOpd9D
   mov dword [FxTabledA2+9Eh*4],FxOpd9E
   mov dword [FxTabledA2+9Fh*4],FxOpd9F

   mov dword [FxTabledA2+0A0h*4],FxOpdA0A2
   mov dword [FxTabledA2+0A1h*4],FxOpdA1A2
   mov dword [FxTabledA2+0A2h*4],FxOpdA2A2
   mov dword [FxTabledA2+0A3h*4],FxOpdA3A2
   mov dword [FxTabledA2+0A4h*4],FxOpdA4A2
   mov dword [FxTabledA2+0A5h*4],FxOpdA5A2
   mov dword [FxTabledA2+0A6h*4],FxOpdA6A2
   mov dword [FxTabledA2+0A7h*4],FxOpdA7A2
   mov dword [FxTabledA2+0A8h*4],FxOpdA8A2
   mov dword [FxTabledA2+0A9h*4],FxOpdA9A2
   mov dword [FxTabledA2+0AAh*4],FxOpdAAA2
   mov dword [FxTabledA2+0ABh*4],FxOpdABA2
   mov dword [FxTabledA2+0ACh*4],FxOpdACA2
   mov dword [FxTabledA2+0ADh*4],FxOpdADA2
   mov dword [FxTabledA2+0AEh*4],FxOpdAEA2
   mov dword [FxTabledA2+0AFh*4],FxOpdAFA2

   mov dword [FxTabledA2+0B0h*4],FxOpdB0
   mov dword [FxTabledA2+0B1h*4],FxOpdB1
   mov dword [FxTabledA2+0B2h*4],FxOpdB2
   mov dword [FxTabledA2+0B3h*4],FxOpdB3
   mov dword [FxTabledA2+0B4h*4],FxOpdB4
   mov dword [FxTabledA2+0B5h*4],FxOpdB5
   mov dword [FxTabledA2+0B6h*4],FxOpdB6
   mov dword [FxTabledA2+0B7h*4],FxOpdB7
   mov dword [FxTabledA2+0B8h*4],FxOpdB8
   mov dword [FxTabledA2+0B9h*4],FxOpdB9
   mov dword [FxTabledA2+0BAh*4],FxOpdBA
   mov dword [FxTabledA2+0BBh*4],FxOpdBB
   mov dword [FxTabledA2+0BCh*4],FxOpdBC
   mov dword [FxTabledA2+0BDh*4],FxOpdBD
   mov dword [FxTabledA2+0BEh*4],FxOpdBE
   mov dword [FxTabledA2+0BFh*4],FxOpdBF

   mov dword [FxTabledA2+0C0h*4],FxOpdC0
   mov dword [FxTabledA2+0C1h*4],FxOpdC1A2
   mov dword [FxTabledA2+0C2h*4],FxOpdC2A2
   mov dword [FxTabledA2+0C3h*4],FxOpdC3A2
   mov dword [FxTabledA2+0C4h*4],FxOpdC4A2
   mov dword [FxTabledA2+0C5h*4],FxOpdC5A2
   mov dword [FxTabledA2+0C6h*4],FxOpdC6A2
   mov dword [FxTabledA2+0C7h*4],FxOpdC7A2
   mov dword [FxTabledA2+0C8h*4],FxOpdC8A2
   mov dword [FxTabledA2+0C9h*4],FxOpdC9A2
   mov dword [FxTabledA2+0CAh*4],FxOpdCAA2
   mov dword [FxTabledA2+0CBh*4],FxOpdCBA2
   mov dword [FxTabledA2+0CCh*4],FxOpdCCA2
   mov dword [FxTabledA2+0CDh*4],FxOpdCDA2
   mov dword [FxTabledA2+0CEh*4],FxOpdCEA2
   mov dword [FxTabledA2+0CFh*4],FxOpdCFA2

   mov dword [FxTabledA2+0D0h*4],FxOpdD0
   mov dword [FxTabledA2+0D1h*4],FxOpdD1
   mov dword [FxTabledA2+0D2h*4],FxOpdD2
   mov dword [FxTabledA2+0D3h*4],FxOpdD3
   mov dword [FxTabledA2+0D4h*4],FxOpdD4
   mov dword [FxTabledA2+0D5h*4],FxOpdD5
   mov dword [FxTabledA2+0D6h*4],FxOpdD6
   mov dword [FxTabledA2+0D7h*4],FxOpdD7
   mov dword [FxTabledA2+0D8h*4],FxOpdD8
   mov dword [FxTabledA2+0D9h*4],FxOpdD9
   mov dword [FxTabledA2+0DAh*4],FxOpdDA
   mov dword [FxTabledA2+0DBh*4],FxOpdDB
   mov dword [FxTabledA2+0DCh*4],FxOpdDC
   mov dword [FxTabledA2+0DDh*4],FxOpdDD
   mov dword [FxTabledA2+0DEh*4],FxOpdDE
   mov dword [FxTabledA2+0DFh*4],FxOpdDFA2

   mov dword [FxTabledA2+0E0h*4],FxOpdE0
   mov dword [FxTabledA2+0E1h*4],FxOpdE1
   mov dword [FxTabledA2+0E2h*4],FxOpdE2
   mov dword [FxTabledA2+0E3h*4],FxOpdE3
   mov dword [FxTabledA2+0E4h*4],FxOpdE4
   mov dword [FxTabledA2+0E5h*4],FxOpdE5
   mov dword [FxTabledA2+0E6h*4],FxOpdE6
   mov dword [FxTabledA2+0E7h*4],FxOpdE7
   mov dword [FxTabledA2+0E8h*4],FxOpdE8
   mov dword [FxTabledA2+0E9h*4],FxOpdE9
   mov dword [FxTabledA2+0EAh*4],FxOpdEA
   mov dword [FxTabledA2+0EBh*4],FxOpdEB
   mov dword [FxTabledA2+0ECh*4],FxOpdEC
   mov dword [FxTabledA2+0EDh*4],FxOpdED
   mov dword [FxTabledA2+0EEh*4],FxOpdEE
   mov dword [FxTabledA2+0EFh*4],FxOpdEFA2

   mov dword [FxTabledA2+0F0h*4],FxOpdF0A2
   mov dword [FxTabledA2+0F1h*4],FxOpdF1A2
   mov dword [FxTabledA2+0F2h*4],FxOpdF2A2
   mov dword [FxTabledA2+0F3h*4],FxOpdF3A2
   mov dword [FxTabledA2+0F4h*4],FxOpdF4A2
   mov dword [FxTabledA2+0F5h*4],FxOpdF5A2
   mov dword [FxTabledA2+0F6h*4],FxOpdF6A2
   mov dword [FxTabledA2+0F7h*4],FxOpdF7A2
   mov dword [FxTabledA2+0F8h*4],FxOpdF8A2
   mov dword [FxTabledA2+0F9h*4],FxOpdF9A2
   mov dword [FxTabledA2+0FAh*4],FxOpdFAA2
   mov dword [FxTabledA2+0FBh*4],FxOpdFBA2
   mov dword [FxTabledA2+0FCh*4],FxOpdFCA2
   mov dword [FxTabledA2+0FDh*4],FxOpdFDA2
   mov dword [FxTabledA2+0FEh*4],FxOpdFEA2
   mov dword [FxTabledA2+0FFh*4],FxOpdFFA2

   mov dword [FxTabledA3+00h*4],FxOpd00
   mov dword [FxTabledA3+01h*4],FxOpd01
   mov dword [FxTabledA3+02h*4],FxOpd02
   mov dword [FxTabledA3+03h*4],FxOpd03
   mov dword [FxTabledA3+04h*4],FxOpd04
   mov dword [FxTabledA3+05h*4],FxOpd05
   mov dword [FxTabledA3+06h*4],FxOpd06
   mov dword [FxTabledA3+07h*4],FxOpd07
   mov dword [FxTabledA3+08h*4],FxOpd08
   mov dword [FxTabledA3+09h*4],FxOpd09
   mov dword [FxTabledA3+0Ah*4],FxOpd0A
   mov dword [FxTabledA3+0Bh*4],FxOpd0B
   mov dword [FxTabledA3+0Ch*4],FxOpd0C
   mov dword [FxTabledA3+0Dh*4],FxOpd0D
   mov dword [FxTabledA3+0Eh*4],FxOpd0E
   mov dword [FxTabledA3+0Fh*4],FxOpd0F

   mov dword [FxTabledA3+10h*4],FxOpd10
   mov dword [FxTabledA3+11h*4],FxOpd11
   mov dword [FxTabledA3+12h*4],FxOpd12
   mov dword [FxTabledA3+13h*4],FxOpd13
   mov dword [FxTabledA3+14h*4],FxOpd14
   mov dword [FxTabledA3+15h*4],FxOpd15
   mov dword [FxTabledA3+16h*4],FxOpd16
   mov dword [FxTabledA3+17h*4],FxOpd17
   mov dword [FxTabledA3+18h*4],FxOpd18
   mov dword [FxTabledA3+19h*4],FxOpd19
   mov dword [FxTabledA3+1Ah*4],FxOpd1A
   mov dword [FxTabledA3+1Bh*4],FxOpd1B
   mov dword [FxTabledA3+1Ch*4],FxOpd1C
   mov dword [FxTabledA3+1Dh*4],FxOpd1D
   mov dword [FxTabledA3+1Eh*4],FxOpd1E
   mov dword [FxTabledA3+1Fh*4],FxOpd1F

   mov dword [FxTabledA3+20h*4],FxOpd20
   mov dword [FxTabledA3+21h*4],FxOpd21
   mov dword [FxTabledA3+22h*4],FxOpd22
   mov dword [FxTabledA3+23h*4],FxOpd23
   mov dword [FxTabledA3+24h*4],FxOpd24
   mov dword [FxTabledA3+25h*4],FxOpd25
   mov dword [FxTabledA3+26h*4],FxOpd26
   mov dword [FxTabledA3+27h*4],FxOpd27
   mov dword [FxTabledA3+28h*4],FxOpd28
   mov dword [FxTabledA3+29h*4],FxOpd29
   mov dword [FxTabledA3+2Ah*4],FxOpd2A
   mov dword [FxTabledA3+2Bh*4],FxOpd2B
   mov dword [FxTabledA3+2Ch*4],FxOpd2C
   mov dword [FxTabledA3+2Dh*4],FxOpd2D
   mov dword [FxTabledA3+2Eh*4],FxOpd2E
   mov dword [FxTabledA3+2Fh*4],FxOpd2F

   mov dword [FxTabledA3+30h*4],FxOpd30
   mov dword [FxTabledA3+31h*4],FxOpd31
   mov dword [FxTabledA3+32h*4],FxOpd32
   mov dword [FxTabledA3+33h*4],FxOpd33
   mov dword [FxTabledA3+34h*4],FxOpd34
   mov dword [FxTabledA3+35h*4],FxOpd35
   mov dword [FxTabledA3+36h*4],FxOpd36
   mov dword [FxTabledA3+37h*4],FxOpd37
   mov dword [FxTabledA3+38h*4],FxOpd38
   mov dword [FxTabledA3+39h*4],FxOpd39
   mov dword [FxTabledA3+3Ah*4],FxOpd3A
   mov dword [FxTabledA3+3Bh*4],FxOpd3B
   mov dword [FxTabledA3+3Ch*4],FxOpd3C
   mov dword [FxTabledA3+3Dh*4],FxOpd3D
   mov dword [FxTabledA3+3Eh*4],FxOpd3E
   mov dword [FxTabledA3+3Fh*4],FxOpd3F

   mov dword [FxTabledA3+40h*4],FxOpd40
   mov dword [FxTabledA3+41h*4],FxOpd41
   mov dword [FxTabledA3+42h*4],FxOpd42
   mov dword [FxTabledA3+43h*4],FxOpd43
   mov dword [FxTabledA3+44h*4],FxOpd44
   mov dword [FxTabledA3+45h*4],FxOpd45
   mov dword [FxTabledA3+46h*4],FxOpd46
   mov dword [FxTabledA3+47h*4],FxOpd47
   mov dword [FxTabledA3+48h*4],FxOpd48
   mov dword [FxTabledA3+49h*4],FxOpd49
   mov dword [FxTabledA3+4Ah*4],FxOpd4A
   mov dword [FxTabledA3+4Bh*4],FxOpd4B
   mov dword [FxTabledA3+4Ch*4],FxOpd4C
   mov dword [FxTabledA3+4Dh*4],FxOpd4D
   mov dword [FxTabledA3+4Eh*4],FxOpd4E
   mov dword [FxTabledA3+4Fh*4],FxOpd4F

   mov dword [FxTabledA3+50h*4],FxOpd50A3
   mov dword [FxTabledA3+51h*4],FxOpd51A3
   mov dword [FxTabledA3+52h*4],FxOpd52A3
   mov dword [FxTabledA3+53h*4],FxOpd53A3
   mov dword [FxTabledA3+54h*4],FxOpd54A3
   mov dword [FxTabledA3+55h*4],FxOpd55A3
   mov dword [FxTabledA3+56h*4],FxOpd56A3
   mov dword [FxTabledA3+57h*4],FxOpd57A3
   mov dword [FxTabledA3+58h*4],FxOpd58A3
   mov dword [FxTabledA3+59h*4],FxOpd59A3
   mov dword [FxTabledA3+5Ah*4],FxOpd5AA3
   mov dword [FxTabledA3+5Bh*4],FxOpd5BA3
   mov dword [FxTabledA3+5Ch*4],FxOpd5CA3
   mov dword [FxTabledA3+5Dh*4],FxOpd5DA3
   mov dword [FxTabledA3+5Eh*4],FxOpd5EA3
   mov dword [FxTabledA3+5Fh*4],FxOpd5FA3

   mov dword [FxTabledA3+60h*4],FxOpd60A3
   mov dword [FxTabledA3+61h*4],FxOpd61A3
   mov dword [FxTabledA3+62h*4],FxOpd62A3
   mov dword [FxTabledA3+63h*4],FxOpd63A3
   mov dword [FxTabledA3+64h*4],FxOpd64A3
   mov dword [FxTabledA3+65h*4],FxOpd65A3
   mov dword [FxTabledA3+66h*4],FxOpd66A3
   mov dword [FxTabledA3+67h*4],FxOpd67A3
   mov dword [FxTabledA3+68h*4],FxOpd68A3
   mov dword [FxTabledA3+69h*4],FxOpd69A3
   mov dword [FxTabledA3+6Ah*4],FxOpd6AA3
   mov dword [FxTabledA3+6Bh*4],FxOpd6BA3
   mov dword [FxTabledA3+6Ch*4],FxOpd6CA3
   mov dword [FxTabledA3+6Dh*4],FxOpd6DA3
   mov dword [FxTabledA3+6Eh*4],FxOpd6EA3
   mov dword [FxTabledA3+6Fh*4],FxOpd6FA3

   mov dword [FxTabledA3+70h*4],FxOpd70
   mov dword [FxTabledA3+71h*4],FxOpd71A3
   mov dword [FxTabledA3+72h*4],FxOpd72A3
   mov dword [FxTabledA3+73h*4],FxOpd73A3
   mov dword [FxTabledA3+74h*4],FxOpd74A3
   mov dword [FxTabledA3+75h*4],FxOpd75A3
   mov dword [FxTabledA3+76h*4],FxOpd76A3
   mov dword [FxTabledA3+77h*4],FxOpd77A3
   mov dword [FxTabledA3+78h*4],FxOpd78A3
   mov dword [FxTabledA3+79h*4],FxOpd79A3
   mov dword [FxTabledA3+7Ah*4],FxOpd7AA3
   mov dword [FxTabledA3+7Bh*4],FxOpd7BA3
   mov dword [FxTabledA3+7Ch*4],FxOpd7CA3
   mov dword [FxTabledA3+7Dh*4],FxOpd7DA3
   mov dword [FxTabledA3+7Eh*4],FxOpd7EA3
   mov dword [FxTabledA3+7Fh*4],FxOpd7FA3

   mov dword [FxTabledA3+80h*4],FxOpd80A3
   mov dword [FxTabledA3+81h*4],FxOpd81A3
   mov dword [FxTabledA3+82h*4],FxOpd82A3
   mov dword [FxTabledA3+83h*4],FxOpd83A3
   mov dword [FxTabledA3+84h*4],FxOpd84A3
   mov dword [FxTabledA3+85h*4],FxOpd85A3
   mov dword [FxTabledA3+86h*4],FxOpd86A3
   mov dword [FxTabledA3+87h*4],FxOpd87A3
   mov dword [FxTabledA3+88h*4],FxOpd88A3
   mov dword [FxTabledA3+89h*4],FxOpd89A3
   mov dword [FxTabledA3+8Ah*4],FxOpd8AA3
   mov dword [FxTabledA3+8Bh*4],FxOpd8BA3
   mov dword [FxTabledA3+8Ch*4],FxOpd8CA3
   mov dword [FxTabledA3+8Dh*4],FxOpd8DA3
   mov dword [FxTabledA3+8Eh*4],FxOpd8EA3
   mov dword [FxTabledA3+8Fh*4],FxOpd8FA3

   mov dword [FxTabledA3+90h*4],FxOpd90
   mov dword [FxTabledA3+91h*4],FxOpd91
   mov dword [FxTabledA3+92h*4],FxOpd92
   mov dword [FxTabledA3+93h*4],FxOpd93
   mov dword [FxTabledA3+94h*4],FxOpd94
   mov dword [FxTabledA3+95h*4],FxOpd95
   mov dword [FxTabledA3+96h*4],FxOpd96
   mov dword [FxTabledA3+97h*4],FxOpd97
   mov dword [FxTabledA3+98h*4],FxOpd98
   mov dword [FxTabledA3+99h*4],FxOpd99
   mov dword [FxTabledA3+9Ah*4],FxOpd9A
   mov dword [FxTabledA3+9Bh*4],FxOpd9B
   mov dword [FxTabledA3+9Ch*4],FxOpd9C
   mov dword [FxTabledA3+9Dh*4],FxOpd9D
   mov dword [FxTabledA3+9Eh*4],FxOpd9E
   mov dword [FxTabledA3+9Fh*4],FxOpd9F

   mov dword [FxTabledA3+0A0h*4],FxOpdA0
   mov dword [FxTabledA3+0A1h*4],FxOpdA1
   mov dword [FxTabledA3+0A2h*4],FxOpdA2
   mov dword [FxTabledA3+0A3h*4],FxOpdA3
   mov dword [FxTabledA3+0A4h*4],FxOpdA4
   mov dword [FxTabledA3+0A5h*4],FxOpdA5
   mov dword [FxTabledA3+0A6h*4],FxOpdA6
   mov dword [FxTabledA3+0A7h*4],FxOpdA7
   mov dword [FxTabledA3+0A8h*4],FxOpdA8
   mov dword [FxTabledA3+0A9h*4],FxOpdA9
   mov dword [FxTabledA3+0AAh*4],FxOpdAA
   mov dword [FxTabledA3+0ABh*4],FxOpdAB
   mov dword [FxTabledA3+0ACh*4],FxOpdAC
   mov dword [FxTabledA3+0ADh*4],FxOpdAD
   mov dword [FxTabledA3+0AEh*4],FxOpdAE
   mov dword [FxTabledA3+0AFh*4],FxOpdAF

   mov dword [FxTabledA3+0B0h*4],FxOpdB0
   mov dword [FxTabledA3+0B1h*4],FxOpdB1
   mov dword [FxTabledA3+0B2h*4],FxOpdB2
   mov dword [FxTabledA3+0B3h*4],FxOpdB3
   mov dword [FxTabledA3+0B4h*4],FxOpdB4
   mov dword [FxTabledA3+0B5h*4],FxOpdB5
   mov dword [FxTabledA3+0B6h*4],FxOpdB6
   mov dword [FxTabledA3+0B7h*4],FxOpdB7
   mov dword [FxTabledA3+0B8h*4],FxOpdB8
   mov dword [FxTabledA3+0B9h*4],FxOpdB9
   mov dword [FxTabledA3+0BAh*4],FxOpdBA
   mov dword [FxTabledA3+0BBh*4],FxOpdBB
   mov dword [FxTabledA3+0BCh*4],FxOpdBC
   mov dword [FxTabledA3+0BDh*4],FxOpdBD
   mov dword [FxTabledA3+0BEh*4],FxOpdBE
   mov dword [FxTabledA3+0BFh*4],FxOpdBF

   mov dword [FxTabledA3+0C0h*4],FxOpdC0
   mov dword [FxTabledA3+0C1h*4],FxOpdC1A3
   mov dword [FxTabledA3+0C2h*4],FxOpdC2A3
   mov dword [FxTabledA3+0C3h*4],FxOpdC3A3
   mov dword [FxTabledA3+0C4h*4],FxOpdC4A3
   mov dword [FxTabledA3+0C5h*4],FxOpdC5A3
   mov dword [FxTabledA3+0C6h*4],FxOpdC6A3
   mov dword [FxTabledA3+0C7h*4],FxOpdC7A3
   mov dword [FxTabledA3+0C8h*4],FxOpdC8A3
   mov dword [FxTabledA3+0C9h*4],FxOpdC9A3
   mov dword [FxTabledA3+0CAh*4],FxOpdCAA3
   mov dword [FxTabledA3+0CBh*4],FxOpdCBA3
   mov dword [FxTabledA3+0CCh*4],FxOpdCCA3
   mov dword [FxTabledA3+0CDh*4],FxOpdCDA3
   mov dword [FxTabledA3+0CEh*4],FxOpdCEA3
   mov dword [FxTabledA3+0CFh*4],FxOpdCFA3

   mov dword [FxTabledA3+0D0h*4],FxOpdD0
   mov dword [FxTabledA3+0D1h*4],FxOpdD1
   mov dword [FxTabledA3+0D2h*4],FxOpdD2
   mov dword [FxTabledA3+0D3h*4],FxOpdD3
   mov dword [FxTabledA3+0D4h*4],FxOpdD4
   mov dword [FxTabledA3+0D5h*4],FxOpdD5
   mov dword [FxTabledA3+0D6h*4],FxOpdD6
   mov dword [FxTabledA3+0D7h*4],FxOpdD7
   mov dword [FxTabledA3+0D8h*4],FxOpdD8
   mov dword [FxTabledA3+0D9h*4],FxOpdD9
   mov dword [FxTabledA3+0DAh*4],FxOpdDA
   mov dword [FxTabledA3+0DBh*4],FxOpdDB
   mov dword [FxTabledA3+0DCh*4],FxOpdDC
   mov dword [FxTabledA3+0DDh*4],FxOpdDD
   mov dword [FxTabledA3+0DEh*4],FxOpdDE
   mov dword [FxTabledA3+0DFh*4],FxOpdDFA3

   mov dword [FxTabledA3+0E0h*4],FxOpdE0
   mov dword [FxTabledA3+0E1h*4],FxOpdE1
   mov dword [FxTabledA3+0E2h*4],FxOpdE2
   mov dword [FxTabledA3+0E3h*4],FxOpdE3
   mov dword [FxTabledA3+0E4h*4],FxOpdE4
   mov dword [FxTabledA3+0E5h*4],FxOpdE5
   mov dword [FxTabledA3+0E6h*4],FxOpdE6
   mov dword [FxTabledA3+0E7h*4],FxOpdE7
   mov dword [FxTabledA3+0E8h*4],FxOpdE8
   mov dword [FxTabledA3+0E9h*4],FxOpdE9
   mov dword [FxTabledA3+0EAh*4],FxOpdEA
   mov dword [FxTabledA3+0EBh*4],FxOpdEB
   mov dword [FxTabledA3+0ECh*4],FxOpdEC
   mov dword [FxTabledA3+0EDh*4],FxOpdED
   mov dword [FxTabledA3+0EEh*4],FxOpdEE
   mov dword [FxTabledA3+0EFh*4],FxOpdEFA3

   mov dword [FxTabledA3+0F0h*4],FxOpdF0
   mov dword [FxTabledA3+0F1h*4],FxOpdF1
   mov dword [FxTabledA3+0F2h*4],FxOpdF2
   mov dword [FxTabledA3+0F3h*4],FxOpdF3
   mov dword [FxTabledA3+0F4h*4],FxOpdF4
   mov dword [FxTabledA3+0F5h*4],FxOpdF5
   mov dword [FxTabledA3+0F6h*4],FxOpdF6
   mov dword [FxTabledA3+0F7h*4],FxOpdF7
   mov dword [FxTabledA3+0F8h*4],FxOpdF8
   mov dword [FxTabledA3+0F9h*4],FxOpdF9
   mov dword [FxTabledA3+0FAh*4],FxOpdFA
   mov dword [FxTabledA3+0FBh*4],FxOpdFB
   mov dword [FxTabledA3+0FCh*4],FxOpdFC
   mov dword [FxTabledA3+0FDh*4],FxOpdFD
   mov dword [FxTabledA3+0FEh*4],FxOpdFE
   mov dword [FxTabledA3+0FFh*4],FxOpdFF
   ret

; normal
SECTION .data
NEWSYM sfxnametab
            db 'STOP    NOP     CACHE   LSR     '
            db 'ROL     BRA     BLT     BGE     '
            db 'BNE     BEQ     BPL     BMI     '
            db 'BCC     BCS     BVC     BVS     '

            db 'TO  R0  TO  R1  TO  R2  TO  R3  '       ; MOVE when B=1
            db 'TO  R4  TO  R5  TO  R6  TO  R7  '
            db 'TO  R8  TO  R9  TO  R10 TO  R11 '
            db 'TO  R12 TO  R13 TO  R14 TO  R15 '

            db 'WITH R0 WITH R1 WITH R2 WITH R3 '
            db 'WITH R4 WITH R5 WITH R6 WITH R7 '
            db 'WITH R8 WITH R9 WITH R10WITH R11'
            db 'WITH R12WITH R13WITH R14WITH R15'

            db 'STW(R0) STW(R1) STW(R2) STW(R3) '
            db 'STW(R4) STW(R5) STW(R6) STW(R7) '
            db 'STW(R8) STW(R9) STW(R10)STW(R11)'
            db 'LOOP    ALT1    ALT2    ALT3    '

            db 'LDW(R0) LDW(R1) LDW(R2) LDW(R3) '
            db 'LDW(R4) LDW(R5) LDW(R6) LDW(R7) '
            db 'LDW(R8) LDW(R9) LDW(R10)LDW(R11)'
            db 'PLOT    SWAP    COLOR   NOT     '

            db 'ADD R0  ADD R1  ADD R2  ADD R3  '
            db 'ADD R4  ADD R5  ADD R6  ADD R7  '
            db 'ADD R8  ADD R9  ADD R10 ADD R11 '
            db 'ADD R12 ADD R13 ADD R14 ADD R15 '

            db 'SUB R0  SUB R1  SUB R2  SUB R3  '
            db 'SUB R4  SUB R5  SUB R6  SUB R7  '
            db 'SUB R8  SUB R9  SUB R10 SUB R11 '
            db 'SUB R12 SUB R13 SUB R14 SUB R15 '

            db 'MERGE   AND R1  AND R2  AND R3  '
            db 'AND R4  AND R5  AND R6  AND R7  '
            db 'AND R8  AND R9  AND R10 AND R11 '
            db 'AND R12 AND R13 AND R14 AND R15 '

            db 'MULT R0 MULT R1 MULT R2 MULT R3 '
            db 'MULT R4 MULT R5 MULT R6 MULT R7 '
            db 'MULT R8 MULT R9 MULT R10MULT R11'
            db 'MULT R12MULT R13MULT R14MULT R15'

            db 'SBK     LINK #$1LINK #$2LINK #$3'
            db 'LINK #$4SEX     ASR     ROR     '
            db 'JMP R8  JMP R9  JMP R10 JMP R11 '
            db 'JMP R12 JMP R13 LOB     FMULT   '

            db 'IBT R0  IBT R1  IBT R2  IBT R3  '
            db 'IBT R4  IBT R5  IBT R6  IBT R7  '
            db 'IBT R8  IBT R9  IBT R10 IBT R11 '
            db 'IBT R12 IBT R13 IBT R14 IBT R15 '

            db 'FROM R0 FROM R1 FROM R2 FROM R3 '       ; MOVES if B=1
            db 'FROM R4 FROM R5 FROM R6 FROM R7 '
            db 'FROM R8 FROM R9 FROM R10FROM R11'
            db 'FROM R12FROM R13FROM R14FROM R15'

            db 'HIB     OR  R1  OR  R2  OR  R3  '
            db 'OR  R4  OR  R5  OR  R6  OR  R7  '
            db 'OR  R8  OR  R9  OR  R10 OR  R11 '
            db 'OR  R12 OR  R13 OR  R14 OR  R15 '

            db 'INC R0  INC R1  INC R2  INC R3  '
            db 'INC R4  INC R5  INC R6  INC R7  '
            db 'INC R8  INC R9  INC R10 INC R11 '
            db 'INC R12 INC R13 INC R14 GETC    '

            db 'DEC R0  DEC R1  DEC R2  DEC R3  '
            db 'DEC R4  DEC R5  DEC R6  DEC R7  '
            db 'DEC R8  DEC R9  DEC R10 DEC R11 '
            db 'DEC R12 DEC R13 DEC R14 GETB    '

            db 'IWT R0  IWT R1  IWT R2  IWT R3  '
            db 'IWT R4  IWT R5  IWT R6  IWT R7  '
            db 'IWT R8  IWT R9  IWT R10 IWT R11 '
            db 'IWT R12 IWT R13 IWT R14 IWT R15 '

;A1
            db 'STOP    NOP     CACHE   LSR     '
            db 'ROL     BRA     BLT     BGE     '
            db 'BNE     BEQ     BPL     BMI     '
            db 'BCC     BCS     BVC     BVS     '

            db 'TO  R0  TO  R1  TO  R2  TO  R3  '       ; MOVE when B=1
            db 'TO  R4  TO  R5  TO  R6  TO  R7  '
            db 'TO  R8  TO  R9  TO  R10 TO  R11 '
            db 'TO  R12 TO  R13 TO  R14 TO  R15 '

            db 'WITH R0 WITH R1 WITH R2 WITH R3 '
            db 'WITH R4 WITH R5 WITH R6 WITH R7 '
            db 'WITH R8 WITH R9 WITH R10WITH R11'
            db 'WITH R12WITH R13WITH R14WITH R15'

            db 'STB(R0) STB(R1) STB(R2) STB(R3) '
            db 'STB(R4) STB(R5) STB(R6) STB(R7) '
            db 'STB(R8) STB(R9) STB(R10)STB(R11)'
            db 'LOOP    ALT1    ALT2    ALT3    '

            db 'LDB(R0) LDB(R1) LDB(R2) LDB(R3) '
            db 'LDB(R4) LDB(R5) LDB(R6) LDB(R7) '
            db 'LDB(R8) LDB(R9) LDB(R10)LDB(R11)'
            db 'PLOT    SWAP    CMODE   NOT     '

            db 'ADC R0  ADC R1  ADC R2  ADC R3  '
            db 'ADC R4  ADC R5  ADC R6  ADC R7  '
            db 'ADC R8  ADC R9  ADC R10 ADC R11 '
            db 'ADC R12 ADC R13 ADC R14 ADC R15 '

            db 'SBC R0  SBC R1  SBC R2  SBC R3  '
            db 'SBC R4  SBC R5  SBC R6  SBC R7  '
            db 'SBC R8  SBC R9  SBC R10 SBC R11 '
            db 'SBC R12 SBC R13 SBC R14 SBC R15 '

            db 'MERGE   BIC R1  BIC R2  BIC R3  '
            db 'BIC R4  BIC R5  BIC R6  BIC R7  '
            db 'BIC R8  BIC R9  BIC R10 BIC R11 '
            db 'BIC R12 BIC R13 BIC R14 BIC R15 '

            db 'UMULTR0 UMULTR1 UMULTR2 UMULTR3 '
            db 'UMULTR4 UMULTR5 UMULTR6 UMULTR7 '
            db 'UMULTR8 UMULTR9 UMULTR10UMULTR11'
            db 'UMULTR12UMULTR13UMULTR14UMULTR15'

            db 'SBK     LINK #$1LINK #$2LINK #$3'
            db 'LINK #$4SEX     DIV2    ROR     '
            db 'LJMP R8 LJMP R9 LJMP R10LJMP R11'
            db 'LJMP R12LJMP R13LOB     LMULT   '

            db 'LMS R0  LMS R1  LMS R2  LMS R3  '
            db 'LMS R4  LMS R5  LMS R6  LMS R7  '
            db 'LMS R8  LMS R9  LMS R10 LMS R11 '
            db 'LMS R12 LMS R13 LMS R14 LMS R15 '

            db 'FROM R0 FROM R1 FROM R2 FROM R3 '       ; MOVES if B=1
            db 'FROM R4 FROM R5 FROM R6 FROM R7 '
            db 'FROM R8 FROM R9 FROM R10FROM R11'
            db 'FROM R12FROM R13FROM R14FROM R15'

            db 'HIB     XOR R1  XOR R2  XOR R3  '
            db 'XOR R4  XOR R5  XOR R6  XOR R7  '
            db 'XOR R8  XOR R9  XOR R10 XOR R11 '
            db 'XOR R12 XOR R13 XOR R14 XOR R15 '

            db 'INC R0  INC R1  INC R2  INC R3  '
            db 'INC R4  INC R5  INC R6  INC R7  '
            db 'INC R8  INC R9  INC R10 INC R11 '
            db 'INC R12 INC R13 INC R14 GETC    '

            db 'DEC R0  DEC R1  DEC R2  DEC R3  '
            db 'DEC R4  DEC R5  DEC R6  DEC R7  '
            db 'DEC R8  DEC R9  DEC R10 DEC R11 '
            db 'DEC R12 DEC R13 DEC R14 GETBH   '

            db 'LM  R0  LM  R1  LM  R2  LM  R3  '
            db 'LM  R4  LM  R5  LM  R6  LM  R7  '
            db 'LM  R8  LM  R9  LM  R10 LM  R11 '
            db 'LM  R12 LM  R13 LM  R14 LM  R15 '
;A2
            db 'STOP    NOP     CACHE   LSR     '
            db 'ROL     BRA     BLT     BGE     '
            db 'BNE     BEQ     BPL     BMI     '
            db 'BCC     BCS     BVC     BVS     '

            db 'TO  R0  TO  R1  TO  R2  TO  R3  '       ; MOVE when B=1
            db 'TO  R4  TO  R5  TO  R6  TO  R7  '
            db 'TO  R8  TO  R9  TO  R10 TO  R11 '
            db 'TO  R12 TO  R13 TO  R14 TO  R15 '

            db 'WITH R0 WITH R1 WITH R2 WITH R3 '
            db 'WITH R4 WITH R5 WITH R6 WITH R7 '
            db 'WITH R8 WITH R9 WITH R10WITH R11'
            db 'WITH R12WITH R13WITH R14WITH R15'

            db 'STW(R0) STW(R1) STW(R2) STW(R3) '
            db 'STW(R4) STW(R5) STW(R6) STW(R7) '
            db 'STW(R8) STW(R9) STW(R10)STW(R11)'
            db 'LOOP    ALT1    ALT2    ALT3    '

            db 'LDW(R0) LDW(R1) LDW(R2) LDW(R3) '
            db 'LDW(R4) LDW(R5) LDW(R6) LDW(R7) '
            db 'LDW(R8) LDW(R9) LDW(R10)LDW(R11)'
            db 'PLOT    SWAP    COLOR   NOT     '

            db 'ADD #$0 ADD #$1 ADD #$2 ADD #$3 '
            db 'ADD #$4 ADD #$5 ADD #$6 ADD #$7 '
            db 'ADD #$8 ADD #$9 ADD #$A ADD #$B '
            db 'ADD #$C ADD #$D ADD #$E ADD #$F '

            db 'SUB #$0 SUB #$1 SUB #$2 SUB #$3 '
            db 'SUB #$4 SUB #$5 SUB #$6 SUB #$7 '
            db 'SUB #$8 SUB #$9 SUB #$A SUB #$B '
            db 'SUB #$C SUB #$D SUB #$E SUB #$F '

            db 'MERGE   AND #$1 AND #$2 AND #$3 '
            db 'AND #$4 AND #$5 AND #$6 AND #$7 '
            db 'AND #$8 AND #$9 AND #$A AND #$B '
            db 'AND #$C AND #$D AND #$E AND #$F '

            db 'MULTIR0 MULTIR1 MULTIR2 MULTIR3 '
            db 'MULTIR4 MULTIR5 MULTIR6 MULTIR7 '
            db 'MULTIR8 MULTIR9 MULTIR10MULTIR11'
            db 'MULTIR12MULTIR13MULTIR14MULTIR15'

            db 'SBK     LINK #$1LINK #$2LINK #$3'
            db 'LINK #$4SEX     ASR     ROR     '
            db 'JMP R8  JMP R9  JMP R10 JMP R11 '
            db 'JMP R12 JMP R13 LOB     FMULT   '

            db 'SMS R0  SMS R1  SMS R2  SMS R3  '
            db 'SMS R4  SMS R5  SMS R6  SMS R7  '
            db 'SMS R8  SMS R9  SMS R10 SMS R11 '
            db 'SMS R12 SMS R13 SMS R14 SMS R15 '

            db 'FROM R0 FROM R1 FROM R2 FROM R3 '       ; MOVES if B=1
            db 'FROM R4 FROM R5 FROM R6 FROM R7 '
            db 'FROM R8 FROM R9 FROM R10FROM R11'
            db 'FROM R12FROM R13FROM R14FROM R15'

            db 'HIB     OR #$1  OR #$2  OR #$3  '
            db 'OR #$4  OR #$5  OR #$6  OR #$7  '
            db 'OR #$8  OR #$9  OR #$A  OR #$B  '
            db 'OR #$C  OR #$D  OR #$E  OR #$F  '

            db 'INC R0  INC R1  INC R2  INC R3  '
            db 'INC R4  INC R5  INC R6  INC R7  '
            db 'INC R8  INC R9  INC R10 INC R11 '
            db 'INC R12 INC R13 INC R14 RAMB    '

            db 'DEC R0  DEC R1  DEC R2  DEC R3  '
            db 'DEC R4  DEC R5  DEC R6  DEC R7  '
            db 'DEC R8  DEC R9  DEC R10 DEC R11 '
            db 'DEC R12 DEC R13 DEC R14 GETBL   '

            db 'SM  R0  SM  R1  SM  R2  SM  R3  '
            db 'SM  R4  SM  R5  SM  R6  SM  R7  '
            db 'SM  R8  SM  R9  SM  R10 SM  R11 '
            db 'SM  R12 SM  R13 SM  R14 SM  R15 '
;A3
            db 'STOP    NOP     CACHE   LSR     '
            db 'ROL     BRA     BLT     BGE     '
            db 'BNE     BEQ     BPL     BMI     '
            db 'BCC     BCS     BVC     BVS     '

            db 'TO  R0  TO  R1  TO  R2  TO  R3  '       ; MOVE when B=1
            db 'TO  R4  TO  R5  TO  R6  TO  R7  '
            db 'TO  R8  TO  R9  TO  R10 TO  R11 '
            db 'TO  R12 TO  R13 TO  R14 TO  R15 '

            db 'WITH R0 WITH R1 WITH R2 WITH R3 '
            db 'WITH R4 WITH R5 WITH R6 WITH R7 '
            db 'WITH R8 WITH R9 WITH R10WITH R11'
            db 'WITH R12WITH R13WITH R14WITH R15'

            db 'STW(R0) STW(R1) STW(R2) STW(R3) '
            db 'STW(R4) STW(R5) STW(R6) STW(R7) '
            db 'STW(R8) STW(R9) STW(R10)STW(R11)'
            db 'LOOP    ALT1    ALT2    ALT3    '

            db 'LDW(R0) LDW(R1) LDW(R2) LDW(R3) '
            db 'LDW(R4) LDW(R5) LDW(R6) LDW(R7) '
            db 'LDW(R8) LDW(R9) LDW(R10)LDW(R11)'
            db 'PLOT    SWAP    COLOR   NOT     '

            db 'ADC #$0 ADC #$1 ADC #$2 ADC #$3 '
            db 'ADC #$4 ADC #$5 ADC #$6 ADC #$7 '
            db 'ADC #$8 ADC #$9 ADC #$A ADC #$B '
            db 'ADC #$C ADC #$D ADC #$E ADC #$F '

            db 'CMP R0  CMP R1  CMP R2  CMP R3  '
            db 'CMP R4  CMP R5  CMP R6  CMP R7  '
            db 'CMP R8  CMP R9  CMP R10 CMP R11 '
            db 'CMP R12 CMP R13 CMP R14 CMP R15 '

            db 'MERGE   BIC #$1 BIC #$2 BIC #$3 '
            db 'BIC #$4 BIC #$5 BIC #$6 BIC #$7 '
            db 'BIC #$8 BIC #$9 BIC #$A BIC #$B '
            db 'BIC #$C BIC #$D BIC #$E BIC #$F '

            db 'UMULIR0 UMULIR1 UMULIR2 UMULIR3 '
            db 'UMULIR4 UMULIR5 UMULIR6 UMULIR7 '
            db 'UMULIR8 UMULIR9 UMULIR10UMULIR11'
            db 'UMULIR12UMULIR13UMULIR14UMULIR15'

            db 'SBK     LINK #$1LINK #$2LINK #$3'
            db 'LINK #$4SEX     ASR     ROR     '
            db 'JMP R8  JMP R9  JMP R10 JMP R11 '
            db 'JMP R12 JMP R13 LOB     FMULT   '

            db 'IBT R0  IBT R1  IBT R2  IBT R3  '
            db 'IBT R4  IBT R5  IBT R6  IBT R7  '
            db 'IBT R8  IBT R9  IBT R10 IBT R11 '
            db 'IBT R12 IBT R13 IBT R14 IBT R15 '

            db 'FROM R0 FROM R1 FROM R2 FROM R3 '       ; MOVES if B=1
            db 'FROM R4 FROM R5 FROM R6 FROM R7 '
            db 'FROM R8 FROM R9 FROM R10FROM R11'
            db 'FROM R12FROM R13FROM R14FROM R15'

            db 'HIB     XOR #$1 XOR #$2 XOR #$3 '
            db 'XOR #$4 XOR #$5 XOR #$6 XOR #$7 '
            db 'XOR #$8 XOR #$9 XOR #$A XOR #$B '
            db 'XOR #$C XOR #$D XOR #$E XOR #$F '

            db 'INC R0  INC R1  INC R2  INC R3  '
            db 'INC R4  INC R5  INC R6  INC R7  '
            db 'INC R8  INC R9  INC R10 INC R11 '
            db 'INC R12 INC R13 INC R14 ROMB    '

            db 'DEC R0  DEC R1  DEC R2  DEC R3  '
            db 'DEC R4  DEC R5  DEC R6  DEC R7  '
            db 'DEC R8  DEC R9  DEC R10 DEC R11 '
            db 'DEC R12 DEC R13 DEC R14 GETBS   '

            db 'IWT R0  IWT R1  IWT R2  IWT R3  '
            db 'IWT R4  IWT R5  IWT R6  IWT R7  '
            db 'IWT R8  IWT R9  IWT R10 IWT R11 '
            db 'IWT R12 IWT R13 IWT R14 IWT R15 '

SECTION .text
NEWSYM FxTableAsmEnd
