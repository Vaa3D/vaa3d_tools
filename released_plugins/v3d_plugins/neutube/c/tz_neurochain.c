/* tz_neurotrain.c
 *
 * 22-Nov-2007 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <math.h>
#include <utilities.h>
#include <regex.h>
#include <string.h>
#include <dirent.h>
#include "tz_error.h"
#include "tz_string.h"
#include "tz_vrml_io.h"
#include "tz_constant.h"
#include "tz_darray.h"
#include "tz_interface.h"
#include "tz_neurochain.h"
#include "tz_geo3d_utils.h"

INIT_EXCEPTION

Trace_History *New_Trace_History()
{
  Trace_History *th = (Trace_History *) Guarded_Malloc(sizeof(Trace_History),
						       "New_Trace_History");
  Trace_History init = {ZERO_BIT_MASK, 0.0, 0.0, 0.0, 0, 0, FALSE, FORWARD};
  
  *th = init;
  
  return th;
}

void Free_Trace_History(Trace_History *th)
{
  if (th != NULL) {
    free(th);
  }
}

void Trace_History_Copy(const Trace_History *src, Trace_History *des)
{
  memcpy(des, src, sizeof(Trace_History));
}

void Trace_History_Set_Score(Trace_History *th, const Neuroseg_Fit_Score *fs)
{
  if (fs == NULL) {
    return;
  }

  int i;
  for (i = 0; i < fs->n; i++) {
    switch (fs->options[i]) {
    case 0:
      th->fit_score = fs->scores[i];
      Bitmask_Set_Bit(TRACE_HIST_FIT_SCORE_MASK, TRUE, &(th->mask));
      break;
    case 1:
      th->fit_corr = fs->scores[i];
      Bitmask_Set_Bit(TRACE_HIST_FIT_CORR_MASK, TRUE, &(th->mask));
      break;
    case 4:
      th->fit_stat = fs->scores[i];
      Bitmask_Set_Bit(TRACE_HIST_FIT_STAT_MASK, TRUE, &(th->mask));
      break;
    default:
      TZ_WARN(ERROR_DATA_VALUE);
    }
  }
}

void Trace_History_Set_Hit_Region(Trace_History *th, int hit_region)
{
  if (th == NULL) {
    return;
  }

  th->hit_region = hit_region;
  Bitmask_Set_Bit(TRACE_HIST_HIT_REGION_MASK, TRUE, &(th->mask));
}

void Trace_History_Set_Index(Trace_History *th, int index)
{
  if (th == NULL) {
    return;
  }

  th->index = index;
  Bitmask_Set_Bit(TRACE_HIST_INDEX_MASK, TRUE, &(th->mask));
}

void Trace_History_Set_Is_Refit(Trace_History *th, BOOL is_refit)
{
  if (th == NULL) {
    return;
  }

  th->is_refit = is_refit;
  Bitmask_Set_Bit(TRACE_HIST_IS_REFIT_MASK, TRUE, &(th->mask));
}

BOOL Trace_History_Is_Refit(Trace_History *th)
{
  if (th == NULL) {
    return FALSE;
  }

  if (Bitmask_Get_Bit(th->mask, TRACE_HIST_IS_REFIT_MASK) == FALSE) {
    return FALSE;
  }

  return th->is_refit;
}

void Trace_History_Set_Direction(Trace_History *th, Direction_e direction)
{
  if (th == NULL) {
    return;
  }

  th->direction = direction;
  Bitmask_Set_Bit(TRACE_HIST_DIRECTION_MASK, TRUE, &(th->mask));
}

void Fprint_Trace_History(FILE *fp, const Trace_History *th)
{
  if (th == NULL) {
    fprintf(fp, "Null tracing history\n");
  } else if (th->mask == ZERO_BIT_MASK) {
    fprintf(fp, "Tracing history: no information recorded.\n");
  } else {
    fprintf(fp, "Tracing history:\n");
    if (Bitmask_Get_Bit(th->mask, TRACE_HIST_FIT_SCORE_MASK) == TRUE) {
      fprintf(fp, "  Fitting score: %g\n", th->fit_score);
    }
    if (Bitmask_Get_Bit(th->mask, TRACE_HIST_FIT_STAT_MASK) == TRUE) {
      fprintf(fp, "  Fitting stat: %g\n", th->fit_stat);
    }
    if (Bitmask_Get_Bit(th->mask, TRACE_HIST_FIT_CORR_MASK) == TRUE) {
      fprintf(fp, "  Fitting correlation: %g\n", th->fit_corr);
    }
    if (Bitmask_Get_Bit(th->mask, TRACE_HIST_HIT_REGION_MASK) == TRUE) {
      fprintf(fp, "  Hit region: %d\n", th->hit_region);
    }
    if (Bitmask_Get_Bit(th->mask, TRACE_HIST_INDEX_MASK) == TRUE) {
      fprintf(fp, "  Chain index: %d\n", th->index);
    }
    if (Bitmask_Get_Bit(th->mask, TRACE_HIST_IS_REFIT_MASK) == TRUE) {
      fprintf(fp, "  Is refit: %d\n", th->is_refit);
    }
    if (Bitmask_Get_Bit(th->mask, TRACE_HIST_DIRECTION_MASK) == TRUE) {
      fprintf(fp, "  Tracing direction: ");
      if (th->direction == FORWARD) {
	fprintf(fp, "FORWARD\n");
      } else {
	fprintf(fp, "BACKWARD\n");
      }
    }
  }
}

void Print_Trace_History(const Trace_History *th)
{
  Fprint_Trace_History(stdout, th);
}

void Trace_History_Fwrite(Trace_History *hist, FILE *fp)
{
  Bitmask_t mask = ZERO_BIT_MASK;
  if (hist != NULL) {
    mask = hist->mask;
  } 
  if (mask == ZERO_BIT_MASK) {
    fwrite(&mask, sizeof(Bitmask_t), 1, fp);
  } else {
    fwrite(hist, sizeof(Trace_History), 1, fp);
  }
}

Trace_History *Trace_History_Fread(FILE *fp)
{
  Trace_History *hist = NULL;
  Bitmask_t mask = ZERO_BIT_MASK;
  fread(&mask, sizeof(Bitmask_t), 1, fp);
  if (mask != ZERO_BIT_MASK) {
    hist = New_Trace_History();
    hist->mask = mask;
    fread((char*)hist + sizeof(Bitmask_t), 
	  sizeof(Trace_History) - sizeof(Bitmask_t), 1, fp);
  }

  return hist;
}

/* New_Neurochain(): New a neurochain node.
 * 
 * Return: a neuronchain node.
 */
Neurochain* New_Neurochain()
{
  Neurochain *chain = Guarded_Malloc(sizeof(Neurochain), "New_Neurochain");
  Default_Local_Neuroseg(&(chain->locseg));
  chain->hist = NULL;
  chain->next = NULL;
  chain->prev = NULL;

  return chain;
}

/* Free_Neurochain(): Free all memories asigned to chain.
 *
 * Note: All nodes, either in front of or behind the current node, will be 
 *       freed. For safety reason, the caller had better set <chain> to NULL 
 *       after freeing.
 */
