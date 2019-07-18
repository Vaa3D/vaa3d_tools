#include "branch_detection_model.h"

Branch_Detection_Model::Branch_Detection_Model()
{
    p4DImage=NULL;
    thres_2d =45;
    window_size=3;
    length_point=2;
    ray_numbers_2d=128;
    x_coordinate=100,y_coordinate=100,z_coordinate=10;
    based_distance=3;
    block_size=3;
}

void Branch_Detection_Model::Get_Image_Date(V3DPluginCallback2 &callback1)
{
    //callback=&callback1;
    v3dhandle curwin=callback1.currentImageWindow();
    if(!curwin)
    {
        v3d_msg("No image is open.");
        return ;
    }
    p4DImage = callback1.getImage(curwin);
    datald = p4DImage->getRawData();

    sz[0] = p4DImage->getXDim();
    sz[1] = p4DImage->getYDim();
    sz[2] = p4DImage->getZDim();
    sz[3] = p4DImage->getCDim();
    size_2D_image=sz[0]*sz[1];
    size_3D_image=sz[0]*sz[1]*sz[2];


}
void Branch_Detection_Model::GUI_input_Thin_Method()
{
    if(p4DImage==NULL)
    {
                    v3d_msg("No image is open.");

    }


      //set update the dialog
      QDialog * dialog = new QDialog();


      if(p4DImage->getZDim() > 1)
              dialog->setWindowTitle("3D neuron image branch point detection Based on Ray-shooting algorithm");
      else
              dialog->setWindowTitle("2D neuron image branch point detection Based on Ray-shooting algorithm");

    QGridLayout * layout = new QGridLayout();


  QSpinBox * thres_2d_spinbox = new QSpinBox();
  thres_2d_spinbox->setRange(-1, 255);
  thres_2d_spinbox->setValue(thres_2d);

  QSpinBox * length_point_spinbox = new QSpinBox();
  length_point_spinbox->setRange(1, 255);
  length_point_spinbox->setValue(length_point);

  QSpinBox * window_size_spinbox = new QSpinBox();
  window_size_spinbox->setRange(1, 255);
  window_size_spinbox->setValue(window_size);

  layout->addWidget(new QLabel("intensity threshold"),0,0);
  layout->addWidget(thres_2d_spinbox,0,1,1,5);

  layout->addWidget(new QLabel(" length center points"),1,0);
  layout->addWidget(length_point_spinbox, 1,1,1,5);

  layout->addWidget(new QLabel(" window size of center points"),2,0);
  layout->addWidget(window_size_spinbox, 2,1,1,5);


  QHBoxLayout * hbox2 = new QHBoxLayout();
  QPushButton * ok = new QPushButton(" ok ");
  ok->setDefault(true);
  QPushButton * cancel = new QPushButton("cancel");
  hbox2->addWidget(cancel);
  hbox2->addWidget(ok);

  layout->addLayout(hbox2,6,0,1,6);
  dialog->setLayout(layout);
  QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
  QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

  //run the dialog

  if(dialog->exec() != QDialog::Accepted)
      {
              if (dialog)
              {
                      delete dialog;
                      dialog=0;
                      cout<<"delete dialog"<<endl;
              }

      }

  //get the dialog return values
   thres_2d = thres_2d_spinbox->value();
   window_size=window_size_spinbox->value();
   length_point=length_point_spinbox->value();

      if (dialog)
      {
              delete dialog;
              dialog=0;
              cout<<"delete dialog"<<endl;
      }
}

