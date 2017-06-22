
#ifndef __VN_RIVULET_H__
#define __VN_RIVULET_H__

struct PARA_RIVULET {
  unsigned char threshold;
  V3DLONG channel=1;
  bool quality;
  bool prune;
  unsigned char iteration;

  QString inimg_file, inmarker_file, outswc_file;

  PARA_RIVULET() {
    threshold = 0;
    inimg_file = "";
    inmarker_file = "";
    outswc_file = "";
    quality=0;
    prune=0;
    iteration=5;
  }

  bool rivulet_dialog() {
    // fetch parameters from dialog
    {
      // set update the dialog
      QDialog *dialog = new QDialog();
      dialog->setWindowTitle("Soma Detection -- Nanjing hackathon");
      QGridLayout *layout = new QGridLayout();

      QSpinBox *channel_spinbox = new QSpinBox();
      channel_spinbox->setRange(1, 3);
      channel_spinbox->setValue(1);

      QSpinBox *bkgthresh_spinbox = new QSpinBox();
      bkgthresh_spinbox->setRange(0, 255);
      bkgthresh_spinbox->setValue(10);


      QSpinBox *itr_spinbox = new QSpinBox();
      itr_spinbox->setRange(0, 40);
      itr_spinbox->setValue(5);

      QCheckBox * quality_checker = new QCheckBox();
      quality_checker->setChecked(false);

      QCheckBox * prune_checker = new QCheckBox();
      prune_checker->setChecked(false);

      // layout->addWidget(new QLabel("color channel"), 0, 0);
      // layout->addWidget(channel_spinbox, 0, 1, 1, 5);
      const char* helper = "Instruction:\n"
                           "background: The background threshold is needed to segment the image.\n";
                           cout<<endl;
                           "iteration: The number of iteration is needed for the somatic evolution.\n";
                           // "Tick prune to remove the short and unconnected branches.\n";
      layout->addWidget(new QLabel(helper), 0, 0, 1, 8);
      layout->addWidget(new QLabel("background:"), 1, 0);
      layout->addWidget(bkgthresh_spinbox, 1, 1, 1, 1);
      layout->addWidget(new QLabel("iteration:"), 1, 3);
      layout->addWidget(itr_spinbox, 1, 4, 1, 1);
      // layout->addWidget(quality_checker, 1, 4, 1, 1);
      // layout->addWidget(new QLabel("prune:"), 1, 6);
      // layout->addWidget(prune_checker, 1, 7, 1, 1);

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
      iteration = itr_spinbox->value();
      cout<<"test : The number of iteration is "<<(int) iteration<<endl;

      if (dialog) {
        delete dialog;
        dialog = 0;
      }
    }

    return true;
  }
};

#endif
