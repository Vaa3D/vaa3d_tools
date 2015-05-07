#ifndef QPIXMAP_TOOLTIP_H
#define QPIXMAP_TOOLTIP_H

#include "../control/CPlugin.h"
#include "../control/CImageUtils.h"

class teramanager::QPixmapToolTip : public QWidget
{
    Q_OBJECT

    private:

        QLabel* imageLabel;         // displays the image / pixmap
        QLabel* textLabel;          // displays text info
        uint8* rawData;             // image data buffer

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
            if (uniqueInstance == 0)
                uniqueInstance = new QPixmapToolTip(parent);
            return uniqueInstance;
        }
        static QPixmapToolTip* getInstance() {return uniqueInstance;}

        static void uninstance()
        {
            if(uniqueInstance)
            {
                delete uniqueInstance;
                uniqueInstance = 0;
            }
        }
        QPixmapToolTip(QWidget *parent = 0);

        // get and set methods
        uint8* raw(){return rawData;}
        void setImage(const QImage& img, uint8* raw_data=0)
        {
            imageLabel->setPixmap( QPixmap::fromImage(img) );
            rawData = raw_data;
            update();
        }
        void setText(const QString &text)
        {
            textLabel->setText(text);
            update();
        }

};

#endif // QPIXMAP_TOOLTIP_H
