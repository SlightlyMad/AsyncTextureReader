
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
#include <vector>
#include <atomic>

static IUnityInterfaces* sUnityInterfaces;
static IUnityGraphics* sUnityGraphics;
static UnityGfxRenderer sDeviceType = kUnityGfxRendererNull;
static RendererAPI* sCurrentAPI = NULL;

FuncPtr DebugLog;

static Status sLastStatus = Status::Succeeded;

// list of resource handles waiting for request
// maximum of 128 resources can be requested at one time. can't be dynamic and thread safe at the same time. 128 should be big enough
// still not necessary thread safe !!!
//std::vector<void*> sResources(128);
static const int sResourcesSize = 128;
static void* sResources[sResourcesSize];

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

		// clear resources array
		for (int i = 0; i < sResourcesSize; ++i)
		{
			sResources[i] = NULL;
		}
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
// FindFreeResourceSlot
//-------------------------------------------------------------------------------------------------
int FindFreeResourceSlot()
{
	for (int i = 0; i < sResourcesSize; ++i)
	{
		if (sResources[i] == NULL)
			return i;
	}

	return -1;

	// can't resize vector. it isn't thread safe
	//sResources.push_back(NULL);
	//return sResources.size() - 1;
}

//-------------------------------------------------------------------------------------------------
// OnRequestTextureEvent
//-------------------------------------------------------------------------------------------------
static void UNITY_INTERFACE_API OnRequestTextureEvent(int eventID)
{
	if (sCurrentAPI != NULL && eventID >= 0 && eventID < sResourcesSize && sResources[eventID] != NULL)
	{
		sCurrentAPI->RequestTextureData_RenderThread(sResources[eventID]);
		// free resource slot for future use
		sResources[eventID] = NULL;
	}
}

//-------------------------------------------------------------------------------------------------
// GetRequestTextureEventFunc
//-------------------------------------------------------------------------------------------------
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRequestTextureEventFunc()
{
	return OnRequestTextureEvent;
}

//-------------------------------------------------------------------------------------------------
// OnRequestBufferEvent
//-------------------------------------------------------------------------------------------------
static void UNITY_INTERFACE_API OnRequestBufferEvent(int eventID)
{
	if (sCurrentAPI != NULL && eventID >= 0 && eventID < sResourcesSize && sResources[eventID] != NULL)
	{
		sCurrentAPI->RequestBufferData_RenderThread(sResources[eventID]);
		// free resource slot for future use
		sResources[eventID] = NULL;
	}
}

//-------------------------------------------------------------------------------------------------
// GetRequestBufferEventFunc
//-------------------------------------------------------------------------------------------------
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRequestBufferEventFunc()
{
	return OnRequestBufferEvent;
}

//-------------------------------------------------------------------------------------------------
// OnReleaseTempResourcesEvent
//-------------------------------------------------------------------------------------------------
static void UNITY_INTERFACE_API OnReleaseTempResourcesEvent(int eventID)
{
	if (sCurrentAPI != NULL && eventID >= 0 && eventID < sResourcesSize && sResources[eventID] != NULL)
	{
		sCurrentAPI->ReleaseTempResources(sResources[eventID]);
		// free resource slot for future use
		sResources[eventID] = NULL;
	}
}

//-------------------------------------------------------------------------------------------------
// GetReleaseTempResourcesEventFunc
//-------------------------------------------------------------------------------------------------
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetReleaseTempResourcesEventFunc()
{
	return OnReleaseTempResourcesEvent;
}

//-------------------------------------------------------------------------------------------------
// ReleaseTempResources
//-------------------------------------------------------------------------------------------------
extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API ReleaseTempResources(void* resourceHandle)
{
	if (resourceHandle == NULL)
	{
		sLastStatus = Status::Error_InvalidArguments;
		return -1;
	}

	// store resource handle
	int resourceSlot = FindFreeResourceSlot();
	if (resourceSlot == -1)
	{
		sLastStatus = Status::Error_TooManyRequests;
		return -1;
	}

	sResources[resourceSlot] = resourceHandle;

	if (sCurrentAPI != NULL)
	{
		sLastStatus = Status::Succeeded;
		return resourceSlot;
	}
	else
	{
		sLastStatus = Status::Error_UnsupportedAPI;
		return -1;
	}
}

//-------------------------------------------------------------------------------------------------
// RequestTextureData
//-------------------------------------------------------------------------------------------------
extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestTextureData(void* textureHandle)
{
	if (textureHandle == NULL)
	{
		sLastStatus = Status::Error_InvalidArguments;
		return -1;
	}

	// store resource handle
	int resourceSlot = FindFreeResourceSlot();
	if (resourceSlot == -1)
	{
		sLastStatus = Status::Error_TooManyRequests;
		return -1;
	}

	sResources[resourceSlot] = textureHandle;

	if (sCurrentAPI != NULL)
	{
		sLastStatus = sCurrentAPI->RequestTextureData_MainThread(textureHandle);
		return resourceSlot;
	}
	else
	{
		sLastStatus = Status::Error_UnsupportedAPI;
		return -1;
	}
}

