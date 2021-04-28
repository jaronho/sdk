/*--------------------------------------------------------------------
 * Author:	jaron.ho
 * Date:	2016-11-01
 * Brief:	Unity扩展函数
--------------------------------------------------------------------*/
using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Sprites;
#if UNITY_EDITOR
using UnityEditor;
#endif

public class UnityEx {
	//--------------------------------------------------------------------
	// 销毁物件
	public static void destroy(Object obj) {
		if (null == obj) {
			return;
		}
		if (typeof(GameObject) == obj.GetType()) {
			GameObject gobj = (GameObject)obj;
			Renderer[] renderers = gobj.GetComponentsInChildren<Renderer>(true);
			for (int i = 0, len = renderers.Length; i < len; ++i) {
				for (int j = 0, l = renderers[i].materials.Length; j < l; ++j) {
					Object.Destroy(renderers[i].materials[j]);
				}
			}
		}
		Object.Destroy(obj);
	}
	//--------------------------------------------------------------------
	// 查找物件
	public static List<GameObject> find(string name, bool onlyRoot = false, bool onlyOne = false) {
		GameObject[] allObjects = (GameObject[])Resources.FindObjectsOfTypeAll(typeof(GameObject));
		List<GameObject> results = new List<GameObject>();
		for (int i = 0, len = allObjects.Length; i < len; ++i) {
			GameObject obj = allObjects[i];
			if (onlyRoot) {
				if (null != obj.transform.parent) {
					continue;
				}
			}
			if (HideFlags.NotEditable == obj.hideFlags || HideFlags.HideAndDontSave == obj.hideFlags) {
				continue;
			}
			#if UNITY_EDITOR
			string sAssetPath = AssetDatabase.GetAssetPath(obj.transform.root.gameObject);
			if (!string.IsNullOrEmpty(sAssetPath)) {
				continue;
			}
			#endif
			if (name.Equals(obj.name)) {
				results.Add(obj);
				if (onlyOne) {
					break;
				}
			}
		}
		return results;
	}
	//--------------------------------------------------------------------
	// 设置父节点
	public static void setParent(GameObject node, GameObject parent = null) {
		if (null == node) {
			return;
		}
		Vector3 oldPositon = node.transform.localPosition;
		Quaternion oldRotation = node.transform.localRotation;
		Vector3 oldScale = node.transform.localScale;
		if (null == parent) {
			node.transform.SetParent(null);
		} else {
			node.transform.SetParent(parent.transform);
		}
		node.transform.localPosition = oldPositon;
		node.transform.localRotation = oldRotation;
		node.transform.localScale = oldScale;
	}
	//--------------------------------------------------------------------
	// 设置父节点
	public static void setParent(GameObject node, GameObject parent, string bone) {
		if (null == node || null == parent) {
			return;
		}
		if (null == bone || "" == bone) {
			setParent(node, parent);
			return;
		}
		Transform[] children = parent.GetComponentsInChildren<Transform>(true);
		for (int i = 0, len = children.Length; i < len; ++i) {
			if (bone == children[i].gameObject.name) {
				setParent(node, children[i].gameObject);
				return;
			}
		}
	}
	//--------------------------------------------------------------------
	// 设置位置
	public static void setPosition(GameObject node, Vector3 pos) {
		if (null == node) {
			return;
		}
		node.transform.localPosition = pos;
	}
	//--------------------------------------------------------------------
	// 设置转向
	public static void setRotate(GameObject node, Vector3 angle) {
		if (null == node) {
			return;
		}
		node.transform.localEulerAngles = angle;
	}
	//--------------------------------------------------------------------
	// 根据鼠标位置获取场景中某个平面上的点,cam:摄像机,mousePos:鼠标位置,inNormal:平面法线,inPoint:平面中心点
	public static Vector3 getPointByMousePosition(Camera cam, Vector3 mousePos, Vector3 inNormal, Vector3 inPoint) {
		if (null == cam) {
			return Vector3.zero;
		}
		Ray ray = cam.ScreenPointToRay(mousePos);	// 获取鼠标位置产生的射线
		Plane plane = new Plane(inNormal, inPoint);	// 构造临时平面
		float distance = 0.0f;
		plane.Raycast(ray, out distance);	// 获取射线到平面的距离
		return ray.GetPoint(distance);	// 获取射线与平面的交点
	}
	//--------------------------------------------------------------------
	// 根据鼠标位置获取场景中原点为(0,0,0),方向向上的平面上的点
	public static Vector3 getPointByMousePosition(float y) {
		return getPointByMousePosition(Camera.main, Input.mousePosition, Vector3.up, new Vector3(0, y, 0));
	}
	//--------------------------------------------------------------------
	// 根据速度获取当前位置,begin:开始位置,end:结束位置;curr:当前位置,speed:速度
	public static Vector3 getPositionBySpeed(Vector3 begin, Vector3 end, Vector3 curr, float speed) {
		float distance = Vector3.Distance(begin, end);
		float dist1 = Vector3.Distance(begin, curr);
		float dist2 = Vector3.Distance(curr, end);
		if ((dist1 + dist2 <= distance + 1) && (dist1 < distance) && (dist2 <= distance)) {
			float time = distance / speed;
			float xV = (end.x - begin.x) / time;
			float yV = (end.y - begin.y) / time;
			float zV = (end.z - begin.z) / time;
			return curr + new Vector3(xV, yV, zV);
		}
		return curr;
	}
	//--------------------------------------------------------------------
	// 根据x和y坐标计算点end相对于begin(begin为原点)在第几象限,右上角为第一象限,逆时针叠加,begin:开始位置,end:结束位置
	public static int calcQuadrant(Vector2 begin, Vector2 end) {
		// 1-4个象限
		if ((end.x > begin.x) && (end.y > begin.y)) {	// 第1象限
			return 1;
		}
		if ((end.x < begin.x) && (end.y > begin.y)) {	// 第2象限
			return 2;
		}
		if ((end.x < begin.x) && (end.y < begin.y)) {	// 第3象限
			return 3;
		}
		if ((end.x > begin.x) && (end.y < begin.y)) {	// 第4象限
			return 4;
		}
		// 4个方向的坐标轴
		if ((end.x > begin.x) && (end.y == begin.y)) {	// +x轴方向
			return 5;
		}
		if ((end.x == begin.x) && (end.y > begin.y)) {	// +z轴方向
			return 6;
		}
		if ((end.x < begin.x) && (end.y == begin.y)) {	// -x轴方向
			return 7;
		}
		if ((end.x == begin.x) && (end.y < begin.y)) {	// -z轴方向
			return 8;
		}
		return 0;		// 原点
	}
	//--------------------------------------------------------------------
	// 获取从点begin到点end的角度,左手坐标系,逆时针,begin:开始位置,end:结束位置
	public static float calcRadius(Vector2 begin, Vector2 end) {
		int quadrant = calcQuadrant(begin, end);
		if (0 == quadrant) {
			return 0.0f;
		}
		if (5 == quadrant) {
			return 0.0f;
		}
		if (6 == quadrant) {
			return 90.0f;
		}
		if (7 == quadrant) {
			return 180.0f;
		}
		if (8 == quadrant) {
			return 270.0f;
		}
		// 构建一个直角三角形,计算直角边
		Vector2 temp = new Vector2(end.x, begin.y);	// 直角点
		float a = Vector2.Distance(begin, temp);
		float b = Vector2.Distance(temp, end);
		float degree = Mathf.Atan2(b, a) * Mathf.Rad2Deg;
		if (1 == quadrant) {
			return degree;
		}
		if (2 == quadrant) {
			return 180.0f - degree;
		}
		if (3 == quadrant) {
			return degree + 180.0f;
		}
		if (4 == quadrant) {
			return 360.0f - degree;
		}
		return 0.0f;
	}
	//--------------------------------------------------------------------
	// 判断是否含有子节点
	public static bool isExistChild(GameObject parent, GameObject child) {
		if (null == parent || null == child) {
			return false;
		}
		Transform[] children = parent.GetComponentsInChildren<Transform>(true);
		for (int i = 0, len = children.Length; i < len; ++i) {
			if (child == children[i].gameObject) {
				return true;
			}
		}
		return false;
	}
	//--------------------------------------------------------------------
	// 获取第1个名称为name的子节点
	public static GameObject getChildByName(GameObject parent, string name) {
		if (null == parent) {
			return null;
		}
		Transform[] children = parent.GetComponentsInChildren<Transform>(true);
		for (int i = 0, len = children.Length; i < len; ++i) {
			if (name == children[i].gameObject.name) {
				return children[i].gameObject;
			}
		}
		return null;
	}
	//--------------------------------------------------------------------
	// 获取名称为name的所有节点
	public static ArrayList getChildrenByName(GameObject parent, string name) {
		ArrayList list = new ArrayList();
		if (null == parent) {
			return list;
		}
		Transform[] children = parent.GetComponentsInChildren<Transform>(true);
		for (int i = 0, len = children.Length; i < len; ++i) {
			if (name == children[i].gameObject.name) {
				list.Add(children[i].gameObject);
			}
		}
		return list;
	}
	//--------------------------------------------------------------------
	// 添加网格碰撞器(模型不能是组合过的,且所有节点只有一个渲染器),name:节点名
	public static void addMeshCollider(GameObject obj, string name) {
		if (null == obj) {
			return;
		}
		MeshRenderer mr = obj.GetComponentInChildren<MeshRenderer>();
		SkinnedMeshRenderer smr = obj.GetComponentInChildren<SkinnedMeshRenderer>();
		if (null == mr && null == smr) {
			return;
		}
		if ("" == name) {
			if (null != mr) {
				mr.gameObject.AddComponent<MeshCollider>();
			} else if (null != smr) {
				smr.gameObject.AddComponent<MeshCollider>().sharedMesh = smr.sharedMesh;
			}
			return;
		}
		Transform[] children = obj.GetComponentsInChildren<Transform>(true);
		for (int i = 0, len = children.Length; i < len; ++i) {
			if (name == children[i].gameObject.name) {
				if (null != smr) {
					children[i].gameObject.AddComponent<MeshCollider>().sharedMesh = smr.sharedMesh;
				}
			}
		}
	}
	//--------------------------------------------------------------------
	// 获取渲染器(所有节点只能有一个渲染器)
	public static Renderer getRenderer(GameObject obj) {
		if (null == obj) {
			return null;
		}
		return obj.GetComponentInChildren<Renderer>();
	}
	//--------------------------------------------------------------------
	// 获取材质列表
	public static Material[] getMaterials(GameObject obj) {
		Material[] materials = {};
		if (null == obj) {
			return materials;
		}
		Renderer renderer = obj.GetComponent<Renderer>();
		if (null == renderer) {
			return materials;
		}
		return renderer.materials;
	}
	//--------------------------------------------------------------------
	// 设置材质
	public static void setMaterial(GameObject obj, int index, Material mat) {
		if (null == obj) {
			return;
		}
		Renderer renderer = obj.GetComponent<Renderer>();
		if (null == renderer) {
			return;
		}
		if (index < 0 || index >= renderer.materials.Length) {
			return;
		}
		Object.Destroy(renderer.materials[index]);
		Material[] materials = renderer.materials;
		materials[index] = mat;
		renderer.materials = materials;
	}
	//--------------------------------------------------------------------
	// 物件是否显示
	public static bool isVisible(GameObject obj) {
		if (null == obj) {
			return false;
		}
		Renderer[] renders = obj.GetComponentsInChildren<Renderer>();
		for (int i = 0, len = renders.Length; i < len; ++i) {
			if (renders[i].enabled) {
				return true;
			}
		}
		return false;
	}
	//--------------------------------------------------------------------
	// 设置物件是否显示
	public static void setVisible(GameObject obj, bool visible) {
		if (null == obj) {
			return;
		}
		Renderer[] renders = obj.GetComponentsInChildren<Renderer>(true);
		for (int i = 0, len = renders.Length; i < len; ++i) {
			renders[i].enabled = visible;
		}
		Collider[] colliders = obj.GetComponentsInChildren<Collider>(true);
		for (int i = 0, len = colliders.Length; i < len; ++i) {
			colliders[i].enabled = visible;
		}
	}
	//--------------------------------------------------------------------
	// 设置物件透明度
	public static void setOpacity(GameObject obj, float opacity) {
		if (null == obj) {
			return;
		}
		opacity = opacity < 0 ? 0 : (opacity > 1 ? 1 : opacity);
		Color tempColor;
		Renderer[] renderers = obj.GetComponentsInChildren<Renderer>(true);
		for (int i = 0, len = renderers.Length; i < len; ++i) {
			tempColor = renderers[i].material.color;
			tempColor.a = opacity;
			renderers[i].material.color = tempColor;
		}
		Light[] lights = obj.GetComponentsInChildren<Light>(true);
		for (int i = 0, len = lights.Length; i < len; ++i) {
			tempColor = lights[i].color;
			tempColor.a = opacity;
			lights[i].color = tempColor;
		}
		Image[] images = obj.GetComponentsInChildren<Image>(true);
		for (int i = 0, len = images.Length; i < len; ++i) {
			tempColor = images[i].color;
			tempColor.a = opacity;
			images[i].color = tempColor;
		}
		RawImage[] rawImages = obj.GetComponentsInChildren<RawImage>(true);
		for (int i = 0, len = rawImages.Length; i < len; ++i) {
			tempColor = rawImages[i].color;
			tempColor.a = opacity;
			rawImages[i].color = tempColor;
		}
		Text[] texts = obj.GetComponentsInChildren<Text>(true);
		for (int i = 0, len = texts.Length; i < len; ++i) {
			tempColor = texts[i].color;
			tempColor.a = opacity;
			texts[i].color = tempColor;
		}
	}
	//--------------------------------------------------------------------
	// 加载图片
	public static Texture2D loadTexture2D(string file) {
		FileStream fs = new FileStream(file, FileMode.Open, FileAccess.Read);
		fs.Seek(0, SeekOrigin.Begin);
		byte[] binary = new byte[fs.Length];
		fs.Read(binary, 0, (int)fs.Length);
		fs.Close();
		fs.Dispose();
		fs = null;
		Texture2D tex = new Texture2D(1, 1);
		tex.LoadImage(binary);
		return tex;
	}
	//--------------------------------------------------------------------
	// 创建精灵
	public static Sprite createSprite(string file, bool isInResources = true) {
		if (isInResources) {
			// method1:file必须放着Assets/Resources目录,且不包含后缀名,例:"Assets/Resources/Pictures/icon.png" => "Pictures/icon"
			return Resources.Load(file, typeof(Sprite)) as Sprite;
		} else {
			// method2:file为全路径名,例:"Assets/Resources/Pictures/icon.png"
			Texture2D tex = loadTexture2D(file);
			return Sprite.Create(tex, new Rect(0, 0, tex.width, tex.height), new Vector2(0.5f, 0.5f));
		}
	}
	//--------------------------------------------------------------------
	// 设置大小
	public static void setRectTransformSize(GameObject obj, Vector2 size) {
		if (null == obj) {
			return;
		}
		RectTransform rectTrans = obj.GetComponent<RectTransform>();
		if (null == rectTrans) {
			return;
		}
		Vector2 oldSize = rectTrans.rect.size;
		Vector2 deltaSize = size - oldSize;
		rectTrans.offsetMin = rectTrans.offsetMin - new Vector2(deltaSize.x * rectTrans.pivot.x, deltaSize.y * rectTrans.pivot.y);
		rectTrans.offsetMax = rectTrans.offsetMax + new Vector2(deltaSize.x * (1.0f - rectTrans.pivot.x), deltaSize.y * (1.0f - rectTrans.pivot.y));
	}
	//--------------------------------------------------------------------
	// 获取曲线上面的所有点
	public static Vector3[] pointListSpline(Vector3[] path, int segment = 10, float tension = 0.5f) {
		int segmentAmount = path.Length * segment;
		Vector3[] pointList = new Vector3[segmentAmount + 1];
		for (int index = 0; index <= segmentAmount; ++index) {
			int p = 0;
			float t = 0;
			Formula.calcCardinalSplineIndex(path.Length, (float)index/segmentAmount, out p, out t);
			Vector3 pt0 = path[(int)Formula.valueRange((float)(p - 1), 0, (float)(path.Length - 1))];
			Vector3 pt1 = path[(int)Formula.valueRange((float)(p + 0), 0, (float)(path.Length - 1))];
			Vector3 pt2 = path[(int)Formula.valueRange((float)(p + 1), 0, (float)(path.Length - 1))];
			Vector3 pt3 = path[(int)Formula.valueRange((float)(p + 2), 0, (float)(path.Length - 1))];
			float[] xyz = Formula.calcCardinalSplinePointAt(pt0.x, pt0.y, pt0.z, pt1.x, pt1.y, pt1.z, pt2.x, pt2.y, pt2.z, pt3.x, pt3.y, pt3.z, t, tension);
			pointList[index].x = xyz[0];
			pointList[index].y = xyz[1];
			pointList[index].z = xyz[2];
		}
		return pointList;
	}
	//--------------------------------------------------------------------
	// 获取曲线长度
	public static float lengthSpline(Vector3[] path, int segment = 10, float tension = 0.5f) {
		int segmentAmount = path.Length * segment;
		float length = 0.0f;
		Vector3 prevPt = path[0];
		for (int index = 1; index <= segmentAmount; ++index) {
			int p = 0;
			float t = 0;
			Formula.calcCardinalSplineIndex(path.Length, (float)index/segmentAmount, out p, out t);
			Vector3 pt0 = path[(int)Formula.valueRange((float)(p - 1), 0, (float)(path.Length - 1))];
			Vector3 pt1 = path[(int)Formula.valueRange((float)(p + 0), 0, (float)(path.Length - 1))];
			Vector3 pt2 = path[(int)Formula.valueRange((float)(p + 1), 0, (float)(path.Length - 1))];
			Vector3 pt3 = path[(int)Formula.valueRange((float)(p + 2), 0, (float)(path.Length - 1))];
			float[] xyz = Formula.calcCardinalSplinePointAt(pt0.x, pt0.y, pt0.z, pt1.x, pt1.y, pt1.z, pt2.x, pt2.y, pt2.z, pt3.x, pt3.y, pt3.z, t, tension);
			Vector3 currPt = new Vector3(xyz[0], xyz[1], xyz[2]);
			length += Vector3.Distance(prevPt, currPt);
			prevPt = currPt;
		}
		return length;
	}
	//--------------------------------------------------------------------
	// 角度取整(分为8份)
	public static float roundAngleBy8(float angle, float offset = 10) {
		offset = (offset >= 0 && offset <= 30) ? offset : 0;
		if ((angle >= 0 && angle <= offset) || (angle >= 360 - offset && angle <= 360)) {
			angle = 0;
		} else if (angle > offset && angle < 90 - offset) {
			angle = 45;
		} else if (angle >= 90 - offset && angle <= 90 + offset) {
			angle = 90;
		} else if (angle > 90 + offset && angle < 180 - offset) {
			angle = 135;
		} else if (angle >= 180 - offset && angle <= 180 + offset) {
			angle = 180;
		} else if (angle > 180 + offset && angle < 270 - offset) {
			angle = 225;
		} else if (angle >= 270 - offset && angle <= 270 + offset) {
			angle = 270;
		} else if (angle > 270 + offset && angle < 360 - offset) {
			angle = 315;
		}
		return angle;
	}
	//--------------------------------------------------------------------
	// 角度取整(分为12份)
	public static float roundAngleBy12(float angle, float offset = 5) {
		offset = (offset >= 0 && offset <= 15) ? offset : 0;
		if ((angle >= 0 && angle <= offset) || (angle >= 360 - offset && angle <= 360)) {
			angle = 0;
		} else if (angle > offset && angle < 45) {
			angle = 30;
		} else if (angle >= 45 && angle < 90 - offset) {
			angle = 60;
		} else if (angle >= 90 - offset && angle <= 90 + offset) {
			angle = 90;
		} else if (angle > 90 + offset && angle < 135) {
			angle = 120;
		} else if (angle >= 135 && angle < 180 - offset) {
			angle = 150;
		} else if (angle >= 180 - offset && angle <= 180 + offset) {
			angle = 180;
		} else if (angle > 180 + offset && angle < 225) {
			angle = 210;
		} else if (angle >= 225 && angle < 270 - offset) {
			angle = 240;
		} else if (angle >= 270 - offset && angle <= 270 + offset) {
			angle = 270;
		} else if (angle > 270 + offset && angle < 315) {
			angle = 300;
		} else if (angle >= 315 && angle < 360 - offset) {
			angle = 330;
		}
		return angle;
	}
	//--------------------------------------------------------------------
}
