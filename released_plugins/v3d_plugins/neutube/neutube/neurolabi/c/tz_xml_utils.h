/**@file tz_xml_utils.h
 * @author Ting Zhao
 * @date 10-May-2009
 */

#ifndef _TZ_XM_LUTILS_H_
#define _TZ_XML_UTILS_H_

#include "tz_cdefs.h"
#if defined(HAVE_LIBXML2)
#include <libxml/xmlreader.h>
#endif
#include "tz_stack_document.h"
#include "tz_image_io.h"
#include "tz_geo3d_point_array.h"
#include "tz_file_list.h"

__BEGIN_DECLS

#ifndef HAVE_LIBXML2
typedef unsigned char xmlChar;
typedef void* xmlNodePtr;
typedef void* xmlDocPtr;
#endif

/* UTF-8 encoded string */
#define XML_STRING(str) ((xmlChar*) str)
#define CONST_XML_STRING(str) ((const xmlChar*) str)

/**@brief Turn an XML string to a native string.
 *
 * Xml_String_To_String() returns a direct copy of the XML string <str> and 
 * casts it into the native string format. The copy has the same binary
 * content as that of <str>.
 */
char* Xml_String_To_String(const xmlChar *str);

char* Xml_Node_Name(const xmlNodePtr node);

xmlNodePtr Xml_Doc_Root_Element(const xmlDocPtr doc);

/**@brief Test if an XML node is a certain element.
 *
 * Xml_Node_Is_Element() returns TRUE if <node> has name <str>; Otherwise it
 * returns FALSE.
 */
BOOL Xml_Node_Is_Element(const xmlNodePtr node, const char *str);

/**@brief Get the string value of an XML node.
 *
 * Xml_Node_String_Value() returns the string value of <node> in <doc>. The
 * caller is responsible for freeing the returned pointer.
 */
char* Xml_Node_String_Value(const xmlDocPtr doc, const xmlNodePtr node);

/**@brief Get the integer value of an XML node.
 *
 * Xml_Node_Int_Value() returns the integer value of <node> in <doc>. The return
 * value is uncertain if the node does not have integer value.
 */
int Xml_Node_Int_Value(const xmlDocPtr doc, const xmlNodePtr node);

/**@brief Get the double value of an XML node.
 *
 * Xml_Node_Double_Value() returns the double value of <node> in <doc>. The 
 * return value is uncertain if the node does not have integer value.
 */
double Xml_Node_Double_Value(const xmlDocPtr doc, const xmlNodePtr node);

/**@brief Get the coordinates from an XML node.
 * 
 * Xml_Coordinate() get the coordinates value from <node> in <doc> and store
 * the result in <coord>. The node is supposed to have fields 'x', 'y', 'z'.
 * The values are uncertain if the retrieval is not successful.
 */
int Xml_Coordinate(const xmlDocPtr doc, const xmlNodePtr node, double *coord);

/**@brief Get file path from an XML node.
 *
 * Xml_Stack_Doc_File_Path() get file path from <node> in <doc>. The result is
 * stored in <path>. The node must be an 'image' element and have a field 'url'.
 */
int Xml_Stack_Doc_File_Path(const xmlDocPtr doc, const xmlNodePtr node, 
			    char **path);

/**@brief Get absolute file path from an XML node.
 *
 * Xml_Stack_Doc_File_Path_R() get file path from <node> in <doc>. The result is
 * stored in <path>. The node must be an 'image' element and have a field 'url'.
 * <curdir> is the current directory. If the file path in <node> is not an
 * absolute path, <curdir> will prepended.
 */
int Xml_Stack_Doc_File_Path_R(const xmlDocPtr doc, const xmlNodePtr node, 
    const char *curdir, char **path);

/**@brief Get file bundle from an XML node.
 */
int Xml_Stack_Doc_File_Bundle(const xmlDocPtr doc, const xmlNodePtr node, 
			      File_Bundle_S *bundle);

/**@brief Get file bundle from an XML node with considering relative path.
 */
int Xml_Stack_Doc_File_Bundle_R(const xmlDocPtr doc, const xmlNodePtr node, 
    const char *curdir, File_Bundle_S *bundle);

int Xml_Stack_Doc_File_List(const xmlDocPtr doc, const xmlNodePtr node, 
			    File_List *file_list);

/**@brief Get stack document from an XML node.
 */
Stack_Document* Xml_Stack_Document(const xmlDocPtr doc, 
				   const xmlNodePtr node, 
				   Stack_Document *stack_doc);

/**@brief Get stack document from an XML node with considering relative path.
 */
Stack_Document* Xml_Stack_Document_R(const xmlDocPtr doc, 
    const xmlNodePtr node, const char *curdir, Stack_Document *stack_doc);

/**@brief Load stack document from an XML file.
 */
Stack_Document* Xml_Read_Stack_Document(const char *filePath);

/* svg routines */

/**@brief Print the header of SVG.
 *
 * Svg_Header_Fprint() prints the XML header and SVG document type declaration
 * in <fp>. <ver> is the version of the SVG document. The latest version to 2009
 * is "1.1", which is taken as default when <ver> is NULL.
 */
void Svg_Header_Fprint(FILE *fp, const char *ver);

/**@brief Print the begin of svg tag.
 *
 * Svg_Begin_Fprint() prints the begin of a svg tag to <fp>. <width> and 
 * <height> are percentages. The 4-element array <viewbox> specifies the 
 * viewing box if it is not NULL. (<viewbox[0]>, <viewbox[1]>) is the top-left 
 * corner and (<viewbox[2]>, <viewbox[3]>) is the bottom-right corner. <ver> 
 * specifies the version.
 */
void Svg_Begin_Fprint(FILE *fp, double width, double height, int *viewbox,
		      const char *ver);

/**@brief Print the end of svg tag
 *
 * Svg_End_Fprint() prints the end of a svg tag to <fp>.
 */
void Svg_End_Fprint(FILE *fp);

/**@brief Print a line.
 *
 * Svg_Line_Fprint() prints the a line from (<x1>,<y1>) to (<x2>,<y2>) into 
 * <fp>. The width is specified by <width> and the color is specified by 
 * <color>.
 */
void Svg_Line_Fprint(FILE *fp, double x1, double y1, double x2, double y2,
		     int width, const char *color);

/**@brief Print a line with a specified RGB color.
 */
void Svg_Line_Fprint_C(FILE *fp, double x1, double y1, double x2, double y2,
		       int width, uint8_t r, uint8_t g, uint8_t b);

/**@brief Print a circle.
 *
 * Svg_Circle_Fprint() prints the a circle at (<x>,<y>) with radius <r> into 
 * <fp>. The width is specified by <width> and the color is specified by 
 * <color>. 
 */
void Svg_Circle_Fprint(FILE *fp, double x, double y, double r,
		       int width, const char *color);

/**@brief Print text element.
 * 
 * Svg_Text_Fprint() prints <text> at (<x>, <y>) in <fp> as a text element. The
 * color of the text is specified by <color>, which can be NULL for default
 * coloring.
 */
void Svg_Text_Fprint(FILE *fp, const char *text, double x, double y, 
		     const char *color, int size);

/**@brief Print a polygonal chain.
 *
 * Svg_Polygonal_Chain_Fprint() outputs the polygonal chain defined by <pa> to
 * <fp>. <pa->value> and <color> defines the color for each segment. <color> is
 * a string to specify the name of a color map. Currenlty only "jet" is
 * available.
 */
void Svg_Polygonal_Chain_Fprint(FILE *fp, Geo3d_Point_Array *pa, 
    const char *color, int width);

__END_DECLS

#endif
