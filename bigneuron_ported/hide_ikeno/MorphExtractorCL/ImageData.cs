using System;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace MorphExtractorCL
{
    class ImageData //: IDisposable
    {
        public bool[, ,] voxel;

        //3次元画像サイズ(縦,横,高さ)
        public int XSize { get; private set; }

        public int YSize { get; private set; }

        public int ZSize { get; private set; }

        public void LoadImageFromFile(IPParams param)
        {
            try
            {
                string[] files;

                files = Directory.GetFiles(param.ipath, "*.bmp");
                if (files.Length == 0)
                    files = Directory.GetFiles(param.ipath, "*.png");
                if (files.Length == 0)
                    files = Directory.GetFiles(param.ipath, "*.jpg");
                if (files.Length == 0)
                    files = Directory.GetFiles(param.ipath, "*.tiff");
                if (files.Length == 0)
                    files = Directory.GetFiles(param.ipath, "*.tif");

                if (files.Length == 0)
                {
                    //例外: 画像がない
                    throw new Exception("Images are not found.");
                }

                Array.Sort<String>(files);

                int pix;
                int stride;

                using (var image = new Bitmap(files[0]))
                {
                    switch (image.PixelFormat)
                    {
                        case System.Drawing.Imaging.PixelFormat.Format32bppArgb:
                            pix = 4;
                            break;

                        case System.Drawing.Imaging.PixelFormat.Format24bppRgb:
                            pix = 3;
                            break;

                        case System.Drawing.Imaging.PixelFormat.Format8bppIndexed:
                            pix = 1;
                            break;

                        default:
                            throw new Exception("This image format is not supported.");
                    }

                    XSize = image.Width;
                    YSize = image.Height;
                    ZSize = files.Length;
                    stride = ReadStride(image);
                }

                voxel = new bool[ZSize, YSize, XSize];

                int z = 0;

                byte[] buffer = new byte[stride * YSize];
                foreach (string filename in files)
                {
                    using (var image = new Bitmap(filename))
                    {
                        if (image.Width != XSize || image.Height != YSize)
                        {
                            //例外: 画像サイズが一致しない
                            throw new Exception("The image size is different for each image.");
                        }

                        BitmapData imagedata = image.LockBits(new Rectangle(0, 0, image.Width, image.Height), ImageLockMode.ReadOnly, image.PixelFormat);
                        Marshal.Copy(imagedata.Scan0, buffer, 0, buffer.Length);

                        for (int y = YSize - 1; y >= 0; y--)
                        {
                            for (int x = 0; x < XSize; x++)
                            {
                                //voxel[z, y, x] = barr[y * stride + pix * x] >= param.binarizeThreshold ? !param.back : param.back;
                                int val;
                                switch (pix)
                                {
                                    case 1:
                                        voxel[z, y, x] = buffer[y * stride + pix * x] >= param.binarizeThreshold ? !param.back : param.back;
                                        break;

                                    case 3:
                                        val = (byte)(buffer[y * stride + pix * x] * 0.114 + buffer[y * stride + pix * x + 1] * 0.587 + buffer[y * stride + pix * x + 2] * 0.299);
                                        voxel[z, y, x] = val >= param.binarizeThreshold ? !param.back : param.back;
                                        break;

                                    case 4:
                                        val = (byte)(buffer[y * stride + pix * x + 1] * 0.114 + buffer[y * stride + pix * x + 2] * 0.587 + buffer[y * stride + pix * x + 3] * 0.299);
                                        voxel[z, y, x] = val >= param.binarizeThreshold ? !param.back : param.back;
                                        break;
                                }
                            }
                        }

                        image.UnlockBits(imagedata);
                    }
                    GC.Collect();
                    z++;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                Environment.Exit(1);
            }
        }

        /// <summary>
        /// 画像のストライドを取得する
        /// 
        /// ストライドとは、ビットマップの1行のピクセルの幅のこと
        // http://msdn.microsoft.com/ja-jp/library/system.drawing.imaging.bitmapdata.stride(v=vs.90).aspx
        /// </summary>
        private static int ReadStride(Bitmap image)
        {
            int stride;
            BitmapData imagedata = image.LockBits(new Rectangle(0, 0, image.Width, image.Height), ImageLockMode.ReadOnly, image.PixelFormat);

            stride = imagedata.Stride;
            image.UnlockBits(imagedata);
            return stride;
        }

        /// <summary>
        /// voxelに入力された三次元画像の縁を全てfalseにする
        /// </summary>
        public void ClearFrame()
        {
            int x, y, z;
            z = 0;
            for (y = 0; y < this.YSize; y++)
                for (x = 0; x < this.XSize; x++)
                    voxel[z, y, x] = false;

            z = ZSize - 1;
            for (y = 0; y < this.YSize; y++)
                for (x = 0; x < this.XSize; x++)
                    voxel[z, y, x] = false;

            y = 0;
            for (z = 0; z < this.ZSize; z++)
                for (x = 0; x < this.XSize; x++)
                    voxel[z, y, x] = false;

            y = YSize - 1;
            for (z = 0; z < this.ZSize; z++)
                for (x = 0; x < this.XSize; x++)
                    voxel[z, y, x] = false;

            x = 0;
            for (y = 0; y < this.YSize; y++)
                for (z = 0; z < this.ZSize; z++)
                    voxel[z, y, x] = false;

            x = XSize - 1;
            for (y = 0; y < this.YSize; y++)
                for (z = 0; z < this.ZSize; z++)
                    voxel[z, y, x] = false;

        }

        public void RemoveIsolatedPoint()
        {
            for (int z = 1; z < this.ZSize - 1; z++)
            {
                for (int y = 1; y < this.YSize - 1; y++)
                {
                    for (int x = 1; x < this.XSize - 1; x++)
                    {
                        if (this.voxel[z, y, x])
                        {
                            bool any = false;
                            for (int dx = -1; dx <= 1; dx++)
                            {
                                for (int dy = -1; dy <= 1; dy++)
                                {
                                    for (int dz = -1; dz <= 1; dz++)
                                    {
                                        if (dx == 0 && dy == 0 && dz == 0)
                                            continue;
                                        if (this.voxel[z + dz, y + dy, x + dx])
                                            any = true;
                                    }
                                }
                            }

                            if (any == false)
                            {
                                //注目ボクセルの26近傍成分に1つも有効なvoxelが存在しないなら、その点は孤立しているとみなす
                                this.voxel[z, y, x] = false;
                            }
                        }
                    }
                }

            }
        }
    }
}
