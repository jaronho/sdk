﻿using UnityEngine;
using UnityEditor;
using System.Collections;
using System.Collections.Generic;

[CustomEditor(typeof(CurveMeshBuilder))]
public class CurveMeshBuilderEditor : Editor {
	private CurveMeshBuilder _script;

	private GUIStyle _guiStyle_Border1;
	private GUIStyle _guiStyle_Border2;
	private GUIStyle _guiStyle_Border3;
	private GUIStyle _guiStyle_Button1;
	private GUIStyle _guiStyle_Button2;

	void Awake() {
		_script = target as CurveMeshBuilder;

		_guiStyle_Border1 = new GUIStyle("sv_iconselector_back");
		_guiStyle_Border1.stretchHeight = false;
		_guiStyle_Border1.padding = new RectOffset(4, 4, 4, 4);
		_guiStyle_Border2 = new GUIStyle("U2D.createRect");
		_guiStyle_Border3 = new GUIStyle("SelectionRect");
		_guiStyle_Border3.padding = new RectOffset(6, 6, 6, 6);
		_guiStyle_Button1 = new GUIStyle("PreButton");
		_guiStyle_Button2 = new GUIStyle("horizontalsliderthumb");
	}

	public override void OnInspectorGUI() {
		base.OnInspectorGUI();

		EditorGUILayout.BeginVertical(_guiStyle_Border1);
		{
			if (_script.getNodeCount() < 2) {
				GUILayout.Label("Key points num should not less than 2 !", "CN EntryWarn");
			}
			for (int i = 0, len = _script.getNodeCount(); i < len; ++i) {
				EditorGUILayout.BeginHorizontal(i == _script.selectedNodeIndex ? _guiStyle_Border2 : _guiStyle_Border3);
				{
					if (GUILayout.Button("", _guiStyle_Button2, GUILayout.Width(20))) {
						_script.selectedNodeIndex = i;
					}
					GUILayout.Space(2);
					GUILayout.Label((i + 1).ToString());
					Vector2 newNodePos = EditorGUILayout.Vector2Field("", _script.getNode(i));
					if (_script.getNode(i) != newNodePos) {
						_script.setNode(i, newNodePos);
					}
					GUILayout.Space(6);
					if (GUILayout.Button("<", _guiStyle_Button1, GUILayout.Width(20))) {
						Vector2 pos = i == 0 ? _script.getNode(i) - Vector2.right : (_script.getNode(i - 1) + _script.getNode(i)) * 0.5f;
						_script.insertNode(i, pos);
						_script.selectedNodeIndex = i;
					}
					GUILayout.Space(2);
					if (GUILayout.Button("✖", _guiStyle_Button1, GUILayout.Width(20))) {
						_script.removeNode(i);
						_script.selectedNodeIndex = i < _script.getNodeCount() ? i : i - 1;
					}
				}
				EditorGUILayout.EndHorizontal();
			}
			EditorGUILayout.BeginHorizontal();
			{
				if (GUILayout.Button("Add", _guiStyle_Button1)) {
					Vector2 pos = _script.getNodeCount() == 0 ? Vector2.zero : _script.getNode(_script.getNodeCount() - 1) + Vector2.right;
					_script.addNode(pos);
					_script.selectedNodeIndex = _script.getNodeCount() - 1;
				}
				if (GUILayout.Button("Clear", _guiStyle_Button1)) {
					_script.clearNodes();
				}
			}
			EditorGUILayout.EndHorizontal();
		}
		EditorGUILayout.EndVertical();

		if (GUILayout.Button("Build Model")) {
			_script.buildMesh();
		}

		if (GUI.changed) {
			EditorUtility.SetDirty(target);
		}
	}
}
