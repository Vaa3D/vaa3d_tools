#ifndef QPIXMAP_TOOLTIP_H
#define QPIXMAP_TOOLTIP_H

#include "../control/CPlugin.h"

class teramanager::QPixmapToolTip : public QWidget
{
    Q_OBJECT

    private:

        QPixmap pix;            // image/pixmap to be displayed

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static QPixmapToolTip* uniqueInstance;

    protected:

        void paintEvent(QPaintEvent *);

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

        void setPixmap(const QPixmap& pixmap){pix = pixmap;}
};

#endif // QPIXMAP_TOOLTIP_H