void Free_Neurochain(Neurochain *chain)
{
  if (chain == NULL) {
    return;
  }

  chain = Neurochain_Head(chain);
  Neurochain *tmpchain = chain->next;

  while(chain != NULL) {
    tmpchain = chain;
    if (chain->hist != NULL) {
      Free_Trace_History(chain->hist);
    }
    free(chain);
    chain = tmpchain->next;
  }
}

/* Neurochain_Next(): Get the next or previous node of <chain>.
 * 
 * Args: chain - the chain to iterate;
 *       d - direction to iterate.
 *
 * Return: the next node if d is forward, previous node if d is backward. It 
 *         returns NULL for a NULL chain.
 */
Neurochain *Neurochain_Next(const Neurochain *chain, Direction_e d)
{						
  if (chain != NULL) {
    if (d == FORWARD) {
      return chain->next;
    } else if (d == BACKWARD) {
      return chain->prev;
    } else {
      THROW(ERROR_DATA_VALUE);
    }
  }

  return NULL;
}

/* Neurochain_Head(Neurochain *chain): Get first non-null node node if possible.
 *
 * Args: chain - a neuron chain. It could be any non-null node of the chain.
 *
 * Return: the head of the chain.
 */
Neurochain *Neurochain_Head(const Neurochain *chain)
{
  if (chain == NULL) {
    return NULL;
  }

  while (chain->prev != NULL) {
    chain = chain->prev;
  }

  return (Neurochain *) chain; /* Type conversion was added to 
				  avoid compiling warning */
}

/* Neurochain_Tail(Neurochain *chain): Get last non-null node if possible.
 */
Neurochain *Neurochain_Tail(const Neurochain *chain)
{
  if (chain == NULL) {
    return NULL;
  }

  while (chain->next != NULL) {
    chain = chain->next;
  }

  return (Neurochain *) chain;
}

/* Neurochain_Length(): Number of the nodes in the chain.
 *
 * Note: The number is counted from the current node.
 */
int Neurochain_Length(const Neurochain *chain, Direction_e d)
{
  int length = 0;

  switch (d) {
  case FORWARD:
    for(; chain != NULL; chain = chain->next) {
      length++;
    }
    break;
  case BACKWARD:
    for(; chain != NULL; chain = chain->prev) {
      length++;
    }
    break;
  case BOTH:
    length = Neurochain_Length(chain, BACKWARD) + 
      Neurochain_Length(chain->next, FORWARD);
    break;
  default:
    THROW(ERROR_DATA_VALUE);
  }

  return length;
}

/* Locate_Neurochain(): Locate to a certain node.
 *
 * Args: chain - original chain;
 *       p - position to be located. It goes backward if the value is 
 *           negative;
 *
 * Return: the pointer to the located node. It returns NULL if the position 
 *         is out of range.
 *
 * Note: It starts from the currrent positon and 0-indexed.
 */
Neurochain *Locate_Neurochain(const Neurochain *chain, int p)
{
  if (chain == NULL) {
    return NULL;
  }

  int i;
  
  if (p > 0) {
    for (i = 0; i < p; i++) {
      chain = chain->next;
      if (chain == NULL) {
	return NULL;
      }
    }
  } else if (p < 0){
    for (i = 0; i < -p;  i++) {
      chain = chain->prev;
      if (chain == NULL) {
	return NULL;
      }
    }
  }

  return (Neurochain *)  chain;
}



/* Init_Neurochain(): Initialize a neurochain node.
 *
 * Args: chain - node to initialize;
 *       seg - neuron segment;
 *       pos - segment position.
 *
 * Return: the initialized node.
 */
Neurochain* Init_Neurochain(Neurochain *chain,  const Local_Neuroseg *seg)
{
  if (chain != NULL) {
    if (seg != NULL) {
      Local_Neuroseg_Copy(&(chain->locseg), seg);
    }
  }

  return chain;
}

/* Append_Neurochain(): append one chain to the other.
 *
 * Args: chain1 - the first chain;
 *       chain2 - the second chain.
 *
 * Return: the head of the resulted chain.
 *
 * Note: chain2 is appended to chain1.
 */
Neurochain* Append_Neurochain(Neurochain *chain1, Neurochain *chain2)
{
  if (chain1 == NULL)
    return chain2;

  if (chain2 == NULL)
    return chain1;

  chain1 = Neurochain_Tail(chain1);
  chain2 = Neurochain_Head(chain2);

  chain1->next = chain2;
  chain2->prev = chain1;
 
  return Neurochain_Head(chain1);
}

/*
 * Neurochain_Remove_Last(): removes the last n nodes from a neuron chain.
 * 
 * Args: chain - current node of the chain.
 *       n - number of nodes to remove.
 *
 * Return: The number of nodes that are actually removed.
 */
int Neurochain_Remove_Last(Neurochain *chain, int n)
{
  if (n <= 0) {
    return 0;
  }

  if (chain == NULL) {
    return 0;
  }

  if (chain->next == NULL) {
    return 0;
  }

  int nremove = 0;
  Neurochain *tail = Neurochain_Tail(chain);
  Neurochain *remove;

  while ((nremove < n) && (chain != tail)){    
    remove = tail;
    tail = tail->prev;
    remove->prev = NULL;
    tail->next = NULL;

    /* free remove after remove and tail are set */
    Free_Neurochain(remove);

    nremove++;
  }

  return nremove;
}

/* Neurochain_Cut_Last(): Cut a neuron chain.
 *
 * Args: chain - current node of the chain.
 *       n - number of nodes to cut.
 *
 * Return: The cut-off part.
 */
Neurochain* Neurochain_Cut_Last(Neurochain *chain, int n)
{
  if (n <= 0) {
    return NULL;
  }

  if (chain == NULL) {
    return NULL;
  }  

  if (chain->next == NULL) {
    return NULL;
  }
  
  Neurochain *cutoff = NULL;

  int length = Neurochain_Length(chain, FORWARD);
  if (length <= n + 1) {
    cutoff = chain->next;
    chain->next = NULL;
    cutoff->prev = NULL;
  } else {
    Neurochain *remain = NULL;
    cutoff = Locate_Neurochain(chain, length - n);
    remain = cutoff->prev;
    remain->next = NULL;
    cutoff->prev = NULL;
  }

  return cutoff;
}

/* Neurochain_Cut_At(): cut a neuron chain at a certain position.
 *
 * Args: chain - chain at the current node;
 *       n - cut position.
 *
 * Return: cut-off chain.
 */
Neurochain* Neurochain_Cut_At(Neurochain *chain, int n)
{
  if (n < 1) {
    return NULL;
  }

  Neurochain *cutoff = NULL;
  Neurochain *remain = NULL;
  cutoff = Locate_Neurochain(chain, n);

  if (cutoff != NULL) {
    remain = cutoff->prev;
    remain->next = NULL;
    cutoff->prev = NULL;
  }

  return cutoff;
}

Neurochain* Neurochain_Cut(Neurochain *chain)
{
  if (chain == NULL) {
    return NULL;
  }

  Neurochain *cutoff = chain->prev;

  if (cutoff != NULL) {
    cutoff->next = NULL;
    chain->prev = NULL;
  }

  return cutoff;
}