void Branch_Detection_Model::Gui_input_Rorate_Method()
{
    if(p4DImage==NULL)
    {
       v3d_msg("No image is open.");
    }


      //set update the dialog
      QDialog * dialog = new QDialog();


      if(p4DImage->getZDim() > 1)
              dialog->setWindowTitle("3D neuron image branch point detection Based on Ray-shooting algorithm");
      else
              dialog->setWindowTitle("2D neuron image branch point detection Based on Ray-shooting algorithm");
          QGridLayout * layout = new QGridLayout();

          QSpinBox * ray_numbers_2d_spinbox = new QSpinBox();
          ray_numbers_2d_spinbox->setRange(1,1000);
          ray_numbers_2d_spinbox->setValue(ray_numbers_2d);

          QSpinBox * thres_2d_spinbox = new QSpinBox();
          thres_2d_spinbox->setRange(-1, 255);
          thres_2d_spinbox->setValue(thres_2d);

          QSpinBox * window_size_spinbox = new QSpinBox();
          window_size_spinbox->setRange(1,10000);
          window_size_spinbox->setValue(window_size);

          QSpinBox * based_distance_spinbox = new QSpinBox();
          based_distance_spinbox->setRange(1,100);
          based_distance_spinbox->setValue(based_distance);

          QSpinBox * block_size_spinbox = new QSpinBox();
          block_size_spinbox->setRange(1,100);
          block_size_spinbox->setValue(block_size);



          layout->addWidget(new QLabel("ray numbers"),0,0);
          layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

          layout->addWidget(new QLabel("intensity threshold"),1,0);
          layout->addWidget(thres_2d_spinbox, 1,1,1,5);

          layout->addWidget(new QLabel("window size of detected center point"),3,0);
          layout->addWidget(window_size_spinbox, 3,1,1,5);

          layout->addWidget(new QLabel("based distance add to maximum radiu"),2,0);
          layout->addWidget(based_distance_spinbox, 2,1,1,5);

          layout->addWidget(new QLabel("block size"),4,0);
          layout->addWidget(block_size_spinbox, 4,1,1,5);

          QHBoxLayout * hbox2 = new QHBoxLayout();
          QPushButton * ok = new QPushButton(" ok ");
          ok->setDefault(true);
          QPushButton * cancel = new QPushButton("cancel");
          hbox2->addWidget(cancel);
          hbox2->addWidget(ok);

          layout->addLayout(hbox2,6,0,1,6);
          dialog->setLayout(layout);
          QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
          QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

          //run the dialog

          if(dialog->exec() != QDialog::Accepted)
              {
                      if (dialog)
                      {
                              delete dialog;
                              dialog=0;
                              cout<<"delete dialog"<<endl;
                      }

              }

          //get the dialog return values
          ray_numbers_2d = ray_numbers_2d_spinbox->value();
          thres_2d = thres_2d_spinbox->value();
          window_size=window_size_spinbox->value();
          based_distance=based_distance_spinbox->value();
          block_size=block_size_spinbox->value();
}

void Branch_Detection_Model::Gui_input_Rorate_display_Method()
{

    if(p4DImage==NULL)
    {
       v3d_msg("No image is open.");
    }

    //set update the dialog
    QDialog * dialog = new QDialog();


    if(p4DImage->getZDim() > 1)
            dialog->setWindowTitle("3D neuron image branch point detection Based on Ray-shooting algorithm");
    else
            dialog->setWindowTitle("2D neuron image branch point detection Based on Ray-shooting algorithm");
        QGridLayout * layout = new QGridLayout();

        QSpinBox * ray_numbers_2d_spinbox = new QSpinBox();
        ray_numbers_2d_spinbox->setRange(1,1000);
        ray_numbers_2d_spinbox->setValue(ray_numbers_2d);

        QSpinBox * thres_2d_spinbox = new QSpinBox();
        thres_2d_spinbox->setRange(-1, 255);
        thres_2d_spinbox->setValue(thres_2d);

        QSpinBox * x_location_spinbox = new QSpinBox();
        x_location_spinbox->setRange(1,10000);
        x_location_spinbox->setValue(x_coordinate);

        QSpinBox * y_location_spinbox = new QSpinBox();
        y_location_spinbox->setRange(1,10000);
        y_location_spinbox->setValue(y_coordinate);


        QSpinBox * based_distance_spinbox = new QSpinBox();
        based_distance_spinbox->setRange(1,100);
        based_distance_spinbox->setValue(based_distance);

        QSpinBox * block_size_spinbox = new QSpinBox();
        block_size_spinbox->setRange(1,100);
        block_size_spinbox->setValue(block_size);


        layout->addWidget(new QLabel("ray numbers"),0,0);
        layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

        layout->addWidget(new QLabel("intensity threshold"),1,0);
        layout->addWidget(thres_2d_spinbox, 1,1,1,5);

        layout->addWidget(new QLabel("x location"),2,0);
        layout->addWidget(x_location_spinbox, 2,1,1,5);

        layout->addWidget(new QLabel("y location"),3,0);
        layout->addWidget(y_location_spinbox, 3,1,1,5);

        layout->addWidget(new QLabel("based distance "),5,0);
        layout->addWidget(based_distance_spinbox, 5,1,1,5);

        layout->addWidget(new QLabel("block size "),4,0);
        layout->addWidget(block_size_spinbox, 4,1,1,5);


        QHBoxLayout * hbox2 = new QHBoxLayout();
        QPushButton * ok = new QPushButton(" ok ");
        ok->setDefault(true);
        QPushButton * cancel = new QPushButton("cancel");
        hbox2->addWidget(cancel);
        hbox2->addWidget(ok);

        layout->addLayout(hbox2,6,0,1,6);
        dialog->setLayout(layout);
        QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
        QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

        //run the dialog

        if(dialog->exec() != QDialog::Accepted)
            {
                    if (dialog)
                    {
                            delete dialog;
                            dialog=0;
                            cout<<"delete dialog"<<endl;
                    }

            }

        //get the dialog return values
        ray_numbers_2d = ray_numbers_2d_spinbox->value();
        thres_2d = thres_2d_spinbox->value();
        y_coordinate=y_location_spinbox->value();
        x_coordinate=x_location_spinbox->value();
        based_distance=based_distance_spinbox->value();
        block_size=block_size_spinbox->value();


        if (dialog)
            {
                    delete dialog;
                    dialog=0;
                    cout<<"delete dialog"<<endl;
            }

}

