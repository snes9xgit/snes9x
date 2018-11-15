/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

//  JMA compressed file support
//  (c) Copyright 2004-2006 NSRT Team (http://nsrt.edgeemu.com)


#ifdef __cplusplus
extern "C" {
#endif
size_t load_jma_file(const char *filename, unsigned char *buffer);
#ifdef __cplusplus
}
#endif
