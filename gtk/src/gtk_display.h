#ifndef __GTK_DISPLAY_H
#define __GTK_DISPLAY_H

#include "gtk_s9x.h"
#include "filter/2xsai.h"
#ifdef USE_HQ2X
#include "filter/hq2x.h"
#endif
#ifdef USE_XBRZ
#include "filter_xbrz.h"
#endif
#include "filter/epx.h"
#include "filter_epx_unsafe.h"

#define FILTER_NONE                 0
#define FILTER_SUPEREAGLE           1
#define FILTER_2XSAI                2
#define FILTER_SUPER2XSAI           3
#define FILTER_EPX                  4
#define FILTER_EPX_SMOOTH           5
#define FILTER_NTSC                 6
#define FILTER_SCANLINES            7
#define FILTER_SIMPLE2X             8
#define FILTER_SIMPLE3X             9
#define FILTER_SIMPLE4X             10
#define FILTER_HQ2X                 11
#define FILTER_HQ3X                 12
#define FILTER_HQ4X                 13
#define FILTER_2XBRZ                14
#define FILTER_3XBRZ                15
#define FILTER_4XBRZ                16
#define NUM_FILTERS                 17

#define NTSC_COMPOSITE              0
#define NTSC_SVIDEO                 1
#define NTSC_RGB                    2

#define ENDIAN_LSB                  0
#define ENDIAN_MSB                  1

#define JOB_FILTER                  0
#define JOB_CONVERT                 1
#define JOB_SCALE_AND_CONVERT       2
#define JOB_CONVERT_YUV             3
#define JOB_CONVERT_MASK            4

typedef struct thread_job_t
{
    int operation_type;
    uint8 *src_buffer;
    int src_pitch;
    uint8 *dst_buffer;
    int dst_pitch;
    int width;
    int height;
    int dst_width;
    int dst_height;
    int bpp;
    int inv_rmask;
    int inv_gmask;
    int inv_bmask;
    int line_start;
    int line_end;

    volatile int complete;
}
thread_job_t;

void S9xRegisterYUVTables (uint8 *y, uint8 *u, uint8 *v);
void S9xSetEndianess (int type);
double S9xGetAspect (void);
void S9xApplyAspect (int&, int&, int&, int&);

void S9xConvertYUV (void *src_buffer,
                    void *dst_buffer,
                    int src_pitch,
                    int dst_pitch,
                    int width,
                    int height);

void S9xConvert (void *src,
                 void *dst,
                 int src_pitch,
                 int dst_pitch,
                 int width,
                 int height,
                 int bpp);

void S9xConvertMask (void *src,
                     void *dst,
                     int  src_pitch,
                     int  dst_pitch,
                     int  width,
                     int  height,
                     int  rshift,
                     int  gshift,
                     int  bshift,
                     int  bpp);

void S9xFilter (uint8 *src_buffer,
                int src_pitch,
                uint8 *dst_buffer,
                int dst_pitch,
                int& width,
                int& height);
void get_filter_scale (int& width, int& height);

void S9xDisplayRefresh (int width, int height);
void S9xDisplayClearBuffers (void);
void S9xReinitDisplay (void);
void S9xDisplayReconfigure (void);
void S9xQueryDrivers (void);

#endif /* __GTK_DISPLAY_H */