void Branch_Detection_Model::Gui_input_Rorate_3D_Method()
{
    if(p4DImage==NULL)
    {
       v3d_msg("No image is open.");
    }


      //set update the dialog
      QDialog * dialog = new QDialog();


      if(p4DImage->getZDim() > 1)
              dialog->setWindowTitle("3D neuron image branch point detection Based on Ray-shooting algorithm");
      else
              dialog->setWindowTitle("2D neuron image branch point detection Based on Ray-shooting algorithm");
          QGridLayout * layout = new QGridLayout();

          QSpinBox * ray_numbers_2d_spinbox = new QSpinBox();
          ray_numbers_2d_spinbox->setRange(1,1000);
          ray_numbers_2d_spinbox->setValue(ray_numbers_2d);

          QSpinBox * thres_2d_spinbox = new QSpinBox();
          thres_2d_spinbox->setRange(-1, 255);
          thres_2d_spinbox->setValue(thres_2d);

          QSpinBox * window_size_spinbox = new QSpinBox();
          window_size_spinbox->setRange(1,10000);
          window_size_spinbox->setValue(window_size);

          QSpinBox * based_distance_spinbox = new QSpinBox();
          based_distance_spinbox->setRange(1,100);
          based_distance_spinbox->setValue(based_distance);

          QSpinBox * number_layer_spinbox = new QSpinBox();
          number_layer_spinbox->setRange(1,100);
          number_layer_spinbox->setValue(num_layer);



          layout->addWidget(new QLabel("ray numbers"),0,0);
          layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

          layout->addWidget(new QLabel("intensity threshold"),1,0);
          layout->addWidget(thres_2d_spinbox, 1,1,1,5);

          layout->addWidget(new QLabel("window size of detected center point"),2,0);
          layout->addWidget(window_size_spinbox, 2,1,1,5);

          layout->addWidget(new QLabel("based distance add to maximum radiu"),3,0);
          layout->addWidget(based_distance_spinbox, 3,1,1,5);

          layout->addWidget(new QLabel("number layer"),4,0);
          layout->addWidget(number_layer_spinbox, 4,1,1,5);


          QHBoxLayout * hbox2 = new QHBoxLayout();
          QPushButton * ok = new QPushButton(" ok ");
          ok->setDefault(true);
          QPushButton * cancel = new QPushButton("cancel");
          hbox2->addWidget(cancel);
          hbox2->addWidget(ok);

          layout->addLayout(hbox2,6,0,1,6);
          dialog->setLayout(layout);
          QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
          QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

          //run the dialog

          if(dialog->exec() != QDialog::Accepted)
              {
                      if (dialog)
                      {
                              delete dialog;
                              dialog=0;
                              cout<<"delete dialog"<<endl;
                      }

              }

          //get the dialog return values
          ray_numbers_2d = ray_numbers_2d_spinbox->value();
          thres_2d = thres_2d_spinbox->value();
          window_size=window_size_spinbox->value();
          based_distance=based_distance_spinbox->value();
          num_layer=number_layer_spinbox->value();

}

 Branch_Detection_Model::~Branch_Detection_Model()
{
    //if(datald) {delete []datald;datald=0;}
}
