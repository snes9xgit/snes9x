//--------------------------------------------------------------------------------------
// File: DXErr.h
//
// DirectX Error Library
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#include <windows.h>
#include <sal.h>

#pragma region Undefine Windows Macros
// Only undefine, if DXGIType.h has not been included yet
#ifndef __dxgitype_h__
#undef D2DERR_WRONG_STATE
#undef D2DERR_NOT_INITIALIZED
#undef D2DERR_UNSUPPORTED_OPERATION
#undef D2DERR_SCANNER_FAILED
#undef D2DERR_SCREEN_ACCESS_DENIED
#undef D2DERR_DISPLAY_STATE_INVALID
#undef D2DERR_ZERO_VECTOR
#undef D2DERR_INTERNAL_ERROR
#undef D2DERR_DISPLAY_FORMAT_NOT_SUPPORTED
#undef D2DERR_INVALID_CALL
#undef D2DERR_NO_HARDWARE_DEVICE
#undef D2DERR_RECREATE_TARGET
#undef D2DERR_TOO_MANY_SHADER_ELEMENTS
#undef D2DERR_SHADER_COMPILE_FAILED
#undef D2DERR_MAX_TEXTURE_SIZE_EXCEEDED
#undef D2DERR_UNSUPPORTED_VERSION
#undef D2DERR_BAD_NUMBER
#undef D2DERR_WRONG_FACTORY
#undef D2DERR_LAYER_ALREADY_IN_USE
#undef D2DERR_POP_CALL_DID_NOT_MATCH_PUSH
#undef D2DERR_WRONG_RESOURCE_DOMAIN
#undef D2DERR_PUSH_POP_UNBALANCED
#undef D2DERR_RENDER_TARGET_HAS_LAYER_OR_CLIPRECT
#undef D2DERR_INCOMPATIBLE_BRUSH_TYPES
#undef D2DERR_WIN32_ERROR
#undef D2DERR_TARGET_NOT_GDI_COMPATIBLE
#undef D2DERR_TEXT_EFFECT_IS_WRONG_TYPE
#undef D2DERR_TEXT_RENDERER_NOT_RELEASED
#undef D2DERR_EXCEEDS_MAX_BITMAP_SIZE
#undef DXGI_STATUS_OCCLUDED
#undef DXGI_STATUS_CLIPPED
#undef DXGI_STATUS_NO_REDIRECTION
#undef DXGI_STATUS_NO_DESKTOP_ACCESS
#undef DXGI_STATUS_GRAPHICS_VIDPN_SOURCE_IN_USE
#undef DXGI_STATUS_MODE_CHANGED
#undef DXGI_STATUS_MODE_CHANGE_IN_PROGRESS
#undef DXGI_ERROR_INVALID_CALL
#undef DXGI_ERROR_NOT_FOUND
#undef DXGI_ERROR_MORE_DATA
#undef DXGI_ERROR_UNSUPPORTED
#undef DXGI_ERROR_DEVICE_REMOVED
#undef DXGI_ERROR_DEVICE_HUNG
#undef DXGI_ERROR_DEVICE_RESET
#undef DXGI_ERROR_WAS_STILL_DRAWING
#undef DXGI_ERROR_FRAME_STATISTICS_DISJOINT
#undef DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE
#undef DXGI_ERROR_DRIVER_INTERNAL_ERROR
#undef DXGI_ERROR_NONEXCLUSIVE
#undef DXGI_ERROR_NOT_CURRENTLY_AVAILABLE
#undef DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED
#undef DXGI_ERROR_REMOTE_OUTOFMEMORY
#undef D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS
#undef D3D11_ERROR_FILE_NOT_FOUND
#undef D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS
#undef D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD
#undef D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS
#undef D3D10_ERROR_FILE_NOT_FOUND
#undef DWRITE_E_FILEFORMAT
#undef DWRITE_E_UNEXPECTED
#undef DWRITE_E_NOFONT
#undef DWRITE_E_FILENOTFOUND
#undef DWRITE_E_FILEACCESS
#undef DWRITE_E_FONTCOLLECTIONOBSOLETE
#undef DWRITE_E_ALREADYREGISTERED
#endif
#pragma endregion

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------------
// DXGetErrorString
//--------------------------------------------------------------------------------------
const char* WINAPI DXGetErrorStringA(_In_ HRESULT hr);
const WCHAR* WINAPI DXGetErrorStringW( _In_ HRESULT hr );

