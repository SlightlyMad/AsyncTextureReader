
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

#include "Unity/IUnityGraphics.h"
#include "PlatformBase.h"
#include "RendererAPI.h"

#include "assert.h"

static IUnityInterfaces* sUnityInterfaces;
static IUnityGraphics* sUnityGraphics;
static UnityGfxRenderer sDeviceType = kUnityGfxRendererNull;
static RendererAPI* sCurrentAPI = NULL;

FuncPtr DebugLog;

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType);

//-------------------------------------------------------------------------------------------------
// UnityPluginLoad
//-------------------------------------------------------------------------------------------------
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
    sUnityInterfaces = unityInterfaces;
    sUnityGraphics = sUnityInterfaces->Get<IUnityGraphics>();
    sUnityGraphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);

    // init
    OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

//-------------------------------------------------------------------------------------------------
// UnityPluginUnload
//-------------------------------------------------------------------------------------------------
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
    sUnityGraphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
}

//-------------------------------------------------------------------------------------------------
// OnGraphicsDeviceEvent
//-------------------------------------------------------------------------------------------------
static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
    if (eventType == kUnityGfxDeviceEventInitialize)
    {
        assert(sCurrentAPI == NULL);
        sDeviceType = sUnityGraphics->GetRenderer();
        sCurrentAPI = CreateRendererAPI(sDeviceType);
    }

    if (sCurrentAPI != NULL)
    {
        sCurrentAPI->ProcessDeviceEvent(eventType, sUnityInterfaces);
    }

    if (eventType == kUnityGfxDeviceEventShutdown)
    {        
        SAFE_DELETE(sCurrentAPI);
        sDeviceType = kUnityGfxRendererNull;
    }
}

//-------------------------------------------------------------------------------------------------
// RequestTextureData
//-------------------------------------------------------------------------------------------------
extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestTextureData(void* textureHandle)
{
	if (textureHandle == NULL)
		return (int)Status::Error_InvalidArguments;

    if (sCurrentAPI != NULL)
        return (int)sCurrentAPI->RequestTextureData(textureHandle);
    else
        return (int)Status::Error_UnsupportedAPI;
    

}

//-------------------------------------------------------------------------------------------------
// RetrieveTextureData
//-------------------------------------------------------------------------------------------------
extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RetrieveTextureData(void* textureHandle, void* data, int dataSize)
{
	// parameters were tested on C# side and can't be invalid
	assert(data != NULL && dataSize >= 0);

	if (textureHandle == NULL)
		return (int)Status::Error_InvalidArguments;

    if (sCurrentAPI != NULL)
        return (int)sCurrentAPI->RetrieveTextureData(textureHandle, data, dataSize);
    else
        return (int)Status::Error_UnsupportedAPI;
}

//-------------------------------------------------------------------------------------------------
// RequestBufferData
//-------------------------------------------------------------------------------------------------
extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestBufferData(void* bufferHandle)
{
	if (bufferHandle == NULL)
		return (int)Status::Error_InvalidArguments;

	if (sCurrentAPI != NULL)
		return (int)sCurrentAPI->RequestBufferData(bufferHandle);
	else
		return (int)Status::Error_UnsupportedAPI;


}

//-------------------------------------------------------------------------------------------------
// RetrieveBufferData
//-------------------------------------------------------------------------------------------------
extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RetrieveBufferData(void* bufferHandle, void* data, int dataSize)
{
	// parameters were tested on C# side and can't be invalid
	assert(data != NULL && dataSize >= 0);

	if (bufferHandle == NULL)
		return (int)Status::Error_InvalidArguments;

	if (sCurrentAPI != NULL)
		return (int)sCurrentAPI->RetrieveBufferData(bufferHandle, data, dataSize);
	else
		return (int)Status::Error_UnsupportedAPI;
}

//-------------------------------------------------------------------------------------------------
// SetDebugFunction
//-------------------------------------------------------------------------------------------------
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetDebugFunction(FuncPtr fp)
{
	DebugLog = fp;
}