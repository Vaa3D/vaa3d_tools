#ifndef C_JSON_H
#define C_JSON_H

#include "tz_json.h"

namespace C_Json {
json_t* makeObject();
json_t* makeInteger(int v);
json_t* makeNumber(double v);
json_t* makeBoolean(bool v);
json_t* makeArray();

/*!
 * \brief Make a JSON array from a double array
 * \param array The array buffer.
 * \param n Number of elements.
 * \return The JSON object if it succeeds, otherwise returns NULL (if \a array
 *         is NULL or \a n is 0).
 */
json_t* makeArray(const double *array, size_t n);

/*!
 * \brief Make a JSON array from an integer array
 * \param array The array buffer.
 * \param n Number of elements.
 * \return The JSON object if it succeeds, otherwise returns NULL (if \a array
 *         is NULL or \a n is 0).
 */
json_t* makeArray(const int *array, size_t n);

void appendArray(json_t *array, json_t *v);
bool dump(const json_t *obj, const char *filePath);
}

#endif // C_JSON_H
