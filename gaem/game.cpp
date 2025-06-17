#define is_down(b) input->buttons[b].is_down
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)

float player_1_p, player_1_dp, player_2_p, player_2_dp;
float arena_half_size_x=85, arena_half_size_y=45;
float player_half_size_x = 2.5, player_half_size_y = 12;
float ball_speed=150,ball_p_x, ball_p_y, ball_dp_x=ball_speed, ball_dp_y;
float cursorx=0, cursory=0;

mesh meshCube;

mat4x4 matProj;
vec3d vCamera;
vec3d vLookDir;
float fYaw;
float fTheta;

internal bool
initialize_game() 
{
	matProj = Matrix_MakeProjection(90.0f, (float)render_state.height / (float)render_state.width, 0.1f, 1000.0f);

	return meshCube.LoadFromObjectFile("icosphere.obj"); 
}

internal void
simulate_player(float* p, float* dp, float ddp, float dt) {
	ddp -= *dp * 10.f;
	*p = *p + (*dp) * dt + ddp * dt * dt * 0.5f;
	*dp = *dp + ddp * dt;

	if (*p + player_half_size_y > arena_half_size_y) {
		*p = arena_half_size_y - player_half_size_y;
		*dp = 0;
	}
	if (*p - player_half_size_y < -arena_half_size_y) {
		*p = -arena_half_size_y + player_half_size_y;
		*dp = 0;
	}
}

