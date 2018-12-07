/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

/*
  Win32 AVI Output module
  (c) 2004 blip
*/

#include <windows.h>
#include <vfw.h>
#include "wsnes9x.h"			// for GUI.hWnd
#include "AVIOutput.h"

#define VIDEO_STREAM	0
#define AUDIO_STREAM	1

struct AVIFile
{
	bool				valid;
	int					fps;
	int					frameskip;

	bool				video_added;
	BITMAPINFOHEADER	bitmap_format;

	bool				sound_added;
	WAVEFORMATEX		wave_format;

	AVISTREAMINFO		avi_video_header;
	AVISTREAMINFO		avi_sound_header;
	PAVIFILE			avi_file;
	PAVISTREAM			streams[2];
	PAVISTREAM			compressed_streams[2];

	AVICOMPRESSOPTIONS	compress_options[2];
	AVICOMPRESSOPTIONS*	compress_options_ptr[2];

	int					video_frames;
	int					sound_samples;

	long				tBytes, ByteBuffer;
};

static TCHAR saved_cur_avi_fnameandext[MAX_PATH];
static TCHAR saved_avi_fname[MAX_PATH];
static TCHAR saved_avi_ext[MAX_PATH];
static int avi_segnum=0;
static struct AVIFile saved_avi_info;
static int use_prev_options=0;

static bool truncate_existing(const TCHAR* filename)
{
	// this is only here because AVIFileOpen doesn't seem to do it for us
	FILE* fd = _tfopen(filename, TEXT("wb"));
	if(fd)
	{
		fclose(fd);
		return true;
	}

	return false;
}

static void clean_up(AVIFile* _avi)
{
	AVIFile& avi = *_avi;

	if(avi.sound_added)
	{
		if(avi.compressed_streams[AUDIO_STREAM])
		{
			AVIStreamClose(avi.compressed_streams[AUDIO_STREAM]);
			avi.compressed_streams[AUDIO_STREAM] = NULL;
			avi.streams[AUDIO_STREAM] = NULL;				// compressed_streams[AUDIO_STREAM] is just a copy of streams[AUDIO_STREAM]
		}
	}

	if(avi.video_added)
	{
		if(avi.compressed_streams[VIDEO_STREAM])
		{
			AVIStreamClose(avi.compressed_streams[VIDEO_STREAM]);
			avi.compressed_streams[VIDEO_STREAM] = NULL;
		}

		if(avi.streams[VIDEO_STREAM])
		{
			AVIStreamClose(avi.streams[VIDEO_STREAM]);
			avi.streams[VIDEO_STREAM] = NULL;
		}
	}

	if(avi.avi_file)
	{
		AVIFileClose(avi.avi_file);
		avi.avi_file = NULL;
	}
}

