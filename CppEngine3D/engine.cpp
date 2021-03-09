#include "engine.h"


void get_mul_mat4x4_v3d(M4x4 m, v3d v, v3d& out)
{
    out = {
        v.x * m.m_elems[0][0] + v.y * m.m_elems[1][0] + v.z * m.m_elems[2][0] + v.w * m.m_elems[3][0],
        v.x * m.m_elems[0][1] + v.y * m.m_elems[1][1] + v.z * m.m_elems[2][1] + v.w * m.m_elems[3][1],
        v.x * m.m_elems[0][2] + v.y * m.m_elems[1][2] + v.z * m.m_elems[2][2] + v.w * m.m_elems[3][2],
        v.x * m.m_elems[0][3] + v.y * m.m_elems[1][3] + v.z * m.m_elems[2][3] + v.w * m.m_elems[3][3]
    };
}


v3d plane_intersectv3d(v3d &point_on_plane, v3d &plane_norm, v3d &ls, v3d &le)
{
    //https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection#Parametric_form
    plane_norm = normv3d(plane_norm);

    float pd = -dotv3d(plane_norm, point_on_plane);
    float ad = dotv3d(ls, plane_norm);
    float bd = dotv3d(le, plane_norm);
    float t = (-pd - ad) / (bd - ad);
    v3d entire_line = le - ls;
    v3d intersect_line = entire_line * t;

    return ls + intersect_line;
}


float _point_plane_closest_dist(v3d point_on_plane, v3d plane_norm, v3d& p)
{
    return (plane_norm.x * p.x + plane_norm.y * p.y + plane_norm.z * p.z - dotv3d(plane_norm, point_on_plane));
}


int clip_tri_plane(v3d point_on_plane, v3d plane_norm, triangle& in_tri, triangle& out_tri1, triangle& out_tri2)
{
    // Count the number of newly formed triangles in the clipping process.
    // And yes, output the newly formed triangles as well (maximum of 2 in the case of quad split).
    // We do so through in_tri1 and in_tri2. 

    // Make sure norm
    plane_norm = normv3d(plane_norm);

    // Array of pointers
    v3d* points_in[3]{};
    v3d* points_out[3]{};

    // Counts
    int points_in_count = 0;
    int points_out_count = 0;

    // Distance of each point from plane (closest)
    float d0 = _point_plane_closest_dist(point_on_plane, plane_norm, in_tri.p[0]);
    float d1 = _point_plane_closest_dist(point_on_plane, plane_norm, in_tri.p[1]);
    float d2 = _point_plane_closest_dist(point_on_plane, plane_norm, in_tri.p[2]);

    // If point is "inside" (on one side (positive side)) of plane, add its address to 
    // the points_in array. Otherwise, it is not "inside" and thus add its address to the points_out array
    if (d0 >= 0.0f) points_in[points_in_count++] = &in_tri.p[0];
    else points_out[points_out_count++] = &in_tri.p[0];

    if (d1 >= 0.0f) points_in[points_in_count++] = &in_tri.p[1];
    else points_out[points_out_count++] = &in_tri.p[1];
    
    if (d2 >= 0.0f) points_in[points_in_count++] = &in_tri.p[2];
    else points_out[points_out_count++] = &in_tri.p[2];

    // Now we go through all possible outcomes of point configurations
    
    if (points_in_count == 0)
    {
        // The triangle is not within our plane (negative side)
        return 0;
    }

    if (points_in_count == 3)
    {
        // Entire triangle is within plane (positive side)
        // Set input to output
        out_tri1 = in_tri;

        return 1;
    }

    if (points_in_count == 1 && points_out_count == 2)
    {
        // 2 points of the triangle lie on the negative side of the plane
        // 1 point lies inside; the triangle must be clipped here

        out_tri1.c = in_tri.c;

        // We keep the point on the inside.
        out_tri1.p[0] = *points_in[0]; // Dereference address

        // We'll set the other 2 points to the 2 intersection points between the triangle and the plane.
        out_tri1.p[1] = plane_intersectv3d(point_on_plane, plane_norm, *points_in[0], *points_out[0]);
        out_tri1.p[2] = plane_intersectv3d(point_on_plane, plane_norm, *points_in[0], *points_out[1]);

        return 1; // One new formed triangle
    }

    if (points_in_count == 2 && points_out_count == 1)
    {
        // In the case that 2 points lie inside and 1 lies outside,
        // a quad is formed. We'll need to split the quad appropriately.

        out_tri1.c = in_tri.c;
        out_tri2.c = in_tri.c;

        out_tri1.p[0] = *points_in[0];
        out_tri1.p[1] = *points_in[1];
        out_tri1.p[2] = plane_intersectv3d(point_on_plane, plane_norm, *points_in[0], *points_out[0]);

        out_tri2.p[0] = *points_in[1];
        out_tri2.p[1] = out_tri1.p[2];
        out_tri2.p[2] = plane_intersectv3d(point_on_plane, plane_norm, *points_in[1], *points_out[0]);

        return 2;
    }
}



v3d v3d::operator +(v3d o)
{
    return { x + o.x, y + o.y, z + o.z };
}

v3d v3d::operator -(v3d o)
{
    return { x - o.x, y - o.y, z - o.z };
}

v3d v3d::operator *(float o)
{
    return { x * o, y * o, z * o };
}

v3d v3d::operator /(float o)
{
    return { x / o, y / o, z / o };
}

