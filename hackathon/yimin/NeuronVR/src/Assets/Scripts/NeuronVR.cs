//======= Copyright (c) Valve Corporation, All rights reserved. ===============
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System.Diagnostics;
using System.IO;
using BitMiracle.LibTiff.Classic;

[RequireComponent(typeof(SteamVR_TrackedObject))]
public class NeuronVR : MonoBehaviour
{
	public GameObject prefab;
	public Rigidbody attachPoint;


    private LineRenderer myline;
    private List<Vector3> pointsList;
    private LineRenderer lines;

    private int gripnum = 0;
    private float defaultwidth = 0.005f;
    private float currentwidth = 0.005f;

    public static int width;
    public static int height;
    public static int newPageNumber;


    SteamVR_TrackedObject trackedObj;
	//FixedJoint joint;

	void Awake()
	{
		trackedObj = GetComponent<SteamVR_TrackedObject>();
        _Init();
    }
    #region _Init()
    private void _Init()
    {
        if (m_init) return;
        m_init = true;
        if (myline == null)
        {
            myline = GameObject.Find("TestCube").AddComponent<LineRenderer>();
        }
        myline.shadowCastingMode = UnityEngine.Rendering.ShadowCastingMode.Off;;
        myline.startWidth = defaultwidth;
        myline.endWidth = defaultwidth;
        myline.startColor = Color.green;
        myline.endColor = Color.green;
        //myline.material = new Material(Shader.Find("Standard"));
        myline.material = new Material(Shader.Find("Sprites/Default"));
        myline.enabled = true;
        myline.numPositions = 0;
        //myline.SetPosition(0, new Vector3(100,100,100));
        myline.useWorldSpace = true;
        pointsList = new List<Vector3>();
    }
    bool m_init = false;//*/
    #endregion

    void Start()
    {


        int[,,] readtiffdata = readtifffile(@"img.tif");//载入图像img.tif
        GameObject.Find("[CameraRig]").transform.position = new Vector3(0.8f, -0.3f, -0.6f);//*/这行代码功能是改变VR主视角位置，使图像显示在一个合适的位置，也可以在Unity引擎中改变




        //UnityEngine.Debug.Log("Run Here.2");
        for (int k=0;k<Mathf.Min(newPageNumber,1000);k++)
        {
            for(int j=0;j<Mathf.Min(height,1000);j++)
            {
                for(int i=0;i< Mathf.Min(width,1000);i++)
                {

                    float  currentgray =(float)(readtiffdata[i, j, k])/255;

                    if (currentgray>0.3)//阈值设置为0.3*255
                    {
                        var go = GameObject.Instantiate(prefab);
 
                        go.transform.position = new Vector3(2f-(float)i*0.005f, 3-(float)j *0.005f,-3+ (float)k *0.005f);
                        go.GetComponent<Renderer>().material.color = new Color(currentgray, currentgray, currentgray, currentgray);

                     }
                }
            }
        }
    }//*/

    void FixedUpdate()
	{

		var device = SteamVR_Controller.Input((int)trackedObj.index);
        #region draw curves
        if (device.GetTouchDown(SteamVR_Controller.ButtonMask.Trigger))
        {
                lines = Instantiate(myline);
                lines.startWidth = currentwidth;
                lines.endWidth = currentwidth;
                //lines.material = new Material(Shader.Find("Sprites/Default"));
                lines.enabled = true;
                lines.numPositions = 0;
                pointsList.RemoveRange(0, pointsList.Count);
                /*lines.startColor = Color.red;
                lines.endColor = Color.red;//*/
                lines.startColor = new Color((float)70 / 255, (float)238 / 255, (float)245 / 255);
                lines.endColor = new Color((float)70 / 255, (float)238 / 255, (float)245 / 255);
            UnityEngine.Debug.Log("Run Here.5");
        }
        if (device.GetPress(SteamVR_Controller.ButtonMask.Trigger))
        {
            Vector3 mousePos = attachPoint.transform.position;
            if (!pointsList.Contains(mousePos))
            {
                lines.numPositions = 0;
                pointsList.Add(mousePos);
                lines.numPositions = pointsList.Count;
            }
            UnityEngine.Debug.Log("Run Here.6");
            //for (int i = 0; i < pointsList.Count; i++)
            //{
            //lines.SetPosition(i, (Vector3)pointsList[i]);
            lines.SetPosition(pointsList.Count - 1, (Vector3)pointsList[pointsList.Count - 1]);  
            //}
        }
        if (device.GetTouchUp(SteamVR_Controller.ButtonMask.ApplicationMenu))
        {
            //UnityEngine.Debug.Log("Run Here.Menu");
            DestoryLine();
        }
        if (device.GetTouchDown(SteamVR_Controller.ButtonMask.Grip))
        {

            switch (gripnum%3)
            {
                case 0:
                    currentwidth = defaultwidth;
                    currentwidth = defaultwidth;
                    break;
                case 1:
                    currentwidth = defaultwidth*2f;
                    currentwidth = defaultwidth*2f;
                    break;
                case 2:
                    currentwidth = defaultwidth*4f;
                    currentwidth = defaultwidth*4f;
                    break;


                default:
                    break;
            }
            gripnum++;
        }
        #endregion
    }
    #region 读取tiff数据到三维数组中
    public static int[,,] readtifffile(string TiffName)
    {
        using (Tiff image = Tiff.Open(TiffName, "r"))
        {
            if (image==null)
            {
                return null;
            }
            //newPageNumber = 1;
            newPageNumber = image.NumberOfDirectories();
            FieldValue[] value = image.GetField(TiffTag.IMAGEWIDTH);
            width = value[0].ToInt();

            value = image.GetField(TiffTag.IMAGELENGTH);
            height = value[0].ToInt();

            int imageSize = width * height;
            short k = 0;//当前帧数

            int[,,] tifffiledata = new int[width, height, newPageNumber];
            do
            {
                image.SetDirectory(k);
                int[] raster = new int[imageSize];
                if (!image.ReadRGBAImage(width, height, raster))
                {
                    return null;
                }
                for (int i = 0; i < width; i++)
                {
                    for (int j = 0; j < height; j++)
                    {
                            tifffiledata[i, j, k] = getRGB(i, j, raster, width, height);
                    }

                }
                k++;
                raster = null;
            }
            while (k < newPageNumber);
            return tifffiledata;
        }

    }
    
    public static int getRGB(int x, int y, int[] raster, int width, int height)
    {
        int offset = (height - y - 1) * width + x;
        int red = Tiff.GetR(raster[offset]);
        //int green = Tiff.GetG(raster[offset]);
        //int blue = Tiff.GetB(raster[offset]);
        // return Color.FromArgb(red, green, blue);
        return red;
    }

    #endregion
    public void DestoryLine()
    {
        while (GameObject.Find("TestCube(Clone)"))
        {
            Object.DestroyImmediate(GameObject.Find("TestCube(Clone)"));
        }
        return;
    }
}