#ifdef __cplusplus
extern "C" {
#endif

void AVICreate(struct AVIFile** avi_out)
{
	*avi_out = new AVIFile;
	memset(*avi_out, 0, sizeof(AVIFile));
	AVIFileInit();
}

void AVIClose(struct AVIFile** avi_out)
{
	if(*avi_out)
	{
		clean_up(*avi_out);
		delete *avi_out;
	}
	*avi_out = NULL;
	AVIFileExit();
}

void AVISetFramerate(const int fps, const int frameskip, struct AVIFile* avi_out)
{
	avi_out->fps = fps;
	avi_out->frameskip = frameskip;
}

void AVISetVideoFormat(const BITMAPINFOHEADER* bitmap_format, struct AVIFile* avi_out)
{
	memcpy(&(avi_out->bitmap_format), bitmap_format, sizeof(BITMAPINFOHEADER));
	avi_out->video_added = true;
}

void AVISetSoundFormat(const WAVEFORMATEX* wave_format, struct AVIFile* avi_out)
{
	memcpy(&(avi_out->wave_format), wave_format, sizeof(WAVEFORMATEX));
	avi_out->sound_added = true;
}

int AVIBegin(const TCHAR* filename, struct AVIFile* _avi_out)
{
	AVIFile& avi = *_avi_out;
	int result = 0;

	do
	{
		if(!avi.video_added)
			break;

		if(!truncate_existing(filename))
			break;

		// open the file
		if(FAILED(AVIFileOpen(&avi.avi_file, filename, OF_CREATE | OF_WRITE, NULL)))
			break;

		// create the video stream
		memset(&avi.avi_video_header, 0, sizeof(AVISTREAMINFO));
		avi.avi_video_header.fccType = streamtypeVIDEO;
		avi.avi_video_header.dwScale = avi.frameskip;
		avi.avi_video_header.dwRate = avi.fps;
		avi.avi_video_header.dwSuggestedBufferSize = avi.bitmap_format.biSizeImage;
		if(FAILED(AVIFileCreateStream(avi.avi_file, &avi.streams[VIDEO_STREAM], &avi.avi_video_header)))
			break;

		if(use_prev_options)
		{
			avi.compress_options[VIDEO_STREAM] = saved_avi_info.compress_options[VIDEO_STREAM];
			avi.compress_options_ptr[VIDEO_STREAM] = &avi.compress_options[0];
		}
		else
		{
			// get compression options
			memset(&avi.compress_options[VIDEO_STREAM], 0, sizeof(AVICOMPRESSOPTIONS));
			avi.compress_options_ptr[VIDEO_STREAM] = &avi.compress_options[0];
			if(!AVISaveOptions(GUI.hWnd, 0, 1, &avi.streams[VIDEO_STREAM], &avi.compress_options_ptr[VIDEO_STREAM]))
				break;
		}

		// create compressed stream
		if(FAILED(AVIMakeCompressedStream(&avi.compressed_streams[VIDEO_STREAM], avi.streams[VIDEO_STREAM], &avi.compress_options[VIDEO_STREAM], NULL)))
			break;

		// set the stream format
		if(FAILED(AVIStreamSetFormat(avi.compressed_streams[VIDEO_STREAM], 0, (void*)&avi.bitmap_format, avi.bitmap_format.biSize)))
			break;

		// add sound (if requested)
		if(avi.sound_added)
		{
			// create the audio stream
			memset(&avi.avi_sound_header, 0, sizeof(AVISTREAMINFO));
			avi.avi_sound_header.fccType = streamtypeAUDIO;
			avi.avi_sound_header.dwQuality = (DWORD)-1;
			avi.avi_sound_header.dwScale = avi.wave_format.nBlockAlign;
			avi.avi_sound_header.dwRate = avi.wave_format.nAvgBytesPerSec;
			avi.avi_sound_header.dwSampleSize = avi.wave_format.nBlockAlign;
			avi.avi_sound_header.dwInitialFrames = 1;
			if(FAILED(AVIFileCreateStream(avi.avi_file, &avi.streams[AUDIO_STREAM], &avi.avi_sound_header)))
				break;

			// AVISaveOptions doesn't seem to work for audio streams
			// so here we just copy the pointer for the compressed stream
			avi.compressed_streams[AUDIO_STREAM] = avi.streams[AUDIO_STREAM];

			// set the stream format
			if(FAILED(AVIStreamSetFormat(avi.compressed_streams[AUDIO_STREAM], 0, (void*)&avi.wave_format, sizeof(WAVEFORMATEX))))
				break;
		}

		// initialize counters
		avi.video_frames = 0;
		avi.sound_samples = 0;
		avi.tBytes = 0;
		avi.ByteBuffer = 0;

		lstrcpyn(saved_cur_avi_fnameandext,filename,MAX_PATH);
		lstrcpyn(saved_avi_fname,filename,MAX_PATH);
		TCHAR* dot = _tcsrchr(saved_avi_fname, TEXT('.'));
		if(dot && dot > _tcsrchr(saved_avi_fname, TEXT('/')) && dot > _tcsrchr(saved_avi_fname, TEXT('\\')))
		{
			lstrcpy(saved_avi_ext,dot);
			dot[0]=TEXT('\0');
		}

		// success
		result = 1;
		avi.valid = true;

	} while(false);

	if(!result)
	{
		clean_up(&avi);
		avi.valid = false;
	}

	return result;
}

int AVIGetVideoFormat(const struct AVIFile* avi_out, const BITMAPINFOHEADER** ppFormat)
{
	if(!avi_out->valid ||
		!avi_out->video_added)
	{
		if(ppFormat)
		{
			*ppFormat = NULL;
		}
		return 0;
	}
	if(ppFormat)
	{
		*ppFormat = &avi_out->bitmap_format;
	}
	return 1;
}

int AVIGetSoundFormat(const struct AVIFile* avi_out, const WAVEFORMATEX** ppFormat)
{
	if(!avi_out->valid ||
		!avi_out->sound_added)
	{
		if(ppFormat)
		{
			*ppFormat = NULL;
		}
		return 0;
	}
	if(ppFormat)
	{
		*ppFormat = &avi_out->wave_format;
	}
	return 1;
}

static int AVINextSegment(struct AVIFile* avi_out)
{
	TCHAR avi_fname[MAX_PATH];
	lstrcpy(avi_fname,saved_avi_fname);
	TCHAR avi_fname_temp[MAX_PATH];
	_stprintf(avi_fname_temp, TEXT("%s_part%d%s"), avi_fname, avi_segnum+2, saved_avi_ext);
	saved_avi_info=*avi_out;
	use_prev_options=1;
	avi_segnum++;
	clean_up(avi_out);
	int ret = AVIBegin(avi_fname_temp, avi_out);
	use_prev_options=0;
	lstrcpy(saved_avi_fname,avi_fname);
	return ret;
}

void AVIAddVideoFrame(void* bitmap_data, struct AVIFile* avi_out)
{
	if(!avi_out->valid ||
		!avi_out->video_added)
	{
		return;
	}

    if(FAILED(AVIStreamWrite(avi_out->compressed_streams[VIDEO_STREAM],
                             avi_out->video_frames,
                             1,
                             bitmap_data,
                             avi_out->bitmap_format.biSizeImage,
                             AVIIF_KEYFRAME, NULL, &avi_out->ByteBuffer)))
	{
		avi_out->valid = false;
		return;
	}

	avi_out->video_frames++;
	avi_out->tBytes += avi_out->ByteBuffer;

	// segment / split AVI when it's almost 2 GB (2000MB, to be precise)
	if(!(avi_out->video_frames % 60) && avi_out->tBytes > 2097152000)
		AVINextSegment(avi_out);
}

void AVIAddSoundSamples(void* sound_data, const int num_samples, struct AVIFile* avi_out)
{
	if(!avi_out->valid ||
		!avi_out->sound_added)
	{
		return;
	}

	int data_length = num_samples * avi_out->wave_format.nBlockAlign;
    if(FAILED(AVIStreamWrite(avi_out->compressed_streams[AUDIO_STREAM],
                             avi_out->sound_samples,
                             num_samples,
                             sound_data,
                             data_length,
                             0, NULL, &avi_out->ByteBuffer)))
	{
		avi_out->valid = false;
		return;
	}

	avi_out->sound_samples += num_samples;
	avi_out->tBytes += avi_out->ByteBuffer;
}

#ifdef __cplusplus
}
#endif
