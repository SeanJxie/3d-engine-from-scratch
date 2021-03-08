#include "matrix4x4.h"

/*

There used to be a whole bunch of for loops in here, iterating over 4 consecutive integers.
We don't want loops, we want speed.

https://en.wikipedia.org/wiki/Loop_unrolling

*/

float dot(float* v1, float* v2)
{
    return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2] + v1[3] * v2[3];
}



float* M4x4::m_get_row(int i)
{
    return m_elems[i];
}


float* M4x4::m_get_col(int j)
{
    static float col[4] = { 0 };

    col[0] = m_elems[0][j];
    col[1] = m_elems[1][j];
    col[2] = m_elems[2][j];
    col[3] = m_elems[3][j];
   
    return col;
}


void M4x4::cast_identity()
{
    m_elems[0][0] = 1.0f;
    m_elems[1][1] = 1.0f;
    m_elems[2][2] = 1.0f;
    m_elems[3][3] = 1.0f;
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

    row = m_get_row(0);
    col = o.m_get_col(0);
    resMat.m_elems[0][0] = dot(row, col);

    row = m_get_row(0);
    col = o.m_get_col(1);
    resMat.m_elems[0][1] = dot(row, col);

    row = m_get_row(0);
    col = o.m_get_col(2);
    resMat.m_elems[0][2] = dot(row, col);

    row = m_get_row(0);
    col = o.m_get_col(3);
    resMat.m_elems[0][3] = dot(row, col);

    row = m_get_row(1);
    col = o.m_get_col(0);
    resMat.m_elems[1][0] = dot(row, col);

    row = m_get_row(1);
    col = o.m_get_col(1);
    resMat.m_elems[1][1] = dot(row, col);

    row = m_get_row(1);
    col = o.m_get_col(2);
    resMat.m_elems[1][2] = dot(row, col);

    row = m_get_row(1);
    col = o.m_get_col(3);
    resMat.m_elems[1][3] = dot(row, col);

    row = m_get_row(2);
    col = o.m_get_col(0);
    resMat.m_elems[2][0] = dot(row, col);

    row = m_get_row(2);
    col = o.m_get_col(1);
    resMat.m_elems[2][1] = dot(row, col);

    row = m_get_row(2);
    col = o.m_get_col(2);
    resMat.m_elems[2][2] = dot(row, col);

    row = m_get_row(2);
    col = o.m_get_col(3);
    resMat.m_elems[2][3] = dot(row, col);

    row = m_get_row(3);
    col = o.m_get_col(0);
    resMat.m_elems[3][0] = dot(row, col);

    row = m_get_row(3);
    col = o.m_get_col(1);
    resMat.m_elems[3][1] = dot(row, col);

    row = m_get_row(3);
    col = o.m_get_col(2);
    resMat.m_elems[3][2] = dot(row, col);

    row = m_get_row(3);
    col = o.m_get_col(3);
    resMat.m_elems[3][3] = dot(row, col);

    return resMat;
    
}

// Not used ---
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

// Not used ---
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