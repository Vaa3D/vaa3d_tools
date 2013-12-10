#ifndef ZXMLDOC_H
#define ZXMLDOC_H

#include <string>

#include "tz_cdefs.h"

/*
#if defined(HAVE_LIBXML2)
#include <libxml/xmlreader.h>
#endif
*/
#include "tz_xml_utils.h"

class ZXmlNode;

class ZXmlDoc
{
public:
  ZXmlDoc();
  ~ZXmlDoc();

  void parseFile(const std::string &filePath);
  ZXmlNode getRootElement();

  void printInfo();

private:
  xmlDocPtr m_doc;
};

class ZXmlNode
{
public:
  ZXmlNode();
  ZXmlNode(xmlNodePtr node, xmlDocPtr doc);

  std::string stringValue();
  double doubleValue();
  int intValue();
  std::string name() const;

  bool empty() const;
  ZXmlNode firstChild() const;
  ZXmlNode nextSibling();
  ZXmlNode next();

  std::string getAttribute(const char *attribute) const;

  int type() const;
  ZXmlNode queryNode(const std::string &nodeName) const;

  void printElementNames(int indent = 0);

private:
  xmlNodePtr m_node;
  xmlDocPtr m_doc;
};

#endif // ZXMLDOC_H
