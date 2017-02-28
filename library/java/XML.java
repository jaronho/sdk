package com.yaxon.hudmain.jh.library;

import java.io.ByteArrayInputStream;
import java.io.IOException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.w3c.dom.Text;
import org.xml.sax.SAXException;

public class XML {
	
	public static String read(String content, String key) throws SAXException, IOException, ParserConfigurationException {
		DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
		DocumentBuilder db = dbf.newDocumentBuilder();
		Document doc = db.parse(new ByteArrayInputStream(content.getBytes()));
		Element root = doc.getDocumentElement();
		NodeList nodes = root.getElementsByTagName(key);
		if (1 == nodes.getLength()) {
			Element e = (Element)nodes.item(0);
			Text t = (Text)e.getFirstChild();
			return t.getNodeValue();
		}
		return null;
	}
}
