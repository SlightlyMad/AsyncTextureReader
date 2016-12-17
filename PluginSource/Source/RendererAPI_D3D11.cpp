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

#include "RendererAPI_D3D11.h"

//-------------------------------------------------------------------------------------------------
// RendererAPI_D3D11::RendererAPI_D3D11()
//-------------------------------------------------------------------------------------------------
RendererAPI_D3D11::RendererAPI_D3D11()
    : _device(NULL), _context(NULL)
{
}

//-------------------------------------------------------------------------------------------------
// RendererAPI_D3D11::~RendererAPI_D3D11()
//-------------------------------------------------------------------------------------------------
RendererAPI_D3D11::~RendererAPI_D3D11()
{
}

//-------------------------------------------------------------------------------------------------
// RendererAPI_D3D11::ProcessDeviceEvent()
//-------------------------------------------------------------------------------------------------
void RendererAPI_D3D11::ProcessDeviceEvent(UnityGfxDeviceEventType eventType, IUnityInterfaces* interfaces)
{
    switch (eventType)
    {
    case kUnityGfxDeviceEventInitialize:
    {
        IUnityGraphicsD3D11* d3d = interfaces->Get<IUnityGraphicsD3D11>();
        _device = d3d->GetDevice();
		_device->GetImmediateContext(&_context);
        break;
    }
    case kUnityGfxDeviceEventShutdown:
		ReleaseResources();
		SAFE_RELEASE(_context);
        break;

    }
}

//-------------------------------------------------------------------------------------------------
// RendererAPI_D3D11::ReleaseResources()
//-------------------------------------------------------------------------------------------------
void RendererAPI_D3D11::ReleaseResources()
{
	// release resource copies in staging memory
	for (TextureMapIter iter = _resourceMap.begin(); iter != _resourceMap.end(); ++iter)
	{
		SAFE_RELEASE(iter->second);
	}

	_resourceMap.clear();
}

//-------------------------------------------------------------------------------------------------
// RendererAPI_D3D11::RequestTextureData()
//-------------------------------------------------------------------------------------------------
Status RendererAPI_D3D11::RequestTextureData(void* textureHandle)
{
	ID3D11Texture2D* texture = (ID3D11Texture2D*)textureHandle;

	ID3D11Texture2D* cpuTexture = (ID3D11Texture2D*)_resourceMap[texture];
	if (cpuTexture == NULL)
	{
		// create cpu texture
		Status status = CreateStagingTexture(texture, &cpuTexture);
		if (status != Status::Succeeded)
			return status;
		_resourceMap[texture] = cpuTexture;
	}

	// request texture copy to cpu memory
	_context->CopyResource(cpuTexture, texture);

    return Status::Succeeded;
}

//-------------------------------------------------------------------------------------------------
// RendererAPI_D3D11::CreateStagingTexture()
//-------------------------------------------------------------------------------------------------
Status RendererAPI_D3D11::CreateStagingTexture(ID3D11Texture2D* gpuTexture, ID3D11Texture2D** stagingTexture)
{
	D3D11_TEXTURE2D_DESC desc;
	gpuTexture->GetDesc(&desc);

	// is format supported?
	if (GetPixelSize(desc.Format) == -1)
		return Status::Error_UnsupportedFormat;
	
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;

	ID3D11Texture2D* cpuTexture = NULL;
	if (FAILED(_device->CreateTexture2D(&desc, NULL, &cpuTexture)))
	{	
		return Status::Error_UnknownError;
	}

	*stagingTexture = cpuTexture;
	return Status::Succeeded;
}

//-------------------------------------------------------------------------------------------------
// RendererAPI_D3D11::RetrieveTextureData()
//-------------------------------------------------------------------------------------------------
Status RendererAPI_D3D11::RetrieveTextureData(void* textureHandle, void* data, int dataSize)
{
	ID3D11Texture2D* gpuTexture = (ID3D11Texture2D*)textureHandle;
	ID3D11Texture2D* cpuTexture = (ID3D11Texture2D*)_resourceMap[gpuTexture];

	// texture data wasn't requested, there's nothing to retrieve
	if (cpuTexture == NULL)
		return Status::Error_NoStagingBuffer;

	D3D11_TEXTURE2D_DESC desc;
	cpuTexture->GetDesc(&desc);

	int pixelSize = GetPixelSize(desc.Format);
	if (pixelSize == -1)
		return Status::Error_UnsupportedFormat;

	// is supplied buffer big enough?
	int size = desc.Width * desc.Height * pixelSize;
	if (size > dataSize)
		return Status::Error_WrongBufferSize;

	// try to map resource
	D3D11_MAPPED_SUBRESOURCE resource;
	HRESULT result = _context->Map(cpuTexture, 0, D3D11_MAP_READ, D3D11_MAP_FLAG_DO_NOT_WAIT, &resource);
	// resource is not ready, return
	if (result == DXGI_ERROR_WAS_STILL_DRAWING)
		return Status::NotReady;
	// something went wrong
	if (FAILED(result))
		return Status::Error_UnknownError;
	
	// copy line by line to managed memory
	for (unsigned int row = 0; row < desc.Height; ++row)
	{
		char* dest = ((char*)data) + row * desc.Width * pixelSize;
		char* src = ((char*)resource.pData) + row * resource.RowPitch;
		memcpy(dest, src, desc.Width * pixelSize);
	}

	_context->Unmap(cpuTexture, 0);

    return Status::Succeeded;
}

