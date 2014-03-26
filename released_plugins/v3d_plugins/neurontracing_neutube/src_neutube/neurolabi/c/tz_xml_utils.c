#include <string.h>
#include <math.h>
#include "tz_error.h"
#include "tz_xml_utils.h"
#include "tz_color.h"
#include "tz_math.h"

char* Xml_String_To_String(const xmlChar *str)
{
  if (str == NULL) {
    return NULL;
  }

  int len = strlen((const char*) str);
  char *str2 = (char*) malloc(sizeof(char) * (len + 1));

  memcpy(str2, str, sizeof(char) * (len + 1));
  
  return str2;
}

xmlNodePtr Xml_Doc_Root_Element(const xmlDocPtr doc)
{
#ifdef HAVE_LIBXML2
  return xmlDocGetRootElement(doc);
#else
  TZ_ERROR(ERROR_NA_FUNC);
  return NULL;
#endif
}

char* Xml_Node_Name(const xmlNodePtr node)
{
#ifdef HAVE_LIBXML2
  return Xml_String_To_String(node->name);
#else
  TZ_ERROR(ERROR_NA_FUNC);
  return NULL;
#endif
}

BOOL Xml_Node_Is_Element(const xmlNodePtr node, const char *str)
{
#ifdef HAVE_LIBXML2
  if (xmlStrcmp(node->name, CONST_XML_STRING(str)) == 0) {
    return TRUE;
  }
#else
  TZ_ERROR(ERROR_NA_FUNC);
#endif

  return FALSE;
}

char* Xml_Node_String_Value(const xmlDocPtr doc, const xmlNodePtr node)
{
#ifdef HAVE_LIBXML2
  xmlChar *value = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
  char *str = Xml_String_To_String(value);
  xmlFree(value);

  return str;
#else
  TZ_ERROR(ERROR_NA_FUNC);
  return NULL;
#endif
}

double Xml_Node_Double_Value(const xmlDocPtr doc, const xmlNodePtr node)
{

  char *value = Xml_Node_String_Value(doc, node);
  double result = atof(value);
  free(value);
  
  return result;
}

int Xml_Node_Int_Value(const xmlDocPtr doc, const xmlNodePtr node)
{

  char *value = Xml_Node_String_Value(doc, node);
  int result = atoi(value);
  free(value);
  
  return result;
}

int Xml_Coordinate(const xmlDocPtr doc, const xmlNodePtr node, double *coord)
{
#ifdef HAVE_LIBXML2
  xmlNodePtr child = node->xmlChildrenNode;
  if (child == NULL) {
    return 2;
  }

  while (child != NULL) {
    if (Xml_Node_Is_Element(child, "x") == TRUE) {
      coord[0] = Xml_Node_Double_Value(doc, child);
      /*
      xmlChar *value = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
      coord[0] = atof((const char*) value);
      xmlFree(value);
      */
    } else if (Xml_Node_Is_Element(child, "y") == TRUE) {
      coord[1] = Xml_Node_Double_Value(doc, child);
      /*
      xmlChar *value = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
      coord[1] = atof((const char*) value);
      xmlFree(value);
      */
    } else if (Xml_Node_Is_Element(child, "z") == TRUE) {
      coord[2] = Xml_Node_Double_Value(doc, child);
      /*
      xmlChar *value = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
      coord[2] = atof((const char*) value);
      xmlFree(value);
      */
    }
    child = child->next;
  }
#else
  TZ_ERROR(ERROR_NA_FUNC);
#endif

  return 0;
}

