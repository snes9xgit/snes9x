#include "win32_webcam.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>

#include <mutex>
#include <thread>
#include <atomic>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

void S9xGBSetCameraCallback(bool (*cb)(unsigned char *dst, int width, int height));

namespace {

std::mutex                 g_lock;
std::vector<unsigned char> g_frame;
int                        g_frameW = 0;
int                        g_frameH = 0;
bool                       g_frameBottomUp = true;
bool                       g_frameValid = false;

std::thread                g_thread;
std::atomic<bool>          g_stop{false};
bool                       g_running = false;
bool                       g_mfStarted = false;

std::vector<std::wstring>  g_symlinks;

bool EnsureMF()
{
	if (g_mfStarted) return true;
	if (FAILED(MFStartup(MF_VERSION, MFSTARTUP_LITE))) return false;
	g_mfStarted = true;
	return true;
}

int EnumDevices(std::vector<std::wstring> *names)
{
	g_symlinks.clear();
	if (names) names->clear();
	if (!EnsureMF()) return 0;

	IMFAttributes *attr = nullptr;
	if (FAILED(MFCreateAttributes(&attr, 1))) return 0;
	attr->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);

	IMFActivate **devs = nullptr;
	UINT32 count = 0;
	HRESULT hr = MFEnumDeviceSources(attr, &devs, &count);
	attr->Release();
	if (FAILED(hr)) return 0;

	for (UINT32 i = 0; i < count; ++i)
	{
		WCHAR *name = nullptr; UINT32 nlen = 0;
		if (SUCCEEDED(devs[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &name, &nlen)) && name)
		{
			if (names) names->push_back(name);
			CoTaskMemFree(name);
		}
		else if (names) names->push_back(L"Camera");

		WCHAR *link = nullptr; UINT32 llen = 0;
		if (SUCCEEDED(devs[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &link, &llen)) && link)
		{
			g_symlinks.push_back(link);
			CoTaskMemFree(link);
		}
		else
		{
			g_symlinks.push_back(std::wstring());
		}

		devs[i]->Release();
	}
	CoTaskMemFree(devs);
	return static_cast<int>(count);
}

IMFSourceReader *OpenReader(const std::wstring &link)
{
	IMFAttributes *sattr = nullptr;
	if (FAILED(MFCreateAttributes(&sattr, 2))) return nullptr;
	sattr->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	sattr->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, link.c_str());

	IMFMediaSource *src = nullptr;
	HRESULT hr = MFCreateDeviceSource(sattr, &src);
	sattr->Release();
	if (FAILED(hr) || !src) return nullptr;

	IMFAttributes *rattr = nullptr;
	MFCreateAttributes(&rattr, 1);
	rattr->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);

	IMFSourceReader *reader = nullptr;
	hr = MFCreateSourceReaderFromMediaSource(src, rattr, &reader);
	if (rattr) rattr->Release();
	src->Release();
	if (FAILED(hr) || !reader) return nullptr;

	IMFMediaType *mt = nullptr;
	MFCreateMediaType(&mt);
	mt->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	mt->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
	hr = reader->SetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), nullptr, mt);
	mt->Release();
	if (FAILED(hr)) { reader->Release(); return nullptr; }

	reader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), TRUE);
	return reader;
}

void ReadFrameSize(IMFSourceReader *reader, int &w, int &h, bool &bottomUp)
{
	IMFMediaType *cur = nullptr;
	if (SUCCEEDED(reader->GetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), &cur)) && cur)
	{
		UINT32 ww = 0, hh = 0;
		MFGetAttributeSize(cur, MF_MT_FRAME_SIZE, &ww, &hh);
		w = static_cast<int>(ww);
		h = static_cast<int>(hh);
		UINT32 stride = 0;
		if (SUCCEEDED(cur->GetUINT32(MF_MT_DEFAULT_STRIDE, &stride)))
			bottomUp = (static_cast<INT32>(stride) < 0);
		cur->Release();
	}
}

