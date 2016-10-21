/*
NOTICE

The software accompanying this notice (the "Software") is provided to you
free of charge to facilitate your use of the data collected by the Mars
Orbiter Camera (the "MOC Data").  Malin Space Science Systems ("MSSS")
grants to you (either as an individual or entity) a personal,
non-transferable, and non-exclusive right (i) to use and reproduce the
Software solely for the purpose of accessing the MOC Data; (ii) to modify
the source code of the Software as necessary to maintain or adapt the
Software to run on alternate computer platforms; and (iii) to compile, use
and reproduce the modified versions of the Software solely for the purpose
of accessing the MOC Data.  In addition, you may distribute the Software,
including any modifications thereof, solely for use with the MOC Data,
provided that (i) you must include this notice with all copies of the
Software to be distributed; (ii) you may not remove or alter any
proprietary notices contained in the Software; (iii) you may not charge any
third party for the Software; and (iv) you will not export the Software
without the appropriate United States and foreign government licenses.

You acknowledge that no title to the intellectual property in the Software
is transferred to you.  You further acknowledge that title and full
ownership rights to the Software will remain the exclusive property of MSSS
or its suppliers, and you will not acquire any rights to the Software
except as expressly set forth above.  The Software is provided to you AS
IS.  MSSS MAKES NO WARRANTY, EXPRESS OR IMPLIED, WITH RESPECT TO THE
SOFTWARE, AND SPECIFICALLY DISCLAIMS THE IMPLIED WARRANTIES OF
NON-INFRINGEMENT OF THIRD PARTY RIGHTS, MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE.  SOME JURISDICTIONS DO NOT ALLOW THE EXCLUSION OR
LIMITATION OF INCIDENTAL OR CONSEQUENTIAL DAMAGES, SO SUCH LIMITATIONS OR
EXCLUSIONS MAY NOT APPLY TO YOU.

Your use or reproduction of the Software constitutes your agreement to the
terms of this Notice.  If you do not agree with the terms of this notice,
promptly return or destroy all copies of the Software in your possession.

Copyright (C) 1999 Malin Space Science Systems.  All Rights Reserved.

SCCSID @(#)predcode.h  1.1 10/04/99
*/
/* SCCSID @(#)predcode.h (predcode.h) 1.2 */

/*
    Predictive compressor code and requant tables.

    NOTE this is a testing release only.  The lossless tables
    are fairly solid.  Table 0 is an identity mapping for
    testing only and should be deleted prior to flight.
    The lossy table should be examined before committing.

    REFINE should PROM space be at a premium, the length tables
    could be encoded into the code tables the same way the
    hardware compressor does it.  The expanded form is still
    needed in RAM because that's what the WA predictive
    compressor uses.
*/

#ifndef predcode_h
#define predcode_h

#include "fs.h"

#ifdef DEFINE_CODE_TABLES

/* IDENTITY; NO COMPRESSION -- dumped from 'default.code' */
uint16 Code0Bits[256] = {
  0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
  0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
  0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
  0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
  0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
  0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
  0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
  0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
  0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
  0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
  0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
  0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
  0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
  0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
  0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
  0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
  0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
  0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
  0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
  0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
  0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
  0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
  0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
  0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
  0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
  0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
  0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
  0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
  0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
  0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
  0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
  0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff,
};

uint8 Code0Len[256] = {
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
};

