#include "engine.h"
#include "obj_loader.h"

/*

My first 3D engine.

I'm using javidx9's tutorial as a guide:
https://www.youtube.com/watch?v=ih20l3pJoeU&list=RDCMUC-yuWVUplUJZvieEligKBkA&index=3

*/


// Program constants
int WINWT = 1000, WINHT = 1000;
int LOGICAL_WT = 1000, LOGICAL_HT = 1000;
const char* WINTT = "C++ Engine3D Demo";


int main(int argc, char* argv[])
{
    // SDL setup
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* hWin = SDL_CreateWindow(WINTT, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINWT, WINHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* hRend = SDL_CreateRenderer(hWin, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_ShowCursor(false);

    //SDL_RenderSetLogicalSize(hRend, LOGICAL_WT, LOGICAL_HT);

    SDL_Event event;
   
    // Program matrices
    M4x4 projMat, matRotX, matRotY, matRotZ, matTranslate, worldMat;

    // Projection matrix
    float aspect = (float)LOGICAL_HT / LOGICAL_WT;
    float fov = RAD(90.0f);
    float zFar = 1000.0f;
    float zNear = 0.1f;

    projMat = get_projection_matrix(fov, aspect, zNear, zFar);

    // Movement vars
    float allRot = 0.0f;
    float rotSpeed = 1.0f;
    float moveSpeed = 0.1f;

    // Camera
    v3d camPos = { 0.0f, 0.0f, 0.0f };
    v3d lookDir;
    v3d upDir;
    v3d target;
    M4x4 cameraMat;
    M4x4 viewMat;
    float camYaw = 0.0f;
    float camPitch = 0.0f;

    mesh object;
    object.tris = load_obj_from_fname("space_shuttle.obj");

    vector<triangle> triBuffer;

    // Loop vars
    bool run = true;

    // Input vars
    bool up = false, down = false, left = false, right = false, in = false, out = false;
    int mposX, mposY;
    float mouseSens = 0.05f;

    while (run)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                run = false;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_w: in = true; break;
                case SDLK_s: out = true; break;
                case SDLK_a: left = true; break;
                case SDLK_d: right = true; break;
                case SDLK_LSHIFT: down = true; break;
                case SDLK_SPACE: up = true; break;

                case SDLK_ESCAPE: run = false; break;
                }
                break;

            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                case SDLK_w: in = false; break;
                case SDLK_s: out = false; break;
                case SDLK_a: left = false; break;
                case SDLK_d: right = false; break;
                case SDLK_LSHIFT: down = false; break;
                case SDLK_SPACE: up = false; break;
                }

                break;
            }
            
        }

        // http://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/
        SDL_GetMouseState(&mposX, &mposY);
        SDL_WarpMouseInWindow(hWin, (int)WINWT / 2, (int)WINHT / 2);

        camYaw += mouseSens * (WINWT / 2.0f - mposX);
        camPitch += mouseSens * (WINHT / 2.0f - mposY);

        upDir = { 0.0f, 1.0f, 0.0f }; // y-axis is up
        target = { 0.0f, 0.0f, 1.0f }; // look z-axis

        // Adjust target position based on y rotation (yaw)
        // Establish a rotation matrix based on yaw
        M4x4 camRotYaw = get_rot_y(RAD(camYaw)); 
        M4x4 camRotPitch = get_rot_x(RAD(camPitch));

        // rotate look direction about the origin with our rotation matrix based on target
        lookDir = get_mul_mat4x4_v3d(camRotPitch * camRotYaw, target);

        // offset our target based on the direction we look and camera position
        target = camPos + lookDir; 

        // Construct camera matrix based on its position, where its looking, and where up is
        cameraMat = get_mat_pointat(camPos, target, upDir);

        // cameraMat specifies everything our camera does. In reality, we move the objects
        // such that it seems as if the camera is moving, relative to the objects. Therefore, our objects need to do the 
        // "opposite" of whatever the camera is said to do; we take the inverse of the camera matrix
        // and apply it to our objects. 
        // The view matrix is a fundemental processing stage, along with projection and transformation:
        viewMat = cameraMat.get_inverse();
        
        // lookDir is unit, scale to account for speed
        // Get forward vector for in/out movement
        v3d temp_forward = lookDir * moveSpeed;

        // Get right vector for left/right movement: simple cross product of up and looking direction
        // https://en.wikipedia.org/wiki/Right-hand_rule
        v3d temp_right = crossv3d(lookDir, upDir) * moveSpeed;

        // What kinda messed up camera has vertical movement based on camera angle... no temp_up vector needed here

        // Process input 
        // Add forwards / right vectors to camera position to move
        if (in) camPos = camPos + temp_forward;
        if (out) camPos = camPos - temp_forward;
        
        if (left) camPos = camPos + temp_right;
        if (right) camPos = camPos - temp_right;

        // For up and down, "up" stays the same, no matter the orientation of our cam.
        // Effectively, the x and y components of our uneeded "temp_up" vector would both be 0, so we just add y components.
        if (up) camPos.y -= moveSpeed;
        if (down) camPos.y += moveSpeed;

        // Define transformation matrices (object, not cam), all relative to the origin
        matRotX = get_rot_x(RAD(90.0f));
        matRotY = get_rot_y(RAD(90.0f));
        matRotZ = get_rot_z(RAD(0.0f));

        matTranslate = get_trans_mat(0.0f, 0.0f, 20.0f);

        // Construct world matrix
        // From wikipedia:
        // "Matrices representing other geometric transformations can be combined with this [identity matrix] and 
        //  each other by matrix multiplication. As a result, any perspective projection of space 
        //  can be represented as a single [world] matrix."

        worldMat.cast_identity(); 
        worldMat = matRotX * matRotY * matRotZ;
        worldMat = worldMat * matTranslate;

        // Clear back buffer from last frame
        SDL_SetRenderDrawColor(hRend, 0, 0, 0, SDL_ALPHA_OPAQUE); 
        SDL_RenderClear(hRend);

        // Go through every triangle in the mesh
        for (auto const &tri : object.tris) // auto const &tri -> can't change tri, no copies are made
        {
            // Stages of transformation
            triangle triProj, triTrans, triView;

            // Apply transformations
            triTrans.p[0] = get_mul_mat4x4_v3d(worldMat, tri.p[0]);
            triTrans.p[1] = get_mul_mat4x4_v3d(worldMat, tri.p[1]);
            triTrans.p[2] = get_mul_mat4x4_v3d(worldMat, tri.p[2]);

            // Apply view transformations
            triView.p[0] = get_mul_mat4x4_v3d(viewMat, triTrans.p[0]);
            triView.p[1] = get_mul_mat4x4_v3d(viewMat, triTrans.p[1]);
            triView.p[2] = get_mul_mat4x4_v3d(viewMat, triTrans.p[2]);

            // Apply projection
            triProj.p[0] = get_mul_mat4x4_v3d(projMat, triView.p[0]);
            triProj.p[1] = get_mul_mat4x4_v3d(projMat, triView.p[1]);
            triProj.p[2] = get_mul_mat4x4_v3d(projMat, triView.p[2]);

            // Normalize
            triProj.p[0] = triProj.p[0] / triProj.p[0].w;
            triProj.p[1] = triProj.p[1] / triProj.p[1].w;
            triProj.p[2] = triProj.p[2] / triProj.p[2].w;

            // With wireframe, don't backface cull. 
            // We simply remove faces not facing the cam, not the ones being "blocked" by others.
            
            // Load faces to buffer so that they may be processed before rendering

            triBuffer.push_back(triProj);
            
        }

        // Painter's algorithm; sort triangle render order from back to front based on midpoint
        sort_tri_buffer(triBuffer);

        // Render triangle buffer
        for (auto &t : triBuffer) // auto &t -> able to change t (for mapping)
        {
            // Scale to center
            map_screen_space(t.p[0], LOGICAL_WT, LOGICAL_HT);
            map_screen_space(t.p[1], LOGICAL_WT, LOGICAL_HT);
            map_screen_space(t.p[2], LOGICAL_WT, LOGICAL_HT);

            // Draw the triangle
            draw_tri_wireF(
                hRend,
                t.p[0].x, t.p[0].y,
                t.p[1].x, t.p[1].y,
                t.p[2].x, t.p[2].y,
                255, 255, 255
            );
        }

        // Present ("flip") back buffer
        SDL_RenderPresent(hRend);

        // Done with this set of triangles. Clear for next loop.
        triBuffer.clear();
    }

    // Destroy renderer, window on quit
    SDL_DestroyRenderer(hRend);
    SDL_DestroyWindow(hWin);
    SDL_Quit();

    return 0;
}