#include "renderer.h"


void _draw_line_bresenham(SDL_Renderer* h, int sx, int sy, int ex, int ey)
{
    /*
    SDL_RenderSetLogicalSize seems to not scale SDL_RenderDrawLine.
    Here's a custom line function.

    */
    // bresenham line
    int x1 = sx, y1 = sy, x2 = ex, y2 = ey;
    int steep = abs(y2 - y1) > abs(x2 - x1), inc = -1;

    if (steep) {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }

    if (x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }

    if (y1 < y2) {
        inc = 1;
    }

    int dx = abs(x2 - x1),
        dy = abs(y2 - y1),
        y = y1, x = x1,
        e = 0;

    for (x; x <= x2; x++) {
        if (steep) {
            SDL_RenderDrawPoint(h, y, x);
        }
        else {
            SDL_RenderDrawPoint(h, x, y);
        }

        if ((e + dy) << 1 < dx) {
            e = e + dy;
        }
        else {
            y += inc;
            e = e + dy - dx;
        }
    }
}


void draw_tri_wireF(SDL_Renderer* h, float x1, float y1, float x2, float y2, float x3, float y3, int r, int g, int b)
{
    SDL_SetRenderDrawColor(h, r, g, b, 255);

    /*
    The source for a single draw call looks something like
    // src/render/SDL_render.c#l1558

        int SDL_RenderDrawPoint(SDL_Renderer * renderer, int x, int y)
        {
            SDL_Point point;

            point.x = x;
            point.y = y;
            return SDL_RenderDrawPoints(renderer, &point, 1);
        }

    So, calling the "s"(plural) function directly for multiple lines is faster.

    */

    SDL_FPoint pVec[4] = { {x1, y1}, {x2, y2}, {x3, y3}, {x1, y1} };
    SDL_RenderDrawLinesF(h, pVec, 4);

    /*_draw_line_bresenham(h, (int)x1, (int)y1, (int)x2, (int)y2);
    _draw_line_bresenham(h, (int)x2, (int)y2, (int)x3, (int)y3);
    _draw_line_bresenham(h, (int)x3, (int)y3, (int)x1, (int)y1);*/
}




void _fill_flat_top(SDL_Renderer* h, float tx, float ty, float mx, float my, float bx, float by)
{
    float invslope1 = (bx - tx) / (by - ty);
    float invslope2 = (bx - mx) / (by - my);

    float curx1 = bx;
    float curx2 = bx;
    float y;

    for (y = by; y >= ty; y--)
    {
        _draw_line_bresenham(h, (int)curx1, (int)y, (int)curx2, (int)y);
        curx1 -= invslope1;
        curx2 -= invslope2;
    }

}



void _fill_flat_bot(SDL_Renderer* h, float tx, float ty, float mx, float my, float bx, float by)
{
    float invslope1 = (mx - tx) / (my - ty);
    float invslope2 = (bx - tx) / (by - ty);

    float curx1 = tx;
    float curx2 = tx;
    float y;

    for (y = ty; y <= my; y++)
    {
        _draw_line_bresenham(h, (int)curx1, (int)y, (int)curx2, (int)y);
        curx1 += invslope1;
        curx2 += invslope2;
    }

}

void draw_tri_raster_stdF(SDL_Renderer* h, float x1, float y1, float x2, float y2, float x3, float y3, int r, int g, int b)
{
    float tx, ty, mx, my, bx, by; // Top mid bot (graphically)
    if (y1 < y2 && y1 < y3)
    {
        tx = x1;
        ty = y1;
        if (y2 < y3)
        {
            mx = x2;
            my = y2;
            bx = x3;
            by = y3;
        }
        else
        {
            mx = x3;
            my = y3;
            bx = x2;
            by = y2;
        }
    }

    else if (y2 < y1 && y2 < y3)
    {
        tx = x2;
        ty = y2;
        if (y1 < y3)
        {
            mx = x1;
            my = y1;
            bx = x3;
            by = y3;
        }
        else
        {
            mx = x3;
            my = y3;
            bx = x1;
            by = y1;
        }
    }
    else
    {
        tx = x3;
        ty = y3;
        if (y1 < y2)
        {
            mx = x1;
            my = y1;
            bx = x2;
            by = y2;
        }
        else
        {
            mx = x2;
            my = y2;
            bx = x1;
            by = y1;
        }
    }

    SDL_SetRenderDrawColor(h, r, g, b, SDL_ALPHA_OPAQUE);

    if (ty == my) // flat top
    {
        _fill_flat_top(h, bx, ty, mx, my, bx, by);
    }

    else if (by == my) // flat bot
    {
        _fill_flat_bot(h, tx, ty, mx, my, bx, by);
    }

    else
    {
        // Split triangle into top/bot half
        float newx = tx + (my - ty) / (by - ty) * (bx - tx);

        _fill_flat_top(h, mx, my, newx, my, bx, by);
        _fill_flat_bot(h, tx, ty, mx, my, newx, my);

    }
}




