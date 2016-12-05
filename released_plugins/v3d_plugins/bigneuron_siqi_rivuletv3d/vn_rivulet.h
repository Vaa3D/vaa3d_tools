
#ifndef __VN_RIVULET_H__
#define __VN_RIVULET_H__

struct PARA_RIVULET {
  unsigned char threshold;
  V3DLONG channel=1;
  bool quality;
  bool prune;

  QString inimg_file, inmarker_file, outswc_file;

  PARA_RIVULET() {
    threshold = 0;
    inimg_file = "";
    inmarker_file = "";
    outswc_file = "";
    quality=0;
    prune=0;
  }

  bool rivulet_dialog() {
    // fetch parameters from dialog
    {
      // set update the dialog
      QDialog *dialog = new QDialog();
      dialog->setWindowTitle("Rivulet2 -- Auto 3D Single Neuron Tracing");
      QGridLayout *layout = new QGridLayout();

      QSpinBox *channel_spinbox = new QSpinBox();
      channel_spinbox->setRange(1, 3);
      channel_spinbox->setValue(1);

      QSpinBox *bkgthresh_spinbox = new QSpinBox();
      bkgthresh_spinbox->setRange(0, 255);
      bkgthresh_spinbox->setValue(10);

      bkgthresh_spinbox->setRange(0, 255);
      bkgthresh_spinbox->setValue(10);

      QCheckBox * quality_checker = new QCheckBox();
      quality_checker->setChecked(false);

      QCheckBox * prune_checker = new QCheckBox();
      prune_checker->setChecked(false);

      // layout->addWidget(new QLabel("color channel"), 0, 0);
      // layout->addWidget(channel_spinbox, 0, 1, 1, 5);
      const char* helper = "HINTS:\n"
                           "The background threshold is needed to segment the image.\n"
                           "Tick quality for better tracing quality with slightly longer running time (worth it though).\n"
                           "Tick prune to remove the short and unconnected branches.\n";
      layout->addWidget(new QLabel(helper), 0, 0, 1, 8);
      layout->addWidget(new QLabel("background:"), 1, 0);
      layout->addWidget(bkgthresh_spinbox, 1, 1, 1, 1);
      layout->addWidget(new QLabel("quality:"), 1, 3);
      layout->addWidget(quality_checker, 1, 4, 1, 1);
      layout->addWidget(new QLabel("prune:"), 1, 6);
      layout->addWidget(prune_checker, 1, 7, 1, 1);

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
      threshold = bkgthresh_spinbox->value();
      quality = quality_checker->isChecked();

      if (dialog) {
        delete dialog;
        dialog = 0;
      }
    }

    return true;
  }
};

#endif
