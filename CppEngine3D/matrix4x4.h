#ifndef MATRIX4X4_H
#define MATRIX4X4_H
#include <iostream>

using namespace std;

float dot(float* v1, float* v2);


class M4x4
{
public:
    float m_elems[4][4] = { 0.0f };

    void cast_identity();
    void print();

    float* m_get_row(int i);
    float* m_get_col(int j);

    // Only works for rotation and translation matrices
    M4x4 get_inverse();

    M4x4 operator *(M4x4 o);
    M4x4 operator +(M4x4 o);
    M4x4 operator -(M4x4 o);
};


#endif