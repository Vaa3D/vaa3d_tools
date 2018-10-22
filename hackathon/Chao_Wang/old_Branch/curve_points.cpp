void SobelGradDirction(Mat &imageSource, Mat &imageSobelX, Mat &imageSobelY)
{

    imageSobelX = Mat::zeros(imageSource.size(), CV_32SC1);
    imageSobelY = Mat::zeros(imageSource.size(), CV_32SC1);
    //取出原图和X和Y梯度图的数组的首地址
    uchar *P = imageSource.data;
    uchar *PX = imageSobelX.data;
    uchar *PY = imageSobelY.data;

    //取出每行所占据的字节数
    int step = imageSource.step;
    int stepXY = imageSobelX.step;

    int index = 0;//梯度方向角的索引
    for (int i = 1; i < imageSource.rows - 1; ++i)
    {
        for (int j = 1; j < imageSource.cols - 1; ++j)
        {
            //通过指针遍历图像上每一个像素
            double gradY = P[(i + 1)*step + j - 1] + P[(i + 1)*step + j] * 2 + P[(i + 1)*step + j + 1] - P[(i - 1)*step + j - 1] - P[(i - 1)*step + j] * 2 - P[(i - 1)*step + j + 1];
            PY[i*stepXY + j*(stepXY / step)] = abs(gradY);

            double gradX = P[(i - 1)*step + j + 1] + P[i*step + j + 1] * 2 + P[(i + 1)*step + j + 1] - P[(i - 1)*step + j - 1] - P[i*step + j - 1] * 2 - P[(i + 1)*step + j - 1];
            PX[i*stepXY + j*(stepXY / step)] = abs(gradX);
        }
    }
    //将梯度数组转换成8位无符号整型
    convertScaleAbs(imageSobelX, imageSobelX);
    convertScaleAbs(imageSobelY, imageSobelY);


        SobelAmpXX = Mat_<float>(imageGradX.size(), CV_32FC1);
        for (int i = 0; i < SobelAmpXX.rows; i++)
        {
            for (int j = 0; j < SobelAmpXX.cols; j++)
            {
                SobelAmpXX.at<float>(i, j) = imageGradX.at<uchar>(i, j)*imageGradX.at<uchar>(i, j);
            }
        }





        SobelAmpYY = Mat_<float>(imageGradY.size(), CV_32FC1);
        for (int i = 0; i < SobelAmpYY.rows; i++)
        {
            for (int j = 0; j < SobelAmpYY.cols; j++)
            {
                SobelAmpYY.at<float>(i, j) = imageGradY.at<uchar>(i, j)*imageGradY.at<uchar>(i, j);
            }
        }


}
