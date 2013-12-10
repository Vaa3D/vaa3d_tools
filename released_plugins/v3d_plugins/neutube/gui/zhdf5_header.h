#ifndef ZHDF5_HEADER_H
#define ZHDF5_HEADER_H

#if defined(_ENABLE_HDF5_)
#include "hdf5.h"
#include "hdf5_hl.h"
#else
typedef int hid_t;
typedef int herr_t;
typedef int hsize_t;
#endif

static const hid_t NULL_FILE = 0;

#endif // ZHDF5_HEADER_H
