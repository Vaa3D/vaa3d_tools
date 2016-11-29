
#ifndef __VN_RIVULET_H__
#define __VN_RIVULET_H__

struct PARA_RIVULET {
  unsigned char threshold;
  V3DLONG channel;

  QString inimg_file, inmarker_file, outswc_file;

  PARA_RIVULET() {
    threshold = 0;
    inimg_file = "";
    inmarker_file = "";
    outswc_file = "";
  }

  bool rivulet_dialog() {
    // fetch parameters from dialog
    {
      // set update the dialog
      QDialog *dialog = new QDialog();
      dialog->setWindowTitle("Rivulet -- Neuron Tracing");
      QGridLayout *layout = new QGridLayout();

      QSpinBox *channel_spinbox = new QSpinBox();
      channel_spinbox->setRange(1, 3);
      channel_spinbox->setValue(1);

      QSpinBox *bkgthresh_spinbox = new QSpinBox();
      bkgthresh_spinbox->setRange(0, 255);
      bkgthresh_spinbox->setValue(10);

      layout->addWidget(new QLabel("color channel"), 0, 0);
      layout->addWidget(channel_spinbox, 0, 1, 1, 5);
      layout->addWidget(new QLabel("background"), 1, 0);
      layout->addWidget(bkgthresh_spinbox, 1, 1, 1, 5);


      QHBoxLayout *hbox3 = new QHBoxLayout();
      QPushButton *ok = new QPushButton(" ok ");
      ok->setDefault(true);
      QPushButton *cancel = new QPushButton("cancel");
      cancel->setDefault(false);
      hbox3->addWidget(ok);
      hbox3->addWidget(cancel);

      layout->addLayout(hbox3, 9, 0, 1, 6);
      dialog->setLayout(layout);
      QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
      QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

      // Run the dialog

      if (dialog->exec() != QDialog::Accepted)
        return false;

      // Get the dialog return values
      channel = channel_spinbox->value() - 1;
      threshold = bkgthresh_spinbox->value();

      if (dialog) {
        delete dialog;
        dialog = 0;
      }
    }

    return true;
  }
};

#endif
