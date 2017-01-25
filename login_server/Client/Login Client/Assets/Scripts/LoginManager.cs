using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.Networking;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;

public class LoginManager : MonoBehaviour 
{
	public static LoginManager Instance;

	public string sessionID;

	public GameObject loginPanel;
	public GameObject registerPanel;
	public GameObject coverTheTouch;
	public GameObject forgotIdPanel;
	public GameObject forgotPasswordPanel;
	public GameObject overlapCheckSuccess;
	public GameObject overlapCheckFail;
	public GameObject alertPanel;

	public bool isRegisterCheckEmail;
	public bool isRegisterCheckID;

	void Awake()
	{
		Instance = this;
	}

	void Start () 
	{
		isRegisterCheckID = false;
		isRegisterCheckEmail = false;
		Screen.SetResolution (1280, 720, false);
	}

	void Update () 
	{
		
	}

	public void OnRegister()
	{
		StartCoroutine ("Register");
	}

	public void OnLogin()
	{
		StartCoroutine ("Login");
	}

	public void OnForgotId()
	{
		coverTheTouch.SetActive (true);
		forgotIdPanel.SetActive (true);

		forgotIdPanel.transform.FindChild ("OK Button").GetComponent<RectTransform> ().anchoredPosition = new Vector2 (-100, -45.6f);
		forgotIdPanel.transform.FindChild ("Cancle Button").gameObject.SetActive (true);
	}

	public void OnForgetIDCancle()
	{
		forgotIdPanel.SetActive (false);
		coverTheTouch.SetActive (false);
	}

	public void OnForgotPassword()
	{
		coverTheTouch.SetActive (true);
		forgotPasswordPanel.SetActive (true);

		forgotPasswordPanel.transform.FindChild ("OK Button").GetComponent<RectTransform> ().anchoredPosition = new Vector2 (-100, -45.6f);
		forgotPasswordPanel.transform.FindChild ("Cancle Button").gameObject.SetActive (true);
	}

	public void OnForgetPasswordCancle()
	{
		forgotPasswordPanel.SetActive (false);
		coverTheTouch.SetActive (false);
	}

	public void OnForgotIDOK()
	{
		if (forgotIdPanel.transform.FindChild ("ID").GetComponent<Text> ().enabled == false)
			StartCoroutine ("FindID");
		else 
		{
			forgotIdPanel.SetActive (false);
			coverTheTouch.SetActive (false);

			forgotIdPanel.transform.FindChild ("Text").gameObject.SetActive (true);
			forgotIdPanel.transform.FindChild ("Email InputField").gameObject.SetActive (true);
			forgotIdPanel.transform.FindChild ("Email InputField").GetComponent<InputField> ().text = "";
			forgotIdPanel.transform.FindChild ("ID").GetComponent<Text> ().enabled = false;
		}
	}

	public void OnForgotPasswordOK()
	{
		if (forgotPasswordPanel.transform.FindChild ("Password").GetComponent<Text> ().enabled == false)
			StartCoroutine ("FindPassword");
		else 
		{
			forgotPasswordPanel.SetActive (false);
			coverTheTouch.SetActive (false);

			forgotPasswordPanel.transform.FindChild ("Text").gameObject.SetActive (true);
			forgotPasswordPanel.transform.FindChild ("ID InputField").gameObject.SetActive (true);
			forgotPasswordPanel.transform.FindChild ("ID InputField").GetComponent<InputField> ().text = "";
			forgotPasswordPanel.transform.FindChild ("Password").GetComponent<Text> ().enabled = false;
		}
	}

	private IEnumerator FindID()
	{
		WWWForm form = new WWWForm();
		form.AddField ("email", forgotIdPanel.transform.FindChild("Email InputField").GetComponent<InputField>().text);

		UnityWebRequest www = UnityWebRequest.Post("http://192.168.1.18:3000/forgot/id", form);

		yield return www.Send();

		if (www.downloadHandler.text != "find fail") 
		{
			forgotIdPanel.transform.FindChild ("ID").GetComponent<Text> ().enabled = true;
			forgotIdPanel.transform.FindChild ("ID").GetComponent<Text> ().text = "ID : " + www.downloadHandler.text;

			forgotIdPanel.transform.FindChild ("Text").gameObject.SetActive (false);
			forgotIdPanel.transform.FindChild ("Email InputField").gameObject.SetActive (false);

			forgotIdPanel.transform.FindChild ("OK Button").GetComponent<RectTransform> ().anchoredPosition = new Vector2 (0, -45.6f);
			forgotIdPanel.transform.FindChild ("Cancle Button").gameObject.SetActive (false);
		}
	}

