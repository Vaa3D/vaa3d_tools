#ifndef __NS_MODEL_MODEL_SECTION_H__
#define __NS_MODEL_MODEL_SECTION_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsprogress.h>
#include <model/nsmodel.h>

NS_DECLS_BEGIN


typedef enum
   {
   NS_MODEL_SECTION_NONE,
   NS_MODEL_SECTION_DEPTH_FIRST,
   NS_MODEL_SECTION_BREADTH_FIRST,

   NS_MODEL_SECTION_NUM_TYPES  /* Dont use! */
   }
   NsModelSectionType;

NS_IMPEXP const nschar* ns_model_section_type_to_string( NsModelSectionType type );


NS_IMPEXP NsError ns_model_section
   (
   NsModel             *model,
   NsModelSectionType   type,
   NsProgress          *progress
   );

NS_DECLS_END

#endif/* __NS_MODEL_MODEL_SECTION_H__ */



