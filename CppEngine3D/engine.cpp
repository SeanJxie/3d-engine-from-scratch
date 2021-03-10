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