	private IEnumerator FindPassword()
	{
		WWWForm form = new WWWForm();
		form.AddField ("id", forgotPasswordPanel.transform.FindChild("ID InputField").GetComponent<InputField>().text);

		UnityWebRequest www = UnityWebRequest.Post("http://192.168.1.18:3000/forgot/password", form);

		yield return www.Send();

		if (www.downloadHandler.text != "find fail") 
		{
			forgotPasswordPanel.transform.FindChild ("Password").GetComponent<Text> ().enabled = true;
			forgotPasswordPanel.transform.FindChild ("Password").GetComponent<Text> ().text = "Password : " + www.downloadHandler.text;

			forgotPasswordPanel.transform.FindChild ("Text").gameObject.SetActive (false);
			forgotPasswordPanel.transform.FindChild ("ID InputField").gameObject.SetActive (false);

			forgotPasswordPanel.transform.FindChild ("OK Button").GetComponent<RectTransform> ().anchoredPosition = new Vector2 (0, -45.6f);
			forgotPasswordPanel.transform.FindChild ("Cancle Button").gameObject.SetActive (false);
		}
	}

	private IEnumerator Login()
	{
		WWWForm form = new WWWForm();
		form.AddField ("id", loginPanel.transform.FindChild("ID InputField").GetComponent<InputField>().text);
		form.AddField ("password", loginPanel.transform.FindChild("Password InputField").GetComponent<InputField>().text);

		UnityWebRequest www = UnityWebRequest.Post("http://192.168.1.18:3000/login", form);

		yield return www.Send();

		//string orginalSID = www.GetResponseHeader("SET-COOKIE");
		//sessionID = orginalSID.Substring (16, 32);

		if (www.downloadHandler.text.Substring(0,2) == "ok")
			SceneManager.LoadScene ("Lobby");
		else 
		{
			alertPanel.SetActive (true);
			coverTheTouch.SetActive (true);
			alertPanel.transform.FindChild ("Text").GetComponent<Text> ().text = "Retry Please";
		}
		
	}

	public void OnRegisterEmailCheck()
	{
		StartCoroutine ("RegisterEmailCheck");
	}

	public void OnRegisterIDCheck()
	{
		StartCoroutine ("RegisterIDCheck");
	}

	public void OnCloseOverlapSuccess()
	{
		overlapCheckSuccess.SetActive (false);
	}

	public void OnCloseOverlapFail()
	{
		overlapCheckFail.SetActive (false);
	}

	public void OnCloseAlertPanel()
	{
		alertPanel.SetActive (false);
		coverTheTouch.SetActive (false);
	}

	private IEnumerator Register()

	{
		if (isRegisterCheckEmail == true && isRegisterCheckID == true) 
		{
			WWWForm form = new WWWForm ();
			form.AddField ("id", registerPanel.transform.FindChild("ID InputField").GetComponent<InputField>().text);
			form.AddField ("password", registerPanel.transform.FindChild("Password InputField").GetComponent<InputField>().text);
			form.AddField ("email", registerPanel.transform.FindChild("Email InputField").GetComponent<InputField>().text);

			UnityWebRequest www = UnityWebRequest.Post("http://192.168.1.18:3000/register", form);

			yield return www.Send();
			 
			if (www.downloadHandler.text == "register ok") 
			{
				Debug.Log ("register ok");
				InputField[] registerInput = registerPanel.transform.GetComponentsInChildren<InputField> (true);

				foreach (var item in registerInput)
					item.text = "";
			}
		}
	}

	private IEnumerator RegisterEmailCheck()
	{
		WWWForm form = new WWWForm ();
		form.AddField ("email", registerPanel.transform.FindChild("Email InputField").GetComponent<InputField>().text);
		UnityWebRequest www = UnityWebRequest.Post("http://192.168.1.18:3000/register/overlap/email", form);
		yield return www.Send();

		if (www.downloadHandler.text == "email ok") 
		{
			isRegisterCheckEmail = true;
			Debug.Log ("email ok");
			overlapCheckSuccess.SetActive (true);
		} 
		else 
		{
			Debug.Log ("check email fail");
			isRegisterCheckEmail = false;
			overlapCheckFail.SetActive (true);
		}
	}

	private IEnumerator RegisterIDCheck()
	{
		if (isRegisterCheckEmail == true) 
		{
			WWWForm form = new WWWForm ();
			form.AddField ("id", registerPanel.transform.FindChild("ID InputField").GetComponent<InputField>().text);
			UnityWebRequest www = UnityWebRequest.Post("http://192.168.1.18:3000/register/overlap/id", form);
			yield return www.Send();

			if (www.downloadHandler.text == "id ok") 
			{
				isRegisterCheckID = true;
				Debug.Log ("id ok");
				overlapCheckSuccess.SetActive (true);
			} 
			else 
			{
				Debug.Log ("check id fail");
				isRegisterCheckID = false;
				overlapCheckFail.SetActive (true);
			}
		}
	}
}
