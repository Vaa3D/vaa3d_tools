#ifndef ZSVGGENERATOR_H
#define ZSVGGENERATOR_H

#include <string>
#include "zpoint.h"

class ZSvgGenerator
{
public:
    ZSvgGenerator();
    ZSvgGenerator(int leftTopX, int leftTopY, int width, int height);
    ~ZSvgGenerator();

    void setSize(int width, int height);

public:
    static std::string header();
    static std::string docType();
    static std::string tag(std::string name, std::string attribute,
                           bool start = true);
    static std::string tag(std::string name, bool start = true);

    std::string svgStartTag(std::string attribute = "") const;
    static std::string svgEndTag();

    static std::string textTag(double x, double y, std::string text,
                               std::string attribute="");
    static std::string rectangleTag(double x, double y,
        double width, double height, std::string attibute = "");

    static std::string circleTag(double cx, double cy, double r,
        std::string attribute = "");

    static std::string lineTag(double x1, double y1, double x2, double y2,
        std::string attribute = "");

    static std::string hyperLinkTag(std::string url,
                                    std::string attribute = "");
    static std::string hyperLinkTag();

    static std::string transformTag(std::string attribute);
    static std::string transformTag();
    static std::string translateAttribute(double x, double y);
    static std::string rotateAttribute(double angle, double x, double y);

public:
    bool write(const char *filePath, const std::string &svgBody) const;
    static const char *namedColor[];
    static int getNamedColorNumber();

private:
    static std::string m_xmlVersion;
    int m_viewBoxLeftTopX;
    int m_viewBoxLeftTopY;
    int m_viewBoxWidth;
    int m_viewBoxHeight;
    int m_width;
    int m_height;
};

#endif // ZSVGGENERATOR_H
