#Introduction 
Native Unity plugin that lets you asynchronously copy textures and buffers from gpu memory to managed system memory.

#Warning
As a native plugin it has the ability to take down Unity if something goes wrong. Use at your own RISK. Save your work often.

#Requirements
- At least Unity 5.2 is required (tested on 5.4). Compute buffers require Unity 5.5.
- DirectX 11 only at the moment

#Getting Started
1. Copy Assets/* to your project
2. In your script, create array big enough to hold your texture: `float[] data = new float[texture.width * texture.height];`
3. Request texture data: `AsyncTextureReader.RequestTextureData(texture);`
4. Call `AsyncTextureReader.RetrieveTextureData(texture, data)` every frame until it returns `AsyncTextureReader.Status.Succeeded`. Little hint. You can call RetrieveTextureData multiple times during a frame to get the data as soon as possible. For example in Update, PreRender and PostRender.

See Test scene for a simple example. Use only from main thread, it isn't thread-safe.

#How it works (high-level overview)
1. User requests texture/buffer data.
2. Plugin creates new identical texture/buffer in system memory (with USAGE_STAGING flag). One time operation. It is kept for future use.
3. Texture/buffer is asynchronously copied to system memory (ID3D11DeviceContext::CopyResource)
4. User tries to retrieve texture/buffer data every frame until it succeeds. (ID3D11DeviceContext::Map with D3D11_MAP_FLAG_DO_NOT_WAIT flag - data is copied from texture/buffer in system memory into managed array supplied by the user)

#How it really works
The process is bit more complicated because there are two threads involved.
- Main thread: executes C# scripts, can't manipulate gpu objects
- Render thread: Manipulates gpu objects

1. User code requests texture data - `AsyncTextureReader.RequestTextureData` is called on main thread. Main thread can't touch gpu data so RequestTextureData only asks Unity to run the request on render thread (through IssuePluginEvent). That should happen later in the frame.
2. Request is executed on render thread. It creates texture in system memory (still a DirectX object) and asks gpu to asynchronously copy texture to this system memory.
3. GPU performs copy operation one or more frames later.
4. User code calls `AsyncTextureReader.RetrieveTextureData`. Call is executed on main thread so it again asks Unity to execute it on render thread. That should happen later that frame.
5. Retrieve call is executed on render thread. Texture object in system memory now has the data we need. The data is copied from this texture object in system memory to plain system memory buffer that can be accessed from main thread. Operation is internally flagged as finished.
6. User code calls `AsyncTextureReader.RetrieveTextureData` again on main thread (possible later that frame). Copy operation is finished and the data is copied from plain system buffer to managed buffer supplied by user code.

This should explain why calling AsyncTextureReader.RetrieveTextureData multiple times throughout a frame can speed things up.

#Build plugin
- Project files are located in AsyncTextureReader/PluginSource/Projects
- Copy dll to Assets/Plugins folder

#Unity Forum
You can discuss it [here](https://forum.unity3d.com/threads/asynchronously-getting-data-from-the-gpu-directx-11-with-rendertexture-or-computebuffer.281346/)

#Native plugin implementation details
## Code organization
- `AsyncTextureReader.cpp` - main plugin file. Unity callbacks are defined here.
- `PlatformBase.h` - definition of platform specific macros
- `RendererAPI.h` - declaration of abstract RednererAPI class, base class for platform specific implementation
- `RendererAPI.cpp` - implementation of CreateRendererAPI function. Function that is responsible for instantiating of RendererAPI object for given platform.
- `RendererAPI_D3D11.h`- declaration of RendererAPI class for DirectX11
- `RendererAPI_D3D11.cpp` - implementation of RendererAPI for DirectX11. 

## How to port it to other platforms
1. Implement RendererAPI interface for target platform. See RendererAPI_D3D11 for example implementation.
2. Add your implementation to CreateRendererAPI function in RendererAPI.cpp file.

### RendererAPI interface
List of functions and what they should do. Only texture related function are listed here. Compute buffer related functions work the same way. Note that the interface was created for DirectX and it isn't necessarily good fit for every rendering API.
- `ProcessDeviceEvent` - Plugin initialization and cleanup. For example, DirectX device and context is retrieved here and all resources created by the plugin are released here.
- `RequestTextureData_MainThread` - Called immediately when user code calls `AsyncTextureReader.RequestTextureData`, before `RequestTextureData_RenderThread` is called on render thread. DirectX implementation uses it to initialize some helper data.
- `RequestTextureData_RenderThread` - Request on render thread. This is where the texture copy takes place.
- `CopyTextureData_RenderThread` - Called on render thread everytime user code calls `AsyncTextureReader.RetrieveTextureData`. DX version checks if texture copy is finished, it then copies texture data to a buffer that is accessible from main thread and flags texture copy as finished.
- `RetrieveTextureData_MainThread` -  Called on main thread when user code calls `AsyncTextureReader.RetrieveTextureData`. It should copies texture data to managed buffer supplied by user code if texture copy is finished on render thread.