int Xml_Stack_Doc_File_Bundle(const xmlDocPtr doc, const xmlNodePtr node, 
			      File_Bundle_S *bundle)
{
#ifdef HAVE_LIBXML2
  if (Xml_Node_Is_Element(node, "image") == FALSE) {
    return 1;
  }

  xmlChar *image_type = xmlGetProp(node, CONST_XML_STRING("type"));
  if (xmlStrcmp(image_type, CONST_XML_STRING("bundle"))) {
    return 3;
  }

  xmlFree(image_type);

  xmlNodePtr child =node->xmlChildrenNode;
  
  xmlChar *prefix = NULL;
  xmlChar *suffix = NULL;
  xmlChar *num_width = NULL;
  xmlChar *first_num = NULL;
  xmlChar *last_num = NULL;

  while (child != NULL) {
    if (Xml_Node_Is_Element(child, "prefix")) {
      prefix = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
    } else if (Xml_Node_Is_Element(child, "suffix")) {
      suffix = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
    } else if (Xml_Node_Is_Element(child, "num_width")) {
      num_width = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
    } else if (Xml_Node_Is_Element(child, "first_num")) {
      first_num = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
    } else if (Xml_Node_Is_Element(child, "last_num")) {
      last_num = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
    }

    child = child->next;
  }
  
  bundle->prefix = Xml_String_To_String(prefix);
  bundle->suffix = Xml_String_To_String(suffix);
  if (num_width == NULL) {
    bundle->num_width = 3;
  } else {
    bundle->num_width = atoi((const char*) num_width);
  }
  if (first_num == NULL) {
    bundle->first_num = 0;
  } else {
    bundle->first_num = atoi((const char*) first_num);
  } 
  if (last_num == NULL) {
    bundle->last_num = -1;
  } else {
    bundle->last_num = atoi((const char*) last_num);
  }

  if (prefix != NULL) {
    xmlFree(prefix);
  }

  if (suffix != NULL) {
    xmlFree(suffix);
  }

  if (num_width != NULL) {
    xmlFree(num_width);
  }

  if (first_num != NULL) {
    xmlFree(first_num);
  }
#else
  TZ_ERROR(ERROR_NA_FUNC);
#endif
 
  return 0;
}

int Xml_Stack_Doc_File_Bundle_R(const xmlDocPtr doc, const xmlNodePtr node, 
    const char *curdir, File_Bundle_S *bundle)
{
#ifdef HAVE_LIBXML2
  if (Xml_Node_Is_Element(node, "image") == FALSE) {
    return 1;
  }

  xmlChar *image_type = xmlGetProp(node, CONST_XML_STRING("type"));
  if (xmlStrcmp(image_type, CONST_XML_STRING("bundle"))) {
    return 3;
  }

  xmlFree(image_type);

  xmlNodePtr child =node->xmlChildrenNode;
  
  xmlChar *prefix = NULL;
  xmlChar *suffix = NULL;
  xmlChar *num_width = NULL;
  xmlChar *first_num = NULL;

  while (child != NULL) {
    if (Xml_Node_Is_Element(child, "prefix")) {
      prefix = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
    } else if (Xml_Node_Is_Element(child, "suffix")) {
      suffix = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
    } else if (Xml_Node_Is_Element(child, "num_width")) {
      num_width = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
    } else if (Xml_Node_Is_Element(child, "first_num")) {
      first_num = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
    }
    child = child->next;
  }
  
  bundle->prefix = Xml_String_To_String(prefix);
  /* Need modification for different OS */
  if ((bundle->prefix[0] != '/') && (curdir != NULL)) {
    char *tmp_prefix = bundle->prefix;
    bundle->prefix = fullpath(curdir, tmp_prefix, NULL);
    free(tmp_prefix);
  }

  bundle->suffix = Xml_String_To_String(suffix);
  bundle->num_width = atoi((const char*) num_width);
  bundle->first_num = atoi((const char*) first_num);

  if (prefix != NULL) {
    xmlFree(prefix);
  }

  if (suffix != NULL) {
    xmlFree(suffix);
  }

  if (num_width != NULL) {
    xmlFree(num_width);
  }

  if (first_num != NULL) {
    xmlFree(first_num);
  }
#else
  TZ_ERROR(ERROR_NA_FUNC);
#endif
 
  return 0;
}

int Xml_Stack_Doc_File_Path(const xmlDocPtr doc, const xmlNodePtr node, 
			    char **path)
{
#ifdef HAVE_LIBXML2
  if (Xml_Node_Is_Element(node, "image") == FALSE) {
    return 1;
  }

  *path = NULL;
  
  xmlNodePtr child = node->xmlChildrenNode;
  
  while (child != NULL) {
    if (Xml_Node_Is_Element(child, "url") == TRUE) {
      xmlChar *value = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
      *path = Xml_String_To_String(value);
      xmlFree(value);
      break;
    }
    child = child->next;
  }
#else
  TZ_ERROR(ERROR_NA_FUNC);
#endif

  return 0;
}

