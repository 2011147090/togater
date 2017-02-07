using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class GUIManager : MonoBehaviour 
{
	public static GUIManager Instance;

	void Awake()
	{
		Instance = this;
	}

	void Start () 
	{
		
	}

	void Update () 
	{
		
	}

	public void ShowMessageBox(string message)
	{
		GameObject window = (GameObject)Resources.Load ("prefabs/message box");
	}
}
