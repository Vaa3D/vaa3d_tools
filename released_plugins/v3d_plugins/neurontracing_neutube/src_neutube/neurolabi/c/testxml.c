#include <stdio.h>
#include <libxml/xmlreader.h>
#include "tz_xml_utils.h"

#ifdef LIBXML_READER_ENABLED

#if 0
/**
 * processNode:
 * @reader: the xmlReader
 *
 * Dump information about the current node
 */
static void
processNode(xmlTextReaderPtr reader) {
    const xmlChar *name, *value;

    name = xmlTextReaderConstName(reader);
    if (name == NULL)
	name = BAD_CAST "--";

    value = xmlTextReaderConstValue(reader);

    printf("%d %d %s %d %d %d", 
	    xmlTextReaderDepth(reader),
	    xmlTextReaderNodeType(reader),
	    name,
	    xmlTextReaderIsEmptyElement(reader),
	   xmlTextReaderHasValue(reader),
	   xmlTextReaderAttributeCount(reader));
    if (value == NULL)
	printf("\n");
    else {
        if (xmlStrlen(value) > 40)
            printf(" %.40s...\n", value);
        else
	    printf(" %s\n", value);
    }
}
#endif

/**
 * streamFile:
 * @filename: the file name to parse
 *
 * Parse and print information about an XML file.
 */
#if 0
static void
streamFile(const char *filename) {
    xmlTextReaderPtr reader;
    int ret;

    reader = xmlReaderForFile(filename, NULL, 0);
    if (reader != NULL) {
        ret = xmlTextReaderRead(reader);
        while (ret == 1) {
            processNode(reader);
            ret = xmlTextReaderRead(reader);
        }
        xmlFreeTextReader(reader);
        if (ret != 0) {
            fprintf(stderr, "%s : failed to parse\n", filename);
        }
    } else {
        fprintf(stderr, "Unable to open %s\n", filename);
    }
}
#endif

int main(int argc, char **argv) {

#if 0
  if (argc != 2)
    return(1);

  /*
   * this initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  LIBXML_TEST_VERSION

    streamFile(argv[1]);

  /*
   * Cleanup function for the XML library.
   */
  xmlCleanupParser();
  /*
   * this is to debug memory for regression tests
   */
  xmlMemoryDump();
#endif

#if 0
  xmlDocPtr doc;
  xmlNodePtr cur;
    
  doc = xmlParseFile(argv[1]);
  if (doc == NULL) {
    fprintf(stderr, "XML parsing failed.\n");
    return 1;
  }

  cur = xmlDocGetRootElement(doc);
  if (cur == NULL) {
    fprintf(stderr, "empty document\n");
    xmlFreeDoc(doc);
    return 1;
  }

  if (xmlStrcmp(cur->name, (const xmlChar*) "trace")) {
    fprintf(stderr, "document of wrong type\n");
    xmlFreeDoc(doc);
    return 1;
  }

  cur = cur->xmlChildrenNode;
  while (cur != NULL) {
    printf("%s\n", cur->name);
    if (xmlStrcmp(cur->name, (const xmlChar*) "data") == 0) {
      Stack_Document *stack_doc = Xml_Stack_Document_R(doc, cur, "/Users", NULL);
      printf("%s\n", Stack_Document_File_Path(stack_doc));
	
      if (stack_doc != NULL) {
	Kill_Stack_Document(stack_doc);
      }
      /*
	xmlNodePtr dataChild = cur->xmlChildrenNode;
	while (dataChild != NULL) {
	if (xmlStrcmp(dataChild->name, (const xmlChar*) "image") == 0) {
	printf("attribure: %s\n", 
	xmlGetProp(dataChild, (const xmlChar*) "type"));
	}
	dataChild = dataChild->next;
	}
      */
    }
    cur = cur->next;
  }

  xmlFreeDoc(doc);
#endif

#if 0 /* svg test */
  FILE *fp = fopen("../data/test2.svg", "w");

  Svg_Header_Fprint(fp, NULL);
  Svg_Begin_Fprint(fp, 100, 100, NULL, NULL);
  Svg_Line_Fprint(fp, 1.0, 1.0, 10.0, 10.0, 1, "red");
  Svg_Circle_Fprint(fp, 5.0, 5.0, 3.0, 1, "blue");
  Svg_End_Fprint(fp);

  fclose(fp);
#endif

#if 0
  FILE *fp = fopen("../data/test.svg", "w");
  Geo3d_Point_Array *pa = Make_Geo3d_Point_Array(10);
  int i;
  for (i = 0; i < pa->n; i++) {
    pa->array[i][0] = i*10;
    pa->array[i][1] = i*10;
    pa->array[i][2] = i*10;
  }
  Svg_Header_Fprint(fp, NULL);
  Svg_Begin_Fprint(fp, 500, 500, NULL, NULL);
  Svg_Polygonal_Chain_Fprint(fp, pa, "jet", 1); 
  Svg_End_Fprint(fp);
  fclose(fp);
#endif

#if 0
  Stack_Document *doc = Xml_Read_Stack_Document("../data/test.xml");

  File_List *list = (File_List*) doc->ci;
  Print_File_List(list);

#endif

  return(0);
}

#else
int main(void) {
  fprintf(stderr, "XInclude support not compiled in\n");
  exit(1);
}
#endif
