/* @file tz_neurocomp_conn.c
 * @author Ting Zhao
 * @date 02-Oct-2008
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utilities.h>
#include "tz_neurocomp_conn.h"

Neurocomp_Conn* New_Neurocomp_Conn()
{
  Neurocomp_Conn *conn = (Neurocomp_Conn *) 
    Guarded_Malloc(sizeof(Neurocomp_Conn), "New_Neurocomp_Conn");

  Default_Neurocomp_Conn(conn);

  return conn;
}

void Default_Neurocomp_Conn(Neurocomp_Conn *conn)
{
  conn->mode = NEUROCOMP_CONN_NONE;
  conn->info[0] = 0;
  conn->info[1] = 0;
  conn->pos[0] = 0.0;
  conn->pos[1] = 0.0;
  conn->pos[2] = 0.0;
  conn->ort[0] = 0.0;
  conn->ort[1] = 0.0;
  conn->ort[2] = 0.0;
  conn->cost = 0.0;
  conn->pdist = -1.0;
  conn->sdist = -1.0;
}

void Delete_Neurocomp_Conn(Neurocomp_Conn *conn)
{
  free(conn);
}

void Neurocomp_Conn_Copy(Neurocomp_Conn *des, const Neurocomp_Conn *src)
{
  memcpy(des, src, sizeof(Neurocomp_Conn));
}

void Print_Neurocomp_Conn(const Neurocomp_Conn *conn)
{
  switch (conn->mode) {
  case NEUROCOMP_CONN_NONE:
    printf("Not connected\n");
    break;
  case NEUROCOMP_CONN_HL:
    printf("Hook-loop mode: ");
    break;
  case NEUROCOMP_CONN_LINK:
    printf("Link mode: ");
    break;
  default:
    printf("Unknown connection mode: ");
    break;
  }
  
  if (conn->mode != NEUROCOMP_CONN_NONE) {
    printf("%d %d %g\n", conn->info[0], conn->info[1], conn->cost);
  }
}

void Neurocomp_Conn_Translate_Mode(int len2, Neurocomp_Conn *conn)
{
  if (conn->mode != NEUROCOMP_CONN_HL) {
    return;
  }

  if (conn->info[1] <= 0) {
    conn->mode = NEUROCOMP_CONN_LINK;
    conn->info[1] = 0;
  } else if (conn->info[1] >= len2 - 1) {
    conn->mode = NEUROCOMP_CONN_LINK;
    conn->info[1] = 1;
  }
}
