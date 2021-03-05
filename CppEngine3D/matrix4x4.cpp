#include "matrix4x4.h"


float dot(float* v1, float* v2)
{
    float res = 0.0f;
    int i;
    for (i = 0; i < 4; i++)
    {
        res += v1[i] * v2[i];
    }
    return res;
}



float* M4x4::m_get_row(int i)
{
    return m_elems[i];
}


float* M4x4::m_get_col(int j)
{
    float col[4];
    int i;
    for (i = 0; i < 4; i++)
    {
        col[i] = m_elems[i][j];
    }

    return col;
}



M4x4 M4x4::operator *(M4x4 o)
{
    M4x4 resMat;
    float* row;
    float* col;
    int i, j;
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            row = m_get_row(i);
            col = o.m_get_col(j);
            resMat.m_elems[i][j] = dot(row, col);
        }
    }


    return resMat;
    
}


M4x4 M4x4::operator +(M4x4 o)
{
    M4x4 resMat;
    int i, j;
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            resMat.m_elems[i][j] = m_elems[i][j] + o.m_elems[i][j];
        }
    }

    return resMat;
}


M4x4 M4x4::operator -(M4x4 o)
{  
    M4x4 resMat;
    int i, j;
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            resMat.m_elems[i][j] = m_elems[i][j] - o.m_elems[i][j];
        }
    }

    return resMat;
}



