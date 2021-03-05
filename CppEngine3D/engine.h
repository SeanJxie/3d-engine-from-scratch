#ifndef ENGINE_H
#define ENGINE_H

#include <SDL.h>
#include <math.h> // math.h is not included in matrix.h
#include <vector>
#include <algorithm>

#include "matrix4x4.h"


#define RAD(X) (X * (float)M_PI / 180.0f)


struct v3d
{
    float x, y, z;
};


struct triangle
{
    v3d p[3];
    int r, g, b;
};


struct mesh
{
    vector<triangle> tris;
};


void mul_mat4x4_v3d(v3d in, v3d& out, M4x4 m);

M4x4 get_projection_matrix(float rfov, float hwaspect, float zNear, float zFar);

void rot_p_x(v3d& p, float rtheta);
void rot_p_y(v3d& p, float rtheta);
void rot_p_z(v3d& p, float rtheta);

void map_screen_space(v3d& p, int winwt, int winht);


void _fill_flat_top(SDL_Renderer* h, float tx, float ty, float mx, float my, float bx, float by);
void _fill_flat_bot(SDL_Renderer* h, float tx, float ty, float mx, float my, float bx, float by);

void draw_line_bresenham(SDL_Renderer* h, int sx, int sy, int ex, int ey);
void draw_triF(SDL_Renderer* h, float x1, float y1, float x2, float y2, float x3, float y3, int r, int g, int b);

void draw_tri_rasterF_std(SDL_Renderer* h, float x1, float y1, float x2, float y2, float x3, float y3, int r, int g, int b);
void draw_tri_rasterF_bresenham(SDL_Renderer* h, float x1, float y1, float x2, float y2, float x3, float y3, int r, int g, int b);

bool _midpoint_compare(triangle t1, triangle t2);
void sort_tri_buffer(vector<triangle>& v);

v3d get_norm(triangle t);
bool check_norm_visible(v3d norm, triangle t, v3d cam);


#endif