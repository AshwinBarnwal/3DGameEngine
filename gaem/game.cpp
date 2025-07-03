#define is_down(b) input->buttons[b].is_down
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)

float player_1_p, player_1_dp, player_2_p, player_2_dp;
float arena_half_size_x=85, arena_half_size_y=45;
float player_half_size_x = 2.5, player_half_size_y = 12;
float ball_speed=150,ball_p_x, ball_p_y, ball_dp_x=ball_speed, ball_dp_y;
float cursorx=0, cursory=0;

mesh meshCube;
float* pDepthBuffer = nullptr;
mat4x4 matProj;
vec3d vCamera;
vec3d vLookDir;
float fYaw;
float fTheta;

Light gLights[] = {
    { Vector_Normalize({  0.0f,  0.0f, -1.0f }), 1.0f },  // key light
    { Vector_Normalize({ 0.0f, 0.0f, 1.0f }), 0.2f },  // fill light
    { Vector_Normalize({  -1.0f,  0.0f, -1.0f }), 0.3f }   // rim light
};

internal bool
initialize_game() 
{
    pDepthBuffer = new float[render_state.width * render_state.height];
	matProj = Matrix_MakeProjection(90.0f, (float)render_state.height / (float)render_state.width, 0.1f, 1000.0f);
    //meshCube.tris = {

    //    // SOUTH
    //    triangle(vec3d(0.0f, 0.0f, 0.0f),    vec3d(0.0f, 1.0f, 0.0f),    vec3d(1.0f, 1.0f, 0.0f) , vec2d(0.0f, 1.0f), vec2d(0.0f, 0.0f), vec2d(1.0f, 0.0f)),
    //    triangle(vec3d(0.0f, 0.0f, 0.0f),    vec3d(1.0f, 1.0f, 0.0f),    vec3d(1.0f, 0.0f, 0.0f) , vec2d(0.0f, 1.0f), vec2d(1.0f, 0.0f), vec2d(1.0f, 1.0f)),

    //    // EAST                                                      
    //    triangle(vec3d(1.0f, 0.0f, 0.0f),    vec3d(1.0f, 1.0f, 0.0f),    vec3d(1.0f, 1.0f, 1.0f) , vec2d(0.0f, 1.0f), vec2d(0.0f, 0.0f), vec2d(1.0f, 0.0f)),
    //    triangle(vec3d(1.0f, 0.0f, 0.0f),    vec3d(1.0f, 1.0f, 1.0f),    vec3d(1.0f, 0.0f, 1.0f) , vec2d(0.0f, 1.0f), vec2d(1.0f, 0.0f), vec2d(1.0f, 1.0f)),

    //    // NORTH                                                     
    //    triangle(vec3d(1.0f, 0.0f, 1.0f),    vec3d(1.0f, 1.0f, 1.0f),    vec3d(0.0f, 1.0f, 1.0f) , vec2d(0.0f, 1.0f), vec2d(0.0f, 0.0f), vec2d(1.0f, 0.0f)),
    //    triangle(vec3d(1.0f, 0.0f, 1.0f),    vec3d(0.0f, 1.0f, 1.0f),    vec3d(0.0f, 0.0f, 1.0f) , vec2d(0.0f, 1.0f), vec2d(1.0f, 0.0f), vec2d(1.0f, 1.0f)),

    //    // WEST                                                      
    //    triangle(vec3d(0.0f, 0.0f, 1.0f),    vec3d(0.0f, 1.0f, 1.0f),    vec3d(0.0f, 1.0f, 0.0f) ,vec2d(0.0f, 1.0f), vec2d(0.0f, 0.0f), vec2d(1.0f, 0.0f)),
    //    triangle(vec3d(0.0f, 0.0f, 1.0f),    vec3d(0.0f, 1.0f, 0.0f),    vec3d(0.0f, 0.0f, 0.0f) , vec2d(0.0f, 1.0f), vec2d(1.0f, 0.0f), vec2d(1.0f, 1.0f)),

    //    // TOP                                                       
    //    triangle(vec3d(0.0f, 1.0f, 0.0f),    vec3d(0.0f, 1.0f, 1.0f),    vec3d(1.0f, 1.0f, 1.0f) , vec2d(0.0f, 1.0f), vec2d(0.0f, 0.0f), vec2d(1.0f, 0.0f)),
    //    triangle(vec3d(0.0f, 1.0f, 0.0f),    vec3d(1.0f, 1.0f, 1.0f),    vec3d(1.0f, 1.0f, 0.0f) , vec2d(0.0f, 1.0f), vec2d(1.0f, 0.0f), vec2d(1.0f, 1.0f)),

    //    // BOTTOM                                                    
    //    triangle(vec3d(1.0f, 0.0f, 1.0f),    vec3d(0.0f, 0.0f, 1.0f),    vec3d(0.0f, 0.0f, 0.0f) , vec2d(0.0f, 1.0f), vec2d(0.0f, 0.0f), vec2d(1.0f, 0.0f)),
    //    triangle(vec3d(1.0f, 0.0f, 1.0f),    vec3d(0.0f, 0.0f, 0.0f),    vec3d(1.0f, 0.0f, 0.0f) , vec2d(0.0f, 1.0f), vec2d(1.0f, 0.0f), vec2d(1.0f, 1.0f)),

    //    };
    //return true;
	return meshCube.LoadFromObjectFile("utah.obj"); 
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
	//fTheta += 1.0f * dt;

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

#pragma omp parallel
    {
        vector<triangle> localList;

#pragma omp for schedule(dynamic)
        for (int i = 0; i < (int)meshCube.tris.size(); ++i)
        {
            const triangle& tri = meshCube.tris[i];

            triangle triProjected{}, triTransformed{}, triViewed{};

            triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
            triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
            triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);
            triTransformed.base = tri.base;
            triTransformed.t[0] = tri.t[0];
            triTransformed.t[1] = tri.t[1];
            triTransformed.t[2] = tri.t[2];

            vec3d line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
            vec3d line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);
            vec3d normal = Vector_Normalize(Vector_CrossProduct(line1, line2));
            vec3d vCameraRay = Vector_Sub(triTransformed.p[0], vCamera);

            if (Vector_DotProduct(normal, vCameraRay) < 0.0f)
            {
                float totalLum = 0.0f;
                for (int i=0;i<3;i++)
                {
                    float dp = max(0.0f, Vector_DotProduct(normal, gLights[i].dir));
                    totalLum += dp * gLights[i].intensity;
                }
                //vec3d light_direction = { cursorx * 0.1f, cursory * 0.0f, -1.0f };
                //light_direction = Vector_Normalize(light_direction);
                //float dp = max(0.1f, Vector_DotProduct(light_direction, normal));
                triTransformed.col = get_color(triTransformed, totalLum);
                //triTransformed.col = triTransformed.base;
                triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
                triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
                triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);
                triViewed.t[0] = triTransformed.t[0];
                triViewed.t[1] = triTransformed.t[1];
                triViewed.t[2] = triTransformed.t[2];
                triangle clipped[2];
                int nClipped = Triangle_ClipAgainstPlane(
                    { 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f },
                    triViewed, clipped[0], clipped[1]);
                // We may end up with multiple triangles form the clip, so project as
                // required
                for (int n = 0; n < nClipped; ++n)
                {
                    // Project triangles from 3D --> 2D
                    triProjected.p[0] = Matrix_MultiplyVector(matProj, clipped[n].p[0]);
                    triProjected.p[1] = Matrix_MultiplyVector(matProj, clipped[n].p[1]);
                    triProjected.p[2] = Matrix_MultiplyVector(matProj, clipped[n].p[2]);
                    triProjected.col = triTransformed.col;
                    
                    triProjected.t[0] = clipped[n].t[0];
					triProjected.t[1] = clipped[n].t[1];
					triProjected.t[2] = clipped[n].t[2];


					triProjected.t[0].u = triProjected.t[0].u / triProjected.p[0].w;
					triProjected.t[1].u = triProjected.t[1].u / triProjected.p[1].w;
					triProjected.t[2].u = triProjected.t[2].u / triProjected.p[2].w;

					triProjected.t[0].v = triProjected.t[0].v / triProjected.p[0].w;
					triProjected.t[1].v = triProjected.t[1].v / triProjected.p[1].w;
					triProjected.t[2].v = triProjected.t[2].v / triProjected.p[2].w;

					triProjected.t[0].w = 1.0f / triProjected.p[0].w;
					triProjected.t[1].w = 1.0f / triProjected.p[1].w;
					triProjected.t[2].w = 1.0f / triProjected.p[2].w;

                    triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
                    triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
                    triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

                    vec3d vOffsetView = { 1.0f, 1.0f, 0.0f };
                    for (int k = 0; k < 3; ++k)
                    {
                        triProjected.p[k] = Vector_Add(triProjected.p[k], vOffsetView);
                        triProjected.p[k].x *= 0.5f * render_state.width;
                        triProjected.p[k].y *= 0.5f * render_state.height;
                    }

                    localList.push_back(triProjected);
                }
            }
        }

