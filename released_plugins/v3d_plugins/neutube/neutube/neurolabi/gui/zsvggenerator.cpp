#include "zsvggenerator.h"

#include <sstream>
#include <fstream>

using namespace std;

std::string ZSvgGenerator::m_xmlVersion = "1.0";

ZSvgGenerator::ZSvgGenerator()
{
  m_viewBoxLeftTopX = 0;
  m_viewBoxLeftTopY = 0;
  m_viewBoxWidth = 800;
  m_viewBoxHeight = 600;
  m_width = 800;
  m_height = 600;
}

ZSvgGenerator::ZSvgGenerator(int leftTopX, int leftTopY, int width, int height)

{
  m_viewBoxLeftTopX = leftTopX;
  m_viewBoxLeftTopY = leftTopY;
  m_viewBoxWidth = width;
  m_viewBoxHeight = height;
  m_width = width;
  m_height = height;
}

ZSvgGenerator::~ZSvgGenerator(void)
{
}

void ZSvgGenerator::setSize(int width, int height)
{
  m_width = width;
  m_height = height;
}

std::string ZSvgGenerator::header()
{
  ostringstream output;

  output << "<?xml version=\"" << m_xmlVersion << "\" standalone=\"no\"?>";

  return output.str();
}

std::string ZSvgGenerator::docType()
{
  ostringstream output;

  output << "<!DOCTYPE svg PUBLIC" <<
            " \"-//W3C//DTD SVG 1.0//EN\" " <<
            "\"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\""
         << ">";

  return output.str();
}

string ZSvgGenerator::tag(string name, string attribute, bool start)
{
  ostringstream output;
  output << "</" << name;
  if (start) {
    output << " " << attribute << ">";
  } else {
    output << ">";
  }

  return output.str();
}

string ZSvgGenerator::tag(string name, bool start)
{
    return tag(name, "", start);
}

string ZSvgGenerator::svgStartTag(std::string attribute) const
{
    ostringstream output;

    output << "<svg " << "width=\"" << m_width << "\" " << "height=\""
           << m_height << "\" "
           << "viewBox=\"" <<
        m_viewBoxLeftTopX << " " << m_viewBoxLeftTopY << " " <<
        m_viewBoxWidth << " " << m_viewBoxHeight << "\" " <<
        attribute << " xmlns=\"http://www.w3.org/2000/svg\" "
              "xmlns:xlink=\"http://www.w3.org/1999/xlink\">";

    return output.str();
}

string ZSvgGenerator::svgEndTag()
{
    return "</svg>";
}

string ZSvgGenerator::textTag(double x, double y, string text, string attribute)
{
    ostringstream output;

    output << "<text x=\"" << x << "\" y=\"" << y << "\" " << attribute <<">"
              << endl;
    output << text << endl;
    output << tag("text", false) << endl;

    return output.str();
}

string ZSvgGenerator::rectangleTag(double x, double y,
                                   double width, double height,
                                   string attribute)
{
    ostringstream output;

    output << "<rect x=\"" << x << "\" y=\"" << y <<
        "\" width=\"" << width << "\" height=\"" << height <<
        "\" " << attribute <<"/>";

    return output.str();
}

string ZSvgGenerator::lineTag(double x1, double y1, double x2, double y2,
                              string attribute)
{
    ostringstream output;

    output << "<line x1=\"" << x1 << "\" y1=\"" << y1 <<
        "\" x2=\"" << x2 << "\" y2=\"" << y2 << "\" " << attribute <<"/>";

    return output.str();
}

string ZSvgGenerator::transformTag(string attribute)
{
    ostringstream output;
    output << "<g transform=\"" << attribute << "\">";

    return output.str();
}

string ZSvgGenerator::transformTag()
{
    return tag("g", false);
}

string ZSvgGenerator::translateAttribute(double x, double y)
{
    ostringstream output;

    output << "translate(" << x << "," << y << ")";

    return output.str();
}

string ZSvgGenerator::rotateAttribute(double angle, double x, double y)
{
    ostringstream output;

    output << "rotate(" << angle << "," << x << "," << y << ")";

    return output.str();
}

string ZSvgGenerator::circleTag(double cx, double cy, double r,
    std::string attribute)
{
    ostringstream output;

    output << "<circle cx=\"" << cx << "\" cy=\"" << cy <<
        "\" r=\"" << r << "\" " << attribute <<"/>";

    return output.str();
}

string ZSvgGenerator::hyperLinkTag(string url, string attribute)
{
  return "<a xlink:href=\"" + url + "\"" + " " + attribute + ">";
}

string ZSvgGenerator::hyperLinkTag()
{
  return "</a>";
}

bool ZSvgGenerator::write(
    const char *filePath,              //Output file path
    const string &svgBody
    ) const
{
    ofstream output(filePath, ios_base::out);

    output << header() << endl;
    output << docType() << endl;
    output << svgStartTag() << endl;

    output << svgBody << endl;

    output << svgEndTag() << endl;

    output.close();

    return true;
}
