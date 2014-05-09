#ifndef LOCK_H
#define LOCK_H

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsformat.h>
#include <std/nsascii.h>
#include <std/nsutil.h>
#include <math/nsrandom.h>

NS_DECLS_BEGIN

#define NS_LICENSE_NUM_DIGITS  40

// generates a lock based on current time
NS_IMPEXP void GenerateLock(nschar* string);

// validates and updates a lock based on time and DAYSTOEND
// return 1 on success or 0 if invalid or expired lock
NS_IMPEXP nsint UpdateLock(nschar* string);

// compute number of days left for license
NS_IMPEXP nsint GetLockDays(nschar* string);

NS_DECLS_END

#endif/* LOCK_H */
