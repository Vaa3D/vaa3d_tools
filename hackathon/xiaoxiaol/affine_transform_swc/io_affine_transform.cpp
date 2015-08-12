#include <newmat.h>
#include <QFile>
#include <QTextStream>
#include "io_affine_transform.h"
#include <iostream>

/* transform.txt
 r11  r12  r13   tx
 r21  r22  r23   ty
 r31  r32  r33   tz

*/

Matrix readTransform(QString filename)
{
    Matrix affineMatrix(3,4);
    QFile qf(filename);
    qf.open (QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&qf);

    for (int i = 1 ; i < 4; i++)
    {
        for (int count=0; count<4 || !qf.atEnd(); count++)
        {
            in >> affineMatrix(i, count+1);
            //                        std::cout<< affineMatrix(i, count+1)<<std::endl ;

        }
    }
    qf.close();
    return affineMatrix;
}


