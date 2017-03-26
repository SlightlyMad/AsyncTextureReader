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

#include "RendererAPI.h"
#include "PlatformBase.h"
#include <atomic>

#if SUPPORT_D3D11

#include <d3d11.h>
#include <map>
#include "Unity/IUnityGraphicsD3D11.h"

enum class CpuResourceStatus
{
	Ready,
	WaitingForGpu,
	CopyFinished
};

//-------------------------------------------------------------------------------------------------
// CpuResource
//-------------------------------------------------------------------------------------------------
struct CpuResource
{
	ID3D11Resource* stagingBuffer;
	void* cpuBuffer;
	int bufferSize;
	std::atomic<CpuResourceStatus> bufferStatus;
	std::atomic<Status> lastStatus;

	CpuResource() : stagingBuffer(NULL), cpuBuffer(NULL), bufferSize(0), bufferStatus(CpuResourceStatus::Ready) {}
};

//-------------------------------------------------------------------------------------------------
// RendererAPI_D3D11
//-------------------------------------------------------------------------------------------------
class RendererAPI_D3D11 : public RendererAPI
{
public:
    RendererAPI_D3D11();
    virtual ~RendererAPI_D3D11();

    virtual void ProcessDeviceEvent(UnityGfxDeviceEventType eventType, IUnityInterfaces* interfaces);

	virtual Status RequestTextureData_MainThread(void* textureHandle);
    virtual Status RequestTextureData_RenderThread(void* textureHandle);
	virtual void CopyTextureData_RenderThread(void* textureHandle);
    virtual Status RetrieveTextureData_MainThread(void* textureHandle, void* data, int dataSize);

	virtual Status RequestBufferData_MainThread(void* bufferHandle);
	virtual Status RequestBufferData_RenderThread(void* bufferHandle);
	virtual void CopyBufferData_RenderThread(void* textureHandle);
	virtual Status RetrieveBufferData_MainThread(void* bufferHandle, void* data, int dataSize);

	virtual void ReleaseTempResources(void* resourceHandle);

private:
	void ReleaseResources();
	Status CreateStagingTexture(ID3D11Texture2D* gpuTexture, CpuResource* cpuResource);
	Status CreateStagingBuffer(ID3D11Buffer* gpuTexture, CpuResource* cpuResource);
	int GetPixelSize(DXGI_FORMAT format);

private:
	// std::map<gpu resource, cpu resource>
	typedef std::map<ID3D11Resource*, CpuResource*> ResourceMap;
	typedef ResourceMap::iterator TextureMapIter;

    ID3D11Device* _device;
	ID3D11DeviceContext* _context;
	
	ResourceMap _resourceMap;
};

//-------------------------------------------------------------------------------------------------
// CreateRendererAPI_D3D11
//-------------------------------------------------------------------------------------------------
RendererAPI* CreateRendererAPI_D3D11()
{
    return new RendererAPI_D3D11();
}

#endif // SUPPORT_D3D11
