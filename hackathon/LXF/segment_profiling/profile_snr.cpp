
#include "profile_snr.h"


bool profile_swc(V3DPluginCallback2 &callback,PARA &PA)
{
    Image4DSimple * img = new Image4DSimple;;
    float dilate_ratio = 3;
    int flip = 0;
    int invert = 0;
    float cut_off_ratio = 0.05;
    QString output_csv_file = PA.img_name + QString(".csv");
    img->setData(PA.data1d,PA.im_cropped_sz[0],PA.im_cropped_sz[1],PA.im_cropped_sz[2],PA.im_cropped_sz[3],V3D_UINT8);
    QList<IMAGE_METRICS> result_metrics = intensity_profile(PA.nt_meanshift, img, dilate_ratio,flip,invert,cut_off_ratio,callback);
    if (result_metrics.isEmpty())
    {
        cout<<"Error in intensity_profile() !"<<endl;
        return false;
    }


    //output
    writeMetrics2CSV(result_metrics, output_csv_file);
//    IMAGE_METRICS m_stats = result_metrics[0];


//    QString disp_text = "";
//    disp_text += "Contrast-to-Background Ratio = " + QString::number(m_stats.cnr) + ";\n";
//    disp_text += "Dynamic Range = " + QString::number(m_stats.dy) + ";\n";
//    disp_text += "Mean FG Intensity = " + QString::number (m_stats.fg_mean)  + ", STD = "    + QString::number(m_stats.fg_std) + ";\n";
//    disp_text += "Mean BG Intensity = " + QString::number (m_stats.bg_mean)  + ", STD = "    + QString::number(m_stats.bg_std) + ";\n";
//    disp_text += "Mean Tubularity = " + QString::number(m_stats.tubularity_mean) + ", STD = " + QString::number(m_stats.tubularity_std) + ".\n";


//    disp_text += "\n Segment type-specific screening metrics are exported in: \n "+ output_csv_file +"\n";
//    v3d_msg(disp_text);
}