/* Elongate_Neurochain(): Elongate neuron chain by one segment.
 *
 * Args: chain - neuron chain to extend;
 *       d - elongating direction.
 *       
 */
#define POS_STEP 0.4
Neurochain* Elongate_Neurochain(Neurochain *chain, Direction_e d)
{
  if (chain == NULL) {
    return NULL;
  }
  
  Neurochain* chain2 = NULL;
  
#define ELONGATE_NEUROCHAIN_NEXT_NODE					\
  chain2 = New_Neurochain();						\
  Next_Local_Neuroseg(&(chain->locseg), &(chain2->locseg), POS_STEP);	\

  switch (d) {
  case FORWARD:
    chain = Neurochain_Tail(chain);
    ELONGATE_NEUROCHAIN_NEXT_NODE
    Append_Neurochain(chain, chain2);
    break;
  case BACKWARD:
    chain = Neurochain_Head(chain);
    ELONGATE_NEUROCHAIN_NEXT_NODE
    Append_Neurochain(chain2, chain);
    break;
  case BOTH:
    Elongate_Neurochain(chain, FORWARD);
    chain2 = Elongate_Neurochain(chain, BACKWARD);
    break;
  default:
    break;
  }

#undef ELONGATE_NEUROCHAIN_NEXT_NODE
  
  return chain2;
}

/* Elongate_Neurochain_Spec(): Elongate neuron chain by a copy of a specified
 *                             neuron segment.
 *
 * Args: chain - neuron chain to extend;
 *       d - elongating direction;
 *       seg - the specified neuron segment.
 *       
 * Return: the new node.
 */
Neurochain* Elongate_Neurochain_Spec(Neurochain *chain, Direction_e d,
				     const Local_Neuroseg *seg,
				     const Trace_History *hist)
{
  if (chain == NULL) {
    return NULL;
  }
  
  Neurochain* chain2 = NULL;
  
#define ELONGATE_NEUROCHAIN_NEXT_NODE					\
  chain2 = New_Neurochain();						\
  if (seg != NULL) {							\
    Local_Neuroseg_Copy(&(chain2->locseg), seg);			\
  }									\
  if (hist != NULL) {							\
    chain2->hist = New_Trace_History();					\
    Trace_History_Copy(hist, chain2->hist);				\
  }

  switch (d) {
  case FORWARD:
    chain = Neurochain_Tail(chain);
    ELONGATE_NEUROCHAIN_NEXT_NODE
    Append_Neurochain(chain, chain2);
    break;
  case BACKWARD:
    chain = Neurochain_Head(chain);
    ELONGATE_NEUROCHAIN_NEXT_NODE
    Append_Neurochain(chain2, chain);
    break;
  case BOTH:
    Elongate_Neurochain_Spec(chain, FORWARD, seg, hist);
    chain2 = Elongate_Neurochain_Spec(chain, BACKWARD, seg, hist);
    break;
  default:
    break;
  }

#undef ELONGATE_NEUROCHAIN_NEXT_NODE
  
  return chain2;
}

/* Extend_Neurochain(): Extend the neurochain.
 * 
 * Args: chain - chain to extend.
 *
 * Return: the new tail of the chain. It returns NULL if the input is NULL.
 *
 * Note: The new segment will be added to the end.
 */
Neurochain* Extend_Neurochain(Neurochain *chain)
{
  return Elongate_Neurochain(chain, FORWARD);
}

/* Extend_Neurochain_Spec(): Extend the neurochain by a specified neuron 
 *                           segment.
 * 
 * Args: chain - chain to extend;
 *       seg - the specified neuron segment.
 *
 * Return: the new tail of the chain. It returns NULL if the input is NULL.
 *
 * Note: The new segment will be added to the end.
 */
Neurochain* Extend_Neurochain_Spec(Neurochain *chain, 
				   const Local_Neuroseg *seg,
				   const Trace_History *hist)
{
  return Elongate_Neurochain_Spec(chain, FORWARD, seg, hist);
}


/* Shrink_Neurochain(): Shrink a neuron chain.
 *
 * Args: chain - the neuron chain to shrink;
 *       ne - option for removing head or/and tail.
 *
 * Return: the removed node.
 *
 * Note: the remove node is just returned but not freed so that the user can
 *       decide how to deal with it. <chain> is set to a pointer to pointer
 *       to be modifiable.
 */
Neurochain* Shrink_Neurochain(Neurochain **pchain, Neurochain_End_e ne)
{
  if (pchain != NULL) {
    Neurochain *head = Neurochain_Head(*pchain);
    Neurochain *tail = Neurochain_Tail(*pchain);
    
    if (head == NULL) {
      return NULL;
    }
    
    if (head == tail){
      *pchain = NULL;
      return head;
    }
    
    
    switch (ne) {
    case NEUROCHAIN_HEAD:
      head->next->prev = NULL;
      if (*pchain == head) {
	*pchain = head->next;
      }
      head->next = NULL;
      return head;
    case NEUROCHAIN_TAIL:
      tail->prev->next = NULL;
      if (*pchain == tail) {
	*pchain = tail->prev;
      }
      tail->prev = NULL;
      return tail;
    case NEUROCHAIN_BOTH_END:
      if (head->next == tail) {
	*pchain = NULL;
	return head;
      }
      
      head = Shrink_Neurochain(pchain, NEUROCHAIN_HEAD);
      tail = Shrink_Neurochain(pchain, NEUROCHAIN_TAIL);
      if (*pchain!= NULL) {
	head->next = tail;
	tail->prev = head;      
      }
      return head;
    default:
      THROW(ERROR_DATA_VALUE);
      break;
    }
  }

  return NULL;
}

/* Cross_Neurochain(): Cross two neuron chains.
 *
 * Args: chain1 - the first chain to cross;
 *       chain2 - the second chain to cross.
 *
 * Return: void.
 */
void Cross_Neurochain(Neurochain *chain1, Neurochain *chain2)
{
  if ((chain1 == NULL) || (chain2 == NULL)) {
    return;
  }

  if ((chain1->prev == NULL) && (chain2->prev == NULL)) {
    return;
  }

  if (chain1->prev == NULL) {
    chain2->prev->next = chain1;
    chain1->prev = chain2->prev;
    chain2->prev = NULL;
    return;
  }

  if (chain2->prev == NULL) {
    chain1->prev->next = chain2;
    chain2->prev = chain1->prev;
    chain1->prev = NULL;
    return;
  }

  chain2->prev->next = chain1;
  chain1->prev->next = chain2;
  Neurochain *tmp_chain = chain1->prev;
  chain1->prev = chain2->prev;
  chain2->prev = tmp_chain;
}

void Fprint_Neurochain(FILE *fp, const Neurochain *chain)
{
  if (chain == NULL) {
    printf("Empty chain\n");
    return;
  }

  int n = 1;

  while (chain != NULL) {
    printf("Node: %d\n", n);
    Fprint_Local_Neuroseg(fp, &(chain->locseg));
    Fprint_Trace_History(fp, chain->hist);
    chain = chain->next;
    n++;
  }  
}

