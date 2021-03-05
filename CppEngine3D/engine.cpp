#include "engine.h"


// Very specific, as 3d projection requires only 4x4 matrices. Consider optimizing matrix class.
void mul_mat4x4_v3d(v3d in, v3d& out, M4x4 m)
{
    out.x = in.x * m.m_elems[0][0] + in.y * m.m_elems[1][0] + in.z * m.m_elems[2][0] + m.m_elems[3][0];
    out.y = in.x * m.m_elems[0][1] + in.y * m.m_elems[1][1] + in.z * m.m_elems[2][1] + m.m_elems[3][1];
    out.z = in.x * m.m_elems[0][2] + in.y * m.m_elems[1][2] + in.z * m.m_elems[2][2] + m.m_elems[3][2];

    float w = in.x * m.m_elems[0][3] + in.y * m.m_elems[1][3] + in.z * m.m_elems[2][3] + m.m_elems[3][3];

    if (w != 0.0f)
    {
        out.x /= w;
        out.y /= w;
        out.z /= w;
    }
}


M4x4 get_projection_matrix(float rfov, float hwaspect, float zNear, float zFar)
{
    // https://www.jwwalker.com/pages/transformations.html
    M4x4 projMat;
    projMat.m_elems[0][0] = hwaspect / tanf(rfov / 2.0f);
    projMat.m_elems[1][1] = 1.0f / tanf(rfov / 2.0f), 0.0f;
    projMat.m_elems[2][2] = zFar / (zFar - zNear);
    projMat.m_elems[2][3] = 1.0f;
    projMat.m_elems[3][2] = -(zFar * zNear) / (zFar - zNear);

    return projMat;
}


void rot_p_x(v3d& p, float rtheta)
{
    // https://wikimedia.org/api/rest_v1/media/math/render/svg/a6821937d5031de282a190f75312353c970aa2df
    M4x4 rotMat;
    rotMat.m_elems[0][0] = 1.0f;
    rotMat.m_elems[1][1] = cosf(rtheta);
    rotMat.m_elems[1][2] = sinf(rtheta);
    rotMat.m_elems[2][1] = -sinf(rtheta);
    rotMat.m_elems[2][2] = cosf(rtheta);

    mul_mat4x4_v3d(p, p, rotMat);
}


void rot_p_y(v3d& p, float rtheta)
{
    M4x4 rotMat;
    rotMat.m_elems[0][0] = cosf(rtheta);
    rotMat.m_elems[0][2] = sinf(rtheta);
    rotMat.m_elems[1][1] = 1.0f;
    rotMat.m_elems[2][0] = -sinf(rtheta);
    rotMat.m_elems[2][2] = cosf(rtheta);

    mul_mat4x4_v3d(p, p, rotMat);
}


void rot_p_z(v3d& p, float rtheta)
{
    M4x4 rotMat;
    rotMat.m_elems[0][0] = cosf(rtheta);
    rotMat.m_elems[0][1] = sinf(rtheta);
    rotMat.m_elems[1][0] = -sinf(rtheta);
    rotMat.m_elems[1][1] = cosf(rtheta);
    rotMat.m_elems[2][2] = 1.0f;

    mul_mat4x4_v3d(p, p, rotMat);
}


void map_screen_space(v3d& p, int winwt, int winht)
{
    // Scale triangle into window view

    // We add 1 to each coord such that the top-left corner of our cube
    // is located at (1, 1); the center of the window.

    // The projection matrix normalizes our window such that
    // Left = -1, right = 1, top = 1, bottom = -1
    p.x += 1.0f;
    p.y += 1.0f;

    // half of window width/height is simply the center of window
    p.x *= 0.5f * (float)winwt;
    p.y *= 0.5f * (float)winht;
}


v3d get_norm(triangle t)
{
    // Vector cross product
    v3d n, l1, l2;
    l1.x = t.p[1].x - t.p[0].x;
    l1.y = t.p[1].y - t.p[0].y;
    l1.z = t.p[1].z - t.p[0].z;

    l2.x = t.p[2].x - t.p[0].x;
    l2.y = t.p[2].y - t.p[0].y;
    l2.z = t.p[2].z - t.p[0].z;

    n.x = l1.y * l2.z - l1.z * l2.y;
    n.y = l1.z * l2.x - l1.x * l2.z;
    n.z = l1.x * l2.y - l1.y * l2.x;

    float mag = sqrtf(n.x * n.x + n.y * n.y + n.z * n.z);
    n.x /= mag;
    n.y /= mag;
    n.z /= mag;

    return n;
}


bool check_norm_visible(v3d norm, triangle t, v3d cam)
{
    // Check if a triangle is visible from the camera.
    // Dot product
    return norm.x * (t.p[0].x - cam.x) + norm.y * (t.p[0].y - cam.y) + norm.z * (t.p[0].z - cam.z) < 0.0f;
}


bool _midpoint_compare(triangle t1, triangle t2)
{
    float z1 = (t1.p[0].z, t1.p[1].z, t1.p[2].z) / 3.0f;
    float z2 = (t2.p[0].z, t2.p[1].z, t2.p[2].z) / 3.0f;

    return z1 > z2;
}


void sort_tri_buffer(vector<triangle>& v)
{
    sort(v.begin(), v.end(), _midpoint_compare);
}


void draw_line_bresenham(SDL_Renderer* h, int sx, int sy, int ex, int ey)
{
    /*
    SDL_RenderSetLogicalSize seems to not scale SDL_RenderDrawLine. 
    Here's a custom line function.
    
    */
    // bresenham line
    int x1 = sx, y1 = sy, x2 = ex, y2 = ey;
    int steep = abs(y2 - y1) > abs(x2 - x1), inc = -1;

    if (steep) {
        swap(x1, y1);
        swap(x2, y2);
    }

    if (x1 > x2) {
        swap(x1, x2);
        swap(y1, y2);
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



void draw_triF(SDL_Renderer* h, float x1, float y1, float x2, float y2, float x3, float y3, int r, int g, int b)
{
    SDL_SetRenderDrawColor(h, r, g, b, 255);

    /*SDL_FPoint pVec[4] = { {x1, y1}, {x2, y2}, {x3, y3}, {x1, y1} };
    SDL_RenderDrawLinesF(h, pVec, 4);*/

    draw_line_bresenham(h, (int)x1, (int)y1, (int)x2, (int)y2);
    draw_line_bresenham(h, (int)x2, (int)y2, (int)x3, (int)y3);
    draw_line_bresenham(h, (int)x3, (int)y3, (int)x1, (int)y1);
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
        draw_line_bresenham(h, (int)curx1, (int)y, (int)curx2, (int)y);
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
        draw_line_bresenham(h, (int)curx1, (int)y, (int)curx2, (int)y);
        curx1 += invslope1;
        curx2 += invslope2;
    }

}



void draw_tri_rasterF_std(SDL_Renderer* h, float x1, float y1, float x2, float y2, float x3, float y3, int r, int g, int b)
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


void draw_tri_rasterF_bresenham(SDL_Renderer* h, float x1, float y1, float x2, float y2, float x3, float y3, int r, int g, int b)
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
        draw_line_bresenham(h, minx, y, maxx, y);    // Draw line from min to max points found on the y
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

        if (dty > dtx) {   // swap values
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
        draw_line_bresenham(h, minx, y, maxx, y);
        if (!changed1) t1x += signtx;
        t1x += t1xp;
        if (!changed2) t2x += signmx;
        t2x += t2xp;
        y += 1;
        if (y > by) return;
    }
}