/* dumped from 'exp01.code' */
uint16 Code1Bits[256] = {
  0x0000, 0x0001, 0x000d, 0x0055, 0x00f5, 0x0375, 0x0135, 0x1135,
  0x5a75, 0x1a75, 0x6a75, 0x2a75, 0x4a75, 0x0a75, 0x7275, 0x3275,
  0x5275, 0x1275, 0x6275, 0x2275, 0x4275, 0x0275, 0x7c75, 0x3c75,
  0x5c75, 0x1c75, 0x6c75, 0x2c75, 0x4c75, 0x0c75, 0x7475, 0x3475,
  0x5475, 0x1475, 0x6475, 0x2475, 0x4475, 0x0475, 0x7875, 0x3875,
  0x5875, 0x1875, 0x6875, 0x2875, 0x4875, 0x0875, 0x7075, 0x3075,
  0x5075, 0x1075, 0x6075, 0x2075, 0x4075, 0x0075, 0x7fb5, 0x3fb5,
  0x5fb5, 0x1fb5, 0x6fb5, 0x2fb5, 0x4fb5, 0x0fb5, 0x77b5, 0x37b5,
  0x57b5, 0x17b5, 0x67b5, 0x27b5, 0x47b5, 0x07b5, 0x7bb5, 0x3bb5,
  0x5bb5, 0x1bb5, 0x6bb5, 0x2bb5, 0x4bb5, 0x0bb5, 0x73b5, 0x33b5,
  0x53b5, 0x13b5, 0x63b5, 0x23b5, 0x43b5, 0x03b5, 0x7db5, 0x3db5,
  0x5db5, 0x1db5, 0x6db5, 0x2db5, 0x4db5, 0x0db5, 0x75b5, 0x35b5,
  0x55b5, 0x15b5, 0x65b5, 0x25b5, 0x45b5, 0x05b5, 0x79b5, 0x39b5,
  0x59b5, 0x19b5, 0x69b5, 0x29b5, 0x49b5, 0x09b5, 0x71b5, 0x31b5,
  0x51b5, 0x11b5, 0x61b5, 0x21b5, 0x41b5, 0x01b5, 0x7eb5, 0x3eb5,
  0x5eb5, 0x1eb5, 0x3a75, 0x2eb5, 0x4eb5, 0x6eb5, 0x6675, 0x1675,
  0x5675, 0x16b5, 0x66b5, 0x26b5, 0x46b5, 0x06b5, 0x7ab5, 0x3ab5,
  0x5ab5, 0x1ab5, 0x6ab5, 0x2ab5, 0x4ab5, 0x0ab5, 0x72b5, 0x32b5,
  0x52b5, 0x12b5, 0x62b5, 0x22b5, 0x42b5, 0x02b5, 0x7cb5, 0x3cb5,
  0x5cb5, 0x1cb5, 0x6cb5, 0x2cb5, 0x4cb5, 0x0cb5, 0x74b5, 0x34b5,
  0x54b5, 0x14b5, 0x64b5, 0x24b5, 0x44b5, 0x04b5, 0x78b5, 0x38b5,
  0x58b5, 0x18b5, 0x68b5, 0x28b5, 0x48b5, 0x08b5, 0x70b5, 0x30b5,
  0x50b5, 0x10b5, 0x60b5, 0x20b5, 0x40b5, 0x00b5, 0x0eb5, 0x3f35,
  0x7f35, 0x1f35, 0x6f35, 0x2f35, 0x4f35, 0x0f35, 0x7735, 0x3735,
  0x5735, 0x1735, 0x6735, 0x2735, 0x4735, 0x0735, 0x7b35, 0x3b35,
  0x5b35, 0x1b35, 0x6b35, 0x2b35, 0x4b35, 0x0b35, 0x7335, 0x3335,
  0x5335, 0x1335, 0x6335, 0x2335, 0x5f35, 0x4335, 0x7d35, 0x3d35,
  0x5d35, 0x1d35, 0x6d35, 0x2d35, 0x4d35, 0x0d35, 0x7535, 0x3535,
  0x5535, 0x1535, 0x6535, 0x0335, 0x2535, 0x0535, 0x7935, 0x3935,
  0x5935, 0x1935, 0x6935, 0x4535, 0x2935, 0x0935, 0x7135, 0x4935,
  0x5135, 0x3135, 0x56b5, 0x36b5, 0x76b5, 0x7a75, 0x0675, 0x4675,
  0x2675, 0x3675, 0x0e75, 0x0175, 0x0035, 0x0015, 0x0005, 0x0003,
};

uint8 Code1Len[256] = {
  1,  3,  4,  7,  8, 10, 13, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 12, 10,  9,  7,  5,  2,
};