internal void
simulate_game(Input* input, float dt) {
	
	clear_screen(0x000000);
	cursorx += 0.1 * (input->cursor_pos.x - cursor_origin.x);
	cursory -= 0.1*(input->cursor_pos.y-cursor_origin.y);
	if (is_down(BUTTON_UP)) vCamera.y += 8.0f * dt;
	if (is_down(BUTTON_DOWN)) vCamera.y -= 8.0f * dt;
	if (is_down(BUTTON_LEFT)) vCamera.x += 8.0f * dt;
	if (is_down(BUTTON_RIGHT)) vCamera.x -= 8.0f * dt;

	vec3d vForward = Vector_Mul(vLookDir, 8.0f * dt);

	if (is_down(BUTTON_W)) vCamera = Vector_Add(vCamera, vForward);
	if (is_down(BUTTON_S)) vCamera = Vector_Sub(vCamera, vForward);
	
	if (is_down(BUTTON_A)) fYaw += 2.0f * dt;
	if (is_down(BUTTON_D)) fYaw -= 2.0f * dt;
	
	mat4x4 matRotZ, matRotX;
	fTheta += 1.0f * dt;

	matRotZ = Matrix_MakeRotationZ(fTheta * 0.5f);
	matRotX = Matrix_MakeRotationX(fTheta);

	mat4x4 matTrans;
	matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 3.0f);

	mat4x4 matWorld;
	matWorld = Matrix_MakeIdentity();	// Form World Matrix
	matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX); // Transform by rotation
	matWorld = Matrix_MultiplyMatrix(matWorld, matTrans); // Transform by translation


	//vLookDir = { 0, 0, 1 };
	vec3d vUp = { 0, 1, 0 };
	vec3d vTarget = { 0, 0, 1 };
	mat4x4 matCameraRot = Matrix_MakeRotationY(fYaw);
	vLookDir = Matrix_MultiplyVector(matCameraRot, vTarget);
	vTarget = Vector_Add(vCamera, vLookDir);

	mat4x4 matCamera = Matrix_PointAt(vCamera, vTarget, vUp);

	mat4x4 matView = Matrix_QuickInverse(matCamera);

	vector<triangle> vecTrianglesToRaster;
	 
	for (auto tri : meshCube.tris) 
	{
		triangle triProjected, triTransformed, triViewed;

		triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
		triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
		triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);

		vec3d line1, line2, normal;

		// Get lines either side of triangle
		line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
		line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);

		// Take cross product of lines to get normal to triangle surface
		normal = Vector_CrossProduct(line1, line2);

		// You normally need to normalise a normal!
		normal = Vector_Normalize(normal);

		//get ray from triangle to cam
		vec3d vCameraRay = Vector_Sub(triTransformed.p[0], vCamera);
		
		if (Vector_DotProduct(normal, vCameraRay) < 0.0f)
		{
			//Illumination
			vec3d light_direction = { cursorx*0.1f, cursory*0.0f, -1.0f };
			light_direction = Vector_Normalize(light_direction);

			// How "aligned" are light direction and triangle surface normal?
			float dp = max(0.1f, Vector_DotProduct(light_direction, normal));
			triTransformed.col = get_color(triTransformed,dp);


			triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
			triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
			triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);

			int nClippedTriangles = 0;
			triangle clipped[2];
			nClippedTriangles = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);

			for (int n = 0; n < nClippedTriangles; n++)
			{

				//Project triangles from 3D --> 2D
				triProjected.p[0] = Matrix_MultiplyVector(matProj, clipped[n].p[0]);
				triProjected.p[1] = Matrix_MultiplyVector(matProj, clipped[n].p[1]);
				triProjected.p[2] = Matrix_MultiplyVector(matProj, clipped[n].p[2]);
				triProjected.col = triTransformed.col;

				triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
				triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
				triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

				// Scale into view
				vec3d vOffsetView = { 1, 1, 0 };

				triProjected.p[0] = Vector_Add(triProjected.p[0], vOffsetView);
				triProjected.p[1] = Vector_Add(triProjected.p[1], vOffsetView);
				triProjected.p[2] = Vector_Add(triProjected.p[2], vOffsetView);
				triProjected.p[0].x *= 0.5f * (float)render_state.width;
				triProjected.p[0].y *= 0.5f * (float)render_state.height;
				triProjected.p[1].x *= 0.5f * (float)render_state.width;
				triProjected.p[1].y *= 0.5f * (float)render_state.height;
				triProjected.p[2].x *= 0.5f * (float)render_state.width;
				triProjected.p[2].y *= 0.5f * (float)render_state.height;

				vecTrianglesToRaster.push_back(triProjected);
			}
		}

		//Sort back to front
		sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2) 
		{
				float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
				float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
				return z1 > z2;
		});

		for (auto& triToRaster : vecTrianglesToRaster)
		{
			// Clip triangles against all four screen edges, this could yield
			// a bunch of triangles, so create a queue that we traverse to 
			//  ensure we only test new triangles generated against planes
			triangle clipped[2];
			list<triangle> listTriangles;

			// Add initial triangle
			listTriangles.push_back(triToRaster);
			int nNewTriangles = 1;

			for (int p = 0; p < 4; p++)
			{
				int nTrisToAdd = 0;
				while (nNewTriangles > 0)
				{
					// Take triangle from front of queue
					triangle test = listTriangles.front();
					listTriangles.pop_front();
					nNewTriangles--;

					// Clip it against a plane. We only need to test each 
					// subsequent plane, against subsequent new triangles
					// as all triangles after a plane clip are guaranteed
					// to lie on the inside of the plane. I like how this
					// comment is almost completely and utterly justified
					switch (p)
					{
					case 0:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 1:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, (float)render_state.height - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 2:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 3:	nTrisToAdd = Triangle_ClipAgainstPlane({ (float)render_state.width - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					}

					// Clipping may yield a variable number of triangles, so
					// add these new ones to the back of the queue for subsequent
					// clipping against next planes
					for (int w = 0; w < nTrisToAdd; w++)
						listTriangles.push_back(clipped[w]);
				}
				nNewTriangles = listTriangles.size();
			}


			// Draw the transformed, viewed, clipped, projected, sorted, clipped triangles
			for (auto& t : listTriangles)
			{
				fill_triangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, t.col);
				//draw_triangle_new(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, 0x000000, 2);
			}
		}
		draw_rect(cursorx, cursory, 1, 1, 0xffffff);
	}
	//draw_rect(ball_p_x, ball_p_y, 1, 1, 0xffffff);
	//draw_rect(80, player_1_p, player_half_size_x, player_half_size_y, 0xff0000);
	//draw_rect(-80, player_2_p, player_half_size_x, player_half_size_y, 0xff0000);
	//draw_triangle(-80, 80, 80, -80, 0, 80,0xffffff, 2);
}