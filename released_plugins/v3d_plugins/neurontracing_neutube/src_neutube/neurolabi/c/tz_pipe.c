#include <stdio.h>
#include <stdlib.h>
#define __USE_BSD
#include <string.h>
#include <image_lib.h>
#include "tz_pipe.h"
#include "tz_string.h"
#include "tz_error.h"

static Pipe p;

static char *getpipefile()
{
  char *pipefile = getenv("TZ_PIPE");
  if(pipefile==NULL) {
    printf("No pipe defined\n");
    exit(1);
  }

  return pipefile;
}

/* initialize pipe in */
void initpipein() 
{
  char* pipefile = getpipefile();
  FILE *fp = fopen(pipefile,"r");
  char *line = (char *) malloc(100);
  char *tmpline = line;
  char *var,*value;
  int nf = 0; /* number of fields loaded */

  while( !feof(fp) ) {
    if( fgets(line,100,fp)==NULL )
      continue;

    #if defined(_WIN64) || defined(_WIN32) 
	
	var = strtok(line, "=");

	#else
    
	var = strsep(&line, "=");

	#endif
    strtrim(var);
    value  = line;
    strtrim(value);
    if( strcmp(var,"language")==0 ) {
      if( strcmp(value,"c")!=0 ) {
	printf("Invalid pipe: language=%s\n",value);
	exit(1);
      }
      strcpy(p.language,value);
      nf++;
    } else if( strcmp(var,"interface")==0 ) {
      strcpy(p.interface,value);
      nf++;
    } else
      printf("Invalid variable: %s\n",var);
  }

  free(tmpline);
  fclose(fp);

  if(nf!=PIPE_FIELD_NUMBER) {
    printf("Invalid pipe file\n");
    exit(1);
  }  
}

/* initialize pipe out */
void initpipeout(char *language,char* interface)
{
  strcpy(p.language,language);
  strcpy(p.interface,interface);
}

/* implement pipe in */
void *pipein()
{
  return loadpipe(&p);
}

void pipeout()
{
  writepipe(&p);
}

/* load a pipe */
void * loadpipe(Pipe *p)
{
  if(strcmp(p->language,"c") != 0)
    printf("Wrong pipe language.");

  char *interface;
  char *fun,*param[PIPE_INTF_MAXPARAM];

  interface = (char *)malloc(sizeof(char)*PIPE_INTF_SIZE);

  strcpy(interface,p->interface);

  /* extract function name */
  #if defined(_WIN64) || defined(_WIN32) 
	
	fun = strtok(interface, ";");

	#else
    
	fun = strsep(&interface, ";");

	#endif

  strtrim(fun);

  /* Get  parameters */  
  int i = 0;
  char *tmpinterface = interface;

  #if defined(_WIN64) || defined(_WIN32) 
	while ( (param[i]=strtok(interface,";"))!=NULL) {
    TRACE(param[i]);
    i++;
  }
	#else
  while ( (param[i]=strsep(&interface,";"))!=NULL) {
    TRACE(param[i]);
    i++;
  }
  #endif

  /*call function*/
  return funcall(fun,param);

  free(tmpinterface);
}

void writepipe(Pipe *p)
{
  char* pipefile = getpipefile();
  FILE *fp = fopen(pipefile,"w");
  if(fp==NULL) {
    printf("Piping error\n");
    exit(1);    
  }

  fprintf(fp,"language=%s\n",p->language);
  fprintf(fp,"interface=%s\n",p->interface);
  
  fclose(fp);
}

void *funcall(char *fun,char *param[])
{
  if(strcmp(fun,"Read_Stack")==0) {
    return (void *) Read_Stack(param[0]);
  }
  
  if(strcmp(fun,"Read_Stack_Planes")==0) {
    File_Bundle fb;
    fb.prefix = param[0];
    //fb.suffix = param[1];
    fb.num_width = atoi(param[2]);
    fb.first_num = atoi(param[3]);
    return (void *) Read_Stack_Planes(&fb);
  }

  return NULL;
}