/* Print_Neurochain(): Print neuron chain to standard output.
 *
 * Args: chain - neuron chain to print.
 *
 * Return: void.
 *
 * Note: It iterates the chain forward only.
 */
void Print_Neurochain(const Neurochain *chain)
{
  Fprint_Neurochain(stdout, chain);
}

/* Print_Neurochain_Info(): Prints brief information of a neuron chain.
 *
 * Args: chain - neuron chain to print.
 *
 * Return: void.
 */
void Print_Neurochain_Info(const Neurochain *chain)
{
  if (chain == NULL) {
    printf("Empty chain\n");
    return;
  }
  
  printf("Neuron chain:\n");
  printf("Total length: %d; Current node: %d\n", Neurochain_Length(chain, BOTH),
	 Neurochain_Length(chain, BACKWARD) - 1);
}

void Write_Neurochain(const char *file_path, const Neurochain *chain)
{
  FILE *fp = fopen(file_path, "w");
  if (fp == NULL) {
    TZ_WARN(ERROR_IO_OPEN);
    return;
  }
  
  Neurochain_Fwrite(chain, fp);
  fclose(fp);
}

Neurochain* Read_Neurochain(const char *file_path)
{
  FILE *fp = fopen(file_path, "r");
  if (fp == NULL) {
    TZ_WARN(ERROR_IO_OPEN);
    return NULL;
  }

  Neurochain *chain = New_Neurochain();
  Neurochain_Fread(chain, fp);
  fclose(fp);
  
  return chain;
}  

Neurochain** Dir_Neurochain(const char *dir_name, const char *ext, int *n)
{
  *n = dir_fnum(dir_name, ext);
  if (*n == 0) {
    return NULL;
  }

  Neurochain **chain_array = 
    (Neurochain **) Guarded_Malloc(sizeof(Neurochain *) * (*n), 
				   "Dir_Neurochain");
  
  DIR *dir = opendir(dir_name);
  struct dirent *ent = readdir(dir);
  int index = 0;

  /* allocate a string for storing full path. 100 is just an initial guess */
  char *full_path = (char *) Guarded_Malloc(strlen(dir_name) + 100, 
					    "Dir_Neurochain");
  strcpy(full_path, dir_name);  
  strcat(full_path, "/");
			 
  while (ent != NULL) {
    const char *extracted_ext = fextn(ent->d_name);
    if (extracted_ext != NULL) {
      if (strcmp(extracted_ext, ext) ==0) {
	int path_length = strlen(dir_name) + strlen(ent->d_name) + 1;
	if (path_length > strlen(full_path)) {
	  full_path = (char *) Guarded_Realloc(full_path, path_length + 1, 
					       "Dir_Neurochain");
	}
	strcpy(full_path + strlen(dir_name) + 1, ent->d_name);
	printf("%s\n", full_path);
	chain_array[index++] = Read_Neurochain(full_path);
	Print_Neurochain_Info(chain_array[index - 1]);
      }
    }
    ent = readdir(dir);
  }
  free(full_path);
  closedir(dir);

  return chain_array;
}

Neurochain** Dir_Neurochain_P(const char *dir_name, const char *pattern, int *n)
{
  *n = dir_fnum_p(dir_name, pattern);
  if (*n == 0) {
    return NULL;
  }

  Neurochain **chain_array = 
    (Neurochain **) Guarded_Malloc(sizeof(Neurochain *) * (*n), 
				   "Dir_Neurochain");
  
  DIR *dir = opendir(dir_name);
  struct dirent *ent = readdir(dir);
  int index = 0;

  /* allocate a string for storing full path. 100 is just an initial guess */
  char *full_path = (char *) Guarded_Malloc(strlen(dir_name) + 100, 
					    "Dir_Neurochain");
  strcpy(full_path, dir_name);  
  strcat(full_path, "/");
	
  regex_t preg;
  regcomp(&preg, pattern, REG_BASIC);

  while (ent != NULL) {
    if (regexec(&preg, ent->d_name, 0, NULL, REG_BASIC) ==0) {
      int path_length = strlen(dir_name) + strlen(ent->d_name) + 1;
      if (path_length > strlen(full_path)) {
	full_path = (char *) Guarded_Realloc(full_path, path_length + 1, 
					     "Dir_Neurochain");
      }
      strcpy(full_path + strlen(dir_name) + 1, ent->d_name);
      printf("%s\n", full_path);
      chain_array[index++] = Read_Neurochain(full_path);
      Print_Neurochain_Info(chain_array[index - 1]);
    }
    ent = readdir(dir);
  }
  free(full_path);
  closedir(dir);

  regfree(&preg);

  return chain_array;
}

Neurochain** Dir_Neurochain_D(const char *dir_name, const char *pattern, int *n,
			      char *file[])
{
  *n = dir_fnum_p(dir_name, pattern);
  if (*n == 0) {
    return NULL;
  }

  Neurochain **chain_array = 
    (Neurochain **) Guarded_Malloc(sizeof(Neurochain *) * (*n), 
				   "Dir_Neurochain");
  
  DIR *dir = opendir(dir_name);
  struct dirent *ent = readdir(dir);
  int index = 0;

  /* allocate a string for storing full path. 100 is just an initial guess */
  char *full_path = (char *) Guarded_Malloc(strlen(dir_name) + 100, 
					    "Dir_Neurochain");
  strcpy(full_path, dir_name);  
  strcat(full_path, "/");
	
  regex_t preg;
  regcomp(&preg, pattern, REG_BASIC);

  while (ent != NULL) {
    if (regexec(&preg, ent->d_name, 0, NULL, REG_BASIC) ==0) {
      int path_length = strlen(dir_name) + strlen(ent->d_name) + 1;
      if (path_length > strlen(full_path)) {
	full_path = (char *) Guarded_Realloc(full_path, path_length + 1, 
					     "Dir_Neurochain");
      }
      strcpy(full_path + strlen(dir_name) + 1, ent->d_name);
//      printf("%s\n", full_path);
      if (file != NULL) {
	strcpy(file[index], full_path);
      }
      chain_array[index++] = Read_Neurochain(full_path);
//      Print_Neurochain_Info(chain_array[index - 1]);
    }
    ent = readdir(dir);
  }
  free(full_path);
  closedir(dir);

  regfree(&preg);

  return chain_array;
}

