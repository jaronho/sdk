using UnityEngine;
using System.Collections;
using HighlightingSystem;

public class HighlighterOccluder : MonoBehaviour
{
	private Highlighter h;

	// 
	void Awake()
	{
		h = GetComponent<Highlighter>();
		if (h == null) { h = gameObject.AddComponent<Highlighter>(); }
	}

	// 
	void OnEnable()
	{
		h.OccluderOn();
	}
}
