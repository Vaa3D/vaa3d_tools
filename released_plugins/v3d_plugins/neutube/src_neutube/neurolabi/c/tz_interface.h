/**@file tz_interface.h
 * @brief shell interface
 * @author Ting Zhao
 * @date 01-Mar-2008
 */

#ifndef _TZ_INTERFACE_H_
#define _TZ_INTERFACE_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

/******* progress reporter definition **********/
/*
 * Usage: 
 *    PROGRESS_BEGIN("What's going on")
 *    loop {
 *      ROGRESS_STATUS(how_many_percents_are_done) 
 *      doing something;
 *      PROGRESS_REFRESH
 *    }
 *    PROGRESS_END("done")
 *
 * Note: To disable progress bar, define ENABLE_PROGRESS as 0. 
 */

#if defined INTERFACE_PROGRESS_OFF
#  define ENABLE_PROGRESS 0
#  define ENABLE_HOURGLASS 0
#else
#  define ENABLE_PROGRESS 1
#  define ENABLE_HOURGLASS 1
#endif

#if defined FORCE_PROGRESS
#  undef ENABLE_PROGRESS
#  undef ENABLE_HOURGLASS
#  define ENABLE_PROGRESS 1
#  define ENABLE_HOURGLASS 1
#endif

#if ENABLE_PROGRESS == 1

#define PROGRESS_BEGIN(msg) printf("%s: ", msg);

#define PROGRESS_STATUS(k)			\
  printf("%3d%%", k);				\
  fflush(stdout);				

#define PROGRESS_REFRESH			\
  printf("\b\b\b\b");

#define PROGRESS_END(msg)			\
  printf("%s\n", msg);


#else

#define PROGRESS_BEGIN(msg)
#define PROGRESS_STATUS(k)
#define PROGRESS_REFRESH
#define PROGRESS_END(msg)

#endif
/**************************/

/******* hourglass definition **********/
/*
 * Usage: 
 *    int hg = 0;
 *    HOURGLASS_BEGIN("Processing")
 *    loop {
 *      HOURGLASS_SHOW(hg) 
 *      doing something;
 *      HOURGLASS_REFRESH(hg)
 *    }
 *    HOURGLASS_END("done")
 *
 * Note: To disable progress bar, define ENABLE_HOURGLASS as 0. 
 */

#if ENABLE_HOURGLASS == 1

const static char *Hourglass[] = {".", ":", "*", "m"};

#define HOURGLASS_BEGIN(msg, hg)		\
  int hg = 0;					\
  printf("%s ... ", msg);			\
  printf("%s", Hourglass[hg]);

#define HOURGLASS_UPDATE(hg)			\
  printf("\b");					\
  hg++;						\
  if (hg > 3) {					\
    hg = 0;					\
  }						\
  printf("%s", Hourglass[hg]);			\
  fflush(stdout);				  

#define HOURGLASS_END(msg)			\
  printf("\b");					\
  printf("%s\n", msg);


#else

#define HOURGLASS_BEGIN(msg, hg)
#define HOURGLASS_SHOW(hg)
#define HOURGLASS_UPDATE(hg)
#define HOURGLASS_END(msg)

#endif
/**************************/

/*
 * Display <message> to ask a user to input something. Press Enter to end the
 * dialog. <data> is used to store the input and <format> specifies how to read
 * the input. It is a string format for scanf(). It returns 1 if <data> is
 * updated; otherwise it returns 0.
 */
int Input_Dialog(const char *message, const char *format, void *data);

__END_DECLS

#endif