/* dumped from 'exp02.code' */
uint16 Code2Bits[256] = {
  0x0002, 0x0000, 0x0005, 0x000c, 0x0011, 0x003c, 0x00fc, 0x0101,
  0x0081, 0x0181, 0x0d81, 0x0dc1, 0x09c1, 0x0ec1, 0x76c1, 0x36c1,
  0x56c1, 0x16c1, 0x66c1, 0x26c1, 0x46c1, 0x06c1, 0x7ac1, 0x3ac1,
  0x5ac1, 0x1ac1, 0x6ac1, 0x2ac1, 0x4ac1, 0x0ac1, 0x72c1, 0x32c1,
  0x52c1, 0x12c1, 0x62c1, 0x22c1, 0x42c1, 0x02c1, 0x7cc1, 0x3cc1,
  0x5cc1, 0x1cc1, 0x6cc1, 0x2cc1, 0x4cc1, 0x0cc1, 0x74c1, 0x34c1,
  0x54c1, 0x14c1, 0x64c1, 0x24c1, 0x44c1, 0x04c1, 0x78c1, 0x38c1,
  0x58c1, 0x18c1, 0x68c1, 0x28c1, 0x48c1, 0x08c1, 0x70c1, 0x30c1,
  0x50c1, 0x10c1, 0x60c1, 0x20c1, 0x40c1, 0x00c1, 0x7f41, 0x3f41,
  0x5f41, 0x1f41, 0x6f41, 0x2f41, 0x4f41, 0x0f41, 0x7741, 0x3741,
  0x5741, 0x1741, 0x6741, 0x2741, 0x4741, 0x0741, 0x7b41, 0x3b41,
  0x5b41, 0x1b41, 0x6b41, 0x2b41, 0x4b41, 0x0b41, 0x7341, 0x3341,
  0x5341, 0x1341, 0x6341, 0x2341, 0x4341, 0x0341, 0x7d41, 0x3d41,
  0x5d41, 0x1d41, 0x6d41, 0x2d41, 0x4d41, 0x0d41, 0x7541, 0x3541,
  0x5541, 0x1541, 0x6541, 0x2541, 0x4541, 0x0541, 0x7941, 0x3941,
  0x7ec1, 0x6ec1, 0x21c1, 0x41c1, 0x4ec1, 0x5941, 0x61c1, 0x11c1,
  0x51c1, 0x1141, 0x6141, 0x2141, 0x4141, 0x0141, 0x7e41, 0x3e41,
  0x5e41, 0x1e41, 0x6e41, 0x2e41, 0x4e41, 0x0e41, 0x7641, 0x3641,
  0x5641, 0x1641, 0x6641, 0x2641, 0x4641, 0x0641, 0x7a41, 0x3a41,
  0x5a41, 0x1a41, 0x6a41, 0x2a41, 0x4a41, 0x0a41, 0x7241, 0x3241,
  0x5241, 0x1241, 0x6241, 0x2241, 0x4241, 0x0241, 0x7c41, 0x3c41,
  0x5c41, 0x1c41, 0x6c41, 0x2c41, 0x4c41, 0x0c41, 0x7441, 0x3441,
  0x5441, 0x1441, 0x1941, 0x2941, 0x4941, 0x0441, 0x7841, 0x3841,
  0x5841, 0x1841, 0x6841, 0x2841, 0x4841, 0x0841, 0x7041, 0x3041,
  0x5041, 0x1041, 0x6041, 0x2041, 0x4041, 0x0041, 0x7f81, 0x3f81,
  0x5f81, 0x1f81, 0x6f81, 0x2f81, 0x4f81, 0x2441, 0x0f81, 0x3781,
  0x5781, 0x1781, 0x6781, 0x2781, 0x4781, 0x0781, 0x7b81, 0x3b81,
  0x5b81, 0x1b81, 0x6b81, 0x7781, 0x2b81, 0x0b81, 0x7381, 0x3381,
  0x5381, 0x1381, 0x4b81, 0x6381, 0x4381, 0x2381, 0x0381, 0x31c1,
  0x4441, 0x6441, 0x7141, 0x0941, 0x2ec1, 0x6941, 0x1ec1, 0x5ec1,
  0x3ec1, 0x01c1, 0x5141, 0x3141, 0x19c1, 0x05c1, 0x0581, 0x03c1,
  0x0281, 0x0001, 0x007c, 0x0021, 0x001c, 0x0009, 0x0004, 0x0003,
};

uint8 Code2Len[256] = {
  2,  3,  3,  5,  5,  7,  8,  9, 10, 11, 12, 12, 13, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 14, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 13, 12, 12, 10, 10,  9,  8,  6,  6,  4,  4,  2,
};

