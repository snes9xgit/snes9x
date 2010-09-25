/*
Copyright (c) 1998-2005 Charles Bilyue'.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <string.h>

int open_error(const char *filename, const char *mode)
{
 printf("Failure opening %s for %s\n",filename,mode);
 return 1;
}

int main(int argc, char **argv)
{
 int c;
 int section_count;
 FILE *in, *out;
 unsigned char section_header[40];

 if (argc < 2 || argc > 3)
 {
  printf("Fixes MS Win32 object files to be compatible with the incorrect\n");
  printf(" implementation in MinGW32.\n");
  printf("Usage: objfix infile [outfile]\n");
  return 1;
 }

 in = fopen(argv[1], (argc == 2 ? "rb+" : "rb"));
 if (!in) return open_error(argv[1], (argc == 2 ? "read" : "update"));

 if (argc == 3)
 {
  out = fopen(argv[2], "wb");
  if (!out) return open_error(argv[1], (argc == 2 ? "read" : "update"));
 }
 else
 {
  out = NULL;
 }

 if (out)
 {
  fputc(fgetc(in), out);
  fputc(fgetc(in), out);

  fputc(section_count = fgetc(in), out);
  fputc(c = fgetc(in), out);
  section_count += c << 8;

  for (c = 4; c < 0x14; c++)
  {
   fputc(fgetc(in), out);
  }

  for (c = 0; c < section_count; c++)
  {
   fread(section_header, 1, 40, in);

   if (!strncmp(section_header, ".bss", 8))
   {
    memcpy(section_header + 8, section_header + 16, 4);
    memset(section_header + 16, 0, 4);
   }
   else
   {
    memset(section_header + 8, 0, 4);
   }

   fwrite(section_header, 1, 40, out);
  }

  while ((c = fgetc(in)) != EOF)
  {
   fputc(c, out);
  }
 }
 else
 {
  fgetc(in);
  fgetc(in);

  section_count = fgetc(in);
  section_count += fgetc(in) << 8;

  fseek(in, 0x14, SEEK_SET);

  for (c = 0; c < section_count; c++)
  {
   fread(section_header, 1, 40, in);

   fseek(in, -40, SEEK_CUR);

   if (!strncmp(section_header, ".bss", 8))
   {
    memcpy(section_header + 8, section_header + 16, 4);
    memset(section_header + 16, 0, 4);
   }
   else
   {
    memset(section_header + 8, 0, 4);
   }

   fwrite(section_header, 1, 40, in);

   fseek(in, 0, SEEK_CUR);
  }
 }

 fclose(in);
 if (out) fclose(out);

 return 0;
}