void CaptureThread(std::wstring link)
{
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	IMFSourceReader *reader = OpenReader(link);
	if (reader)
	{
		int w = 0, h = 0; bool bottomUp = true;
		ReadFrameSize(reader, w, h, bottomUp);

		while (!g_stop.load())
		{
			DWORD streamIndex = 0, flags = 0;
			LONGLONG ts = 0;
			IMFSample *sample = nullptr;
			HRESULT hr = reader->ReadSample(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM),
			                                0, &streamIndex, &flags, &ts, &sample);
			if (FAILED(hr)) break;
			if (flags & MF_SOURCE_READERF_ENDOFSTREAM) { if (sample) sample->Release(); break; }
			if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
				ReadFrameSize(reader, w, h, bottomUp);

			if (sample && w > 0 && h > 0)
			{
				IMFMediaBuffer *buf = nullptr;
				if (SUCCEEDED(sample->ConvertToContiguousBuffer(&buf)) && buf)
				{
					BYTE *data = nullptr; DWORD maxLen = 0, curLen = 0;
					if (SUCCEEDED(buf->Lock(&data, &maxLen, &curLen)))
					{
						const size_t need = static_cast<size_t>(w) * h * 4;
						if (curLen >= need)
						{
							std::lock_guard<std::mutex> lk(g_lock);
							g_frame.assign(data, data + need);
							g_frameW = w;
							g_frameH = h;
							g_frameBottomUp = bottomUp;
							g_frameValid = true;
						}
						buf->Unlock();
					}
					buf->Release();
				}
			}
			if (sample) sample->Release();
		}
		reader->Release();
	}
	{
		std::lock_guard<std::mutex> lk(g_lock);
		g_frameValid = false;
	}
	CoUninitialize();
}

bool CameraGetImageCB(unsigned char *dst, int outW, int outH)
{
	std::lock_guard<std::mutex> lk(g_lock);
	if (!g_frameValid || g_frameW <= 0 || g_frameH <= 0 || g_frame.empty())
		return false;

	const unsigned char *sp = g_frame.data();
	const int sw = g_frameW, sh = g_frameH;
	for (int y = 0; y < outH; ++y)
	{
		int sy = y * sh / outH;
		if (g_frameBottomUp) sy = sh - 1 - sy;
		if (sy < 0) sy = 0; else if (sy >= sh) sy = sh - 1;
		for (int x = 0; x < outW; ++x)
		{
			int sx = sw - 1 - (x * sw / outW);
			if (sx < 0) sx = 0; else if (sx >= sw) sx = sw - 1;
			const unsigned char *px = sp + (static_cast<size_t>(sy) * sw + sx) * 4;
			const int b = px[0], g = px[1], r = px[2];
			dst[y * outW + x] = static_cast<unsigned char>((r * 54 + g * 183 + b * 19) >> 8);
		}
	}
	return true;
}

} // namespace

void GBCameraRegister()
{
	S9xGBSetCameraCallback(&CameraGetImageCB);
}

void GBCameraEnumerate(std::vector<std::wstring> &names)
{
	EnumDevices(&names);
}

bool GBCameraStart(int deviceIndex)
{
	GBCameraStop();
	if (!EnsureMF()) return false;

	std::vector<std::wstring> names;
	const int n = EnumDevices(&names);
	if (deviceIndex < 0 || deviceIndex >= n) return false;
	const std::wstring link = g_symlinks[deviceIndex];
	if (link.empty()) return false;

	g_stop.store(false);
	g_thread = std::thread(CaptureThread, link);
	g_running = true;
	return true;
}

void GBCameraStop()
{
	if (g_running)
	{
		g_stop.store(true);
		if (g_thread.joinable()) g_thread.join();
		g_running = false;
	}
	std::lock_guard<std::mutex> lk(g_lock);
	g_frameValid = false;
}

bool GBCameraIsRunning()
{
	return g_running;
}
