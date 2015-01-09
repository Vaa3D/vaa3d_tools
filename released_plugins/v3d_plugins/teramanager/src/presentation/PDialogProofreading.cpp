#include "PDialogProofreading.h"
#include "PMain.h"
#include "../control/CVolume.h"

std::vector<itm::block_t> itm::PDialogProofreading::blocks;
int itm::PDialogProofreading::blocks_res=0;

/*********************************************************************************
* Singleton design pattern: this class can have one instance only,  which must be
* instantiated by calling static method "istance(...)"
**********************************************************************************/
itm::PDialogProofreading* itm::PDialogProofreading::uniqueInstance = 0;
void itm::PDialogProofreading::uninstance()
{
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = 0;
    }
}

itm::PDialogProofreading::PDialogProofreading(itm::PMain *_parent) : QWidget(0)
{
    /**/itm::debug(itm::LEV3, 0, __itm__current__function__);

    parent = _parent;

    QFont tinyFont = QApplication::font();
    #ifndef _USE_NATIVE_FONTS
    tinyFont.setPointSize(9);
    #endif
    QFont bigFont = QApplication::font();
    bigFont.setPointSize(12);
    this->setFont(tinyFont);

    resolution_cbox = new QComboBox();
    for(int i=0; i<parent->resolution_cbox->count(); i++)
        resolution_cbox->insertItem(i, parent->resolution_cbox->itemText(i));
    resolution_cbox->setCurrentIndex(resolution_cbox->count()-1);
    resolution_cbox->setEditable(true);
    resolution_cbox->lineEdit()->setAlignment(Qt::AlignHCenter);
    for (int i = 0; i < resolution_cbox->count(); ++i)
        resolution_cbox->setItemData(i, Qt::AlignHCenter, Qt::TextAlignmentRole);

    scan_method_cbox = new QComboBox();
    scan_method_cbox->addItem("X -> Y -> Z");
    scan_method_cbox->addItem("Y -> X -> Z");
    scan_method_cbox->addItem("X -> Z -> Y");
    scan_method_cbox->addItem("Y -> Z -> X");
    scan_method_cbox->addItem("Z -> Y -> X");
    scan_method_cbox->addItem("Z -> X -> Y");
    PMain::setEnabledComboBoxItem(scan_method_cbox, 1, false);
    PMain::setEnabledComboBoxItem(scan_method_cbox, 2, false);
    PMain::setEnabledComboBoxItem(scan_method_cbox, 3, false);
    PMain::setEnabledComboBoxItem(scan_method_cbox, 4, false);
    PMain::setEnabledComboBoxItem(scan_method_cbox, 5, false);
    scan_method_cbox->setEditable(true);
    scan_method_cbox->lineEdit()->setAlignment(Qt::AlignHCenter);
    for (int i = 0; i < scan_method_cbox->count(); ++i)
        scan_method_cbox->setItemData(i, Qt::AlignHCenter, Qt::TextAlignmentRole);

    overlap_sbox = new QSpinBox();
    overlap_sbox->setAlignment(Qt::AlignCenter);
    overlap_sbox->setSuffix("\%");
    overlap_sbox->setPrefix("overlap ");
    overlap_sbox->setValue(20);
    overlap_sbox->setMinimum(1);
    overlap_sbox->setMaximum(50);

    perblock_time_sbox = new QSpinBox();
    perblock_time_sbox->setAlignment(Qt::AlignCenter);
    perblock_time_sbox->setSuffix(" (seconds)");
    perblock_time_sbox->setMinimum(1);
    perblock_time_sbox->setMaximum(10000);
    perblock_time_sbox->setValue(100);

    VOI_field = new QLineEdit();
    VOI_field->setReadOnly(true);
    VOI_field->setAlignment(Qt::AlignCenter);
    VOI_field->setStyleSheet("QLineEdit {background: rgb(246,250,205);}");
    VOI_field->setFont(tinyFont);
    VOI_field->installEventFilter(this);

    blocks_size_field = new QLineEdit();
    blocks_size_field->setReadOnly(true);
    blocks_size_field->setAlignment(Qt::AlignCenter);
    blocks_size_field->setStyleSheet("QLineEdit {background: rgb(246,250,205);}");
    blocks_size_field->setFont(tinyFont);
    blocks_size_field->installEventFilter(this);

    coverage_field = new QLineEdit();
    coverage_field->setReadOnly(true);
    coverage_field->setAlignment(Qt::AlignCenter);
    coverage_field->setStyleSheet("QLineEdit {background: rgb(246,250,205);}");
    coverage_field->setFont(tinyFont);

    blocks_field = new QLineEdit();
    blocks_field->setReadOnly(true);
    blocks_field->setAlignment(Qt::AlignCenter);
    blocks_field->setStyleSheet("QLineEdit {background: rgb(246,250,205);}");
    blocks_field->setFont(tinyFont);

    est_time_field = new QLineEdit();
    est_time_field->setReadOnly(true);
    est_time_field->setAlignment(Qt::AlignCenter);
    est_time_field->setStyleSheet("QLineEdit {background: rgb(246,250,205);}");
    est_time_field->setFont(tinyFont);

    show_blocks = new QPushButton("Show blocks ->");
    connect(show_blocks, SIGNAL(clicked()), this, SLOT(showBlocksButtonClicked()));

    blocks_text = new QTextEdit();
    blocks_text->setReadOnly(true);
    blocks_text->setFixedHeight(100);
    QFont font = QFont("Courier New", 8);
    blocks_text->setFont(font);
    blocks_text->setWordWrapMode(QTextOption::NoWrap);
    blocks_text->setText("Click here to update");

    start_button = new QPushButton(" Start");
    start_button->setIcon(QIcon(":/icons/start.png"));
    start_button->setIconSize(QSize(25,25));
    start_button->setFixedHeight(35);
    start_button->setFont(bigFont);

    cancel_button = new QPushButton(" Cancel");
    cancel_button->setIcon(QIcon(":/icons/stop.png"));
    cancel_button->setIconSize(QSize(25,25));
    cancel_button->setFixedHeight(35);
    cancel_button->setFont(bigFont);

    connect(perblock_time_sbox, SIGNAL(valueChanged(int)), this, SLOT(updateBlocks(int)));
    connect(resolution_cbox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateBlocks(int)));
    connect(overlap_sbox, SIGNAL(valueChanged(int)), this, SLOT(updateBlocks(int)));
    connect(start_button, SIGNAL(clicked()), this, SLOT(startButtonClicked()));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(close()));


    //****LAYOUT SECTIONS****
    QVBoxLayout *layout = new QVBoxLayout();
    // center layout
    QGridLayout *center_layout = new QGridLayout();
    QLabel *resolution_label = new QLabel("Resolution:");
    resolution_label->setFixedWidth(120);
    center_layout->setSpacing(10);
    center_layout->addWidget(new QLabel("VOI:"),                0, 0, 1, 1);
    center_layout->addWidget(VOI_field,                         0, 1, 1, 1);
    center_layout->addWidget(new QLabel("Block size:"),         1, 0, 1, 1);
    center_layout->addWidget(blocks_size_field,                 1, 1, 1, 1);
    center_layout->addWidget(resolution_label,                  2, 0, 1, 1);
    center_layout->addWidget(resolution_cbox,                   2, 1, 1, 1);
    center_layout->addWidget(new QLabel("Scan pattern:"),       3, 0, 1, 1);
    center_layout->addWidget(scan_method_cbox,                  3, 1, 1, 1);
    center_layout->addWidget(new QLabel("Block overlap:"),      4, 0, 1, 1);
    center_layout->addWidget(overlap_sbox,                      4, 1, 1, 1);
    center_layout->addWidget(new QLabel("Per-block time:"),     5, 0, 1, 1);
    center_layout->addWidget(perblock_time_sbox,                5, 1, 1, 1);
    center_layout->addWidget(new QLabel("Volume coverage:"),    6, 0, 1, 1);
    center_layout->addWidget(coverage_field,                    6, 1, 1, 1);
    center_layout->addWidget(new QLabel("No. of blocks:"),      7, 0, 1, 1);
    center_layout->addWidget(blocks_field,                      7, 1, 1, 1);
    center_layout->addWidget(new QLabel("Estimated time:"),     8, 0, 1, 1);
    center_layout->addWidget(est_time_field,                    8, 1, 1, 1);
    center_layout->addWidget(show_blocks,                       9, 0, 1, 1);
    center_layout->addWidget(blocks_text,                       9, 1, 1, 1);
    // south layout
    QHBoxLayout* buttons_layout = new QHBoxLayout();
    buttons_layout->addWidget(start_button);
    buttons_layout->addWidget(cancel_button);
    // GLOBAL layout
    QLabel *instructionsLabel = new QLabel("TeraFly will guide you through a complete semi-automated scan of the selected volume of interest (VOI). Use the settings below to tailor the trade-off between image resolution and scan duration.");
    instructionsLabel->setWordWrap(true);
    //instructionsLabel->setFont(QApplication::font());
    layout->addWidget(instructionsLabel);
    layout->addSpacing(20);
    layout->addLayout(center_layout);
    layout->addSpacing(20);
    layout->addLayout(buttons_layout);
    layout->setContentsMargins(10,10,10,10);
    layout->setSizeConstraint( QLayout::SetFixedSize );
    setLayout(layout);

    setWindowTitle("Proofreading mode");
    this->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);
    setMinimumWidth(600);
    move(QApplication::desktop()->screen()->rect().center() - rect().center());

    updateBlocks(0);
}

