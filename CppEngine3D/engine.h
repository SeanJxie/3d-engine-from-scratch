#ifndef ENGINE_H
#define ENGINE_H

#include <SDL.h>
#include <SDL_opengl.h>
#include <math.h> // math.h is not included in matrix.h
#include <vector>
#include <list>
#include <algorithm>

#include "matrix4x4.h"


#define RAD(X) (X * (float)M_PI / 180.0f)

// Basic data structs
struct v3d
{
    // Yes technically 4d but we dont really care about w :(
    // Here's more on the topic: 
    // https://en.wikipedia.org/wiki/Homogeneous_coordinates#Use_in_computer_graphics_and_computer_vision
    float x = 0.0f, y = 0.0f, z = 0.0f, w = 1.0f; 

    v3d operator +(v3d o);
    v3d operator -(v3d o);
    v3d operator *(float o);
    v3d operator /(float o);
};


struct triangle
{
    v3d p[3];
    SDL_Color c = { 0 };
};


struct mesh
{
    vector<triangle> tris;
};


// Mini vector library
void get_mul_mat4x4_v3d(M4x4 m, v3d v, v3d& out);
float dotv3d(v3d a, v3d b);
float magv3d(v3d v);
v3d normv3d(v3d v);
v3d crossv3d(v3d a, v3d b);

void get_projection_matrix(float rfov, float hwaspect, float zNear, float zFar, M4x4& out);

void get_trans_mat(float x, float y, float z, M4x4& out);
void get_rot_x(float rtheta, M4x4& out);
void get_rot_y(float rtheta, M4x4& out);
void get_rot_z(float rtheta, M4x4& out);
void get_mat_pointat(v3d pos, v3d t, v3d up, M4x4& out);

void map_screen_space(v3d& p, int winwt, int winht);

// point_on_plane and plane_norm is data for constructing our plane

float _point_plane_closest_dist(v3d point_on_plane, v3d plane_norm, v3d p);
v3d plane_intersectv3d(v3d& point_on_plane, v3d& plane_norm, v3d& ls, v3d& le);
int clip_tri_plane(v3d point_on_plane, v3d plane_norm, triangle& in_tri, triangle& out_tri1, triangle& out_tri2);

void _fill_flat_top(SDL_Renderer* h, float tx, float ty, float mx, float my, float bx, float by);
void _fill_flat_bot(SDL_Renderer* h, float tx, float ty, float mx, float my, float bx, float by);
void _draw_line_bresenham(SDL_Renderer* h, int sx, int sy, int ex, int ey);

void draw_tri_wireF(SDL_Renderer* h, float x1, float y1, float x2, float y2, float x3, float y3, int r, int g, int b);

void draw_tri_raster_stdF(SDL_Renderer* h, float x1, float y1, float x2, float y2, float x3, float y3, int r, int g, int b);
void draw_tri_raster_bresenhamF(SDL_Renderer* h, float x1, float y1, float x2, float y2, float x3, float y3, int r, int g, int b);

bool _midpoint_compare(triangle t1, triangle t2);
void sort_tri_buffer(vector<triangle>& v);

v3d get_norm(triangle t);
bool check_tri_visible(triangle t, v3d cam);

#endif