Neurochain** Dir_Neurochain_N(const char *dir_name, const char *pattern, int *n,
			      int *file_num)
{
  *n = dir_fnum_p(dir_name, pattern);
  if (*n == 0) {
    return NULL;
  }

  Neurochain **chain_array = 
    (Neurochain **) Guarded_Malloc(sizeof(Neurochain *) * (*n), 
				   "Dir_Neurochain");
  
  DIR *dir = opendir(dir_name);
  struct dirent *ent = readdir(dir);
  int index = 0;

  /* allocate a string for storing full path. 100 is just an initial guess */
  char *full_path = (char *) Guarded_Malloc(strlen(dir_name) + 100, 
					    "Dir_Neurochain");
  strcpy(full_path, dir_name);  
  strcat(full_path, "/");
	
  regex_t preg;
  regcomp(&preg, pattern, REG_BASIC);

  while (ent != NULL) {
    if (regexec(&preg, ent->d_name, 0, NULL, REG_BASIC) ==0) {
      int path_length = strlen(dir_name) + strlen(ent->d_name) + 1;
      if (path_length > strlen(full_path)) {
	full_path = (char *) Guarded_Realloc(full_path, path_length + 1, 
					     "Dir_Neurochain");
      }
      strcpy(full_path + strlen(dir_name) + 1, ent->d_name);
      printf("%s\n", full_path);
      if (file_num != NULL) {
	file_num[index] = String_Last_Integer(ent->d_name);
      }
      chain_array[index++] = Read_Neurochain(full_path);
      Print_Neurochain_Info(chain_array[index - 1]);
    }
    ent = readdir(dir);
  }
  free(full_path);
  closedir(dir);

  regfree(&preg);

  return chain_array;
}


void Neurochain_Fwrite(const Neurochain *chain, FILE *fp)
{
  if (chain != NULL) {
    int length = Neurochain_Length(chain, FORWARD);
    fwrite(&length, sizeof(int), 1, fp);
  }

  while (chain != NULL) {
    Local_Neuroseg_Fwrite(&(chain->locseg), fp);
    Trace_History_Fwrite(chain->hist, fp);
    chain = chain->next;
  }
}

void Neurochain_Fread(Neurochain *chain, FILE *fp)
{
  if ((chain == NULL) || (fp == NULL)) {
    TZ_ERROR(ERROR_POINTER_NULL);
  }

  if (chain->next != NULL) {
    fprintf(stderr, "Possible memory leak.");
    TZ_WARN(ERROR_DATA_VALUE);
  }

  int length;
  fread(&length, sizeof(int), 1, fp);

  if (length < 0) {
    printf("%d\n", length);
    TZ_ERROR(ERROR_DATA_VALUE);
  }

  int i;
  for (i = 0; i < length; i++) {
    Local_Neuroseg_Fread(&(chain->locseg), fp);
    chain->hist = Trace_History_Fread(fp);
    if (i < length - 1) {
      chain = Extend_Neurochain_Spec(chain, NULL, NULL);
    }
  }
}

void Neurochain_Fprint_Vrml(const Neurochain *chain, FILE *fp)
{
  if ((chain == NULL) || (fp == NULL)) {
    return;
  }

  while(chain != NULL) {
    Local_Neuroseg_Fprint_Vrml(&(chain->locseg), fp);
    chain = chain->next;
  }
}

void Write_Neurochain_Vrml( const char *file_path, const Neurochain *chain)
{
  FILE *fp = fopen(file_path, "w");
  fprintf(fp, "#VRML V2.0 utf8\n");
  //Neurochain_Fprint_Vrml(chain, fp);
  Vrml_Neurochain_Fprint(fp, chain, 0);
  fclose(fp);
}

/* Neurochain_Label(): draw a neuron chain in a stack. 
 *
 * Args: stack - input stack, which is supposed to have three channels;
 *       chain - neuron chain model;
 *
 * Return: void.
 *
 * Note: It iterates the chain forward only.
 */
void Neurochain_Label(Stack *stack, const Neurochain *chain, double z_scale)
{
  int color = 0;

  while (chain != NULL) {
#if 0
    Print_Local_Neuroseg(&(chain->locseg));
#endif
    if (Trace_History_Is_Refit(chain->hist) == TRUE) {
      Local_Neuroseg_Label(&(chain->locseg), stack, 2, z_scale);
    } else {
      Local_Neuroseg_Label(&(chain->locseg), stack, color, z_scale);
    }
    chain = chain->next;
    if (color == 0) {
      color = 1;
    } else {
      color = 0;
    }
  }
}

void Neurochain_Label_E(Stack *stack, const Neurochain *chain, double z_scale,
			int begin, int end, double ratio, double diff)
{
  int color = 0;
  Local_Neuroseg *tmp_seg = New_Local_Neuroseg();

  int i = 0;

  while (chain != NULL) {
    if ((i >= begin) && (i <= end)) {
      Local_Neuroseg_Copy(tmp_seg, &(chain->locseg));
      Neuroseg_Change_Thickness(&(tmp_seg->seg), ratio, diff);
      Local_Neuroseg_Label(tmp_seg, stack, color, z_scale);
      if (color == 0) {
	color = 1;
      } else {
	color = 0;
      }	 
    }
    chain = chain->next;
    i++;
  }

  Delete_Local_Neuroseg(tmp_seg);
}

/* Neurochain_Erase(): Erase a stack by a neuron chain.
 *
 * Args: stack - stack to erase;
 *       chain - erasing neuron chain.
 *
 * Return: void.
 *
 * Note: It iterates forward only.
 */
void Neurochain_Erase(Stack *stack, const Neurochain *chain, double z_scale)
{
  int color = -1;

  while (chain != NULL) {
    Local_Neuroseg_Label(&(chain->locseg), stack, color, z_scale);
    chain = chain->next;
  }
}

void Neurochain_Erase_E(Stack *stack, const Neurochain *chain, double z_scale,
			int begin, int end, double ratio, double diff)
{
  int color = -1;

  Local_Neuroseg *tmp_seg = New_Local_Neuroseg();

  int i = 0;

  while (chain != NULL) {
    if ((i >= begin) && (i <= end)) {
      Local_Neuroseg_Copy(tmp_seg, &(chain->locseg));
      Neuroseg_Change_Thickness(&(tmp_seg->seg), ratio, diff);
      Local_Neuroseg_Label(tmp_seg, stack, color, z_scale);
    }
    chain = chain->next;
    i++;
  }

  Delete_Local_Neuroseg(tmp_seg);
}

void Neurochain_Label_G(Stack *stack, const Neurochain *chain, double z_scale,
			int begin, int end, double ratio, double diff, 
			int value)
{
  Local_Neuroseg *tmp_seg = New_Local_Neuroseg();

  int i = 0;

  while (chain != NULL) {
    if ((i >= begin) && (i <= end)) {
      Local_Neuroseg_Copy(tmp_seg, &(chain->locseg));
      Neuroseg_Change_Thickness(&(tmp_seg->seg), ratio, diff);
      Local_Neuroseg_Label_G(tmp_seg, stack, -1, value, z_scale);
    }
    chain = chain->next;
    i++;
  }

  Delete_Local_Neuroseg(tmp_seg);
}

/* Neurochain_Translate(): Translate a neuron chain.
 *
 * Args: chain - neuron chain to translate;
 *       x, y, z - translation offset.
 *
 * Return: void.
 */
void Neurochain_Translate(Neurochain *chain, double x, double y, double z)
{
  chain = Neurochain_Head(chain);
  while (chain != NULL) {
    Neuropos_Translate(chain->locseg.pos, x, y, z);
  }
}

/* Neurochain_Score(): Calculate the total score of a neuron chain.
 *
 * Note: It iterates forward only.
 */