/* dumped from 'exp04.code' */
uint16 Code3Bits[256] = {
  0x0004, 0x0006, 0x0007, 0x0005, 0x000b, 0x000d, 0x0018, 0x001d,
  0x0038, 0x007d, 0x00f8, 0x0003, 0x0001, 0x0183, 0x01c3, 0x0343,
  0x02c3, 0x0683, 0x0e81, 0x08c3, 0x0181, 0x0c83, 0x1181, 0x04c3,
  0x5f41, 0x1f41, 0x6f41, 0x2f41, 0x4f41, 0x0f41, 0x7741, 0x3741,
  0x5741, 0x1741, 0x6741, 0x2741, 0x4741, 0x0741, 0x7b41, 0x3b41,
  0x5b41, 0x1b41, 0x6b41, 0x2b41, 0x4b41, 0x0b41, 0x7341, 0x3341,
  0x5341, 0x1341, 0x6341, 0x2341, 0x4341, 0x0341, 0x7d41, 0x3d41,
  0x5d41, 0x1d41, 0x6d41, 0x2d41, 0x4d41, 0x0d41, 0x7541, 0x3541,
  0x5541, 0x1541, 0x6541, 0x2541, 0x4541, 0x0541, 0x7941, 0x3941,
  0x5941, 0x1941, 0x6941, 0x2941, 0x4941, 0x0941, 0x7141, 0x3141,
  0x5141, 0x1141, 0x6141, 0x2141, 0x4141, 0x0141, 0x7e41, 0x3e41,
  0x5e41, 0x1e41, 0x6e41, 0x2e41, 0x4e41, 0x0e41, 0x7641, 0x3641,
  0x5641, 0x1641, 0x6641, 0x2641, 0x4641, 0x0641, 0x7a41, 0x3a41,
  0x5a41, 0x1a41, 0x6a41, 0x2a41, 0x4a41, 0x0a41, 0x7241, 0x4883,
  0x7f41, 0x5883, 0x1883, 0x0483, 0x7883, 0x3f41, 0x3241, 0x3c41,
  0x5c41, 0x1c41, 0x2483, 0x4483, 0x4c41, 0x0c41, 0x6483, 0x1483,
  0x5483, 0x1441, 0x6441, 0x2441, 0x4441, 0x0441, 0x7841, 0x3841,
  0x5841, 0x1841, 0x6841, 0x2841, 0x4841, 0x0841, 0x7041, 0x3041,
  0x5041, 0x1041, 0x6041, 0x2041, 0x4041, 0x0041, 0x7f81, 0x3f81,
  0x5f81, 0x1f81, 0x6f81, 0x2f81, 0x4f81, 0x0f81, 0x7781, 0x3781,
  0x5781, 0x1781, 0x5241, 0x0241, 0x4781, 0x7c41, 0x6781, 0x3b81,
  0x5b81, 0x1b81, 0x6b81, 0x2b81, 0x4b81, 0x0b81, 0x7381, 0x3381,
  0x5381, 0x1381, 0x6381, 0x2381, 0x4381, 0x0381, 0x7d81, 0x3d81,
  0x5d81, 0x1d81, 0x6d81, 0x2781, 0x0781, 0x7b81, 0x2d81, 0x3581,
  0x5581, 0x1581, 0x6581, 0x2581, 0x4581, 0x0581, 0x7981, 0x7581,
  0x4d81, 0x1981, 0x6981, 0x2981, 0x4981, 0x5981, 0x0981, 0x3181,
  0x7181, 0x24c3, 0x3981, 0x0d81, 0x2241, 0x6241, 0x1241, 0x0083,
  0x4083, 0x2083, 0x6083, 0x2883, 0x4241, 0x6883, 0x1083, 0x5083,
  0x3083, 0x3883, 0x5441, 0x3441, 0x7441, 0x2c41, 0x6c41, 0x7083,
  0x0883, 0x3483, 0x14c3, 0x1c83, 0x0cc3, 0x00c3, 0x0681, 0x0283,
  0x0281, 0x0143, 0x0081, 0x0043, 0x0101, 0x00c1, 0x0078, 0x003d,
  0x0023, 0x0021, 0x0013, 0x0011, 0x0008, 0x0009, 0x0000, 0x0002,
};

uint8 Code3Len[256] = {
  3,  3,  3,  4,  4,  5,  6,  6,  7,  7,  8,  8,  9,  9,  9, 10,
  10, 11, 12, 12, 13, 13, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 13, 13, 12, 12, 12, 11,
  11, 10, 10,  9,  9,  8,  8,  7,  6,  6,  5,  5,  5,  4,  4,  3,
};

