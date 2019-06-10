/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

//  JMA compressed file support
//  (c) Copyright 2004-2006 NSRT Team (http://nsrt.edgeemu.com)


#include "snes9x.h"
#include "memmap.h"

#include <vector>
using namespace std;

#include "s9x-jma.h"
#include "jma.h"

size_t load_jma_file(const char *filename, unsigned char *buffer)
{
  try
  {
    JMA::jma_open JMAFile(filename);
    vector<JMA::jma_public_file_info> file_info = JMAFile.get_files_info();

    string our_file_name;
    size_t our_file_size = 0;

    for (vector<JMA::jma_public_file_info>::iterator i = file_info.begin(); i != file_info.end(); i++)
    {
      //Check for valid ROM based on size
      if ((i->size <= CMemory::MAX_ROM_SIZE+512) && (i->size > our_file_size))
      {
        our_file_name = i->name;
        our_file_size = i->size;
      }
    }

    if (!our_file_size)
    {
      return(0);
    }

    JMAFile.extract_file(our_file_name, buffer);

    return(our_file_size);
  }
  catch (JMA::jma_errors jma_error)
  {
    return(0);
  }
}