int Xml_Stack_Doc_File_List(const xmlDocPtr doc, const xmlNodePtr node, 
			    File_List *file_list)
{
#ifdef HAVE_LIBXML2
  if (Xml_Node_Is_Element(node, "image") == FALSE) {
    return 1;
  }

  char *path = NULL;
  char *ext = NULL;
  
  xmlNodePtr child = node->xmlChildrenNode;
  
  while (child != NULL) {
    if (Xml_Node_Is_Element(child, "url") == TRUE) {
      xmlChar *value = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
      path = Xml_String_To_String(value);
      xmlFree(value);
    }
    if (Xml_Node_Is_Element(child, "ext") == TRUE) {
      xmlChar *value = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
      ext = Xml_String_To_String(value);
      xmlFree(value);
    }
    child = child->next;
  }

  if ((path != NULL) && (ext != NULL)) {
    File_List_Load_Dir(path, ext, file_list);
    File_List_Sort_By_Number(file_list);
  }

  free(path);
  free(ext);
#else
  TZ_ERROR(ERROR_NA_FUNC);
#endif

  return 0;
}

int Xml_Stack_Doc_File_Path_R(const xmlDocPtr doc, const xmlNodePtr node, 
    const char *curdir, char **path)
{
#ifdef HAVE_LIBXML2
  if (Xml_Node_Is_Element(node, "image") == FALSE) {
    return 1;
  }

  *path = NULL;
  
  xmlNodePtr child = node->xmlChildrenNode;
  
  while (child != NULL) {
    if (Xml_Node_Is_Element(child, "url") == TRUE) {
      xmlChar *value = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
      TZ_ASSERT(xmlStrlen(value) > 1, "File path too short");
#if defined(_WIN32) || defined(_WIN64)
      if ((value[1] == ':')
#else
      if ((value[0] == '/')
#endif
          || (path == NULL)) { /* need modification for different OS */
	*path = Xml_String_To_String(value);
      } else {
	char *cvalue = Xml_String_To_String(value);
	*path = fullpath(curdir, cvalue, NULL);
	free(cvalue);
      }
      
      xmlFree(value);
      break;
    }
    child = child->next;
  }
#else
  TZ_ERROR(ERROR_NA_FUNC);
#endif

  return 0;
}

Stack_Document* Xml_Stack_Document(const xmlDocPtr doc, const xmlNodePtr node, 
				   Stack_Document *stack_doc)
{
#ifdef HAVE_LIBXML2
  if (Xml_Node_Is_Element(node, "data") == FALSE) {
    return NULL;
  }

  if (stack_doc == NULL) {
    stack_doc = New_Stack_Document();
  }

  int type = STACK_DOC_UNDEFINED;

  xmlNodePtr child = node->xmlChildrenNode;
  
  while (child != NULL) {
    if (Xml_Node_Is_Element(child, "image") == TRUE) {
      xmlChar *image_type = xmlGetProp(child, CONST_XML_STRING("type"));
      void *ci = NULL;
      File_Bundle_S fb;
      if (xmlStrcmp(image_type, CONST_XML_STRING("bundle")) == 0) {
	if (Xml_Stack_Doc_File_Bundle(doc, child, &fb) == 0) { 
	  ci = (void*) (&fb);
	}
	type = STACK_DOC_FILE_BUNDLE;
      } else if (xmlStrcmp(image_type, CONST_XML_STRING("tif")) == 0) {
	char *path = NULL;
	Xml_Stack_Doc_File_Path(doc, child, &path);
	ci = path;
	type = STACK_DOC_TIF_FILE;
      } else if (xmlStrcmp(image_type, CONST_XML_STRING("lsm")) == 0) {
	char *path = NULL;
	Xml_Stack_Doc_File_Path(doc, child, &path);
	ci = path;
	type = STACK_DOC_LSM_FILE;
      } else if (xmlStrcmp(image_type, CONST_XML_STRING("raw")) == 0) {
	char *path = NULL;
	Xml_Stack_Doc_File_Path(doc, child, &path);
	ci = path;
	type = STACK_DOC_RAW_FILE;
      } else if (xmlStrcmp(image_type, CONST_XML_STRING("dir")) == 0) {
        File_List *file_list = New_File_List();
	if (Xml_Stack_Doc_File_List(doc, child, file_list) == 0) { 
	  ci = (void*) file_list;
	}
	type = STACK_DOC_FILE_LIST;
      }

      xmlFree(image_type);
      
      Set_Stack_Document(stack_doc, type, ci);
      
      if ((stack_doc->type != STACK_DOC_FILE_BUNDLE) && 
          (stack_doc->type != STACK_DOC_FILE_LIST)) {
	free(ci);
      }
    } else if (Xml_Node_Is_Element(child, "resolution") == TRUE) {
      Xml_Coordinate(doc, child, stack_doc->resolution);
    } else if (Xml_Node_Is_Element(child, "channel") == TRUE) {
      stack_doc->channel = Xml_Node_Int_Value(doc, child);
    } else if (Xml_Node_Is_Element(child, "offset") == TRUE) {
      Xml_Coordinate(doc, child, stack_doc->offset);
    } else if (Xml_Node_Is_Element(child, "unit") == TRUE) {
      stack_doc->unit = Xml_Node_String_Value(doc, child)[0];
    }

    child = child->next;
  }
#else
  TZ_ERROR(ERROR_NA_FUNC);
#endif

  return stack_doc;
}

Stack_Document* Xml_Stack_Document_R(const xmlDocPtr doc, 
    const xmlNodePtr node, const char *curdir, Stack_Document *stack_doc)
{
#ifdef HAVE_LIBXML2
  if (Xml_Node_Is_Element(node, "data") == FALSE) {
    return NULL;
  }

  if (stack_doc == NULL) {
    stack_doc = New_Stack_Document();
  }

  int type = STACK_DOC_UNDEFINED;

  xmlNodePtr child = node->xmlChildrenNode;
  
  while (child != NULL) {
    if (Xml_Node_Is_Element(child, "image") == TRUE) {
      xmlChar *image_type = xmlGetProp(child, CONST_XML_STRING("type"));
      void *ci = NULL;
      File_Bundle_S fb;
      if (xmlStrcmp(image_type, CONST_XML_STRING("bundle")) == 0) {
	if (Xml_Stack_Doc_File_Bundle(doc, child, &fb) == 0) { 
	  ci = (void*) (&fb);
	}
	type = STACK_DOC_FILE_BUNDLE;
      } else if (xmlStrcmp(image_type, CONST_XML_STRING("tif")) == 0) {
	char *path = NULL;
	Xml_Stack_Doc_File_Path_R(doc, child, curdir, &path);
	ci = path;
	type = STACK_DOC_TIF_FILE;
      } else if (xmlStrcmp(image_type, CONST_XML_STRING("lsm")) == 0) {
	char *path = NULL;
	Xml_Stack_Doc_File_Path_R(doc, child, curdir, &path);
	ci = path;
	type = STACK_DOC_LSM_FILE;
      } else if (xmlStrcmp(image_type, CONST_XML_STRING("raw")) == 0) {
	char *path = NULL;
	Xml_Stack_Doc_File_Path_R(doc, child, curdir, &path);
	ci = path;
	type = STACK_DOC_RAW_FILE;
      }

      xmlFree(image_type);
      
      Set_Stack_Document(stack_doc, type, ci);
      
      if (stack_doc->type != STACK_DOC_FILE_BUNDLE) {
	free(ci);
      }
    } else if (Xml_Node_Is_Element(child, "resolution") == TRUE) {
      Xml_Coordinate(doc, child, stack_doc->resolution);
    } else if (Xml_Node_Is_Element(child, "channel") == TRUE) {
      stack_doc->channel = Xml_Node_Int_Value(doc, child);
    } else if (Xml_Node_Is_Element(child, "offset") == TRUE) {
      Xml_Coordinate(doc, child, stack_doc->offset);
    } else if (Xml_Node_Is_Element(child, "unit") == TRUE) {
      stack_doc->unit = Xml_Node_String_Value(doc, child)[0];
    }

    child = child->next;
  }
#else
  TZ_ERROR(ERROR_NA_FUNC);
#endif

  return stack_doc;

}

Stack_Document* Xml_Read_Stack_Document(const char *filePath)
{
#ifdef HAVE_LIBXML2
  xmlDocPtr doc;
  xmlNodePtr cur;

  doc = xmlParseFile(filePath);
  if (doc == NULL) {
    return NULL;
  }

  cur = xmlDocGetRootElement(doc);
  if (cur == NULL) {
    xmlFreeDoc(doc);
    return NULL;
  }

  Stack_Document *stack_doc = NULL;

  cur = cur->xmlChildrenNode;
  while (cur != NULL) {
    if (Xml_Node_Is_Element(cur, "data") == TRUE) {
      stack_doc = Xml_Stack_Document(doc, cur, NULL);
      break;
    }
    cur = cur->next;
  }
  xmlFreeDoc(doc);

  return stack_doc;
#else
  TZ_ERROR(ERROR_NA_FUNC);
  return NULL;
#endif  
}

void Svg_Header_Fprint(FILE *fp, const char *ver)
{
  TZ_ASSERT(fp != NULL, "Null file pointer");

  fprintf(fp, "<?xml version=\"1.0\" standalone=\"no\"?>\n");
  if ((ver == NULL) || (strcmp(ver, "1.1") == 0)) {
    fprintf(fp, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
  } else {
    TZ_ERROR(ERROR_DATA_VALUE);
  }
}

void Svg_Begin_Fprint(FILE *fp, double width, double height, int *viewbox,
		      const char *ver)
{
  TZ_ASSERT(fp != NULL, "Null file pointer");

  if (ver == NULL) {
    ver = "1.1";
  }

  fprintf(fp, "<svg width=\"%g%%\" height=\"%g%%\" ", width, height);

  if (viewbox != NULL) {
    fprintf(fp, "viewBox=\"%d %d %d %d\" ", 
	    viewbox[0], viewbox[1], viewbox[2], viewbox[3]);
  }
  
  fprintf(fp, "version=\"%s\" xmlns=\"http://www.w3.org/2000/svg\">\n", ver);
}

void Svg_End_Fprint(FILE *fp)
{
  TZ_ASSERT(fp != NULL, "Null file pointer");
  
  fprintf(fp, "</svg>\n");
}

void Svg_Line_Fprint(FILE *fp, double x1, double y1, double x2, double y2,
		     int width, const char *color)
{
  TZ_ASSERT(fp != NULL, "Null file pointer");

  fprintf(fp, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2 = \"%g\" stroke=\"%s\" stroke-width=\"%d\"/>\n", x1, y1, x2, y2, color, width);
}

void Svg_Line_Fprint_C(FILE *fp, double x1, double y1, double x2, double y2,
		       int width, uint8_t r, uint8_t g, uint8_t b)
{
  TZ_ASSERT(fp != NULL, "Null file pointer");

  fprintf(fp, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2 = \"%g\" stroke=\"rgb(%u, %u, %u)\" stroke-width=\"%d\"/>\n", x1, y1, x2, y2, r, g, b, width);
}

void Svg_Circle_Fprint(FILE *fp, double x, double y, double r,
		       int width, const char *color)
{
  TZ_ASSERT(fp != NULL, "Null file pointer");

  fprintf(fp, "<circle cx=\"%g\" cy=\"%g\" r=\"%g\" stroke=\"%s\" stroke-width=\"%d\"/>\n", x, y, r, color, width);
}

void Svg_Text_Fprint(FILE *fp, const char *text, double x, double y, 
		     const char *color, int size)
{
  TZ_ASSERT(fp != NULL, "Null file pointer");
  fprintf(fp, "<text x=\"%g\" y=\"%g\"", x, y);

  if (color != NULL) {
    fprintf(fp, " stroke=\"%s\"", color);
  }

  fprintf(fp, " font-size=\"%d\"", size);

  fprintf(fp, ">%s</text>\n", text);
}

void Svg_Polygonal_Chain_Fprint(FILE *fp, Geo3d_Point_Array *pa, 
    const char *color, int width) {
  if (color == NULL) {
    color = "jet";
  }

  if (strcmp(color, "jet") == 0) {
    int i;
    double z;
    int index;
    Rgb_Color rgb;
    for (i = 1; i < pa->n; i++) { 
      z = (pa->array[i-1][2] + pa->array[i][2]) / 2.0;
      index = iround(z * 63.0);
      if (index > 63) {
	index = 63;
      }
      Set_Color_Jet(&rgb, index);

      Svg_Line_Fprint_C(fp, pa->array[i-1][0], pa->array[i-1][1], 
	  pa->array[i][0], pa->array[i][1], width, rgb.r, rgb.g, rgb.b);
    }
  }
}