float dotv3d(v3d a, v3d b)
{
    // There is a dot function for standard arrays defined in matrix4x4.h used for matrix mul
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float magv3d(v3d v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

v3d normv3d(v3d v)
{
    float m = magv3d(v);
    return { v.x / m, v.y / m, v.z / m };
}

v3d crossv3d(v3d a, v3d b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}



void get_projection_matrix(float rfov, float hwaspect, float zNear, float zFar, M4x4& out)
{
    // https://www.jwwalker.com/pages/transformations.html
    out.m_elems[0][0] = hwaspect / tanf(rfov / 2.0f);
    out.m_elems[1][1] = 1.0f / tanf(rfov / 2.0f);
    out.m_elems[2][2] = zFar / (zFar - zNear);
    out.m_elems[2][3] = 1.0f;
    out.m_elems[3][2] = -(zFar * zNear) / (zFar - zNear);
}


void get_trans_mat(float x, float y, float z, M4x4& out)
{
    out.m_elems[0][0] = 1.0f;
    out.m_elems[1][1] = 1.0f;
    out.m_elems[2][2] = 1.0f;
    out.m_elems[3][3] = 1.0f;
    out.m_elems[3][0] = x;
    out.m_elems[3][1] = y;
    out.m_elems[3][2] = z;
}


void get_rot_x(float rtheta, M4x4& out)
{
    // https://wikimedia.org/api/rest_v1/media/math/render/svg/a6821937d5031de282a190f75312353c970aa2df
    out.m_elems[0][0] = 1.0f;
    out.m_elems[1][1] = cosf(rtheta);
    out.m_elems[1][2] = sinf(rtheta);
    out.m_elems[2][1] = -sinf(rtheta);
    out.m_elems[2][2] = cosf(rtheta);
    out.m_elems[3][3] = 1.0f;
}


void get_rot_y(float rtheta, M4x4& out)
{
    out.m_elems[0][0] = cosf(rtheta);
    out.m_elems[0][2] = sinf(rtheta);
    out.m_elems[1][1] = 1.0f;
    out.m_elems[2][0] = -sinf(rtheta);
    out.m_elems[2][2] = cosf(rtheta);
    out.m_elems[3][3] = 1.0f;

}


void get_rot_z(float rtheta, M4x4& out)
{
    out.m_elems[0][0] = cosf(rtheta);
    out.m_elems[0][1] = sinf(rtheta);
    out.m_elems[1][0] = -sinf(rtheta);
    out.m_elems[1][1] = cosf(rtheta);
    out.m_elems[2][2] = 1.0f;
    out.m_elems[3][3] = 1.0f;
    
}


void get_mat_pointat(v3d pos, v3d t, v3d up, M4x4& out)
{
    // To point at a postion, we change the forwards, up, right, direction of the entire
    // cord system

    // new forward
    v3d new_f = normv3d(t - pos);

    // new up
    v3d temp = new_f * dotv3d(up, new_f);
    v3d new_u = normv3d(up - temp);

    // new right, sticks out of new_u, new_f plane
    v3d new_r = crossv3d(new_u, new_f);

    // Construct dimension change matrix 
    out.m_elems[0][0] = new_r.x;	
    out.m_elems[1][0] = new_u.x;	
    out.m_elems[2][0] = new_f.x;	
    out.m_elems[3][0] = pos.x;
    
    out.m_elems[0][1] = new_r.y;
    out.m_elems[1][1] = new_u.y;
    out.m_elems[2][1] = new_f.y;
    out.m_elems[3][1] = pos.y;

    out.m_elems[0][2] = new_r.z;
    out.m_elems[1][2] = new_u.z;
    out.m_elems[2][2] = new_f.z;
    out.m_elems[3][2] = pos.z;
    
    out.m_elems[3][3] = 1.0f; 
}



void map_screen_space(v3d& p, int winwt, int winht)
{
    // Scale triangle into window view

    // We add 1 to each coord such that the top-left corner of our cube
    // is located at (1, 1); the center of the window.

    // The projection matrix normalizes our window such that
    // Left = -1, right = 1, top = 1, bottom = -1
    v3d offSetToCenter = { 1.0f, 1.0f, 0.0f };
    p = p + offSetToCenter;

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


bool check_tri_visible(triangle t, v3d cam)
{
    // Check if a triangle is visible from the camera.
    
    // Cross to get normal
    // Take 2 edges of triangle for cross
    v3d l1 = t.p[1] - t.p[0], l2 = t.p[2] - t.p[0];
    v3d norm = normv3d(crossv3d(l1, l2));
    
    // Shoot ray at a vertex of the triangle
    v3d camRay = t.p[0] - cam;

    /*Check similarity of normal and camera vectors by looking at how much 
    the norm vector projects onto the camera vector.
    Projecting by a negative amount (< 0.0f) means the vectors face away from one another.
    If they were perpendicular, dotv3d(norm, camRay) == 0.0f.*/

    return dotv3d(norm, camRay) < 0.0f;
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
        SDL_RenderDrawLine(h, minx, y, maxx, y); // Draw line from min to max points found on the y
        //_draw_line_bresenham(h, minx, y, maxx, y);    
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
        SDL_RenderDrawLine(h, minx, y, maxx, y);
        //_draw_line_bresenham(h, minx, y, maxx, y);
        if (!changed1) t1x += signtx;
        t1x += t1xp;
        if (!changed2) t2x += signmx;
        t2x += t2xp;
        y += 1;
        if (y > by) return;
    }
}

