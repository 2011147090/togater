using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Events;

public class Prompt : MonoBehaviour 
{
	private UnityAction<string> commandEvent;

	private Text titleText;
	private Text answerText;

	private Button okButton;
	private Button cancleButton;

	private InputField inputField;

	private RectTransform rectTransform;

	void Start () 
	{
		
	}

	void Update () 
	{

	}

	public void Initialize(string titleMessage, UnityAction<string> action)
	{
		rectTransform = GetComponent<RectTransform> ();
		rectTransform.SetParent (GameObject.Find ("Main Panel").transform);
		rectTransform.anchoredPosition = new Vector2 (0, 0);

		okButton = transform.FindChild ("OK Button").GetComponent<Button> ();
		cancleButton = transform.FindChild ("Cancle Button").GetComponent<Button> ();

		commandEvent = action;

		okButton.onClick.AddListener (CommandEvent);
		cancleButton.onClick.AddListener (CloseWindow);

		titleText = transform.FindChild ("Title Text").GetComponent<Text> ();
		titleText.text = titleMessage;

		answerText = transform.FindChild ("Answer").GetComponent<Text> ();

		inputField = transform.FindChild ("InputField").GetComponent<InputField> ();
	}

	private void CommandEvent()
	{
		commandEvent (inputField.text);
		Destroy (gameObject);
	}

	private void CloseWindow()
	{
		Destroy (gameObject);
	}
}