double Neurochain_Score(const Neurochain *chain, const Stack *stack, 
			double z_scale, Neuroseg_Fit_Score *fs)
{
  double score = 0.0;

  while (chain != NULL) {
    score += Local_Neuroseg_Score(&(chain->locseg), stack, z_scale, fs);
    //printf("score: %g\n", score);
    chain = chain->next;
  }

  return score;
}

double Neurochain_Geolen(const Neurochain *chain)
{
  if (chain == NULL) {
    return 0.0;
  }

  chain = Neurochain_Head(chain);
  if (chain->next == NULL) {
    return chain->locseg.seg.h;
  }

  double dist = chain->locseg.seg.h / 2.0;
  double pos1[3];
  double pos2[3];
  Local_Neuroseg_Center(&(chain->locseg), pos1);
  while (chain->next != NULL) {
    Local_Neuroseg_Center(&(chain->next->locseg), pos2);
    dist += sqrt(Geo3d_Dist_Sqr(pos1[0], pos1[1], pos1[2], 
				pos2[0], pos2[1], pos2[2]));
    pos1[0] = pos2[0];
    pos1[1] = pos2[1];
    pos1[2] = pos2[2];
    chain = chain->next;
  }
  dist += chain->locseg.seg.h / 2.0;

  return dist;
}

double Neurochain_Thickness_Mean(const Neurochain *chain)
{
  double thickness = 0.0;
  int length = 0;
  for (; chain != NULL; chain = chain->next) {
    thickness += (chain->locseg.seg.r1 + NEUROSEG_R2(&(chain->locseg.seg))) / 2.0;
    length++;
  }

  if (length > 0) {
    thickness /= (double) length;
  }

  return thickness;
}

double Neurochain_Thickness_Var(const Neurochain *chain)
{
  double x = 0.0;
  double x2 = 0.0;
  double thickness = 0.0;

  int length = 0;
  
  for (; chain != NULL; chain = chain->next) {
    thickness = (chain->locseg.seg.r1 + NEUROSEG_R2(&(chain->locseg.seg))) / 2.0;
    x += thickness;
    x2 += thickness * thickness;
    length++;
  }

  double var = 0.0;
  if (length > 0) {
    x /= length;
    x2 /= length;
    var = x2 - x * x;
  }

  return var;
}

double Neurochain_Thickness_Change_Mean(const Neurochain *chain)
{
  double thickness = 0.0;
  int length = 0;
  for (; chain != NULL; chain = chain->next) {
    thickness += 2.0 * fabs(NEUROSEG_R2(&(chain->locseg.seg)) - chain->locseg.seg.r1) / 
      (NEUROSEG_R2(&(chain->locseg.seg)) + chain->locseg.seg.r1) / 
      chain->locseg.seg.h;
    length++;
  }

  if (length > 0) {
    thickness /= (double) length;
  }

  return thickness;
}

double Neurochain_Angle_Mean(const Neurochain *chain)
{
  double angle = 0.0;
  int length = 0;
  
  for (; chain->next != NULL; chain = chain->next) {
    angle += Neuroseg_Angle_Between(&(chain->locseg.seg), 
				    &(chain->next->locseg.seg));
    length++;
  }

  if (length > 0) {
    angle /= (double) length;
  }

  return angle;
}

/* Neurochain_Hit_Test(): Test if a point overlaps a chain.
 *
 * Args: chain - the chain to be tested;
 *       nseg - number of segments to iterate. If nseg is 0, all the segments
 *              will be iterated at the spcified direction;
 *       d - iterate forward or backward;
 *       x, y, z - the coordinate of the point.
 *
 * Return: the index (1-indexing) of the segment overlapping the point if they 
 *         overlap, 0 if they do not.
 */
int Neurochain_Hit_Test(const Neurochain *chain,  Direction_e d, int nseg,
			double x, double y, double z)
{
  int i = 1;
  for (; chain != NULL; chain = Neurochain_Next(chain, d)) { 
    if (Local_Neuroseg_Hit_Test(&(chain->locseg), x, y, z) == TRUE) {
      return i;
    }

    if (nseg > 0) {
      if (i >= nseg) {
	return 0;
      }
    }

    i++;
  }

  return 0;
}

int Neurochain_Hit_Test_Seg(const Neurochain *chain, Direction_e d, int nseg,
			    const Local_Neuroseg *locseg)
{
  int i = 0;
  double pos[3];
  double bottom[3];
  Local_Neuroseg_Bottom(locseg, bottom);
  double z;

  for (; chain != NULL; chain = Neurochain_Next(chain, d)) { 
    i++;
    
    double center1[3];
    double center2[3];
    Local_Neuroseg_Center(&(chain->locseg), center1);
    Local_Neuroseg_Center(locseg, center2);

    double cdist_sqr = Geo3d_Dist_Sqr(center1[0], center1[1], center1[2],
				      center2[0], center2[1], center2[2]);

    double max_dist = 
      dmax3(chain->locseg.seg.h / 2.0, chain->locseg.seg.r1, 
	    NEUROSEG_R2(&(chain->locseg.seg))) +
      dmax3(locseg->seg.h / 2.0, locseg->seg.r1, NEUROSEG_R2(&(chain->locseg.seg)));
    if (cdist_sqr > max_dist * max_dist) {
      continue;
    }
       
    for (z = 0.0; z <= locseg->seg.h; z += 2.0) {
      Neuroseg_Axis_Offset(&(locseg->seg), z, pos);
      pos[0] += bottom[0];
      pos[1] += bottom[1];
      pos[2] += bottom[2];
      
      if (Local_Neuroseg_Hit_Test(&(chain->locseg), pos[0], pos[1], pos[2])
	  == TRUE) {
	if (chain->next != NULL) {
	  double center3[3];
	  Local_Neuroseg_Center(&(chain->next->locseg), center3);
	  double cdist_sqr_next = 
	    Geo3d_Dist_Sqr(center3[0], center3[1], center3[2],
			   center2[0], center2[1], center2[2]);
	  if (cdist_sqr_next < cdist_sqr) {
	    i++;
	  }
	}
	return i;
      }
    }

    if (nseg > 0) {
      if (i >= nseg) {
	return 0;
      }
    }
  }

  return 0;
}

void Neurochain_Hit_Stack(const Neurochain *chain, Stack *region, int hit[])
{  
  coordinate_3d_t pos[5];
  int i;
  int x, y, z;
  hit[0] = 0;
  hit[1] = 0;

  int nsample = imin2(Neurochain_Length(chain, BOTH) / 2,  5);
  if (nsample == 0) {
    nsample = 1;
  }

  int n = Neurochain_End_Points(chain, nsample, pos);
  for (i = 0; i < n; i++) {
    x = (int) (pos[i][0] + 0.5);
    y = (int) (pos[i][1] + 0.5);
    z = (int) (pos[i][2] + 0.5);
    if ((x < 0) || (y < 0) || (z < 0) || (x >= region->width) ||
	(y >= region->height) || (z >= region->depth)) {
      hit[0] = -1;
    } else {
      int hit_value = (int) Get_Stack_Pixel(region, x, y, z, 0);
      if (hit_value > 0) {
	hit[0] = imax2(hit[0], hit_value);
      }
    }
  }

  n = Neurochain_End_Points(chain, -nsample, pos); 

  for (i = 0; i < n; i++) {
    x = (int) (pos[i][0] + 0.5);
    y = (int) (pos[i][1] + 0.5);
    z = (int) (pos[i][2] + 0.5);
    if ((x < 0) || (y < 0) || (z < 0) || (x >= region->width) ||
	(y >= region->height) || (z >= region->depth)) {
      hit[1] = -1;
    } else {
      int hit_value = (int) Get_Stack_Pixel(region, x, y, z, 0);
      if ((hit_value > 0) && (hit_value != hit[0])) {
	hit[1] = imax2(hit[1], hit_value);
      }
    }
  }

  if ((hit[0] == 20) || (hit[1] == 20)) {
    printf("testing point\n");
  }
}

