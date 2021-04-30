using System.Collections;
using System.Collections.Generic;

public static class MessageQueue {
	private static List<Message> mMessageList = new List<Message>();

	private class Message {
		public Message(string type, string msg) {
			this.type = type;
			this.msg = msg;
		}
		public string type;
		public string msg;
	}

	// 插入消息(会删除已有的同类消息,然后在末尾添加新消息)
	public static void push(string type, string msg) {
		for (int i = 0, len = mMessageList.Count; i < len; ++i) {
			if (type == mMessageList[i].type) {
				mMessageList.RemoveAt(i);
				break;
			}
		}
		mMessageList.Add(new Message(type, msg));
	}

	// 弹出消息(获取第一个)
	public static string pop() {
		string msg = "";
		if (mMessageList.Count > 0) {
			msg = mMessageList[0].msg;
			mMessageList.RemoveAt(0);
		}
		return msg;
	}

	// 获取消息列表
	public static List<string> getAll() {
		List<string> msgList = new List<string>();
		for (int i = 0, len = mMessageList.Count; i < len; ++i) {
			msgList.Add(mMessageList[i].msg);
		}
		mMessageList.Clear();
		return msgList;
	}
}
