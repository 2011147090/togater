using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class ChattingManager : MonoBehaviour 
{
	public InputField chatField;
	public GameObject chatText;

	public UIInput nguiChatInfut;

	void Start () 
	{
		
	}

	void Update () 
	{
		
	}

	public int count = 0;

	public void ChatTest(string val)
	{
		chatField.text = "";

		GameObject text = Instantiate (chatText, Vector3.zero, Quaternion.identity) as GameObject;
		text.transform.SetParent (GameObject.Find ("List").transform);
		count += 1;

		if (count > 10) 
		{
			GameObject.Find ("List").GetComponent<RectTransform> ().sizeDelta = new Vector2 (
				GameObject.Find ("List").GetComponent<RectTransform> ().sizeDelta.x, 
				GameObject.Find ("List").GetComponent<RectTransform> ().sizeDelta.y + 45);
			
			GameObject.Find ("List").GetComponent<RectTransform> ().anchoredPosition = new Vector2 (
				GameObject.Find ("List").GetComponent<RectTransform> ().anchoredPosition.x,
				GameObject.Find ("List").GetComponent<RectTransform> ().anchoredPosition.y + 25);
		}
	}

	public void TesetChatMessage()
	{
		
	}

	public void SendChatMessage()
	{
		if (chatField.text == "")
			return;
		
		GameObject text = Instantiate (chatText, Vector3.zero, Quaternion.identity) as GameObject;
		text.transform.SetParent (GameObject.Find ("List").transform);
		text.GetComponent<Text> ().text = chatField.text + Input.compositionString;


		count += 1;

		if (count > 10) 
		{
			GameObject.Find ("List").GetComponent<RectTransform> ().sizeDelta = new Vector2 (
				GameObject.Find ("List").GetComponent<RectTransform> ().sizeDelta.x, 
				GameObject.Find ("List").GetComponent<RectTransform> ().sizeDelta.y + 45);

			GameObject.Find ("List").GetComponent<RectTransform> ().anchoredPosition = new Vector2 (
				GameObject.Find ("List").GetComponent<RectTransform> ().anchoredPosition.x,
				GameObject.Find ("List").GetComponent<RectTransform> ().anchoredPosition.y + 25);
		}

		Application.ExternalCall ("socket.emit", "chat", chatField.text);
		chatField.text = "";
		chatField.ActivateInputField ();
	}

	public void RecvChatMessage(string chatMessage)
	{
		//chatField.text = "";

		GameObject text = Instantiate (chatText, Vector3.zero, Quaternion.identity) as GameObject;
		text.transform.SetParent (GameObject.Find ("List").transform);
		text.GetComponent<Text> ().text = chatMessage;
		count += 1;

		if (count > 10) 
		{
			GameObject.Find ("List").GetComponent<RectTransform> ().sizeDelta = new Vector2 (
				GameObject.Find ("List").GetComponent<RectTransform> ().sizeDelta.x, 
				GameObject.Find ("List").GetComponent<RectTransform> ().sizeDelta.y + 45);

			GameObject.Find ("List").GetComponent<RectTransform> ().anchoredPosition = new Vector2 (
				GameObject.Find ("List").GetComponent<RectTransform> ().anchoredPosition.x,
				GameObject.Find ("List").GetComponent<RectTransform> ().anchoredPosition.y + 25);
		}
	}
}
