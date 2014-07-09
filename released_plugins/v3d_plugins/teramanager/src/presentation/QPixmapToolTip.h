#ifndef QPIXMAP_TOOLTIP_H
#define QPIXMAP_TOOLTIP_H

#include "../control/CPlugin.h"

class teramanager::QPixmapToolTip : public QWidget
{
    Q_OBJECT

    private:

        QPixmap pix;            // image/pixmap to be displayed
        uint8 *rawData;
        QLabel* imageLabel;
        QLabel* textLabel;

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static QPixmapToolTip* uniqueInstance;


    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "instance(...)"
        **********************************************************************************/
        static QPixmapToolTip* instance(QWidget *parent=0)
        {
            /**/itm::debug(itm::LEV_MAX, 0, __itm__current__function__);

            if (uniqueInstance == 0)
                uniqueInstance = new QPixmapToolTip(parent);
            return uniqueInstance;
        }
        static void uninstance()
        {
            if(uniqueInstance)
            {
                delete uniqueInstance;
                uniqueInstance = 0;
            }
        }
        QPixmapToolTip(QWidget *parent = 0);

        void setPixmap(const QPixmap& pixmap, uint8* raw_data=0){pix = pixmap; imageLabel->setPixmap(pix); rawData = raw_data; update();}
        void setText(const QString &text){textLabel->setText(text);}        
        QPixmap& pixmap(){return pix;}
        uint8* raw(){return rawData;}
};

#endif // QPIXMAP_TOOLTIP_H