/* dumped from 'exp06.code' */
uint16 Code4Bits[256] = {
  0x0005, 0x0007, 0x0004, 0x0001, 0x0010, 0x000a, 0x0009, 0x0018,
  0x001a, 0x0019, 0x0038, 0x003a, 0x0039, 0x0078, 0x007a, 0x0000,
  0x0002, 0x0006, 0x0300, 0x0102, 0x0306, 0x0480, 0x0482, 0x0486,
  0x0280, 0x0282, 0x0682, 0x0a80, 0x1680, 0x1d46, 0x0b46, 0x3e80,
  0x2d46, 0x2680, 0x0680, 0x3646, 0x5646, 0x1646, 0x6646, 0x2646,
  0x4646, 0x0646, 0x7a46, 0x3a46, 0x5a46, 0x1a46, 0x6a46, 0x2a46,
  0x4a46, 0x0a46, 0x7246, 0x3246, 0x5246, 0x1246, 0x6246, 0x2246,
  0x4246, 0x0246, 0x7c46, 0x3c46, 0x5c46, 0x1c46, 0x6c46, 0x2c46,
  0x4c46, 0x0c46, 0x7446, 0x3446, 0x5446, 0x1446, 0x6446, 0x2446,
  0x4446, 0x0446, 0x7846, 0x3846, 0x5846, 0x1846, 0x6846, 0x2846,
  0x4846, 0x0846, 0x7046, 0x3046, 0x5046, 0x1046, 0x6046, 0x2046,
  0x4046, 0x0046, 0x7f86, 0x3f86, 0x5f86, 0x1f86, 0x6f86, 0x2f86,
  0x4f86, 0x0f86, 0x7786, 0x0546, 0x2546, 0x4546, 0x6786, 0x2786,
  0x4786, 0x0786, 0x7b86, 0x3b86, 0x5b86, 0x1b86, 0x6b86, 0x2b86,
  0x7646, 0x3786, 0x7386, 0x1546, 0x6546, 0x1386, 0x6386, 0x2386,
  0x4386, 0x0386, 0x3546, 0x5546, 0x5d86, 0x1d86, 0x7546, 0x0d46,
  0x4d46, 0x0d86, 0x7586, 0x3586, 0x5586, 0x1586, 0x6586, 0x2586,
  0x4586, 0x0586, 0x7986, 0x3986, 0x5986, 0x1986, 0x6986, 0x2986,
  0x4986, 0x5786, 0x4b86, 0x3186, 0x5186, 0x1186, 0x6186, 0x0b86,
  0x0986, 0x0186, 0x7e86, 0x3e86, 0x5e86, 0x1e86, 0x6e86, 0x2e86,
  0x4e86, 0x0e86, 0x7686, 0x3686, 0x5686, 0x1686, 0x6686, 0x2686,
  0x7186, 0x2186, 0x7a86, 0x4186, 0x4686, 0x1a86, 0x6a86, 0x2a86,
  0x4a86, 0x0a86, 0x7286, 0x0686, 0x3a86, 0x5a86, 0x3286, 0x2286,
  0x4286, 0x5286, 0x6680, 0x0286, 0x6286, 0x1286, 0x1786, 0x0e46,
  0x4e46, 0x2e46, 0x4d86, 0x2d86, 0x6d86, 0x3d86, 0x7d86, 0x5386,
  0x3386, 0x6e46, 0x1e46, 0x5e46, 0x3e46, 0x7e46, 0x0146, 0x4146,
  0x2146, 0x6146, 0x1146, 0x5146, 0x3146, 0x7146, 0x0946, 0x4946,
  0x2946, 0x6946, 0x1946, 0x5946, 0x3946, 0x7946, 0x3346, 0x4680,
  0x1346, 0x1e80, 0x1b46, 0x0346, 0x0e80, 0x1a80, 0x0e82, 0x0a82,
  0x0746, 0x0086, 0x0082, 0x0080, 0x0106, 0x0302, 0x0100, 0x0182,
  0x0180, 0x00c6, 0x00fa, 0x00f8, 0x0079, 0x0042, 0x0040, 0x0026,
  0x0022, 0x0020, 0x0016, 0x0012, 0x0008, 0x000e, 0x000c, 0x0003,
};

