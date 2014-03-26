/**@file tz_pipe.h
 * @brief pipe for multi-language interface
 * @author Ting Zhao
 */

#ifndef _TZ_PIPE_H_
#define _TZ_PIPE_H_

#define PIPE_LANG_SIZE 10 /* maximum length of the language name */
#define PIPE_INTF_SIZE 100 /* maximum length of the interface string */
#define PIPE_INTF_MAXPARAM 10 /* maximum number of the interface parameters */
#define PIPE_FIELD_NUMBER 2 /* number of fields in the pipe structure */

typedef struct {
  char language[PIPE_LANG_SIZE]; /* programming language */
  char interface[PIPE_INTF_SIZE]; /* interface */
} Pipe;

/* pipe initialization */
void initpipein();
void initpipeout(char *language,char* interface);

/* pipe implementation */
void *pipein();
void pipeout();

/* helper function for pipes */
void * loadpipe(Pipe *p);
void writepipe(Pipe *p);
void *funcall(char *fun,char *param[]);

#endif
