#ifndef COLORLISTS_H
#define COLORLISTS_H

#include <vector>
#include <QColor>
#include <QIcon>

// main color list class
struct ColorListBase
{
private:
    std::vector<QColor> colorList;
    std::vector<QIcon> iconList;
    const int iconSize;

    inline QPixmap createIcon( const QColor &c ) {
        QPixmap icon( iconSize, iconSize );
        icon.fill( c );

        return icon;
    }

    inline void updateIcon( unsigned int idx ) {
        iconList.at(idx) = createIcon( colorList.at(idx) );
    }

public:
    ColorListBase() : iconSize(32) { }

    inline void addColor( const QColor &c ) {
        colorList.push_back( c );
        iconList.push_back( createIcon(c) );
    }

    inline void replaceColor( unsigned int idx, const QColor &c )
    {
        colorList.at(idx) = c;
        updateIcon(idx);
    }

    inline const QColor& getColor(unsigned int idx) const {
        return colorList.at(idx);
    }

    inline const QIcon& getIcon(unsigned int idx) const {
        return iconList.at(idx);
    }

    inline unsigned int count() const { return colorList.size(); }
};

struct LabelColorList : public ColorListBase
{
public:
    LabelColorList() : ColorListBase()
    {
        addColor( QColor(Qt::magenta) );
        addColor( QColor(0x8A, 0x00, 0xF5) );
        addColor( QColor(Qt::blue) );
    }
};

struct OverlayColorList : public ColorListBase
{
    OverlayColorList() : ColorListBase()
    {
        addColor( QColor(0xFF, 0xC2, 0x0B) );
        addColor( QColor(0xFF, 0x0B, 0xFF) );
        addColor( QColor(0xFF, 0xFF, 0x00) );
        addColor( QColor(0x0B, 0xFF, 0xC2) );
        addColor( QColor(0xC2, 0xFF, 0xFF) );
    }
};

class SelectionColor : public QColor
{
public:
    SelectionColor() : QColor(Qt::green) { }
};

class ScoreColor : public QColor
{
public:
    ScoreColor() : QColor(0xFF, 0x00, 0x00) { }
};


#endif // COLORLISTS_H
