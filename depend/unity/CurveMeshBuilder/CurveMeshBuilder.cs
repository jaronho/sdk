using UnityEngine;
using System.Collections;
using System.Collections.Generic;

/// <summary>
/// Dynamic build curve mesh by given key points
/// Curve type is Catmul-Rom
/// </summary>

[ExecuteInEditMode]
[RequireComponent(typeof(MeshFilter), typeof(MeshRenderer))]
public class CurveMeshBuilder : MonoBehaviour {
	public bool drawGizmos = true;
	public float tension = 0.5f;
	public int segment = 5;
	public float width = 0.2f;
	public float uvTiling = 1f;
	#if UNITY_EDITOR
	public float gizmosNodeBallSize = 0.1f;
	[System.NonSerialized]
	public int selectedNodeIndex = -1;
	#endif
	public List<Vector2> nodeList = new List<Vector2>();

	private struct CurveSegment2D {
		public Vector2 point1;
		public Vector2 point2;

		public CurveSegment2D(Vector2 pt1, Vector2 pt2) {
			point1 = pt1;
			point2 = pt2;
		}

		public Vector2 vector {
			get {
				return point2 - point1;
			}
		}
	}

	private Mesh _mesh;

	void Awake() {
		buildMesh();
	}

	#if UNITY_EDITOR
	//Draw the spline in the scene view
	void OnDrawGizmos() {
		if (!drawGizmos) {
			return;
		}
		Vector3 prevPosition = Vector3.zero;
		for (int i = 0; i < nodeList.Count; ++i) {
			if (0 == i) {
				prevPosition = transform.TransformPoint(nodeList[i]);
			} else {
				Vector3 currPosition = transform.TransformPoint(nodeList[i]);
				Gizmos.DrawLine(prevPosition, currPosition);
				prevPosition = currPosition;
			}
			if (i == selectedNodeIndex) {
				Color c = Gizmos.color;
				Gizmos.color = Color.red;
				Gizmos.DrawSphere(prevPosition, gizmosNodeBallSize * UnityEditor.HandleUtility.GetHandleSize(prevPosition) * 1.5f);
				Gizmos.color = c;
			} else {
				Gizmos.DrawSphere(prevPosition, gizmosNodeBallSize * UnityEditor.HandleUtility.GetHandleSize(prevPosition));
			}
		}
	}
	#endif

	public void addNode(Vector2 position) {
		nodeList.Add(position);
	}

	public void insertNode(int index, Vector2 position) {
		index = Mathf.Max(index, 0);
		if (index >= nodeList.Count) {
			addNode(position);
		} else {
			nodeList.Insert(index, position);
		}
	}

	public void removeNode(int index) {
		if (index < 0 || index >= nodeList.Count) {
			return;
		}
		nodeList.RemoveAt(index);
	}

	public void clearNodes() {
		nodeList.Clear();
	}

	public Vector2 getNode(int index) {
		index = Mathf.Max(index, 0);
		index = Mathf.Min(index, nodeList.Count - 1);
		return nodeList[index];
	}

	public void setNode(int index, Vector2 newPosition) {
		index = Mathf.Max(index, 0);
		nodeList[index] = newPosition;
	}

	public int getNodeCount() {
		return nodeList.Count;
	}

	public bool buildMesh() {
		if (null == _mesh) {
			_mesh = new Mesh();
			_mesh.name = "CurveMesh_" + System.Guid.NewGuid().ToString();
			GetComponent<MeshFilter>().mesh = _mesh;
		}
		_mesh.Clear();
		if (nodeList.Count < 2) {
			return false;
		}
		Vector3[] path = new Vector3[nodeList.Count];
		for (int i = 0, len = nodeList.Count; i < len; ++i) {
			path[i] = new Vector3(nodeList[i].x, nodeList[i].y, 0);
		}
		Vector3[] points = segment <= 1 ? path : UnityEx.pointListSpline(path, segment, tension);
		List<Vector2> curvePoints = new List<Vector2>();
		for (int i = 0, len = points.Length; i < len; ++i) {
			curvePoints.Add(new Vector2(points[i].x, points[i].y));
		}
		List<Vector2> vertices = getVertices(curvePoints, width * 0.5f);
		List<Vector2> verticesUV = getVerticesUV(curvePoints);
		Vector3[] _vertices = new Vector3[vertices.Count];
		Vector2[] _uv = new Vector2[verticesUV.Count];
		int[] _triangles = new int[(vertices.Count - 2) * 3];
		for (int i = 0; i < vertices.Count; ++i) {
			_vertices[i].Set(vertices[i].x, vertices[i].y, 0);
		}
		for (int i = 0; i < verticesUV.Count; ++i) {
			_uv[i].Set(verticesUV[i].x, verticesUV[i].y);
		}
		for (int i = 2; i < vertices.Count; i += 2) {
			int index = (i - 2) * 3;
			_triangles[index] = i - 2;
			_triangles[index + 1] = i - 0;
			_triangles[index + 2] = i - 1;
			_triangles[index + 3] = i - 1;
			_triangles[index + 4] = i - 0;
			_triangles[index + 5] = i + 1;
		}
		_mesh.vertices = _vertices;
		_mesh.triangles = _triangles;
		_mesh.uv = _uv;
		_mesh.RecalculateNormals();
		return true;
	}

