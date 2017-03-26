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
using System.Collections.Generic;

/// <summary>
/// AsyncTextureReader
/// </summary>
public class AsyncTextureReader
{
    /// <summary>
    /// 
    /// </summary>
    public enum Status
    {
        /// <summary>
        /// Succeeded.
        /// </summary>
        Succeeded = 0,
        /// <summary>
        /// Data isn't ready yet. Try next frame.
        /// </summary>
        NotReady,
        /// <summary>    
        /// Unsupported rendering API.
        /// </summary>
        Error_UnsupportedAPI,
        /// <summary>
        /// Unknown Error.
        /// </summary>
        Error_UnknownError,
        /// <summary>
        /// Unsupported texture format.
        /// </summary>
        Error_UnsupportedFormat,
        /// <summary>
        /// Supplied data buffer is too small. Can't copy data.
        /// </summary>
        Error_WrongBufferSize,
        /// <summary>
        /// Staging (temp) buffer/texture doesn't exist. Are you trying to retrieve data without requesting them first?
        /// </summary>
        Error_NoRequest,
        /// <summary>
        /// Invalid arguments.
        /// </summary>
        Error_InvalidArguments,
        /// <summary>
        /// Too many simultaneous requests
        /// </summary>
        Error_TooManyRequests,
        /// <summary>
        /// Can't request, copy operation is already in progress
        /// </summary>
        Error_CopyInProgress
    }    

