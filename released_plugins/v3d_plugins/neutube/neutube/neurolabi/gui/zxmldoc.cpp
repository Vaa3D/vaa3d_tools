#include "zxmldoc.h"

#include <iostream>
#include <iomanip>

using namespace std;

ZXmlDoc::ZXmlDoc() : m_doc(NULL)
{
}

ZXmlDoc::~ZXmlDoc()
{
#if defined(HAVE_LIBXML2)
  if (m_doc != NULL) {
    xmlFreeDoc(m_doc);
    xmlCleanupParser();
  }
#endif
}

void ZXmlDoc::parseFile(const std::string &filePath)
{
#if defined(HAVE_LIBXML2)
  if (m_doc != NULL) {
    xmlFreeDoc(m_doc);
  }

  m_doc = xmlParseFile(filePath.c_str());
#endif
}

ZXmlNode ZXmlDoc::getRootElement()
{
#if defined(HAVE_LIBXML2)
  return ZXmlNode(Xml_Doc_Root_Element(m_doc), m_doc);
#else
  return ZXmlNode(NULL, NULL);
#endif
}

void ZXmlDoc::printInfo()
{
  getRootElement().printElementNames();
}

ZXmlNode::ZXmlNode() : m_node(NULL), m_doc(NULL)
{
}

ZXmlNode::ZXmlNode(xmlNodePtr node, xmlDocPtr doc) : m_node(node), m_doc(doc)
{
}

string ZXmlNode::name() const
{
#if defined(HAVE_LIBXML2)
  if (empty()) {
    return "";
  }

  char* nameStr = Xml_Node_Name(m_node);

  string name(nameStr);

  free(nameStr);

  return name;
#else
  return "";
#endif
}

string ZXmlNode::stringValue()
{
#if defined(HAVE_LIBXML2)
  if (empty()) {
    return "";
  }

  char *value = Xml_Node_String_Value(m_doc, m_node);
  string strValue;
  if (value != NULL) {
    strValue = value;
  }
  free(value);

  return strValue;
#else
  return "";
#endif
}

double ZXmlNode::doubleValue()
{
#if defined(HAVE_LIBXML2)
  if (empty()) {
    return NaN;
  }

  return Xml_Node_Double_Value(m_doc, m_node);
#else
  return 0.0;
#endif
}

int ZXmlNode::intValue()
{
#if defined(HAVE_LIBXML2)
  if (empty()) {
    return 0;
  }

  return Xml_Node_Int_Value(m_doc, m_node);
#else
  return 0;
#endif
}

bool ZXmlNode::empty() const
{
  return (m_doc == NULL || m_node == NULL);
}

ZXmlNode ZXmlNode::firstChild() const
{
  ZXmlNode child;
#if defined(HAVE_LIBXML2)
  if (!empty()) {
    child.m_node = m_node->xmlChildrenNode;
    child.m_doc = m_doc;
  }
#endif
  return child;
}

ZXmlNode ZXmlNode::nextSibling()
{
  ZXmlNode sibling;
#if defined(HAVE_LIBXML2)
  if (!empty()) {
    sibling.m_node = m_node->next;
    sibling.m_doc = m_doc;
  }
#endif

  return ZXmlNode();
}

ZXmlNode ZXmlNode::next()
{
  ZXmlNode nextNode;
#if defined(HAVE_LIBXML2)
  if (!empty()) {
    nextNode.m_node = m_node->next;
    nextNode.m_doc = m_doc;
  }
#endif
  return nextNode;
}

int ZXmlNode::type() const
{
#if defined(HAVE_LIBXML2)
  if (m_node != NULL) {
    return m_node->type;
  }
#endif

  return 0;
}

void ZXmlNode::printElementNames(int indent)
{
#if defined(HAVE_LIBXML2)
  if (!empty()) {
    if (type() == XML_ELEMENT_NODE) {
      cout << setfill(' ') << setw(indent) << "";
      cout << name() << endl;
      ZXmlNode child = firstChild();
      while (!child.empty()) {
        child.printElementNames(indent + 2);
        child = child.next();
      }
    }
  }
#endif
}

ZXmlNode ZXmlNode::queryNode(const std::string &nodeName) const
{
  ZXmlNode node;
#if defined(HAVE_LIBXML2)
  if (!empty()) {
    if (type() == XML_ELEMENT_NODE) {
      if (name() == nodeName) {
        node = *this;
      } else {
        ZXmlNode child = firstChild();
        while (!child.empty()) {
          node = child.queryNode(nodeName);
          if (!node.empty()) {
            break;
          }
          child = child.next();
        }
      }
    }
  }
#endif

  return node;
}

string ZXmlNode::getAttribute(const char *attribute) const
{
  string attributeValue;
#if defined(HAVE_LIBXML2)
  if (m_node != NULL) {
    xmlChar *prop = xmlGetProp(m_node, CONST_XML_STRING(attribute));
    attributeValue = Xml_String_To_String(prop);
  }
#endif
  return attributeValue;
}
