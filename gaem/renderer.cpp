global_variable float render_scale = 0.01f;


struct vec3d
{
	float x=0, y=0, z=0, w=1;
};

struct triangle
{
	vec3d p[3];
	u32 base = 0xffffff;
	u32 col;
};

struct mesh
{
	vector<triangle> tris;

	bool LoadFromObjectFile(string sFilename)
	{
		ifstream f(sFilename);
		if (!f.is_open())
			return false;

		vector<vec3d> verts;
		while (!f.eof()) 
		{
			char line[128];
			f.getline(line, 128);
			
			strstream s;
			s << line;

			char junk;
			if (line[0] == 'v') 
			{
				vec3d v;
				s >> junk >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}
			
			if (line[0] == 'f')
			{
				int f[3];
				s >> junk >> f[0] >> f[1] >> f[2];
				tris.push_back({ verts[f[0] - 1], verts[f[1] - 1] , verts[f[2] - 1] });
			}
		}
		return true;
	}
};

struct mat4x4
{
	float m[4][4] = { 0 };
};

internal u32
get_color(triangle& tri, float lum) {
	if (lum <= 0) return 0x000000; // No light, fully black

	// Extract RGB components
	u8 r = ((u8)((tri.base >> 16) & 0xFF) * lum); // Max red (255) scaled by lum
	u8 g = ((u8)((tri.base >> 8) & 0xFF) * lum); // Max green (255) scaled by lum
	u8 b = ((u8)((tri.base) & 0xFF) * lum); // Max blue (255) scaled by lum

	// Pack into a 32-bit color (assuming ARGB or RGBX format)
	return (r << 16) | (g << 8) | b;
}


