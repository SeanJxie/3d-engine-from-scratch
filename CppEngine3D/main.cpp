#include "engine.h"
#include "obj_loader.h"

/*

My first 3D engine.

I'm using javidx9's tutorial as a guide:
https://www.youtube.com/watch?v=ih20l3pJoeU&list=RDCMUC-yuWVUplUJZvieEligKBkA&index=3

*/


#if _WIN64 // 64-bit SDL
#define RENDER_FLAG SDL_RENDERER_SOFTWARE

// Program constants
int WINWT = 800, WINHT = 800;
int LOGICAL_WT = 200, LOGICAL_HT = 200; // For res change
const char* WINTT = "C++ Engine3D Demo";


int main(int argc, char* argv[])
{   
	cout << "PROGRAM START\n\n";
	// SDL setup
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* hWin = SDL_CreateWindow(WINTT, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINWT, WINHT, SDL_WINDOW_SHOWN);
	SDL_Renderer* hRend = SDL_CreateRenderer(hWin, -1, RENDER_FLAG);
	SDL_ShowCursor(false);

	SDL_RenderSetLogicalSize(hRend, LOGICAL_WT, LOGICAL_HT);

	SDL_Event event;
   
	// Program matrices
	M4x4 projMat, matRotX, matRotY, matRotZ, matTranslate, worldMat;

	// Projection matrix
	float aspect = (float)LOGICAL_HT / LOGICAL_WT;
	float fov = RAD(90.0f);
	float zFar = 100.0f;
	float zNear = 0.1f;

	get_projection_matrix(fov, aspect, zNear, zFar, projMat);

	// Movement vars
	float allRot = 0.0f;
	float rotSpeed = 1.0f;
	float horizontalSpeed = 0.01f;
	float verticalSpeed = 0.01f;

	// Camera
	v3d camPos = { 0.0f, 0.0f, 0.0f };
	v3d lookDir;
	v3d upDir;
	v3d target;
	M4x4 cameraMat;
	M4x4 viewMat;
	float camYaw = 0.0f;
	float camPitch = 0.0f;

	M4x4 camRotYaw, camRotPitch;
	v3d forwardVec, rightwardVec;

	const float ABSMAXPITCH = 89.999f; // standard

	// Mesh/triangle/vertex 
	mesh object;
	object.tris = load_obj_mtl_fname("teapot.obj", true, "teapot.mtl");

	vector<triangle> triBuffer;
	triangle triProj, triTrans, triView;

	int clipped_tris = 0;
	triangle clipped[2]; // max 2 clipped triangles in the case of quad split

	triangle tempTri, clippedPair[2]; // Maxiumum of 2 resultant traingles after a triangle clip
	list<triangle> clippedTriList;  // Store all triangles we want to draw
	int newTriCount = 1;     // There is 1 new triangle in the list

	// Loop vars
	bool run = true;
	Uint64 curr = SDL_GetPerformanceCounter(), prev = 0;
	float dt;

	// Input vars
	bool up = false, down = false, left = false, right = false, in = false, out = false;
	int mposX, mposY;
	float mouseSens = 0.1f;

	while (run)
	{
		// Process delta time
		prev = curr;
		curr = SDL_GetPerformanceCounter();
		dt = ((curr - prev) * 1000 / (float)SDL_GetPerformanceFrequency());
		//cout << "FPS: " << 1.0f / (dt * 0.001f) << '\r';

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
				case SDLK_EQUALS: 
					LOGICAL_WT += 1; 
					LOGICAL_HT += 1; 
					SDL_RenderSetLogicalSize(hRend, LOGICAL_WT, LOGICAL_HT);
					break;
				case SDLK_MINUS: 
					LOGICAL_WT -= 1; 
					LOGICAL_HT -= 1; 
					SDL_RenderSetLogicalSize(hRend, LOGICAL_WT, LOGICAL_HT);
					break;
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
		
		camPitch += mouseSens * (WINHT / 2.0f - mposY);
		camYaw += mouseSens * (WINWT / 2.0f - mposX);
		
		// We don't want the user to rotate beyond 90 deg up/down
		if (fabs(camPitch) > ABSMAXPITCH) camPitch = ABSMAXPITCH * ((camPitch > 0.0f) ? 1.0f : -1.0f);

		upDir = { 0.0f, 1.0f, 0.0f }; // y-axis is up
		target = { 0.0f, 0.0f, 1.0f }; // look z-axis

		// Adjust target position based on y rotation (yaw)
		// Establish a rotation matrix based on yaw
		get_rot_y(RAD(camYaw), camRotYaw);
		get_rot_x(RAD(camPitch), camRotPitch);

		// rotate look direction about the origin with our rotation matrix based on target
		get_mul_mat4x4_v3d(camRotPitch * camRotYaw, target, lookDir);

		// offset our target based on the direction we look and camera position
		target = camPos + lookDir; 

		// Construct camera matrix based on its position, where its looking, and where up is
		get_mat_pointat(camPos, target, upDir, cameraMat);

		// cameraMat specifies everything our camera does. In reality, we move the objects
		// such that it seems as if the camera is moving, relative to the objects. Therefore, our objects need to do the 
		// "opposite" of whatever the camera is said to do; we take the inverse of the camera matrix
		// and apply it to our objects. 
		// The view matrix is a fundemental processing stage, along with projection and transformation:
		viewMat = cameraMat.get_inverse();
		
		

		// Get right vector for left/right movement: simple cross product of up and looking direction. It is already normalized
		// https://en.wikipedia.org/wiki/Right-hand_rule
		rightwardVec = normv3d(crossv3d(lookDir, upDir)) * horizontalSpeed * dt;

		// Forward vector is the negative cross of our right vector and up vector
		forwardVec = normv3d(crossv3d(rightwardVec, upDir)) * horizontalSpeed * dt;
		forwardVec.y = 0.0f; // We don't want to move up at the direction we look, only horizontally

		// What kinda messed up camera has vertical movement based on camera angle... no temp_up vector needed here

		// Process input 
		// Add forwards / right vectors to camera position to move
		if (in) camPos = camPos - forwardVec; // We subtract here to compensate for the negative of the crossed right vector
		if (out) camPos = camPos + forwardVec;
		
		if (left) camPos = camPos + rightwardVec;
		if (right) camPos = camPos - rightwardVec;

		// For up and down, "up" stays the same, no matter the orientation of our cam.
		// Effectively, the x and y components of our uneeded "temp_up" vector would both be 0, so we just add y components.
		if (up) camPos.y -= verticalSpeed * dt;
		if (down) camPos.y += verticalSpeed * dt;

		// Define transformation matrices (object, not cam), all relative to the origin
		get_rot_x(RAD(90.0f), matRotX);
		get_rot_y(RAD(90.0f), matRotY);
		get_rot_z(RAD(90.0f), matRotZ);

		get_trans_mat(0.0f, 0.0f, 0.0f, matTranslate);

		// Construct world matrix
		// From wikipedia:
		// "Matrices representing other geometric transformations can be combined with this [identity matrix] and 
		//  each other by matrix multiplication. As a result, any perspective projection of space 
		//  can be represented as a single [world] matrix."

		worldMat.cast_identity(); 
		worldMat = matRotX * matRotY * matRotZ;
		worldMat = worldMat * matTranslate;

		// Go through every triangle in the mesh
		for (auto &tri : object.tris) // auto const &tri -> can't change tri, no copies are made
		{
			// Stages of transformation
			/*
			(1) -> We want to orient our objects. How? World matrix.
			(2) -> We want to transform our objects such that they obey the properties of out camera. How? View matrix.
			(3) -> We want to convert our 3D points to 2D points to be drawn on the window. How? Projection matrix.
			*/
			
			// Apply transformations
			get_mul_mat4x4_v3d(worldMat, tri.p[0], triTrans.p[0]);
			get_mul_mat4x4_v3d(worldMat, tri.p[1], triTrans.p[1]);
			get_mul_mat4x4_v3d(worldMat, tri.p[2], triTrans.p[2]);

			// With wireframe, don't backface cull. 
			// We simply remove faces not facing the cam, not the ones being "blocked" by others.
			if (check_tri_visible(triTrans, camPos))
			{
				// Apply view transformations
				get_mul_mat4x4_v3d(viewMat, triTrans.p[0], triView.p[0]);
				get_mul_mat4x4_v3d(viewMat, triTrans.p[1], triView.p[1]);
				get_mul_mat4x4_v3d(viewMat, triTrans.p[2], triView.p[2]);

				// We've already transformed our points via the view matrix. We can now easily
				// Apply our clipping algorithm with the zNear plane
				clipped_tris = 0;

				// Notice the zNear normal runs along the z-axis. So, we can hardcode { 0.0f, 0.0f, 1.0f } as our plane normal
				clipped_tris = clip_tri_plane({ 0.0f, 0.0f, zNear }, { 0.0f, 0.0f, 1.0f }, triView, clipped[0], clipped[1]);
				// Go through all clipped tris
				for (int i = 0; i < clipped_tris; i++)
				{
					// Apply projection on our clipped triangles
					get_mul_mat4x4_v3d(projMat, clipped[i].p[0], triProj.p[0]);
					get_mul_mat4x4_v3d(projMat, clipped[i].p[1], triProj.p[1]);
					get_mul_mat4x4_v3d(projMat, clipped[i].p[2], triProj.p[2]);
					
					// Normalize
					triProj.p[0] = triProj.p[0] / triProj.p[0].w;
					triProj.p[1] = triProj.p[1] / triProj.p[1].w;
					triProj.p[2] = triProj.p[2] / triProj.p[2].w;


					// Scale to center
					map_screen_space(triProj.p[0], LOGICAL_WT, LOGICAL_HT);
					map_screen_space(triProj.p[1], LOGICAL_WT, LOGICAL_HT);
					map_screen_space(triProj.p[2], LOGICAL_WT, LOGICAL_HT);

					// Load faces to buffer so that they may be processed before rendering
					triProj.c = tri.c;
					triBuffer.push_back(triProj);     
				}
			}
		}

		// Painter's algorithm; sort triangle render order from back to front based on midpoint
		// https://en.wikipedia.org/wiki/Painter%27s_algorithm
		sort_tri_buffer(triBuffer);

		// Clear back buffer from last frame
		SDL_SetRenderDrawColor(hRend, 25, 217, 253, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(hRend);

		for (auto& t : triBuffer) // auto &t -> able to change t (for mapping)
		{
			clippedTriList.clear();  // Store all triangles we want to draw
			clippedTriList.push_back(t);    // The current triangle is indeed a triangle we need to draw. Add it to queue.
			newTriCount = 1;     // There is 1 new triangle in the list

			for (int p = 0; p < 4; p++)
			{
				int trisToAdd = 0;
				while (newTriCount > 0)
				{
					tempTri = clippedTriList.front();
					clippedTriList.pop_front();
					newTriCount--;

					switch (p)
					{
					case 0:	
						trisToAdd = clip_tri_plane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, tempTri, clippedPair[0], clippedPair[1]); break;
					case 1:	
						trisToAdd = clip_tri_plane({ 0.0f, (float)LOGICAL_HT - 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, tempTri, clippedPair[0], clippedPair[1]); break;
					case 2:	
						trisToAdd = clip_tri_plane({ (float)LOGICAL_WT - 1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, tempTri, clippedPair[0], clippedPair[1]); break;
					case 3:	
						trisToAdd = clip_tri_plane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, tempTri, clippedPair[0], clippedPair[1]); break;	
					}

					for (int w = 0; w < trisToAdd; w++)
					{
						clippedTriList.push_back(clippedPair[w]);
					}
				}

				newTriCount = (int)clippedTriList.size();
			}

			for (auto& t_final : clippedTriList)
			{
				// Draw the triangle
				draw_tri_raster_bresenhamF(
					hRend,
					t_final.p[0].x, t_final.p[0].y,
					t_final.p[1].x, t_final.p[1].y,
					t_final.p[2].x, t_final.p[2].y,
					t_final.c.r, t_final.c.g, t_final.c.b
				);
			}
		}

		// Done with this set of triangles. Clear for next loop.
		triBuffer.clear();


		// Render "over lays" (nothing to do with projection)
		SDL_Rect crosshair = { (int)LOGICAL_WT / 2 - 1, (int)LOGICAL_HT / 2 - 1, 2, 2 };
		SDL_SetRenderDrawColor(hRend, 255, 255, 255, SDL_ALPHA_OPAQUE);
		SDL_RenderDrawRect(hRend, &crosshair);

		// Present ("flip") back buffer
		SDL_RenderPresent(hRend);
	}

	// Destroy renderer, window on quit
	SDL_DestroyRenderer(hRend);
	SDL_DestroyWindow(hWin);
	SDL_Quit();

	cout << "\nPROGRAM END\n";

	return 0;
}

#endif