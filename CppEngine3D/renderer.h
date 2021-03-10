#ifndef RENDERER_H
#define RENDERER_H

#include <SDL.h>
#include <algorithm>

void _fill_flat_top(SDL_Renderer* h, float tx, float ty, float mx, float my, float bx, float by);
void _fill_flat_bot(SDL_Renderer* h, float tx, float ty, float mx, float my, float bx, float by);
void _draw_line_bresenham(SDL_Renderer* h, int sx, int sy, int ex, int ey);

void draw_tri_wireF(SDL_Renderer* h, float x1, float y1, float x2, float y2, float x3, float y3, int r, int g, int b);

void draw_tri_raster_stdF(SDL_Renderer* h, float x1, float y1, float x2, float y2, float x3, float y3, int r, int g, int b);
void draw_tri_raster_bresenhamF(SDL_Renderer* h, float x1, float y1, float x2, float y2, float x3, float y3, int r, int g, int b);

#endif