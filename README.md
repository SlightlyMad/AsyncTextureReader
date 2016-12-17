#Introduction 
Native Unity plugin that lets you asynchronously copy textures and buffers from gpu memory to managed system memory.

#Warning
"Early access" version. Mostly untested. As a native plugin it has the ability to take down Unity if something goes wrong. Use at your own RISK. Save your work often.

#Requirements
- At least Unity 5.2 (tested on 5.4) is required. Compute buffers require Unity 5.5.
- DirectX 11 only at the moment

#How it works
1. User requests texture/buffer data.
2. Plugin creates new identical texture/buffer in system memory (with USAGE_STAGING flag). One time operation. It is kept for future use.
3. Texture/buffer is asynchronously copied to system memory (ID3D11DeviceContext::CopyResource)
4. User tries to retrieve texture/buffer data every frame until it succeeds. (ID3D11DeviceContext::Map with D3D11_MAP_FLAG_DO_NOT_WAIT flag - data is copied from texture/buffer in system memory into managed array supplied by the user)

#Getting Started
1. Copy Assets/* to your project
2. In your script, create array big enough to hold your texture: `float[] data = new float[texture.width * texture.height];`
3. Request texture data: `AsyncTextureReader.RequestTextureData(texture);`
4. Call `AsyncTextureReader.RetrieveTextureData(texture, data)` every frame until it returns `AsyncTextureReader.Status.Succeeded`.

See Test scene for a simple example. Use only from main thread, it isn't thread-safe.

#Build plugin
- Project files are located in AsyncTextureReader/PluginSource/Projects
- Copy dll to Assets/Plugins folder