BOOL Neurochain_Hit_Bound(const Neurochain *chain, int width, int height,
			  int depth, int margin)
{
  Neurochain *head = Neurochain_Head(chain);
  Neurochain *tail = Neurochain_Tail(chain);
  
  width -= margin;
  height -= margin;
  depth -= margin;

  double pos[3];
  Local_Neuroseg_Bottom(&(head->locseg), pos);
  int x, y, z;
  x = (int) (pos[0] + 0.5);
  y = (int) (pos[1] + 0.5);
  z = (int) (pos[2] + 0.5);
  if ((x < margin) || (y < margin) || (z < margin) || (x >= width) ||
      (y >= height) || (z >= depth)) {
    return TRUE;
  }

  Local_Neuroseg_Top(&(tail->locseg), pos);
  x = (int) (pos[0] + 0.5);
  y = (int) (pos[1] + 0.5);
  z = (int) (pos[2] + 0.5);
  if ((x < margin) || (y < margin) || (z < margin) || (x >= width) ||
      (y >= height) || (z >= depth)) {
    return TRUE;
  }

  return FALSE;
}

Neurochain* Neurochain_Flip_Link(Neurochain *chain, Direction_e d)
{
  if (chain == NULL) {
    return NULL;
  }

  Neurochain *new_end = NULL;

  switch (d) {
  case FORWARD:
    if(chain->next == NULL) {
      return chain;
    } else {
      Neurochain *cur = Neurochain_Tail(chain);
      new_end = cur;
      Neurochain *prev = cur->prev;
      
      while (cur != chain) {
	cur->prev = cur->next;
	cur->next = prev;
	cur = prev;
	prev = prev->prev;
      }

      new_end->prev = chain->prev;
      if (chain->prev != NULL) {
	chain->prev->next = new_end;
      }

      chain->prev = chain->next;
      chain->next = NULL;
    }
    break;
  case BACKWARD:
    if(chain->prev == NULL) {
      return chain;
    } else {
      Neurochain *cur = Neurochain_Head(chain);
      new_end = cur;
      Neurochain *next = cur->next;
      
      while (cur != chain) {
	cur->next = cur->prev;
	cur->prev = next;
	cur = next;
	next = next->next;
      }

      new_end->next = chain->next;
      if (chain->next != NULL) {
	chain->next->prev = new_end;
      }

      chain->next = chain->prev;
      chain->prev = NULL;
    }
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
    break;
  }

  return new_end;
}

void Neurochain_Get_Locseg(const Neurochain *chain, Local_Neuroseg *locseg)
{
  Local_Neuroseg_Copy(locseg, &(chain->locseg));
  if (chain->hist != NULL) {
    if (Bitmask_Get_Bit(chain->hist->mask, TRACE_HIST_DIRECTION_MASK)
	== TRUE) {
      if (chain->hist->direction == BACKWARD) {
	Flip_Local_Neuroseg(locseg);
      }
    }
  }
}

#define NEUROCHAIN_NORMALIZE_DIRECTION(chain)				\
  if (chain->hist != NULL) {						\
    if (Bitmask_Get_Bit(chain->hist->mask, TRACE_HIST_DIRECTION_MASK)	\
	== TRUE) {							\
      if (chain->hist->direction == BACKWARD) {				\
	Flip_Local_Neuroseg(&(chain->locseg));				\
	chain->hist->direction = FORWARD;				\
      }									\
    }									\
  }

void Neurochain_Normalize_Direction(Neurochain *chain, Direction_e d)
{
  switch (d) {
  case FORWARD:
    while (chain != NULL) {
      NEUROCHAIN_NORMALIZE_DIRECTION(chain);
      chain = chain->next;
    }
    break;
  case BACKWARD:
    while (chain != NULL) {
      NEUROCHAIN_NORMALIZE_DIRECTION(chain);
      chain = chain->prev;
    }
    break;
  case BOTH:
    chain = Neurochain_Head(chain);
    Neurochain_Normalize_Direction(chain, FORWARD);
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
  }  
}

Neurochain *Neurochain_Remove_Head(Neurochain *chain)
{
  Neurochain *head = Neurochain_Head(chain);
  chain = head;
  head = head->next;
  chain->next = NULL;
  head->prev = NULL;
  Free_Neurochain(chain);

  return head;
}

Neurochain *Neurochain_Remove_Tail(Neurochain *chain)
{
  Neurochain *tail = Neurochain_Tail(chain);
  chain = tail;
  tail = tail->prev;
  tail->next = NULL;
  chain->prev = NULL;
  Free_Neurochain(chain);

  return tail;
}

int Neurochain_End_Points(const Neurochain *chain, int n, 
			  coordinate_3d_t pts[])
{
  ASSERT(n != 0, "0 points");

  int i;
  if (n > 0) {
    Neurochain *head = Neurochain_Head(chain);
    if (head->hist != NULL) {
      if (head->hist->index == 0) {
	Local_Neuroseg_Bottom(&(head->locseg), pts[0]);
      } else {
	Local_Neuroseg_Top(&(head->locseg), pts[0]);
      }
    } else {
      Local_Neuroseg_Bottom(&(head->locseg), pts[0]);
    }
          
    for (i = 1; i < n; i++) {
      Local_Neuroseg_Center(&(head->locseg), pts[i]);
      head = head->next;
      if (head == NULL) {
	break;
      }
    }
  } else {
    n = -n;
    Neurochain *tail = Neurochain_Tail(chain);
    
    Local_Neuroseg_Top(&(tail->locseg), pts[0]);
    
    for (i = 1; i < n; i++) {
      Local_Neuroseg_Center(&(tail->locseg), pts[i]);      
      tail = tail->prev;
      if (tail == NULL) {
	break;
      }
    }
  }

  return i;
}

