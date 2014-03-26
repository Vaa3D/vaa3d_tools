/**@file tz_neurocomp_conn.h
 * @brief neuron component connection
 * @author Ting Zhao
 * @date 2-Oct-2008
 */

#ifndef _TZ_NEUROCOMP_CONN_H_
#define _TZ_NEUROCOMP_CONN_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

enum {
  NEUROCOMP_CONN_NONE, /* not connected  */
  NEUROCOMP_CONN_HL,   /* hook-loop mode */
  NEUROCOMP_CONN_LINK  /* link mode      */
};

/** @typedef _Neurocomp_Conn Neurocomp_Conn
 *  @brief neuron components connection information
 *
 *  The meaning of info depends on the connection mode:
 *    mode = NEUROCOMP_CONN_NONE: not connected. info has no meaning.
 *    mode = NEUROCOMP_CONN_HL: info[0] is the position of the hook. info[1] is 
 *      the position of the loop. hook position can only be the end index 
 *      (0 for head and 1 for tail).
 *    mode = NEUROCOMP_CONN_LINK: info[0] is the end index of the first
 *      component and info[1] is the end index of the second component.
 *
 *  cost is the connection cost, which has no predefined lower or upper bound.
 */
typedef struct _Neurocomp_Conn {
  int mode;
  int info[2];
  double pos[3];  /* an additional position parameter for hook mode */
  double ort[3];  /* connection orientation verctor */
  double cost;  /* connection cost */
  double pdist; /* planar distance */
  double sdist; /* surface distance */
} Neurocomp_Conn;

Neurocomp_Conn* New_Neurocomp_Conn();
void Default_Neurocomp_Conn(Neurocomp_Conn *conn);
void Delete_Neurocomp_Conn(Neurocomp_Conn *conn);

void Neurocomp_Conn_Copy(Neurocomp_Conn *des, const Neurocomp_Conn *src);

/**@brief translate mode according to the component lengths
 *
 * It is mainly designed to translate HL mode to link mode.
 */
void Neurocomp_Conn_Translate_Mode(int len2, Neurocomp_Conn *conn);

void Print_Neurocomp_Conn(const Neurocomp_Conn *conn);

__END_DECLS

#endif
