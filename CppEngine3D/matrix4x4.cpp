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
    static float col[4] = { 0 };
    int i;
    for (i = 0; i < 4; i++)
    {
        col[i] = m_elems[i][j];
    }

    return col;
}


void M4x4::cast_identity()
{
    for (int ij = 0; ij < 4; ij++)
    {
        m_elems[ij][ij] = 1.0f;
    }
}

M4x4 M4x4::get_inverse()
{
    // https://en.wikipedia.org/wiki/Invertible_matrix
    M4x4 resMat;
    resMat.m_elems[0][0] = m_elems[0][0]; resMat.m_elems[0][1] = m_elems[1][0]; resMat.m_elems[0][2] = m_elems[2][0]; resMat.m_elems[0][3] = 0.0f;
    resMat.m_elems[1][0] = m_elems[0][1]; resMat.m_elems[1][1] = m_elems[1][1]; resMat.m_elems[1][2] = m_elems[2][1]; resMat.m_elems[1][3] = 0.0f;
    resMat.m_elems[2][0] = m_elems[0][2]; resMat.m_elems[2][1] = m_elems[1][2]; resMat.m_elems[2][2] = m_elems[2][2]; resMat.m_elems[2][3] = 0.0f;
    resMat.m_elems[3][0] = -(m_elems[3][0] * resMat.m_elems[0][0] + m_elems[3][1] * resMat.m_elems[1][0] + m_elems[3][2] * resMat.m_elems[2][0]);
    resMat.m_elems[3][1] = -(m_elems[3][0] * resMat.m_elems[0][1] + m_elems[3][1] * resMat.m_elems[1][1] + m_elems[3][2] * resMat.m_elems[2][1]);
    resMat.m_elems[3][2] = -(m_elems[3][0] * resMat.m_elems[0][2] + m_elems[3][1] * resMat.m_elems[1][2] + m_elems[3][2] * resMat.m_elems[2][2]);
    resMat.m_elems[3][3] = 1.0f;
    
    return resMat;

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


void M4x4::print()
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            cout << m_elems[i][j] << ' ';
        }

        cout << endl;
    }

    cout << endl;
}