	private List<CurveSegment2D> getSegments(List<Vector2> points) {
		List<CurveSegment2D> segments = new List<CurveSegment2D>(points.Count - 1);
		for (int i = 1; i < points.Count; ++i) {
			segments.Add(new CurveSegment2D(points[i - 1], points[i]));
		}
		return segments;
	}

	private List<Vector2> getVertices(List<Vector2> points, float expands) {
		List<CurveSegment2D> segments = getSegments(points);

		List<CurveSegment2D> segments1 = new List<CurveSegment2D>(segments.Count);
		List<CurveSegment2D> segments2 = new List<CurveSegment2D>(segments.Count);

		for (int i = 0; i < segments.Count; ++i) {
			Vector2 vOffset = new Vector2(-segments[i].vector.y, segments[i].vector.x).normalized;
			segments1.Add(new CurveSegment2D(segments[i].point1 + vOffset * expands, segments[i].point2 + vOffset * expands));
			segments2.Add(new CurveSegment2D(segments[i].point1 - vOffset * expands, segments[i].point2 - vOffset * expands));
		}

		List<Vector2> points1 = new List<Vector2>(points.Count);
		List<Vector2> points2 = new List<Vector2>(points.Count);

		for (int i = 0; i < segments1.Count; ++i) {
			if (0 == i) {
				points1.Add(segments1[0].point1);
			} else {
				Vector2 crossPoint;
				if (!tryCalculateLinesIntersection(segments1[i - 1], segments1[i], out crossPoint, 0.1f)) {
					crossPoint = segments1[i].point1;
				}
				points1.Add(crossPoint);
			}
			if (i == segments1.Count - 1) {
				points1.Add(segments1[i].point2);
			}
		}
		for (int i = 0; i < segments2.Count; ++i) {
			if (0 == i) {
				points2.Add(segments2[0].point1);
			} else {
				Vector2 crossPoint;
				if (!tryCalculateLinesIntersection(segments2[i - 1], segments2[i], out crossPoint, 0.1f)) {
					crossPoint = segments2[i].point1;
				}
				points2.Add(crossPoint);
			}
			if (i == segments2.Count - 1) {
				points2.Add(segments2[i].point2);
			}
		}

		List<Vector2> combinePoints = new List<Vector2>(points.Count * 2);
		for (int i = 0; i < points.Count; ++i) {
			combinePoints.Add(points1[i]);
			combinePoints.Add(points2[i]);
		}
		return combinePoints;
	}

	private List<Vector2> getVerticesUV(List<Vector2> points) {
		List<Vector2> uvs = new List<Vector2>(points.Count * 2);
		float totalLength = 0;
		float totalLengthReciprocal = 0;
		float curLength = 0;
		for (int i = 1; i < points.Count; ++i) {
			totalLength += Vector2.Distance(points[i - 1], points[i]);
		}
		totalLengthReciprocal = uvTiling / totalLength;
		for (int i = 0; i < points.Count; ++i) {
			if (0 == i) {
				uvs.Add(new Vector2(0, 1));
				uvs.Add(new Vector2(0, 0));
			} else {
				if (i == points.Count - 1) {
					uvs.Add(new Vector2(uvTiling, 1));
					uvs.Add(new Vector2(uvTiling, 0));
				} else {
					curLength += Vector2.Distance(points[i - 1], points[i]);
					float uvx = curLength * totalLengthReciprocal;

					uvs.Add(new Vector2(uvx, 1));
					uvs.Add(new Vector2(uvx, 0));
				}
			}
		}
		return uvs;
	}

	private bool tryCalculateLinesIntersection(CurveSegment2D segment1, CurveSegment2D segment2, out Vector2 intersection, float angleLimit) {
		intersection = new Vector2();

		Vector2 p1 = segment1.point1;
		Vector2 p2 = segment1.point2;
		Vector2 p3 = segment2.point1;
		Vector2 p4 = segment2.point2;

		float denominator = (p2.y - p1.y) * (p4.x - p3.x) - (p1.x - p2.x) * (p3.y - p4.y);
		// If denominator is 0, means parallel
		if (0 == denominator) {
			return false;
		}

		// Check angle between segments
		float angle = Vector2.Angle(segment1.vector, segment2.vector);
		// if the angle between two segments is too small, we treat them as parallel
		if (angle < angleLimit || (180f - angle) < angleLimit) {
			return false;
		}

		float x = ((p2.x - p1.x) * (p4.x - p3.x) * (p3.y - p1.y)
			+ (p2.y - p1.y) * (p4.x - p3.x) * p1.x
			- (p4.y - p3.y) * (p2.x - p1.x) * p3.x) / denominator;
		float y = -((p2.y - p1.y) * (p4.y - p3.y) * (p3.x - p1.x)
			+ (p2.x - p1.x) * (p4.y - p3.y) * p1.y
			- (p4.x - p3.x) * (p2.y - p1.y) * p3.y) / denominator;

		intersection.Set(x, y);
		return true;
	}
}
