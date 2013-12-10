/*
 * zcurve_tmpl.cpp
 *
 *  Created on: May 28, 2009
 *      Author: zhaot
 */

template<class T>
void ZCurve::loadArray(const T *value, int n)
{
  m_data.clear();
  int i;
  for (i = 0; i < n; i++) {
    m_data.append(QPointF(i, value[i]));
  }
}