//-------------------------------------------------------------------------------------------------
// OnCopyTextureEvent
//-------------------------------------------------------------------------------------------------
static void UNITY_INTERFACE_API OnCopyTextureEvent(int eventID)
{
	if (sCurrentAPI != NULL && eventID >= 0 && eventID < sResourcesSize && sResources[eventID] != NULL)
	{
		sCurrentAPI->CopyTextureData_RenderThread(sResources[eventID]);
		// free resource slot for future use
		sResources[eventID] = NULL;
	}
}

//-------------------------------------------------------------------------------------------------
// GetCopyTextureEventFunc
//-------------------------------------------------------------------------------------------------
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetCopyTextureEventFunc()
{
	return OnCopyTextureEvent;
}

//-------------------------------------------------------------------------------------------------
// RetrieveTextureData
//-------------------------------------------------------------------------------------------------
extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RetrieveTextureData(void* textureHandle, void* data, int dataSize)
{
	// parameters were tested on C# side and can't be invalid
	assert(data != NULL && dataSize >= 0);

	if (textureHandle == NULL)
	{
		sLastStatus = Status::Error_InvalidArguments;
		return -1;
	}

	if (sCurrentAPI != NULL)
	{
		sLastStatus = sCurrentAPI->RetrieveTextureData_MainThread(textureHandle, data, dataSize);
		if (sLastStatus == Status::NotReady)
		{
			int slot = FindFreeResourceSlot();
			// save texture for issue plugin event call
			sResources[slot] = textureHandle;
			return slot;
		}

		return -1;
	}
	else
	{
		sLastStatus = Status::Error_UnsupportedAPI;
		return -1;
	}
}

//-------------------------------------------------------------------------------------------------
// RequestBufferData
//-------------------------------------------------------------------------------------------------
extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestBufferData(void* bufferHandle)
{
	if (bufferHandle == NULL)
	{
		sLastStatus = Status::Error_InvalidArguments;
		return -1;
	}

	// store resource handle
	int resourceSlot = FindFreeResourceSlot();
	if (resourceSlot == -1)
	{
		sLastStatus = Status::Error_TooManyRequests;
		return -1;
	}
	
	sResources[resourceSlot] = bufferHandle;

	if (sCurrentAPI != NULL)
	{
		sLastStatus = sCurrentAPI->RequestBufferData_MainThread(bufferHandle);
		return resourceSlot;
	}
	else
	{
		sLastStatus = Status::Error_UnsupportedAPI;
		return -1;
	}
}

//-------------------------------------------------------------------------------------------------
// OnCopyBufferEvent
//-------------------------------------------------------------------------------------------------
static void UNITY_INTERFACE_API OnCopyBufferEvent(int eventID)
{
	if (sCurrentAPI != NULL && eventID >= 0 && eventID < sResourcesSize && sResources[eventID] != NULL)
	{
		sCurrentAPI->CopyBufferData_RenderThread(sResources[eventID]);
		// free resource slot for future use
		sResources[eventID] = NULL;
	}
}

//-------------------------------------------------------------------------------------------------
// GetCopyBufferEventFunc
//-------------------------------------------------------------------------------------------------
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetCopyBufferEventFunc()
{
	return OnCopyBufferEvent;
}

//-------------------------------------------------------------------------------------------------
// RetrieveBufferData
//-------------------------------------------------------------------------------------------------
extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RetrieveBufferData(void* bufferHandle, void* data, int dataSize)
{
	// parameters were tested on C# side and can't be invalid
	assert(data != NULL && dataSize >= 0);

	if (bufferHandle == NULL)
	{
		sLastStatus = Status::Error_InvalidArguments;
		return -1;
	}

	if (sCurrentAPI != NULL)
	{
		sLastStatus = sCurrentAPI->RetrieveBufferData_MainThread(bufferHandle, data, dataSize);

		if (sLastStatus == Status::NotReady)
		{
			int slot = FindFreeResourceSlot();
			// save texture for issue plugin event call
			sResources[slot] = bufferHandle;
			return slot;
		}

		return -1;
	}
	else
	{
		sLastStatus = Status::Error_UnsupportedAPI;
		return -1;
	}
}

//-------------------------------------------------------------------------------------------------
// GetLastStatus
//-------------------------------------------------------------------------------------------------
extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetLastStatus()
{
	return (int)sLastStatus;
}

//-------------------------------------------------------------------------------------------------
// SetDebugFunction
//-------------------------------------------------------------------------------------------------
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetDebugFunction(FuncPtr fp)
{
	DebugLog = fp;
}