internal vec3d
Vector_Add(vec3d& v1, vec3d& v2)
{
	return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

internal vec3d
Vector_Sub(vec3d& v1, vec3d& v2)
{
	return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

internal vec3d
Vector_Mul(vec3d& v1, float k)
{
	return { v1.x * k , v1.y * k , v1.z * k };
}

internal vec3d
Vector_Div(vec3d& v1, float k)
{
	return { v1.x / k , v1.y / k , v1.z / k };
}

internal float
Vector_DotProduct(vec3d& v1, vec3d& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

internal float
Vector_Length(vec3d& v)
{
	return sqrtf(Vector_DotProduct(v, v));
}

internal vec3d
Vector_Normalize(vec3d& v)
{
	float l = Vector_Length(v);
	return { v.x / l, v.y / l, v.z / l };
}

internal vec3d
Vector_CrossProduct(vec3d& v1, vec3d& v2)
{
	vec3d v;
	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;
	return v;
}

internal vec3d 
Matrix_MultiplyVector(mat4x4 &m, vec3d &i)
{
	vec3d v;
	v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
	v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
	v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
	v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
	return v;
}

mat4x4 Matrix_MakeIdentity()
{
	mat4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	return matrix;
}

mat4x4 Matrix_MakeRotationX(float fAngleRad)
{
	mat4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = cosf(fAngleRad);
	matrix.m[1][2] = sinf(fAngleRad);
	matrix.m[2][1] = -sinf(fAngleRad);
	matrix.m[2][2] = cosf(fAngleRad);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

mat4x4 Matrix_MakeRotationY(float fAngleRad)
{
	mat4x4 matrix;
	matrix.m[0][0] = cosf(fAngleRad);
	matrix.m[0][2] = sinf(fAngleRad);
	matrix.m[2][0] = -sinf(fAngleRad);
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = cosf(fAngleRad);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

mat4x4 Matrix_MakeRotationZ(float fAngleRad)
{
	mat4x4 matrix;
	matrix.m[0][0] = cosf(fAngleRad);
	matrix.m[0][1] = sinf(fAngleRad);
	matrix.m[1][0] = -sinf(fAngleRad);
	matrix.m[1][1] = cosf(fAngleRad);
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	return matrix;
}

mat4x4 Matrix_MakeTranslation(float x, float y, float z)
{
	mat4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	matrix.m[3][0] = x;
	matrix.m[3][1] = y;
	matrix.m[3][2] = z;
	return matrix;
}

mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
{
	float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
	mat4x4 matrix;
	matrix.m[0][0] = fAspectRatio * fFovRad;
	matrix.m[1][1] = fFovRad;
	matrix.m[2][2] = fFar / (fFar - fNear);
	matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
	matrix.m[2][3] = 1.0f;
	matrix.m[3][3] = 0.0f;
	return matrix;
}

mat4x4 Matrix_MultiplyMatrix(mat4x4& m1, mat4x4& m2)
{
	mat4x4 matrix;
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
	return matrix;
}

mat4x4 Matrix_PointAt(vec3d& pos, vec3d& target, vec3d& up)
{
	// Calculate new forward direction
	vec3d newForward = Vector_Sub(target, pos);
	newForward = Vector_Normalize(newForward);

	// Calculate new Up direction
	vec3d a = Vector_Mul(newForward, Vector_DotProduct(up, newForward));
	vec3d newUp = Vector_Sub(up, a);
	newUp = Vector_Normalize(newUp);

	// New Right direction is easy, its just cross product
	vec3d newRight = Vector_CrossProduct(newUp, newForward);

	// Construct Dimensioning and Translation Matrix	
	mat4x4 matrix;
	matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
	matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
	return matrix;

}

mat4x4 Matrix_QuickInverse(mat4x4& m) // Only for Rotation/Translation Matrices
{
	mat4x4 matrix;
	matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
	matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
	matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
	matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

vec3d Vector_IntersectPlane(vec3d& plane_p, vec3d& plane_n, vec3d& lineStart, vec3d& lineEnd)
{
	plane_n = Vector_Normalize(plane_n);
	float plane_d = -Vector_DotProduct(plane_n, plane_p);
	float ad = Vector_DotProduct(lineStart, plane_n);
	float bd = Vector_DotProduct(lineEnd, plane_n);
	float t = (-plane_d - ad) / (bd - ad);
	vec3d lineStartToEnd = Vector_Sub(lineEnd, lineStart);
	vec3d lineToIntersect = Vector_Mul(lineStartToEnd, t);
	return Vector_Add(lineStart, lineToIntersect);
}

int Triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n, triangle& in_tri, triangle& out_tri1, triangle& out_tri2)
{
	// Make sure plane normal is indeed normal
	plane_n = Vector_Normalize(plane_n);

	// Return signed shortest distance from point to plane, plane normal must be normalised
	auto dist = [&](vec3d& p)
	{
		vec3d n = Vector_Normalize(p);
		return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - Vector_DotProduct(plane_n, plane_p));
	};

	// Create two temporary storage arrays to classify points either side of plane
	// If distance sign is positive, point lies on "inside" of plane
	vec3d* inside_points[3];  int nInsidePointCount = 0;
	vec3d* outside_points[3]; int nOutsidePointCount = 0;

	// Get signed distance of each point in triangle to plane
	float d0 = dist(in_tri.p[0]);
	float d1 = dist(in_tri.p[1]);
	float d2 = dist(in_tri.p[2]);

	if (d0 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[0]; }
	else { outside_points[nOutsidePointCount++] = &in_tri.p[0]; }
	if (d1 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[1]; }
	else { outside_points[nOutsidePointCount++] = &in_tri.p[1]; }
	if (d2 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[2]; }
	else { outside_points[nOutsidePointCount++] = &in_tri.p[2]; }

	// Now classify triangle points, and break the input triangle into 
	// smaller output triangles if required. There are four possible
	// outcomes...

	if (nInsidePointCount == 0)
	{
		// All points lie on the outside of plane, so clip whole triangle
		// It ceases to exist

		return 0; // No returned triangles are valid
	}

	if (nInsidePointCount == 3)
	{
		// All points lie on the inside of plane, so do nothing
		// and allow the triangle to simply pass through
		out_tri1 = in_tri;

		return 1; // Just the one returned original triangle is valid
	}

	if (nInsidePointCount == 1 && nOutsidePointCount == 2)
	{
		// Triangle should be clipped. As two points lie outside
		// the plane, the triangle simply becomes a smaller triangle

		// Copy appearance info to new triangle
		out_tri1.col = in_tri.col;
		//out_tri1.col = 0x0000ff;

		//out_tri1.sym = in_tri.sym;

		// The inside point is valid, so keep that...
		out_tri1.p[0] = *inside_points[0];

		// but the two new points are at the locations where the 
		// original sides of the triangle (lines) intersect with the plane
		out_tri1.p[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
		out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1]);

		return 1; // Return the newly formed single triangle
	}

	if (nInsidePointCount == 2 && nOutsidePointCount == 1)
	{
		// Triangle should be clipped. As two points lie inside the plane,
		// the clipped triangle becomes a "quad". Fortunately, we can
		// represent a quad with two new triangles

		// Copy appearance info to new triangles
		out_tri1.col = in_tri.col;
		//out_tri1.col = 0xff0000;
		out_tri2.col = in_tri.col;
		//out_tri2.col = 0x00ff00;

		// The first triangle consists of the two inside points and a new
		// point determined by the location where one side of the triangle
		// intersects with the plane
		out_tri1.p[0] = *inside_points[0];
		out_tri1.p[1] = *inside_points[1];
		out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);

		// The second triangle is composed of one of he inside points, a
		// new point determined by the intersection of the other side of the 
		// triangle and the plane, and the newly created point above
		out_tri2.p[0] = *inside_points[1];
		out_tri2.p[1] = out_tri1.p[2];
		out_tri2.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0]);

		return 2; // Return two newly formed triangles which form a quad
	}
}

