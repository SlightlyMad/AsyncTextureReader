using UnityEngine;
using System.Collections;

public class Test : MonoBehaviour {

    public Texture DebugTexture;
    private byte[] Pixels;
    
    // Use this for initialization
    void Start ()
    {
        AsyncTextureReader.InitDebugLogs();

        Pixels = new byte[DebugTexture.width * DebugTexture.height * 4];
        

        Debug.Log("Request Status: " + AsyncTextureReader.RequestTextureData(DebugTexture));
        Debug.Log("Retrieve Status: " + AsyncTextureReader.RetrieveTextureData(DebugTexture, Pixels));
    }
    
    // Update is called once per frame
    void Update ()
    {
        if (Pixels == null)
            return;

        AsyncTextureReader.Status status = AsyncTextureReader.RetrieveTextureData(DebugTexture, Pixels);
        Debug.Log("Retrieve Status: " + status);
        if (status == AsyncTextureReader.Status.Succeeded)
        {
            // print RGBA of first pixel
            Debug.LogFormat("Pixel RGBA: {0}; {1}; {2}; {3}", Pixels[0], Pixels[1], Pixels[2], Pixels[3]);
            Pixels = null;
        }

    }
}