Neurochain* Neurochain_Remove_Overlap_Ends(Neurochain *chain)
{
  Neurochain *head = Neurochain_Head(chain);
    
  if (Neurochain_Length(head, FORWARD) < 2) {
    return head;
  }

  Neurochain *tail = Neurochain_Tail(chain);

  double top[3];
  double bottom[3];
  Local_Neuroseg_Top(&(head->locseg), top);
  Local_Neuroseg_Bottom(&(head->locseg), bottom);
 
  if ((Local_Neuroseg_Hit_Test(&(head->next->locseg), top[0], top[1],
			       top[2]) == TRUE) &&
      (Local_Neuroseg_Hit_Test(&(head->next->locseg), bottom[0], 
			       bottom[1], bottom[2]) == TRUE)) {
    chain = head;
    head = head->next;
    chain->next = NULL;
    head->prev = NULL;
    Free_Neurochain(chain);
  }

  if (Neurochain_Length(head, FORWARD) < 2) {
    return head;
  }

  Local_Neuroseg_Top(&(tail->locseg), top);
  Local_Neuroseg_Bottom(&(tail->locseg), bottom);
 
  if ((Local_Neuroseg_Hit_Test(&(tail->prev->locseg), top[0], top[1],
			       top[2]) == TRUE) &&
      (Local_Neuroseg_Hit_Test(&(tail->prev->locseg), bottom[0], 
			       bottom[1], bottom[2]) == TRUE)) {
    chain = tail;
    tail = tail->prev;
    chain->prev = NULL;
    tail->next = NULL;
    Free_Neurochain(chain);
  }
 
  return head;
}

BOOL neuroseg_contain(const Local_Neuroseg *container, 
		     const Local_Neuroseg *seg)
{
  double top[3];
  double bottom[3];
  Local_Neuroseg_Top(seg, top);
  Local_Neuroseg_Bottom(seg, bottom);

  return (Local_Neuroseg_Hit_Test(container, top[0], top[1], top[2]) &&
	  Local_Neuroseg_Hit_Test(container, bottom[0], bottom[1], bottom[2]));
}

Neurochain* Neurochain_Remove_Overlap_Segs(Neurochain *chain)
{
  Neurochain *head = Neurochain_Head(chain);

  Neurochain *cur_node = head;
  Neurochain *tmp_node;

  while (cur_node->next != NULL) {
    if (neuroseg_contain(&(cur_node->locseg), &(cur_node->next->locseg))
	== TRUE) {
      tmp_node = cur_node->next;
      cur_node->next = tmp_node->next;
      if (tmp_node->next != NULL) {
	tmp_node->next->prev = cur_node;
      }
      tmp_node->prev = NULL;
      tmp_node->next = NULL;
      Free_Neurochain(tmp_node);
    } else if (neuroseg_contain(&(cur_node->next->locseg), &(cur_node->locseg))
	       == TRUE) {
      
      tmp_node = cur_node;
      cur_node = cur_node->next;
      cur_node->prev = tmp_node->prev;
      if (cur_node->prev != NULL) {
	cur_node->prev->next = cur_node;
      }
      tmp_node->prev = NULL;
      tmp_node->next = NULL;
      Free_Neurochain(tmp_node);
    } else {
      cur_node = cur_node->next;
    }
  }
  
  return Neurochain_Head(cur_node);
}

Neurochain* Neurochain_Remove_Turn_Ends(Neurochain *chain, double max_angle)
{
  Neurochain *head = Neurochain_Head(chain);
    
  if (Neurochain_Length(head, FORWARD) < 2) {
    return head;
  }

  Neurochain *tail = Neurochain_Tail(chain);
 
  double angle = Neuroseg_Angle_Between(&(head->locseg.seg), 
					&(head->next->locseg.seg));
  if (angle > TZ_PI) {
    angle = TZ_2PI - angle;
  }

  if (angle > max_angle) {
    chain = head;
    head = head->next;
    chain->next = NULL;
    head->prev = NULL;
    Free_Neurochain(chain);
  }

  if (Neurochain_Length(head, FORWARD) < 2) {
    return head;
  }

  angle = Neuroseg_Angle_Between(&(tail->locseg.seg), 
				 &(tail->prev->locseg.seg));
  if (angle > TZ_PI) {
    angle = TZ_2PI - angle;
  }

  if (angle > max_angle) {
    chain = tail;
    tail = tail->prev;
    chain->prev = NULL;
    tail->next = NULL;
    Free_Neurochain(chain);
  }
 
  return head;  
}

Graph* Neurochain_Graph(Neurochain **chain, int n, Int_Arraylist *hit_spots)
{
  Graph *graph = Make_Graph(n, n, FALSE);

  int i, j;
  Neurochain *head = NULL;
  Neurochain *tail = NULL;

  int hit = 0;

  PROGRESS_BEGIN("Build neuron chain graph");
  for (i = 0; i < n; i++) {
    PROGRESS_STATUS(i * 100 / n);

    head = Neurochain_Head(chain[i]);
    tail = Neurochain_Tail(chain[i]);
    for (j = 0; j < n; j++) {
      if (i != j) {
	hit = Neurochain_Hit_Test_Seg(Neurochain_Head(chain[j]), FORWARD, 0, 
				      &(head->locseg));

	if ((hit > 1) && (hit < Neurochain_Length(chain[j], BOTH))) {
	  Graph_Add_Edge(graph, i, j);
	  if (hit_spots != NULL) {
	    Int_Arraylist_Add(hit_spots, -hit);
	  }
	  //continue;
	} else {	 
	  hit = Neurochain_Hit_Test_Seg(Neurochain_Head(chain[j]), FORWARD, 0, 
					&(tail->locseg));
	  if ((hit > 1) && (hit < Neurochain_Length(chain[j], BOTH))){
	    Graph_Add_Edge(graph, i, j);
	    if (hit_spots != NULL) {
	      Int_Arraylist_Add(hit_spots, hit);
	    }
	  }
	}
      }
    }
    PROGRESS_REFRESH
  }

  PROGRESS_END("done");

  return graph;
}

Locseg_Chain* Neurochain_To_Locseg_Chain(Neurochain *chain)
{
  chain = Neurochain_Head(chain);
  Neurochain_Normalize_Direction(chain, FORWARD);
  Locseg_Chain *newchain = New_Locseg_Chain();

  while (chain != NULL) {
    Local_Neuroseg *locseg = New_Local_Neuroseg();
    Local_Neuroseg_Copy(locseg, &(chain->locseg));
    Locseg_Chain_Add(newchain, locseg, NULL, DL_TAIL);
    chain = chain->next;
  }

  return newchain;
}

Neurochain* Neurochain_From_Locseg_Chain(Locseg_Chain *chain)
{
  if (chain->list == NULL) {
    return NULL;
  }

  Neurochain *newchain = New_Neurochain();
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Local_Neuroseg *locseg = Locseg_Chain_Next_Seg(chain);
  Local_Neuroseg_Copy(&(newchain->locseg), locseg);
  
  while((locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    newchain = Extend_Neurochain(newchain);
    Local_Neuroseg_Copy(&(newchain->locseg), locseg);
  }

  return Neurochain_Head(newchain);
}


double *Neurochain_Average_Signal(Neurochain *chain, const Stack *stack,
				  double z_scale, double *value)
{
  chain = Neurochain_Head(chain);
  int n = Neurochain_Length(chain, FORWARD);

  if (value == NULL) {
    value = darray_malloc(n);
  }

  int i;
  for (i = 0; i < n; i++) {
    ASSERT(chain != NULL, "Null chain. bug found.");
    value[i] = Local_Neuroseg_Average_Signal(&(chain->locseg), stack, z_scale);
    chain = chain->next;
  }
  
  return value;
}
