/**@file tz_objtempl.h
 * @brief object code template
 * @author Ting Zhao
 */

void Reset_<Object>(<Object> *sw);

void Construct_<Object>(<Object> * sw, int size);
void Clean_<Object>(<Object> * sw);

<Object>* Make_<Object>();

/* Functions defined in the macros */
<Object>* New_<Object>();
<Object> *Copy_<Object>(const <Object> *sw);
void   Pack_<Object>(<Object> *sw);
void   Free_<Object>(<Object> *sw);
void   Kill_<Object>(<Object> *sw);
void   Reset_<Object>();
int    <Object>_Usage();

int Save_<Object>(<Object> *sw, char *file_path);
<Object>* Load_<Object>(char *file_path);
/***********************************/
