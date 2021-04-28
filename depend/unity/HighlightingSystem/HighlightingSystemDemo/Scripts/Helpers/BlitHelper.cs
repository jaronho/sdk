using UnityEngine;
using System.Collections;

[ExecuteInEditMode]
[RequireComponent(typeof(Camera))]
public class BlitHelper : MonoBehaviour
{
	// Dummy to force rendering of the referenced camera to the RenderTexture. 
	// Without this, camera will render directly to the framebuffer (and it will not be accessible by the HighlightingRenderer) in case no other Image Effects is applied.
	void OnRenderImage(RenderTexture src, RenderTexture dst)
	{
		Graphics.Blit(src, dst);
	}
}
