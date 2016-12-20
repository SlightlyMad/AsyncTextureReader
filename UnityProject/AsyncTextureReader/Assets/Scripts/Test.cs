using UnityEngine;
using System.Collections;
using System;

public class Test : MonoBehaviour
{

    public Texture DebugTexture;
    private byte[] Pixels;

    // Use this for initialization
    void Start()
    {
        AsyncTextureReader.InitDebugLogs();

        Pixels = new byte[DebugTexture.width * DebugTexture.height * 4];

        Debug.LogFormat("Frame: {0}; Request Status: {1}", Time.frameCount, AsyncTextureReader.RequestTextureData(DebugTexture));
        Debug.LogFormat("Frame: {0}; Retrieve Status: {1}", Time.frameCount, AsyncTextureReader.RetrieveTextureData(DebugTexture, Pixels));
    }

    private void GetData()
    {
        if (Pixels == null)
            return;

        AsyncTextureReader.Status status = AsyncTextureReader.RetrieveTextureData(DebugTexture, Pixels);
        Debug.LogFormat("Frame: {0}; Retrieve Status: {1}", Time.frameCount, status);
        if (status == AsyncTextureReader.Status.Succeeded)
        {
            // print RGBA of first pixel
            Debug.LogFormat("Pixel RGBA: {0}; {1}; {2}; {3}", Pixels[0], Pixels[1], Pixels[2], Pixels[3]);
            Pixels = null;
        }
    }

    // Update is called at the beginning of frame
    void Update()
    {
        GetData();
    }

    // OnPostRender is called at the end of frame
    public void OnPostRender()
    {
        GetData();
    }
}
