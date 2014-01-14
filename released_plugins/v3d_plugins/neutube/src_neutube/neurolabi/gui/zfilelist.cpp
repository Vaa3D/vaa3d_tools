#include "zfilelist.h"

#include <string.h>
#include <vector>
#include <algorithm>

#include "neurolabi_config.h"
#include "zstring.h"
#include "tz_xml_utils.h"
#include "tz_error.h"

using namespace std;

ZFileList::ZFileList()
{
  m_fileList.file_number = 0;
  m_fileList.file_path = NULL;
}

ZFileList::ZFileList(int n)
{
  if (n > 0) {
    m_fileList.file_number = n;
    m_fileList.file_path = (File_Path_String_t*)
        Guarded_Malloc(sizeof(File_Path_String_t) * n,
                       "ZFileList");
  }
}

ZFileList::~ZFileList()
{
  Clean_File_List(&m_fileList);
}

char* ZFileList::getFilePath(int index) const
{
  return m_fileList.file_path[index];
}

struct StringCompare {
  bool operator() (const string s1, const string s2) {
    string ls1 = s1;
    string ls2 = s2;
    transform(s1.begin(), s1.end(), ls1.begin(), ::tolower);
    transform(s2.begin(), s2.end(), ls2.begin(), ::tolower);
    return strcmp(ls1.c_str(), ls2.c_str()) < 0;
  }
};

void ZFileList::load(const string &dir, const string &ext, ESortingOption option)
{
  File_List_Load_Dir(dir.c_str(), ext.c_str(), &m_fileList);

  switch (option) {
  case SORT_BY_LAST_NUMBER:
    File_List_Sort_By_Number(&m_fileList);
    break;
  case SORT_ALPHABETICALLY:
  {
    vector<string> tmpFileList;
    for (int i = 0; i < size(); i++) {
      tmpFileList.push_back(getFilePath(i));
    }
    sort(tmpFileList.begin(), tmpFileList.end(), StringCompare());
    for (int i = 0; i < size(); i++) {
      setFilePath(i, tmpFileList[i]);
    }
  }
    break;
  default:
    break;
  }
}

int ZFileList::startNumber() const
{
  return String_Last_Integer(getFilePath(0));
}

int ZFileList::endNumber() const
{
  return String_Last_Integer(getFilePath(size() - 1));
}

void ZFileList::setFilePath(int index, string path)
{
  strcpy(m_fileList.file_path[index], path.c_str());
}

void ZFileList::importFromXml(const string &filePath)
{
#if defined(HAVE_LIBXML2)
  xmlDocPtr doc;
  xmlNodePtr cur;

  doc = xmlParseFile(filePath.c_str());
  if (doc == NULL) {
    return;
  }

  xmlNodePtr root = xmlDocGetRootElement(doc);
  if (root == NULL) {
    xmlFreeDoc(doc);
    return;
  }

  vector<string> filePathArray;

  cur = root->xmlChildrenNode;
  while (cur != NULL) {
    if (Xml_Node_Is_Element(cur, "url") == TRUE) {
      filePathArray.push_back(Xml_Node_String_Value(doc, cur));
    } else {
      xmlNodePtr child = cur->xmlChildrenNode;
      while (child != NULL) {
        if (Xml_Node_Is_Element(child, "url") == TRUE) {
          filePathArray.push_back(Xml_Node_String_Value(doc, child));
        }
      }
    }
    cur = cur->next;
  }

  xmlFreeDoc(doc);

  Clean_File_List(&m_fileList);
  m_fileList.file_number = filePathArray.size();
  if (m_fileList.file_number > 0) {
    m_fileList.file_path = (File_Path_String_t*) Guarded_Malloc(
          sizeof(File_Path_String_t) * m_fileList.file_number,
          "File_List_Load_Dir");
  }

  for (int i = 0; i < m_fileList.file_number; i++) {
    setFilePath(i, filePathArray[i]);
  }
#else
  TZ_ERROR(ERROR_NA_FUNC);
#endif
}