#ifdef UNICODE
#define DXGetErrorString DXGetErrorStringW
#else
#define DXGetErrorString DXGetErrorStringA
#endif 

//--------------------------------------------------------------------------------------
// DXGetErrorDescription has to be modified to return a copy in a buffer rather than
// the original static string.
//--------------------------------------------------------------------------------------
void WINAPI DXGetErrorDescriptionA(_In_ HRESULT hr, _Out_cap_(count) char* desc, _In_ size_t count);
void WINAPI DXGetErrorDescriptionW( _In_ HRESULT hr, _Out_cap_(count) WCHAR* desc, _In_ size_t count );

#ifdef UNICODE
#define DXGetErrorDescription DXGetErrorDescriptionW
#else
#define DXGetErrorDescription DXGetErrorDescriptionA
#endif 

//--------------------------------------------------------------------------------------
//  DXTrace
//
//  Desc:  Outputs a formatted error message to the debug stream
//
//  Args:  WCHAR* strFile   The current file, typically passed in using the 
//                         __FILEW__ macro.
//         DWORD dwLine    The current line number, typically passed in using the 
//                         __LINE__ macro.
//         HRESULT hr      An HRESULT that will be traced to the debug stream.
//         CHAR* strMsg    A string that will be traced to the debug stream (may be NULL)
//         BOOL bPopMsgBox If TRUE, then a message box will popup also containing the passed info.
//
//  Return: The hr that was passed in.  
//--------------------------------------------------------------------------------------
HRESULT WINAPI DXTraceA(_In_z_ const char* strFile, _In_ DWORD dwLine, _In_ HRESULT hr, _In_opt_ const char* strMsg, _In_ bool bPopMsgBox);
HRESULT WINAPI DXTraceW( _In_z_ const WCHAR* strFile, _In_ DWORD dwLine, _In_ HRESULT hr, _In_opt_ const WCHAR* strMsg, _In_ bool bPopMsgBox );

#ifdef UNICODE
#define DXTrace DXTraceW
#else
#define DXTrace DXTraceA
#endif 

//--------------------------------------------------------------------------------------
//
// Helper macros
//
//--------------------------------------------------------------------------------------
#if defined(DEBUG) || defined(_DEBUG)
#ifdef UNICODE
#define DXTRACE_MSG(str)              DXTrace( __FILEW__, (DWORD)__LINE__, 0, str, false )
#define DXTRACE_ERR(str,hr)           DXTrace( __FILEW__, (DWORD)__LINE__, hr, str, false )
#define DXTRACE_ERR_MSGBOX(str,hr)    DXTrace( __FILEW__, (DWORD)__LINE__, hr, str, true )
#else
#define DXTRACE_MSG(str)              DXTrace( __FILE__, (DWORD)__LINE__, 0, str, false )
#define DXTRACE_ERR(str,hr)           DXTrace( __FILE__, (DWORD)__LINE__, hr, str, false )
#define DXTRACE_ERR_MSGBOX(str,hr)    DXTrace( __FILE__, (DWORD)__LINE__, hr, str, true )
#endif
#else
#define DXTRACE_MSG(str)              (0L)
#define DXTRACE_ERR(str,hr)           (hr)
#define DXTRACE_ERR_MSGBOX(str,hr)    (hr)
#endif

#ifdef __cplusplus
}
#endif //__cplusplus
