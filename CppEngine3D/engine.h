#ifndef ENGINE_H
#define ENGINE_H

#include <SDL.h>
#include <math.h> // math.h is not included in matrix.h
#include <vector>
#include <algorithm>

#include "matrix4x4.h"


#define RAD(X) (X * (float)M_PI / 180.0f)

// Basic data structs
struct v3d
{
    float x = 0.0f, y = 0.0f, z = 0.0f, w = 1.0f; // Yes technically 4d but we dont really care about w :(

    v3d operator +(v3d o);
    v3d operator -(v3d o);
    v3d operator *(float o);
    v3d operator /(float o);
};


struct triangle
{
    v3d p[3];
    int r = 0, g = 0, b = 0;
};


struct mesh
{
    vector<triangle> tris;
};


// Mini vector library
v3d get_mul_mat4x4_v3d(M4x4 m, v3d v);
float dotv3d(v3d a, v3d b);
float magv3d(v3d v);
v3d normv3d(v3d v);
v3d crossv3d(v3d a, v3d b);

M4x4 get_projection_matrix(float rfov, float hwaspect, float zNear, float zFar);

M4x4 get_trans_mat(float x, float y, float z);
M4x4 get_rot_x(float rtheta);
M4x4 get_rot_y(float rtheta);
M4x4 get_rot_z(float rtheta);

void map_screen_space(v3d& p, int winwt, int winht);


void _fill_flat_top(SDL_Renderer* h, float tx, float ty, float mx, float my, float bx, float by);
void _fill_flat_bot(SDL_Renderer* h, float tx, float ty, float mx, float my, float bx, float by);
void _draw_line_bresenham(SDL_Renderer* h, int sx, int sy, int ex, int ey);

void draw_tri_wireF(SDL_Renderer* h, float x1, float y1, float x2, float y2, float x3, float y3, int r, int g, int b);

void draw_tri_raster_stdF(SDL_Renderer* h, float x1, float y1, float x2, float y2, float x3, float y3, int r, int g, int b);
void draw_tri_raster_bresenhamF(SDL_Renderer* h, float x1, float y1, float x2, float y2, float x3, float y3, int r, int g, int b);

bool _midpoint_compare(triangle t1, triangle t2);
void sort_tri_buffer(vector<triangle>& v);

v3d get_norm(triangle t);
bool check_norm_visible(v3d norm, triangle t, v3d cam);


#endif