/**********************************************************************************
* Called when <start_button> emits <click()> signal.
***********************************************************************************/
void itm::PDialogProofreading::startButtonClicked()
{
    /**/itm::debug(itm::LEV3, 0, __itm__current__function__);

    blocks_res = resolution_cbox->currentIndex();
    parent->PRstart();
    close();
}


//overrides closeEvent method of QWidget
void itm::PDialogProofreading::closeEvent(QCloseEvent *evt)
{
    /**/itm::debug(itm::LEV3, 0, __itm__current__function__);

    if(evt)
    {
        evt->accept();
        PDialogProofreading::uninstance();
    }
}

/**********************************************************************************
* Called when <show_blocks_button> emits <click()> signal.
***********************************************************************************/
void itm::PDialogProofreading::showBlocksButtonClicked()
{
    setCursor(Qt::WaitCursor);
    blocks_text->clear();
    for(int i=0; i<blocks.size(); i++)
        blocks_text->append(itm::strprintf("block %d: X=[%d,%d], Y=[%d,%d], Z=[%d,%d]", i, blocks[i].xInt.start+1, blocks[i].xInt.end,
                                           blocks[i].yInt.start+1, blocks[i].yInt.end, blocks[i].zInt.start+1, blocks[i].zInt.end).c_str());
    blocks_text->moveCursor (QTextCursor::Start) ;
    blocks_text->ensureCursorVisible();
    setCursor(Qt::ArrowCursor);
}