uint8 Code4Len[256] = {
  3,  3,  4,  4,  5,  5,  5,  6,  6,  6,  7,  7,  7,  8,  8,  9,
  9,  9, 10, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 13, 14,
  14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 15,
  14, 14, 13, 13, 13, 13, 12, 12, 11, 11, 11, 11, 10, 10, 10,  9,
  9,  8,  8,  8,  7,  7,  7,  6,  6,  6,  5,  5,  5,  4,  4,  3,
};

/* dumped from 'exp08.code' */
uint16 Code5Bits[256] = {
  0x0008, 0x000c, 0x0006, 0x000d, 0x0007, 0x0012, 0x0009, 0x000b,
  0x0010, 0x001a, 0x0019, 0x001b, 0x0040, 0x004a, 0x0041, 0x007b,
  0x00f0, 0x00fa, 0x0003, 0x00c3, 0x000a, 0x0001, 0x0083, 0x0100,
  0x010a, 0x028a, 0x0183, 0x0300, 0x030a, 0x0301, 0x0043, 0x0480,
  0x008a, 0x0081, 0x0a43, 0x1080, 0x048a, 0x0679, 0x0643, 0x1e43,
  0x148a, 0x3a79, 0x0c80, 0x1643, 0x4079, 0x0079, 0x7f81, 0x3f81,
  0x5f81, 0x1f81, 0x6f81, 0x2f81, 0x4f81, 0x0f81, 0x7781, 0x3781,
  0x5781, 0x1781, 0x6781, 0x2781, 0x4781, 0x0781, 0x7b81, 0x3b81,
  0x5b81, 0x1b81, 0x6b81, 0x2b81, 0x4b81, 0x0b81, 0x7381, 0x3381,
  0x5381, 0x1381, 0x6381, 0x2381, 0x4381, 0x0381, 0x7d81, 0x3d81,
  0x5d81, 0x1d81, 0x6d81, 0x2d81, 0x4d81, 0x0d81, 0x7581, 0x3581,
  0x5581, 0x1581, 0x6581, 0x2581, 0x4581, 0x0581, 0x6c79, 0x6079,
  0x7c79, 0x3c79, 0x6279, 0x2279, 0x3279, 0x5279, 0x7181, 0x3181,
  0x5181, 0x1181, 0x6181, 0x2079, 0x7981, 0x0181, 0x7e81, 0x3e81,
  0x5e81, 0x1e81, 0x6e81, 0x0a79, 0x7279, 0x0e81, 0x7681, 0x3681,
  0x5681, 0x1681, 0x2a79, 0x4a79, 0x4681, 0x0681, 0x6a79, 0x1a79,
  0x5a79, 0x1a81, 0x6a81, 0x2a81, 0x4a81, 0x3981, 0x2181, 0x3281,
  0x5281, 0x1281, 0x6281, 0x2281, 0x4181, 0x0a81, 0x7c81, 0x3c81,
  0x5c81, 0x1c81, 0x6c81, 0x2c81, 0x4c81, 0x0c81, 0x7481, 0x3481,
  0x5481, 0x7281, 0x4281, 0x2481, 0x0281, 0x1481, 0x7881, 0x3881,
  0x5881, 0x1881, 0x6881, 0x6481, 0x4481, 0x2881, 0x748a, 0x4881,
  0x0881, 0x4c80, 0x348a, 0x0481, 0x6981, 0x1981, 0x5981, 0x1079,
  0x5079, 0x3079, 0x7079, 0x1c79, 0x4981, 0x2981, 0x5c79, 0x0879,
  0x4879, 0x2879, 0x6879, 0x0279, 0x0981, 0x4279, 0x1879, 0x5879,
  0x3879, 0x1279, 0x5a81, 0x3a81, 0x7a81, 0x2681, 0x6681, 0x4e81,
  0x2e81, 0x7879, 0x0479, 0x4479, 0x2479, 0x6479, 0x1479, 0x5479,
  0x3479, 0x7479, 0x0c79, 0x4c79, 0x2c79, 0x3643, 0x2e79, 0x0e79,
  0x2c80, 0x0e43, 0x1e79, 0x1679, 0x1c80, 0x0080, 0x0243, 0x0c8a,
  0x088a, 0x0880, 0x0443, 0x0701, 0x070a, 0x0700, 0x0383, 0x0101,
  0x0280, 0x0143, 0x0179, 0x018a, 0x0180, 0x0000, 0x00f9, 0x007a,
  0x0070, 0x003b, 0x0039, 0x003a, 0x0030, 0x0023, 0x0021, 0x002a,
  0x0020, 0x0013, 0x0011, 0x0002, 0x000f, 0x0005, 0x000e, 0x0004,
};

