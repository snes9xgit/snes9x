/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

/*
  Win32 AVI Output module
  (c) 2004 blip
*/

#ifndef __AVIOUTPUT_H_
#define __AVIOUTPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

// create a new avi output object
void AVICreate(struct AVIFile** avi_out);

// set up avi output options
void AVISetFramerate(const int fps, const int frameskip, struct AVIFile* avi_out);
void AVISetVideoFormat(const BITMAPINFOHEADER* bitmap_format, struct AVIFile* avi_out);
void AVISetSoundFormat(const WAVEFORMATEX* wave_format, struct AVIFile* avi_out);

// after setting up output options, start writing with this
// returns 1 if successful, 0 otherwise
// check the return value!
int AVIBegin(const TCHAR* filename, struct AVIFile* avi_out);

// get the format in use from an already existing avi output object
// returns 1 if successful, 0 otherwise
int AVIGetVideoFormat(const struct AVIFile* avi_out, const BITMAPINFOHEADER** ppFormat);
int AVIGetSoundFormat(const struct AVIFile* avi_out, const WAVEFORMATEX** ppFormat);

// write video frames and sound samples to an avi output object
// that's been successfully Begun()
void AVIAddVideoFrame(void* bitmap_data, struct AVIFile* avi_out);
void AVIAddSoundSamples(void* sound_data, const int num_samples, struct AVIFile* avi_out);

// when done writing, close and delete the object with this
void AVIClose(struct AVIFile** avi_out);

#ifdef __cplusplus
}
#endif

#endif /* __AVIOUTPUT_H_ */
