#include "engine.h"
#include "obj_loader.h"

/*

My first 3D engine.

I'm following javidx9's tutorial:
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
    
    SDL_RenderSetLogicalSize(hRend, LOGICAL_WT, LOGICAL_HT);

    SDL_Event event;
   
    // Program vars
    float aspect = (float)LOGICAL_HT / LOGICAL_WT;
    float fov = RAD(90.0f);
    float zFar = 1000.0f;
    float zNear = 0.1f;

    // The key to 3D
    M4x4 matProj;
    matProj = get_projection_matrix(fov, aspect, zNear, zFar);

    float xRot = 0.0f, yRot = 0.0f, zRot = 0.0f;
    float xPos = 0.0f, yPos = 0.0f, zPos = 100.0f;
    float rotSpeed = 0.001f;

    float moveSpeed = 0.0001f;

    v3d camPos = { 0.0f, 0.0f, 0.0f };

    mesh object;
    object.tris = load_obj_from_fname("cube.obj");

    vector<triangle> triBuffer;

    // Loop vars
    bool run = true;

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
        // Render
        SDL_SetRenderDrawColor(hRend, 0, 0, 0, 255); // Clear white
        SDL_RenderClear(hRend);

        for (auto &tri : object.tris)
        {
            triangle triProjected, triTranslated, triRotated;


            triRotated = tri;

            if (rotX)  xRot -= rotSpeed;
            if (nrotX) xRot += rotSpeed;
            if (rotY) yRot -= rotSpeed;
            if (nrotY) yRot += rotSpeed;
            if (rotZ) zRot -= rotSpeed;
            if (nrotZ) zRot += rotSpeed;
            
        
            rot_p_x(triRotated.p[0], RAD(xRot));
            rot_p_x(triRotated.p[1], RAD(xRot));
            rot_p_x(triRotated.p[2], RAD(xRot));

            rot_p_y(triRotated.p[0], RAD(yRot));
            rot_p_y(triRotated.p[1], RAD(yRot));
            rot_p_y(triRotated.p[2], RAD(yRot));

            rot_p_z(triRotated.p[0], RAD(zRot));
            rot_p_z(triRotated.p[1], RAD(zRot));
            rot_p_z(triRotated.p[2], RAD(zRot));


            triTranslated = triRotated;

            if (up) yPos += moveSpeed;
            if (down) yPos -= moveSpeed;
            if (left)  xPos += moveSpeed;
            if (right) xPos -= moveSpeed;
            if (in) zPos -= moveSpeed;
            if (out) zPos += moveSpeed;

            triTranslated.p[0].x += xPos;
            triTranslated.p[1].x += xPos;
            triTranslated.p[2].x += xPos;

            triTranslated.p[0].y += yPos;
            triTranslated.p[1].y += yPos;
            triTranslated.p[2].y += yPos;

            triTranslated.p[0].z += zPos;
            triTranslated.p[1].z += zPos;
            triTranslated.p[2].z += zPos;

            // back-face culling
            v3d normal = get_norm(triTranslated);

            if (check_norm_visible(normal, triTranslated, camPos))
            {
                // Input/output format parameters
                mul_mat4x4_v3d(triTranslated.p[0], triProjected.p[0], matProj);
                mul_mat4x4_v3d(triTranslated.p[1], triProjected.p[1], matProj);
                mul_mat4x4_v3d(triTranslated.p[2], triProjected.p[2], matProj);

                triBuffer.push_back(triProjected);
            }   
        }

        sort_tri_buffer(triBuffer);

        for (auto& t : triBuffer)
        {
            // Scale to center
            map_screen_space(t.p[0], LOGICAL_WT, LOGICAL_HT);
            map_screen_space(t.p[1], LOGICAL_WT, LOGICAL_HT);
            map_screen_space(t.p[2], LOGICAL_WT, LOGICAL_HT);

            // Draw the triangle
  
            
            draw_tri_rasterF_bresenham(
                hRend,
                t.p[0].x, t.p[0].y,
                t.p[1].x, t.p[1].y,
                t.p[2].x, t.p[2].y,
                t.r, t.g, t.b
            );

            //cout << t.r << ' ' << t.g << ' ' << t.b << endl;
        }

        SDL_RenderPresent(hRend);

        triBuffer.clear();
    }


    SDL_DestroyRenderer(hRend);
    SDL_DestroyWindow(hWin);
    SDL_Quit();

    return 0;
}