// re-computes blocks and updates GUI
void itm::PDialogProofreading::updateBlocks(int)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    CViewer* curWin = CViewer::getCurrent();
    if(curWin)
    {
        // get VOI coordinates in the selected resolution
        int VOIxs_cr = CVolume::scaleHCoord(parent->H0_sbox->value()-1, CImport::instance()->getResolutions()-1, resolution_cbox->currentIndex());
        int VOIxe_cr = CVolume::scaleHCoord(parent->H1_sbox->value()-1, CImport::instance()->getResolutions()-1, resolution_cbox->currentIndex());
        int VOIys_cr = CVolume::scaleVCoord(parent->V0_sbox->value()-1, CImport::instance()->getResolutions()-1, resolution_cbox->currentIndex());
        int VOIye_cr = CVolume::scaleVCoord(parent->V1_sbox->value()-1, CImport::instance()->getResolutions()-1, resolution_cbox->currentIndex());
        int VOIzs_cr = CVolume::scaleDCoord(parent->D0_sbox->value()-1, CImport::instance()->getResolutions()-1, resolution_cbox->currentIndex());
        int VOIze_cr = CVolume::scaleDCoord(parent->D1_sbox->value()-1, CImport::instance()->getResolutions()-1, resolution_cbox->currentIndex());

//        printf("VOI is X=[%d,%d], Y=[%d,%d], Z=[%d,%d]\n", VOIxs_cr, VOIxe_cr, VOIys_cr, VOIye_cr, VOIzs_cr, VOIze_cr);
        int dimX   = VOIxe_cr-VOIxs_cr+1;
        int dimY   = VOIye_cr-VOIys_cr+1;
        int dimZ   = VOIze_cr-VOIzs_cr+1;
        int blockX = parent->Hdim_sbox->value();
        int blockY = parent->Vdim_sbox->value();
        int blockZ = parent->Ddim_sbox->value();
        int ovlX  = ( overlap_sbox->value()/100.0f )*parent->Hdim_sbox->value();
        int ovlY  = ( overlap_sbox->value()/100.0f )*parent->Vdim_sbox->value();
        int ovlZ  = ( overlap_sbox->value()/100.0f )*parent->Ddim_sbox->value();
        int tolerance = 30;

        // generate X intervals
        vector<itm::interval_t> xInts;
        int count = 0;
        while(xInts.empty() || xInts.back().end < dimX)
        {
            xInts.push_back(interval_t(count, count+blockX));
            count += blockX-ovlX;
        }
        xInts.back().end = dimX;
        xInts.back().start = std::max(0,dimX-blockX);
        if(xInts.size() > 1 &&                                                  // more than one segment
           xInts[xInts.size()-1].end - xInts[xInts.size()-2].end < tolerance)   // last two segments difference is below tolerance
        {
            xInts.pop_back();           // remove last segment
            xInts.back().end = dimX;    // extend last segment up to the end
        }

        // generate Y intervals
        vector<itm::interval_t> yInts;
        count = 0;
        while(yInts.empty() || yInts.back().end < dimY)
        {
            yInts.push_back(interval_t(count, count+blockY));
            count += blockY-ovlY;
        }
        yInts.back().end = dimY;
        yInts.back().start = std::max(0,dimY-blockY);
        if(yInts.size() > 1 &&                                                  // more than one segment
           yInts[yInts.size()-1].end - yInts[yInts.size()-2].end < tolerance)   // last two segments difference is below tolerance
        {
            yInts.pop_back();           // remove last segment
            yInts.back().end = dimY;    // extend last segment up to the end
        }

        // generate Z intervals
        vector<itm::interval_t> zInts;
        count = 0;
        while(zInts.empty() || zInts.back().end < dimZ)
        {
            zInts.push_back(interval_t(count, count+blockZ));
            count += blockZ-ovlZ;
        }
        zInts.back().end = dimZ;
        zInts.back().start = std::max(0,dimZ-blockZ);
        if(zInts.size() > 1 &&                                                  // more than one segment
           zInts[zInts.size()-1].end - zInts[zInts.size()-2].end < tolerance)   // last two segments difference is below tolerance
        {
            zInts.pop_back();           // remove last segment
            zInts.back().end = dimZ;    // extend last segment up to the end
        }

        // apply offset
        for(int i=0; i<xInts.size(); i++)
        {
            xInts[i].start += VOIxs_cr;
            xInts[i].end   += VOIxs_cr;
        }
        for(int i=0; i<yInts.size(); i++)
        {
            yInts[i].start += VOIys_cr;
            yInts[i].end   += VOIys_cr;
        }
        for(int i=0; i<zInts.size(); i++)
        {
            zInts[i].start += VOIzs_cr;
            zInts[i].end   += VOIzs_cr;
        }

        // generate 3D blocks
        blocks.clear();
        for(int z=0; z<zInts.size(); z++)
            for(int y=0; y<yInts.size(); y++)
                for(int x=0; x<xInts.size(); x++)
                    blocks.push_back(block_t(xInts[x], yInts[y], zInts[z]));

        // update GUI elements
        blocks_size_field->setText(itm::strprintf("%d (X) x %d (Y) x %d (Z)", parent->Hdim_sbox->value(), parent->Vdim_sbox->value(), parent->Ddim_sbox->value()).c_str());
        VOI_field->setText(itm::strprintf("[%d, %d] (X) x [%d, %d] (Y) x [%d, %d] (Z)", parent->H0_sbox->value(), parent->H1_sbox->value(),
                                          parent->V0_sbox->value(), parent->V1_sbox->value(), parent->D0_sbox->value(), parent->D1_sbox->value()).c_str());
        blocks_field->setText(QString::number(blocks.size()));
        itm::uint64 cur_res_size = (uint64)1*
                                   CImport::instance()->getVolume(resolution_cbox->currentIndex())->getDIM_V() *
                                   CImport::instance()->getVolume(resolution_cbox->currentIndex())->getDIM_H() *
                                   CImport::instance()->getVolume(resolution_cbox->currentIndex())->getDIM_D();
        itm::uint64 cur_sel_size = (uint64)1 * dimX * dimY * dimZ;
        coverage_field->setText( (itm::strprintf("%.3f", (cur_sel_size*100.0/cur_res_size)) + "\%").c_str());
        if(blocks.size() < 2)
        {
            start_button->setEnabled(false);
            blocks_field->setStyleSheet("QLineEdit {background: rgb(240,144,161);}");
        }
        else
        {
            start_button->setEnabled(true);
            blocks_field->setStyleSheet("QLineEdit {background: rgb(246,250,205);}");
        }
        int est_time_minutes = ((perblock_time_sbox->value()*blocks.size())%3600)/60;
        int est_time_hours   = (perblock_time_sbox->value()*blocks.size())/3600;
        est_time_field->setText(itm::strprintf("%d hours and %d minutes", est_time_hours, est_time_minutes).c_str());

    }
}

/**********************************************************************************
* For mouse-enter tooltips
***********************************************************************************/
bool itm::PDialogProofreading::eventFilter(QObject *obj, QEvent *evt)
{
    if(obj == VOI_field && evt->type()==QEvent::Enter)
    {
        QPoint gpos = VOI_field->mapToGlobal(QPoint(0,10));
        QToolTip::showText(gpos, "To modify this, use Vaa3D volume cut scrollbars", VOI_field);
    }
    else if(obj == blocks_size_field && evt->type()==QEvent::Enter)
    {
        QPoint gpos = blocks_size_field->mapToGlobal(QPoint(0,10));
        QToolTip::showText(gpos, "To modify this, use spinboxes in TeraFly->Viewer->Max dims", blocks_size_field);
    }
    return false;
}