internal void 
clear_screen(u32 color) {
	u32* pixel = (u32*)render_state.memory;
	for (int y = 0; y < render_state.height; y++) {
		for (int x = 0; x < render_state.width; x++) {
			*pixel++ = color + y/10;
		}
	}
}

internal void 
draw_rect_in_pixels(int x0, int y0, int x1, int y1, u32 color) {

	x0 = clamp(0, x0, render_state.width);
	x1 = clamp(0, x1, render_state.width);
	y0 = clamp(0, y0, render_state.height);
	y1 = clamp(0, y1, render_state.height);
	for (int y = y0; y < y1; y++) {
		u32* pixel = (u32*)render_state.memory + x0 + y * render_state.width;
		for (int x = x0; x < x1; x++) {
			*pixel++ = color;
		}
	}
}


internal void 
draw_line_in_pixels(int x0, int y0, int x1, int y1, u32 color, int thickness) {
	x0 = clamp(0, x0, render_state.width);
	x1 = clamp(0, x1, render_state.width);
	y0 = clamp(0, y0, render_state.height);
	y1 = clamp(0, y1, render_state.height);
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2;

	int half_thickness = thickness / 2;

	while (true) {
		// Draw a thicker line by filling a small rectangle at each point
		for (int i = -half_thickness; i <= half_thickness; i++) {
			for (int j = -half_thickness; j <= half_thickness; j++) {
				int px = x0 + i, py = y0 + j;
				if (px >= 0 && px < render_state.width && py >= 0 && py < render_state.height) {
					((u32*)render_state.memory)[px + py * render_state.width] = color;
				}
			}
		}

		if (x0 == x1 && y0 == y1) break; // Exit when reaching the end

		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; }
		if (e2 <= dx) { err += dx; y0 += sy; }
	}
}