#pragma omp critical
        vecTrianglesToRaster.insert(vecTrianglesToRaster.end(),
            localList.begin(), localList.end());
    } // end parallel region

    std::sort(std::execution::par_unseq,
        vecTrianglesToRaster.begin(),
        vecTrianglesToRaster.end(),
        [](const triangle& t1, const triangle& t2)
        {
            float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
            float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
            return z1 > z2;
        });

    for (auto& triToRaster : vecTrianglesToRaster)
    {
        triangle clipped[2];
        std::list<triangle> listTriangles;
        listTriangles.push_back(triToRaster);

        int nNewTriangles = 1;
        for (int p = 0; p < 4; ++p)
        {
            while (nNewTriangles > 0)
            {
                triangle test = listTriangles.front();
                listTriangles.pop_front();
                --nNewTriangles;

                int nTrisToAdd = 0;
                switch (p)
                {
                case 0: nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
                case 1: nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, (float)render_state.height - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
                case 2: nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
                case 3: nTrisToAdd = Triangle_ClipAgainstPlane({ (float)render_state.width - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
                }

                for (int w = 0; w < nTrisToAdd; ++w)
                    listTriangles.push_back(clipped[w]);
            }
            nNewTriangles = (int)listTriangles.size();
        }

        for (auto& t : listTriangles)
            fill_triangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, t.col);
            //draw_triangle_new(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, 0xffffff, 2);
    }

    draw_rect(cursorx, cursory, 1.0f, 1.0f, 0xffffff);
}