//-------------------------------------------------------------------------------------------------
// RendererAPI_D3D11::GetPixelSize()
//-------------------------------------------------------------------------------------------------
int RendererAPI_D3D11::GetPixelSize(DXGI_FORMAT format)
{
	int pixelSize = -1;
	switch (format)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		pixelSize = 4 * 4;
		break;

	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		pixelSize = 4 * 3;
		break;

	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
		pixelSize = 4 * 2;
		break;

	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
		pixelSize = 4 * 1;
		break;

	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
		pixelSize = 4 * 1;
		break;

	default:
		pixelSize = -1;
	}

	return pixelSize;
}

//-------------------------------------------------------------------------------------------------
// RendererAPI_D3D11::RequestBufferData()
//-------------------------------------------------------------------------------------------------
Status RendererAPI_D3D11::RequestBufferData(void* bufferHandle)
{
	ID3D11Buffer* buffer = (ID3D11Buffer*)bufferHandle;

	ID3D11Buffer* cpuBuffer = (ID3D11Buffer*)_resourceMap[buffer];
	if (cpuBuffer == NULL)
	{
		// create cpu buffer
		Status status = CreateStagingBuffer(buffer, &cpuBuffer);
		if (status != Status::Succeeded)
			return status;
		_resourceMap[buffer] = cpuBuffer;
	}

	// request buffer copy to cpu memory
	_context->CopyResource(cpuBuffer, buffer);

	return Status::Succeeded;
}

//-------------------------------------------------------------------------------------------------
// RendererAPI_D3D11::CreateStagingBuffer()
//-------------------------------------------------------------------------------------------------
Status RendererAPI_D3D11::CreateStagingBuffer(ID3D11Buffer* gpuBuffer, ID3D11Buffer** stagingBuffer)
{
	D3D11_BUFFER_DESC desc;
	gpuBuffer->GetDesc(&desc);
	
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;

	ID3D11Buffer* cpuBuffer = NULL;
	if (FAILED(_device->CreateBuffer(&desc, NULL, &cpuBuffer)))
	{
		return Status::Error_UnknownError;
	}

	*stagingBuffer = cpuBuffer;
	return Status::Succeeded;
}

//-------------------------------------------------------------------------------------------------
// RendererAPI_D3D11::RetrieveBufferData()
//-------------------------------------------------------------------------------------------------
Status RendererAPI_D3D11::RetrieveBufferData(void* bufferHandle, void* data, int dataSize)
{
	ID3D11Buffer* gpuBuffer = (ID3D11Buffer*)bufferHandle;
	ID3D11Buffer* cpuBuffer = (ID3D11Buffer*)_resourceMap[gpuBuffer];

	// texture data wasn't requested, there's nothing to retrieve
	if (cpuBuffer == NULL)
		return Status::Error_NoStagingBuffer;

	D3D11_BUFFER_DESC desc;
	cpuBuffer->GetDesc(&desc);

	if (desc.ByteWidth > (unsigned int)dataSize)
		return Status::Error_WrongBufferSize;
	
	D3D11_MAPPED_SUBRESOURCE resource;
	HRESULT result = _context->Map(cpuBuffer, 0, D3D11_MAP_READ, D3D11_MAP_FLAG_DO_NOT_WAIT, &resource);
	// buffer isn't ready yet, return
	if (result == DXGI_ERROR_WAS_STILL_DRAWING)
		return Status::NotReady;
	if (FAILED(result))
		return Status::Error_UnknownError;

	// copy to sys mem
	memcpy(data, resource.pData, desc.ByteWidth);

	_context->Unmap(cpuBuffer, 0);

	return Status::Succeeded;
}
