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

using UnityEngine;
using System.Runtime.InteropServices;
using System;

/// <summary>
/// AsyncTextureReader
/// </summary>
public class AsyncTextureReader
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate void MyDelegate(string str);

    #region DllImport
    [DllImport("AsyncTextureReader")]
    private static extern int RequestTextureData(IntPtr textureHandle);
    [DllImport("AsyncTextureReader")]
    private static extern int RetrieveTextureData(IntPtr textureHandle, int[] data);
    [DllImport("AsyncTextureReader")]
    private static extern int RetrieveTextureData(IntPtr textureHandle, float[] data);
    [DllImport("AsyncTextureReader")]
    private static extern int RetrieveTextureData(IntPtr textureHandle, byte[] data);

    [DllImport("AsyncTextureReader")]
    private static extern int RequestBufferData(IntPtr textureHandle);
    [DllImport("AsyncTextureReader")]
    private static extern int RetrieveBufferData(IntPtr bufferHandle, int[] data);
    [DllImport("AsyncTextureReader")]
    private static extern int RetrieveBufferData(IntPtr bufferHandle, float[] data);
    [DllImport("AsyncTextureReader")]
    private static extern int RetrieveBufferData(IntPtr bufferHandle, byte[] data);

    [DllImport("AsyncTextureReader")]
    private static extern int SetDebugFunction(IntPtr functionPointer);
    #endregion

    /// <summary>
    /// 
    /// </summary>
    public enum Status
    {
        UnsupportedAPI = 0,
        Succeeded,
        NotReady,
        Failed,
        UnsupportedFormat,
        InvalidArguments
    }

    /// <summary>
    /// 
    /// </summary>
    public static void InitDebugLogs()
    {
        MyDelegate callback_delegate = new MyDelegate(DebugLogFunction);
        
        IntPtr intptr_delegate = Marshal.GetFunctionPointerForDelegate(callback_delegate);        
        SetDebugFunction(intptr_delegate);
    }
    
    /// <summary>
    /// 
    /// </summary>
    /// <param name="texture"></param>
    /// <returns></returns>
    public static Status RequestTextureData(Texture texture)
    {
        if (texture == null)
            return Status.InvalidArguments;
        return (Status)RequestTextureData(texture.GetNativeTexturePtr());
    }
    
    /// <summary>
    /// 
    /// </summary>
    /// <param name="texture"></param>
    /// <param name="data"></param>
    /// <returns></returns>
    public static Status RetrieveTextureData(Texture texture, int[] data)
    {
        if (texture == null)
            return Status.InvalidArguments;
        return (Status)RetrieveTextureData(texture.GetNativeTexturePtr(), data);
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="texture"></param>
    /// <param name="data"></param>
    /// <returns></returns>
    public static Status RetrieveTextureData(Texture texture, float[] data)
    {
        if (texture == null)
            return Status.InvalidArguments;
        return (Status)RetrieveTextureData(texture.GetNativeTexturePtr(), data);
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="texture"></param>
    /// <param name="data"></param>
    /// <returns></returns>
    public static Status RetrieveTextureData(Texture texture, byte[] data)
    {
        if (texture == null)
            return Status.InvalidArguments;
        return (Status)RetrieveTextureData(texture.GetNativeTexturePtr(), data);
    }

#if UNITY_5_5_OR_NEWER
    /// <summary>
    /// 
    /// </summary>
    /// <param name="buffer"></param>
    /// <returns></returns>
    public static Status RequestBufferData(ComputeBuffer buffer)
    {
        if (buffer == null)
            return Status.InvalidArguments;
        return (Status)RequestBufferData(buffer.GetNativeBufferPtr());
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="buffer"></param>
    /// <param name="data"></param>
    /// <returns></returns>
    public static Status RetrieveBufferData(ComputeBuffer buffer, int[] data)
    {
        if (buffer == null)
            return Status.InvalidArguments;
        return (Status)RetrieveBufferData(buffer.GetNativeBufferPtr(), data);
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="buffer"></param>
    /// <param name="data"></param>
    /// <returns></returns>
    public static Status RetrieveBufferData(ComputeBuffer buffer, float[] data)
    {
        if (buffer == null)
            return Status.InvalidArguments;
        return (Status)RetrieveBufferData(buffer.GetNativeBufferPtr(), data);
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="buffer"></param>
    /// <param name="data"></param>
    /// <returns></returns>
    public static Status RetrieveBufferData(ComputeBuffer buffer, byte[] data)
    {
        if (buffer == null)
            return Status.InvalidArguments;
        return (Status)RetrieveBufferData(buffer.GetNativeBufferPtr(), data);
    }
#endif

    private static void DebugLogFunction(string str)
    {
        Debug.Log("AsyncTextureReader: " + str);
    }
}