void draw_tri_raster_bresenhamF(SDL_Renderer* h, float x1, float y1, float x2, float y2, float x3, float y3, int r, int g, int b)
{
    /*
    * draw_tri_rasterF_std() works, but is a little flickery; hope to solve in the future.
    *
    * Credit to:
    * http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
    * https://www.avrfreaks.net/sites/default/files/triangles.c
    * https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    */

    SDL_SetRenderDrawColor(h, r, g, b, SDL_ALPHA_OPAQUE);

    int tx, ty, mx, my, bx, by; // Top mid bot (graphically)
    if (y1 < y2 && y1 < y3)
    {
        tx = (int)x1;
        ty = (int)y1;
        if (y2 < y3)
        {
            mx = (int)x2;
            my = (int)y2;
            bx = (int)x3;
            by = (int)y3;
        }
        else
        {
            mx = (int)x3;
            my = (int)y3;
            bx = (int)x2;
            by = (int)y2;
        }
    }

    else if (y2 < y1 && y2 < y3)
    {
        tx = (int)x2;
        ty = (int)y2;
        if (y1 < y3)
        {
            mx = (int)x1;
            my = (int)y1;
            bx = (int)x3;
            by = (int)y3;
        }
        else
        {
            mx = (int)x3;
            my = (int)y3;
            bx = (int)x1;
            by = (int)y1;
        }
    }
    else
    {
        tx = (int)x3;
        ty = (int)y3;
        if (y1 < y2)
        {
            mx = (int)x1;
            my = (int)y1;
            bx = (int)x2;
            by = (int)y2;
        }
        else
        {
            mx = (int)x2;
            my = (int)y2;
            bx = (int)x1;
            by = (int)y1;
        }
    }

    int t1x, t2x, y, minx, maxx, t1xp, t2xp;
    bool changed1 = false;
    bool changed2 = false;
    int signtx, signmx, dtx, dty, dmx, dmy;
    int e1, e2;
    // Sort vertices
    if (ty > my) { std::swap(ty, my); std::swap(tx, mx); }
    if (ty > by) { std::swap(ty, by); std::swap(tx, bx); }
    if (my > by) { std::swap(my, by); std::swap(mx, bx); }

    t1x = t2x = tx; y = ty;   // Starting points
    dtx = (int)(mx - tx);
    if (dtx < 0) { dtx = -dtx; signtx = -1; }
    else signtx = 1;
    dty = (int)(my - ty);

    dmx = (int)(bx - tx);
    if (dmx < 0) { dmx = -dmx; signmx = -1; }
    else signmx = 1;
    dmy = (int)(by - ty);

    if (dty > dtx) { std::swap(dtx, dty); changed1 = true; }
    if (dmy > dmx) { std::swap(dmy, dmx); changed2 = true; }

    e2 = (int)(dmx >> 1);
    // Flat top, just process the second half
    if (ty == my) goto next;
    e1 = (int)(dtx >> 1);

    for (int i = 0; i < dtx;) {
        t1xp = 0; t2xp = 0;
        if (t1x < t2x) { minx = t1x; maxx = t2x; }
        else { minx = t2x; maxx = t1x; }
        // process first line until y value is about to change
        while (i < dtx) {
            i++;
            e1 += dty;
            while (e1 >= dtx) {
                e1 -= dtx;
                if (changed1) t1xp = signtx;//t1x += signtx;
                else          goto next1;
            }
            if (changed1) break;
            else t1x += signtx;
        }
        // Move line
    next1:
        // process second line until y value is about to change
        while (1) {
            e2 += dmy;
            while (e2 >= dmx) {
                e2 -= dmx;
                if (changed2) t2xp = signmx;//t2x += signmx;
                else          goto next2;
            }
            if (changed2)     break;
            else              t2x += signmx;
        }
    next2:
        if (minx > t1x) minx = t1x;
        if (minx > t2x) minx = t2x;
        if (maxx < t1x) maxx = t1x;
        if (maxx < t2x) maxx = t2x;
        //SDL_RenderDrawLine(h, minx, y, maxx, y); // Draw line from min to max points found on the y
        _draw_line_bresenham(h, minx, y, maxx, y);
        // Now increase y
        if (!changed1) t1x += signtx;
        t1x += t1xp;
        if (!changed2) t2x += signmx;
        t2x += t2xp;
        y += 1;
        if (y == my) break;

    }
next:
    // Second half
    dtx = (int)(bx - mx); if (dtx < 0) { dtx = -dtx; signtx = -1; }
    else signtx = 1;
    dty = (int)(by - my);
    t1x = mx;

    if (dty > dtx) {   // std::swap values
        std::swap(dty, dtx);
        changed1 = true;
    }
    else changed1 = false;

    e1 = (int)(dtx >> 1);

    for (int i = 0; i <= dtx; i++) {
        t1xp = 0; t2xp = 0;
        if (t1x < t2x) { minx = t1x; maxx = t2x; }
        else { minx = t2x; maxx = t1x; }
        // process first line until y value is about to change
        while (i < dtx) {
            e1 += dty;
            while (e1 >= dtx) {
                e1 -= dtx;
                if (changed1) { t1xp = signtx; break; }//t1x += signtx;
                else          goto next3;
            }
            if (changed1) break;
            else   	   	  t1x += signtx;
            if (i < dtx) i++;
        }
    next3:
        // process second line until y value is about to change
        while (t2x != bx) {
            e2 += dmy;
            while (e2 >= dmx) {
                e2 -= dmx;
                if (changed2) t2xp = signmx;
                else          goto next4;
            }
            if (changed2)     break;
            else              t2x += signmx;
        }
    next4:

        if (minx > t1x) minx = t1x;
        if (minx > t2x) minx = t2x;
        if (maxx < t1x) maxx = t1x;
        if (maxx < t2x) maxx = t2x;
        //SDL_RenderDrawLine(h, minx, y, maxx, y);
        _draw_line_bresenham(h, minx, y, maxx, y);
        if (!changed1) t1x += signtx;
        t1x += t1xp;
        if (!changed2) t2x += signmx;
        t2x += t2xp;
        y += 1;
        if (y > by) return;
    }
}
