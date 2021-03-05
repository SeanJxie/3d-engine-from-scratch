#include "engine.h"
#include "obj_loader.h"

/*

My first 3D engine.

I'm using javidx9's tutorial as a guide:
https://www.youtube.com/watch?v=ih20l3pJoeU&list=RDCMUC-yuWVUplUJZvieEligKBkA&index=3

*/


// Program constants
int WINWT = 700, WINHT = 700;
int LOGICAL_WT = 700, LOGICAL_HT = 700;
const char* WINTT = "C++ Engine3D Demo";


int main(int argc, char* argv[])
{
    // SDL setup
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* hWin = SDL_CreateWindow(WINTT, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINWT, WINHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* hRend = SDL_CreateRenderer(hWin, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    //SDL_RenderSetLogicalSize(hRend, LOGICAL_WT, LOGICAL_HT);

    SDL_Event event;
   
    // Program vars
    float aspect = (float)LOGICAL_HT / LOGICAL_WT;
    float fov = RAD(90.0f);
    float zFar = 1000.0f;
    float zNear = 0.1f;

    // Set up matrices
    M4x4 matProj, matRotX, matRotY, matRotZ, matTranslate, worldMat;
    matProj = get_projection_matrix(fov, aspect, zNear, zFar);

    // Movement vars
    float xRot = 0.0f, yRot = 0.0f, zRot = 0.0f;
    float xPos = 0.0f, yPos = 0.0f, zPos = 0.0f;
    float allRot = 0.0f;
    float rotSpeed = 1.0f;

    float moveSpeed = 0.01f;

    v3d camPos = { 0.0f, 0.0f, 0.0f };

    mesh object;
    object.tris = load_obj_from_fname("space_shuttle.obj");

    vector<triangle> triBuffer;

    // Loop vars
    bool run = true;

    // Input vars
    bool rotX = false, nrotX = false, rotY = false, nrotY = false, rotZ = false, nrotZ = false;
    bool up = false, down = false, left = false, right = false, in = false, out = false;

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

                case SDLK_UP: rotX = true; break;
                case SDLK_DOWN: nrotX = true; break;
                case SDLK_LEFT: rotZ = true; break;
                case SDLK_RIGHT: nrotZ = true; break;
                case SDLK_RSHIFT: rotY = true; break;
                case SDLK_RCTRL: nrotY = true; break;
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

                case SDLK_UP: rotX = false; break;
                case SDLK_DOWN: nrotX = false; break;
                case SDLK_LEFT: rotZ = false; break;
                case SDLK_RIGHT: nrotZ = false; break;
                case SDLK_RSHIFT: rotY = false; break;
                case SDLK_RCTRL: nrotY = false; break;
                }

                break;
            }
            
        }

        allRot += rotSpeed;

        // Define transformation matrices
        matRotX = get_rot_x(RAD(allRot));
        matRotY = get_rot_y(RAD(allRot));
        matRotZ = get_rot_z(RAD(allRot));

        matTranslate = get_trans_mat(0.0f, 0.0f, 15.0f);

        // Construct world matrix
        worldMat.cast_identity(); 
        worldMat = matRotX * matRotY * matRotZ;
        worldMat = worldMat * matTranslate;

        // Clear draw buffer
        SDL_SetRenderDrawColor(hRend, 0, 0, 0, SDL_ALPHA_OPAQUE); 
        SDL_RenderClear(hRend);

        for (auto& tri : object.tris)
        {
            triangle triProj, triTrans;

            triTrans.p[0] = get_mul_mat4x4_v3d(worldMat, tri.p[0]);
            triTrans.p[1] = get_mul_mat4x4_v3d(worldMat, tri.p[1]);
            triTrans.p[2] = get_mul_mat4x4_v3d(worldMat, tri.p[2]);

            triProj.p[0] = get_mul_mat4x4_v3d(matProj, triTrans.p[0]);
            triProj.p[1] = get_mul_mat4x4_v3d(matProj, triTrans.p[1]);
            triProj.p[2] = get_mul_mat4x4_v3d(matProj, triTrans.p[2]);

            // Normalize
            triProj.p[0] = triProj.p[0] / triProj.p[0].w;
            triProj.p[1] = triProj.p[1] / triProj.p[1].w;
            triProj.p[2] = triProj.p[2] / triProj.p[2].w;

            // With wireframe, don't backface cull
            triBuffer.push_back(triProj);
        }

        // Painter's algorithm; sort triangle render order from back to front based on midpoint
        sort_tri_buffer(triBuffer);

        // Render triangle buffer
        for (auto& t : triBuffer)
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

            //cout << t.r << ' ' << t.g << ' ' << t.b << endl;
        }

        SDL_RenderPresent(hRend);

        // Done with this set of triangles. Clear for next loop.
        triBuffer.clear();
    }


    SDL_DestroyRenderer(hRend);
    SDL_DestroyWindow(hWin);
    SDL_Quit();

    return 0;
}