internal void
draw_rect(float x, float y, float half_size_x, float half_size_y, u32 color) {
	
	x *= render_state.height*render_scale;
	y *= render_state.height * render_scale;
	half_size_x *= render_state.height * render_scale;
	half_size_y *= render_state.height * render_scale;

	//centering
	x += render_state.width / 2.f;
	y += render_state.height / 2.f;
	
	int x0 = x - half_size_x;
	int x1 = x + half_size_x;
	int y0 = y - half_size_y;
	int y1 = y + half_size_y;

	draw_rect_in_pixels(x0, y0, x1, y1, color);
}

internal void
draw_line(float x1, float y1, float x2, float y2, u32 color, int thickness) {
	x1 *= render_state.width * render_scale;
	y1 *= render_state.height * render_scale;
	x2 *= render_state.width * render_scale;
	y2 *= render_state.height * render_scale;

	x1 += render_state.width / 2.f;
	y1 += render_state.height / 2.f;
	x2 += render_state.width / 2.f;
	y2 += render_state.height / 2.f;

	draw_line_in_pixels((int)x1,(int)y1,(int)x2,(int)y2,color,thickness);
}

internal void
draw_triangle_new(int x1, int y1, int x2, int y2, int x3, int y3, u32 color, int thickness)
{
	draw_line_in_pixels(x1, y1, x2, y2, color, thickness);
	draw_line_in_pixels(x2, y2, x3, y3, color, thickness);
	draw_line_in_pixels(x3, y3, x1, y1, color, thickness);
}

internal void
fill_triangle(int x1, int y1, int x2, int y2, int x3, int y3, u32 color)
{
	// Sort vertices by y-coordinate (y1 <= y2 <= y3)
	if (y1 > y2) { swap(x1, x2); swap(y1, y2); }
	if (y1 > y3) { swap(x1, x3); swap(y1, y3); }
	if (y2 > y3) { swap(x2, x3); swap(y2, y3); }

	// Clamping
	x1 = clamp(0, x1, render_state.width - 1);
	y1 = clamp(0, y1, render_state.height - 1);
	x2 = clamp(0, x2, render_state.width - 1);
	y2 = clamp(0, y2, render_state.height - 1);
	x3 = clamp(0, x3, render_state.width - 1);
	y3 = clamp(0, y3, render_state.height - 1);

	// Edge cases
	if (y1 == y2 && y2 == y3) return; // All points on same line, no area to fill

	// Calculate inverse slopes
	float inv_slope1 = (y1 != y2) ? (float)(x2 - x1) / (y2 - y1) : 0;
	float inv_slope2 = (y2 != y3) ? (float)(x3 - x2) / (y3 - y2) : 0;
	float inv_slope3 = (y1 != y3) ? (float)(x3 - x1) / (y3 - y1) : 0;

	// Function to draw horizontal spans
	auto draw_span = [&](int x_start, int x_end, int y) {
		if (y < 0 || y >= render_state.height) return;
		if (x_start > x_end) swap(x_start, x_end);
		x_start = clamp(0, x_start, render_state.width - 1);
		x_end = clamp(0, x_end, render_state.width - 1);

		u32* pixel = (u32*)render_state.memory + x_start + y * render_state.width;
		for (int x = x_start; x <= x_end; x++) {
			*pixel++ = color;
		}
	};

	// bottom part
	if (y1 != y2) {
		float xL = x1, xR = x1, mL=min(inv_slope1,inv_slope3), mR=max(inv_slope1,inv_slope3);
		for (int y = y1; y < y2; y++) {
			draw_span((int)xL, (int)xR, y);
			xL += mL;
			xR += mR;
		}
	}

	// upper part
	if (y2 != y3) {
		float xL = x3, xR = x3, mL = min(inv_slope2, inv_slope3), mR = max(inv_slope2, inv_slope3);
		for (int y = y3; y >= y2; y--) {
			draw_span((int)xL, (int)xR, y);
			xL -= mL;
			xR -= mR;
		}
	}
}