uint8 Code5Len[256] = {
  4,  4,  4,  4,  4,  5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,
  8,  8,  8,  8,  9,  9,  9, 10, 10, 10, 10, 11, 11, 11, 11, 12,
  12, 12, 12, 13, 13, 13, 13, 13, 14, 14, 15, 14, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 14, 14, 14, 14, 13, 13, 13, 13, 13, 12, 12,
  12, 12, 11, 11, 11, 11, 10, 10, 10,  9,  9,  9,  9,  9,  8,  8,
  8,  7,  7,  7,  7,  6,  6,  6,  6,  5,  5,  5,  4,  4,  4,  4,
};

/* dumped from 'exp10.code' */
uint16 Code6Bits[256] = {
  0x000a, 0x000e, 0x0005, 0x000f, 0x0008, 0x001c, 0x0011, 0x000b,
  0x0010, 0x0014, 0x0012, 0x0019, 0x0023, 0x0040, 0x0044, 0x0032,
  0x0039, 0x003b, 0x0070, 0x0084, 0x0072, 0x00f9, 0x00fb, 0x01f0,
  0x01f4, 0x00f2, 0x0103, 0x0000, 0x0204, 0x0002, 0x0009, 0x0183,
  0x0500, 0x0304, 0x0302, 0x0509, 0x0383, 0x0900, 0x0f04, 0x0702,
  0x0003, 0x0f83, 0x0004, 0x0804, 0x1f02, 0x1789, 0x1783, 0x0f00,
  0x2102, 0x0102, 0x1803, 0x3100, 0x1100, 0x3803, 0x6e89, 0x2e89,
  0x4e89, 0x0e89, 0x7689, 0x3689, 0x5689, 0x1689, 0x6689, 0x2689,
  0x4689, 0x0689, 0x7a89, 0x3a89, 0x5a89, 0x1a89, 0x6a89, 0x2a89,
  0x4a89, 0x0a89, 0x7289, 0x3289, 0x5289, 0x1289, 0x6289, 0x2289,
  0x4289, 0x0289, 0x7c89, 0x3c89, 0x5c89, 0x1c89, 0x6c89, 0x2c89,
  0x4c89, 0x0c89, 0x7589, 0x3589, 0x6d89, 0x2d89, 0x7d89, 0x3d89,
  0x6389, 0x2389, 0x7389, 0x3389, 0x2b89, 0x4b89, 0x6889, 0x3e89,
  0x5e89, 0x0889, 0x7089, 0x3089, 0x5089, 0x1089, 0x6089, 0x2089,
  0x4089, 0x0089, 0x7f09, 0x1b89, 0x6b89, 0x1f09, 0x6f09, 0x2f09,
  0x4f09, 0x0f09, 0x3b89, 0x5b89, 0x5709, 0x1709, 0x7b89, 0x0789,
  0x4789, 0x7709, 0x7b09, 0x3b09, 0x5b09, 0x1b09, 0x6b09, 0x2b09,
  0x4b09, 0x3709, 0x4709, 0x3309, 0x0709, 0x0b09, 0x6309, 0x2309,
  0x4309, 0x7309, 0x5309, 0x0309, 0x7102, 0x3102, 0x5804, 0x1309,
  0x5489, 0x1e89, 0x7489, 0x1489, 0x6489, 0x7e89, 0x5589, 0x0189,
  0x4189, 0x2189, 0x6189, 0x0d89, 0x4489, 0x2489, 0x4d89, 0x1189,
  0x5189, 0x3189, 0x7189, 0x1d89, 0x7889, 0x0489, 0x5d89, 0x0989,
  0x4989, 0x2989, 0x6989, 0x0389, 0x5889, 0x3889, 0x4389, 0x1989,
  0x5989, 0x3989, 0x7989, 0x1389, 0x1889, 0x5389, 0x0589, 0x4589,
  0x2589, 0x0b89, 0x4889, 0x2709, 0x6709, 0x2889, 0x3489, 0x5f09,
  0x3f09, 0x6589, 0x1589, 0x5100, 0x1804, 0x7100, 0x2789, 0x3804,
  0x1102, 0x2f00, 0x0783, 0x0803, 0x0f02, 0x1004, 0x1f00, 0x0100,
  0x0f89, 0x0902, 0x0704, 0x0700, 0x0403, 0x0109, 0x0502, 0x0404,
  0x0300, 0x0203, 0x0209, 0x0202, 0x0104, 0x0200, 0x0083, 0x01f2,
  0x00f4, 0x00f0, 0x007b, 0x0079, 0x0082, 0x0074, 0x0080, 0x0043,
  0x0049, 0x0042, 0x0034, 0x0030, 0x001b, 0x0029, 0x0022, 0x0024,
  0x0020, 0x0013, 0x0001, 0x000c, 0x0018, 0x0007, 0x000d, 0x0006,
};