    /// <summary>
    /// 
    /// </summary>
    /// <param name="status"></param>
    /// <returns></returns>
    public static bool Failed(Status status)
    {
        if (status == Status.Succeeded || status == Status.NotReady)
            return false;
        else
            return true;
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="texture"></param>
    /// <returns></returns>
    public static Status ReleaseTempResources(Texture texture)
    {
        Status status = Status.Succeeded;
        if (texture == null)
        {
            status = Status.Error_InvalidArguments;
        }
        else
        {
            int requestSlot = ReleaseTempResources(GetTexturePtr(texture));
            if (requestSlot == -1)
                status = (Status)GetLastStatus();
            else
                GL.IssuePluginEvent(GetReleaseTempResourcesEventFunc(), requestSlot);
        }

#if UNITY_EDITOR // check for errors in editor
        if (Failed(status))
            Debug.LogError("RequestTextureData failed: " + status);
#endif // UNITY_EDITOR
        return status;
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="texture"></param>
    /// <returns></returns>
    public static Status RequestTextureData(Texture texture)
    {
        Status status = Status.Succeeded;
        if (texture == null)
        {
            status = Status.Error_InvalidArguments;
        }
        else
        {
            int requestSlot = RequestTextureData(GetTexturePtr(texture));
            if (requestSlot == -1)
                status = (Status)GetLastStatus();
            else
                GL.IssuePluginEvent(GetRequestTextureEventFunc(), requestSlot);
        }

#if UNITY_EDITOR // check for errors in editor
        if(Failed(status))
            Debug.LogError("RequestTextureData failed: " + status);
#endif // UNITY_EDITOR
        return status;
    }
    
    /// <summary>
    /// 
    /// </summary>
    /// <param name="texture"></param>
    /// <param name="data"></param>
    /// <returns></returns>
    public static Status RetrieveTextureData(Texture texture, int[] data)
    {
        Status status;
        if (texture == null || data == null)
            status = Status.Error_InvalidArguments;
        else
        {
            int slot = RetrieveTextureData(GetTexturePtr(texture), data, data.Length * sizeof(int));
            if (slot != -1)
            {
                status = Status.NotReady;
                GL.IssuePluginEvent(GetCopyTextureEventFunc(), slot);
            }
            else
            {
                status = (Status)GetLastStatus();
            }
        }

#if UNITY_EDITOR // check for errors in editor
        if (Failed(status))
            Debug.LogError("RetrieveTextureData failed: " + status);
#endif // UNITY_EDITOR
        return status;
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="texture"></param>
    /// <param name="data"></param>
    /// <returns></returns>
    public static Status RetrieveTextureData(Texture texture, float[] data)
    {
        Status status;
        if (texture == null || data == null)
            status = Status.Error_InvalidArguments;
        else
        {
            int slot = RetrieveTextureData(GetTexturePtr(texture), data, data.Length * sizeof(float));
            if (slot != -1)
            {
                status = Status.NotReady;
                GL.IssuePluginEvent(GetCopyTextureEventFunc(), slot);
            }
            else
            {
                status = (Status)GetLastStatus();
            }
        }

#if UNITY_EDITOR // check for errors in editor
        if (Failed(status))
            Debug.LogError("RetrieveTextureData failed: " + status);
#endif // UNITY_EDITOR
        return status;
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="texture"></param>
    /// <param name="data"></param>
    /// <returns></returns>
    public static Status RetrieveTextureData(Texture texture, byte[] data)
    {
        Status status;
        if (texture == null || data == null)
            status = Status.Error_InvalidArguments;
        else
        {
            int slot = RetrieveTextureData(GetTexturePtr(texture), data, data.Length * sizeof(byte));
            if(slot != -1)
            {
                status = Status.NotReady;
                GL.IssuePluginEvent(GetCopyTextureEventFunc(), slot);
            }
            else
            {
                status = (Status)GetLastStatus();
            }
        }

#if UNITY_EDITOR // check for errors in editor
        if (Failed(status))
            Debug.LogError("RetrieveTextureData failed: " + status);
#endif // UNITY_EDITOR
        return status;
    }

#if UNITY_5_5_OR_NEWER
    /// <summary>
    /// 
    /// </summary>
    /// <param name="texture"></param>
    /// <returns></returns>
    public static Status ReleaseTempResources(ComputeBuffer buffer)
    {
        Status status = Status.Succeeded;
        if (buffer == null)
        {
            status = Status.Error_InvalidArguments;
        }
        else
        {
            int requestSlot = ReleaseTempResources(GetBufferPtr(buffer));
            if (requestSlot == -1)
                status = (Status)GetLastStatus();
            else
                GL.IssuePluginEvent(GetReleaseTempResourcesEventFunc(), requestSlot);
        }

#if UNITY_EDITOR // check for errors in editor
        if (Failed(status))
            Debug.LogError("RequestTextureData failed: " + status);
#endif // UNITY_EDITOR
        return status;
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="buffer"></param>
    /// <returns></returns>
    public static Status RequestBufferData(ComputeBuffer buffer)
    {
        Status status = Status.Succeeded;
        if (buffer == null)
            status = Status.Error_InvalidArguments;
        else
        {
            int requestSlot = RequestBufferData(GetBufferPtr(buffer));
            if (requestSlot == -1)
                status = (Status)GetLastStatus();
            else
                GL.IssuePluginEvent(GetRequestBufferEventFunc(), requestSlot);
        }

#if UNITY_EDITOR // check for errors in editor
        if (Failed(status))
            Debug.LogError("RequestBufferData failed: " + status);
#endif // UNITY_EDITOR
        return status;
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="buffer"></param>
    /// <param name="data"></param>
    /// <returns></returns>
    public static Status RetrieveBufferData(ComputeBuffer buffer, int[] data)
    {
        Status status;
        if (buffer == null || data == null)
            status = Status.Error_InvalidArguments;
        else
        {
            int slot = RetrieveBufferData(GetBufferPtr(buffer), data, data.Length * sizeof(int));
            if (slot != -1)
            {
                status = Status.NotReady;
                GL.IssuePluginEvent(GetCopyBufferEventFunc(), slot);
            }
            else
            {
                status = (Status)GetLastStatus();
            }
        }

#if UNITY_EDITOR // check for errors in editor
        if (Failed(status))
            Debug.LogError("RetrieveBufferData failed: " + status);
#endif // UNITY_EDITOR
        return status;
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="buffer"></param>
    /// <param name="data"></param>
    /// <returns></returns>
    public static Status RetrieveBufferData(ComputeBuffer buffer, float[] data)
    {
        Status status;
        if (buffer == null || data == null)
            status = Status.Error_InvalidArguments;
        else
        {
            int slot = RetrieveBufferData(GetBufferPtr(buffer), data, data.Length * sizeof(float));
            if (slot != -1)
            {
                status = Status.NotReady;
                GL.IssuePluginEvent(GetCopyBufferEventFunc(), slot);
            }
            else
            {
                status = (Status)GetLastStatus();
            }
        }

#if UNITY_EDITOR // check for errors in editor
        if (Failed(status))
            Debug.LogError("RetrieveBufferData failed: " + status);
#endif // UNITY_EDITOR
        return status;
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="buffer"></param>
    /// <param name="data"></param>
    /// <returns></returns>
    public static Status RetrieveBufferData(ComputeBuffer buffer, byte[] data)
    {
        Status status;
        if (buffer == null || data == null)
            status = Status.Error_InvalidArguments;
        else
        {
            int slot = RetrieveBufferData(GetBufferPtr(buffer), data, data.Length * sizeof(byte));
            if (slot != -1)
            {
                status = Status.NotReady;
                GL.IssuePluginEvent(GetCopyBufferEventFunc(), slot);
            }
            else
            {
                status = (Status)GetLastStatus();
            }
        }

#if UNITY_EDITOR // check for errors in editor
        if (Failed(status))
            Debug.LogError("RetrieveBufferData failed: " + status);
#endif // UNITY_EDITOR
        return status;
    }

    private static IntPtr GetBufferPtr(ComputeBuffer buffer)
    {
        IntPtr ptr;
        if (_bufferHandles.TryGetValue(buffer, out ptr))
            return ptr;

        ptr = buffer.GetNativeBufferPtr();
        _bufferHandles.Add(buffer, ptr);
        return ptr;
    }
#endif // UNITY_5_5_OR_NEWER

    /// <summary>
    /// 
    /// </summary>
    public static void InitDebugLogs()
    {
        MyDelegate callback_delegate = new MyDelegate(DebugLogFunction);

        IntPtr intptr_delegate = Marshal.GetFunctionPointerForDelegate(callback_delegate);
        SetDebugFunction(intptr_delegate);
    }

    private static void DebugLogFunction(string str)
    {
        Debug.Log("AsyncTextureReader: " + str);
    }

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate void MyDelegate(string str);

    private static IntPtr GetTexturePtr(Texture texture)
    {
        IntPtr ptr;
        if (_textureHandles.TryGetValue(texture, out ptr))
            return ptr;

        ptr = texture.GetNativeTexturePtr();
        _textureHandles.Add(texture, ptr);
        return ptr;
    }

    private static Dictionary<Texture, IntPtr> _textureHandles = new Dictionary<Texture, IntPtr>();
    private static Dictionary<ComputeBuffer, IntPtr> _bufferHandles = new Dictionary<ComputeBuffer, IntPtr>();

    #region DllImport
    [DllImport("AsyncTextureReader")]
    private static extern int ReleaseTempResources(IntPtr resourceHandle);
    [DllImport("AsyncTextureReader")]
    private static extern int RequestTextureData(IntPtr textureHandle);
    [DllImport("AsyncTextureReader")]
    private static extern int RetrieveTextureData(IntPtr textureHandle, int[] data, int dataSize);
    [DllImport("AsyncTextureReader")]
    private static extern int RetrieveTextureData(IntPtr textureHandle, float[] data, int dataSize);
    [DllImport("AsyncTextureReader")]
    private static extern int RetrieveTextureData(IntPtr textureHandle, byte[] data, int dataSize);

    [DllImport("AsyncTextureReader")]
    private static extern int RequestBufferData(IntPtr textureHandle);
    [DllImport("AsyncTextureReader")]
    private static extern int RetrieveBufferData(IntPtr bufferHandle, int[] data, int dataSize);
    [DllImport("AsyncTextureReader")]
    private static extern int RetrieveBufferData(IntPtr bufferHandle, float[] data, int dataSize);
    [DllImport("AsyncTextureReader")]
    private static extern int RetrieveBufferData(IntPtr bufferHandle, byte[] data, int dataSize);

    [DllImport("AsyncTextureReader")]
    private static extern IntPtr GetReleaseTempResourcesEventFunc();
    [DllImport("AsyncTextureReader")]
    private static extern IntPtr GetRequestTextureEventFunc();
    [DllImport("AsyncTextureReader")]
    private static extern IntPtr GetRequestBufferEventFunc();
    [DllImport("AsyncTextureReader")]
    private static extern IntPtr GetCopyTextureEventFunc();
    [DllImport("AsyncTextureReader")]
    private static extern IntPtr GetCopyBufferEventFunc();

    [DllImport("AsyncTextureReader")]
    private static extern int GetLastStatus();

    [DllImport("AsyncTextureReader")]
    private static extern int SetDebugFunction(IntPtr functionPointer);
    #endregion    
}
