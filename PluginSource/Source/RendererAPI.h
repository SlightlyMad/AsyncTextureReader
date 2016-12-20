//  Copyright(c) 2016, Michal Skalsky
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without modification,
//  are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  3. Neither the name of the copyright holder nor the names of its contributors
//     may be used to endorse or promote products derived from this software without
//     specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
//  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.IN NO EVENT
//  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
//  OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
//  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
//  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "Unity/IUnityGraphics.h"

enum class Status
{
    Succeeded = 0,
    NotReady,
	Error_UnsupportedAPI,
	Error_UnknownError,
	Error_UnsupportedFormat,
	Error_WrongBufferSize,
	Error_NoRequest,
	Error_InvalidArguments,
	Error_TooManyRequests,
	Error_CopyInProgress
};

typedef void(*FuncPtr)(const char *);
extern FuncPtr DebugLog;

//-------------------------------------------------------------------------------------------------
// RendererAPI
//-------------------------------------------------------------------------------------------------
class RendererAPI
{
public:
    virtual ~RendererAPI() {}

    virtual void ProcessDeviceEvent(UnityGfxDeviceEventType eventType, IUnityInterfaces* interfaces) = 0;

    virtual Status RequestTextureData(void* textureHandle) = 0;
	virtual void CopyTextureData(void* textureHandle) = 0;
    virtual Status RetrieveTextureData(void* textureHandle, void* data, int dataSize) = 0;

	virtual Status RequestBufferData(void* bufferHandle) = 0;
	virtual void CopyBufferData(void* textureHandle) = 0;
	virtual Status RetrieveBufferData(void* bufferHandle, void* data, int dataSize) = 0;
};

RendererAPI* CreateRendererAPI(UnityGfxRenderer apiType);