uint8 Code6Len[256] = {
  4,  4,  4,  4,  5,  5,  5,  5,  6,  6,  6,  6,  6,  7,  7,  7,
  7,  7,  8,  8,  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 10, 10,
  11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 14,
  14, 14, 14, 15, 15, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 14,
  14, 14, 13, 13, 13, 13, 13, 13, 12, 12, 12, 12, 11, 11, 11, 11,
  11, 10, 10, 10, 10, 10,  9,  9,  9,  9,  8,  8,  8,  8,  8,  7,
  7,  7,  7,  7,  6,  6,  6,  6,  6,  5,  5,  5,  5,  4,  4,  4,
};

/* LOSSY -- dumped from 'tmspread10.code' */
uint16 Code7Bits[256] = {
  0x0005, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001,
  0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
  0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
  0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
  0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
  0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
  0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
  0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d,
  0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d,
  0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d,
  0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d,
  0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d,
  0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d,
  0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d,
  0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d,
  0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d, 0x001d,
  0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d,
  0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d,
  0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d,
  0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d,
  0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d,
  0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d,
  0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d,
  0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d,
  0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d,
  0x000d, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003,
  0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003,
  0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003,
  0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003,
  0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003,
  0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003,
  0x0003, 0x0003, 0x0003, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
};

uint8 Code7Len[256] = {
  4,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
};

/* dumped from 'tmspread10.requant' */
uint8 Code7Requant[256] = {
  0,   1,   1,   1,   1,   1,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
  10,  10,  10,  10,  10,  10,  10,  10, 100, 100, 100, 100, 100, 100, 100, 100,
  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
  156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156,
  156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156,
  156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156,
  156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156, 156,
  156, 156, 156, 156, 156, 156, 156, 156, 156, 246, 246, 246, 246, 246, 246, 246,
  246, 246, 246, 246, 246, 246, 246, 246, 246, 246, 246, 246, 246, 246, 246, 246,
  246, 246, 246, 246, 246, 246, 246, 246, 246, 246, 246, 246, 246, 246, 246, 246,
  246, 246, 246, 246, 246, 246, 246, 246, 246, 246, 246, 255, 255, 255, 255, 255,
};

/* dumped from 'default.requant' */
/* REFINE this could be calculated instead of put into PROM (obviously) */
uint8 CodeIdentRequant[256] = {
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
  96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
  112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
  128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
  144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
  160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
  176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
  192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
  208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
  224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
  240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
};

uint16 *CodeBitsVec[] = { Code0Bits, Code1Bits, Code2Bits, Code3Bits,
                          Code4Bits, Code5Bits, Code6Bits, Code7Bits
                        };

uint8 *CodeLenVec[] = { Code0Len, Code1Len, Code2Len, Code3Len,
                        Code4Len, Code5Len, Code6Len, Code7Len
                      };

uint8 *CodeRequantVec[] = { CodeIdentRequant, CodeIdentRequant,
                            CodeIdentRequant, CodeIdentRequant,
                            CodeIdentRequant, CodeIdentRequant,
                            CodeIdentRequant, Code7Requant
                          };

#else
extern uint16 *CodeBitsVec[8];
extern uint8 *CodeLenVec[8];
extern uint8 *CodeRequantVec[8];
#endif

#endif