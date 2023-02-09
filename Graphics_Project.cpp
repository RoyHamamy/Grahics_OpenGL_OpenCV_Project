#include <windows.h>		// Header File For Windows
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <stdlib.h>

#pragma comment(lib, "opengl32.lib")                // Link OpenGL32.lib
#pragma comment(lib, "glu32.lib")               // Link Glu32.lib
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define     MAP_SIZE    1024                // Size Of Our .RAW Height Map ( NEW )
#define     HEIGHT_RATIO    1.5f                // Ratio That The Y Is Scaled According To The X And Z ( NEW )

HDC     hDC = NULL;                   // Private GDI Device Context
HGLRC       hRC = NULL;                   // Permanent Rendering Context
HWND        hWnd = NULL;                  // Holds Our Window Handle
HINSTANCE   hInstance;                  // Holds The Instance Of The Application

bool        keys[256];                  // Array Used For The Keyboard Routine
bool        fullscreen = TRUE;                // Fullscreen Flag Set To TRUE By Default
bool        bRender = TRUE;                 // Polygon Flag Set To TRUE By Default ( NEW )

Mat global_HMap;
bool resolution = true;
int verticaler = 0;
int big_verticaler = 0;
int triangle_horizontal = 0;
int triangle_horizontal_big = 0;
float scaleValue = 0.25f;
float leftRightRotate = 0.0f;
float upDownRotate = 0.25f;
float lookRightLeft = 0.25f;
float lookUpDown = 0.25f;
int pickedIndex = -1;
int mouseX;
int mouseY;
int Triangles = 0;
int numOfBig_Tringles = 0;
int Lines = 0;
int numOfBig_Lines = 0;
int Rain_Amount = 5000;
bool isRainning = false;
bool rained = false;
bool isWindy = false;
bool isRes = false;
float fogScale = 0.0f;
float wind_left_right = 0.0f;
float gravity = -0.01;
float rainMovement = 0.0f;
float rainMovement_leftRight = 0.0f;

float color_rain_r = 0.0;
float color_rain_g = 0.0;
float color_rain_b = 1.0;

float color_fog_r = 0.5;
float color_fog_g = 0.5;
float color_fog_b = 0.8;

bool isClicked = false;


bool	bp;
bool	fp;

GLuint	screener;
GLuint	txtr[3];

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);       // Declaration For WndProc

bool	active = TRUE;		// Window Active Flag Set To TRUE By Default
GLfloat     rtri;                       // Angle For The Triangle ( NEW )
GLfloat     rquad;                      // Angle For The Quad     ( NEW )

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height == 0)										// Prevent A Divide By Zero By
	{
		height = 1;										// Making Height Equal One
	}

	glViewport(0, 0, width, height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

														// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

// Loads The .RAW File And Stores It In pHeightMap
/*
void LoadRawFile(LPSTR strName, int nSize, BYTE *pHeightMap)
{
	FILE *pFile = NULL;

	// Open The File In Read / Binary Mode.
	pFile = fopen(strName, "rb");

	// Check To See If We Found The File And Could Open It
	if (pFile == NULL)
	{
		// Display Error Message And Stop The Function
		MessageBox(NULL, "Can't Find The Height Map!", "Error", MB_OK);
		return;
	}
	// Here We Load The .RAW File Into Our pHeightMap Data Array
	// We Are Only Reading In '1', And The Size Is (Width * Height)
	fread(pHeightMap, 1, nSize, pFile);

	// After We Read The Data, It's A Good Idea To Check If Everything Read Fine
	int result = ferror(pFile);

	// Check If We Received An Error
	if (result)
	{
		MessageBox(NULL, "Failed To Get Data!", "Error", MB_OK);
	}

	// Close The File
	fclose(pFile);
}
*/

struct edge_line {
	Vec3f x;
	Vec3f y;
};

struct Triangle 
{
	bool isPicked = false;
	Vec3f x;
	Vec3f y;
	Vec3f z;

	Vec3f X_color;
	Vec3f Y_color;
	Vec3f Z_color;

	int neighbor_above;
	int neighbor_under;
	int neighbor_right;
	int neighbor_left;
};

typedef struct {
	float x, y, z;
	float firstY;
	float firstX1;
	float firstX2;
	float velocity;
} Rain_Drop;


vector<vector<Triangle>> all_triangles;
vector<vector<edge_line>> all_lines;
vector<Rain_Drop> rain;


float Height(int X, int Y)          
{
	int x = X % global_HMap.rows;                   
	int y = Y % global_HMap.cols;                   

	if (! &global_HMap) return 0;  
	int point = global_HMap.at<Vec3b>(Point(y, x)).val[0];
	int calcHeight = 61 * point;
	return  calcHeight / 255;       
} 

Vec3f setVertexColor(float x, float y)     
{									 
	float calc_point = ((Height(x,y))/ 30.0f) * 61.0f;
	if (calc_point == 0)
		return Vec3f(0, 0, 0);
	else if (calc_point < 10)
		return Vec3f(0, 0, 1);
	else if (calc_point < 15.7)
		return Vec3f(0, 0.5, 1);
	else if (calc_point < 21.45)
		return Vec3f(0, 1, 1);
	else if (calc_point < 27.1)
		return Vec3f(0, 1, 0.5);
	else if (calc_point < 28.4)
		return Vec3f(0, 1, 0);
	else if (calc_point < 32.75)
		return Vec3f(0.3, 1, 0);
	else if (calc_point < 38.4)
		return Vec3f(0.5, 0.5, 0);
	else if (calc_point < 44.05)
		return Vec3f(0.7, 0.3, 0);
	else if (calc_point < 49.7)
		return Vec3f(1, 0.3, 0);
	else if (calc_point < 55.35)
		return Vec3f(1, 0, 0);
	else if (calc_point < 61)
		return Vec3f(1, 0, 0);
	return Vec3f(1, 1, 1);
}

void MakeRain()
{
	if (color_rain_r > 1) {
		color_rain_r = 1.0;
	}

	if (color_rain_r < 0) {
		color_rain_r = 0.0;
	}

	if (color_rain_g > 1) {
		color_rain_g = 1.0;
	}

	if (color_rain_g < 0) {
		color_rain_g = 0.0;
	}

	if (color_rain_b > 1) {
		color_rain_b = 1.0;
	}

	if (color_rain_b < 0) {
		color_rain_b = 0.0;
	}

	glColor3f(color_rain_r, color_rain_g, color_rain_b);

	for (int i = 0; i < rain.size(); i++) {

		if (gravity > 0) {
			gravity = -0.01;
		}
		Rain_Drop p = rain[i];

		glBegin(GL_LINES);
		glVertex3f(p.x, p.y, p.z);
		glVertex3f(p.x, p.y - 0.5, p.z);
		glEnd();

		rain[i].x += wind_left_right + rainMovement_leftRight;
		rain[i].y += rain[i].velocity;
		rain[i].z += rainMovement;
		rain[i].velocity += gravity;

		if (rain[i].y <= 0) {
			rain[i].y = rain[i].firstY;
			rain[i].velocity = 0.0f;
		}

		if (isWindy) {
			if (rain[i].x > 200 && rainMovement_leftRight == 0) {
				rain[i].x = rain[i].firstX1;
			}
			if (rain[i].x < 0 && rainMovement_leftRight == 0) {
				rain[i].x = 200.0f;
			}
		}
		
		
		
	}
}

void MakeFog()
{
	if (color_fog_r > 1) {
		color_fog_r = 1.0;
	}

	if (color_fog_r < 0) {
		color_fog_r = 0.0;
	}

	if (color_fog_g > 1) {
		color_fog_g = 1.0;
	}

	if (color_fog_g < 0) {
		color_fog_g = 0.0;
	}

	if (color_fog_b > 1) {
		color_fog_b = 1.0;
	}

	if (color_fog_b < 0) {
		color_fog_b = 0.0;
	}

	float fogColor[4] = { color_fog_r, color_fog_g, color_fog_b, 0.2f };

	glFogi(GL_FOG_MODE, GL_EXP2);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, fogScale);
	glHint(GL_FOG_HINT, GL_DONT_CARE);
	glFogf(GL_FOG_START, 1.0f);
	glFogf(GL_FOG_END, 25.0f);
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_EXP2);
}

void MakeRainDrops()
{
	int X = global_HMap.cols;
	int Z = global_HMap.rows;
	for (int i = 0; i < Rain_Amount; i++) {

		Rain_Drop p;
		p.x = float(rand() % X);
		p.y = float(rand() % 40);
		p.z = float(rand() % Z);

		p.velocity = 0.0f;
		p.firstY = float(rand() % 40);
		p.firstX1 = 10.0f;
		p.firstX2 = 900.0f;

		rain.push_back(p);
	}
}

void draweTriangles(int stepper) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	int rows = global_HMap.rows;
	int cols = global_HMap.cols;

	for (int X = 0; X < (rows - stepper); X += stepper) {
		triangle_horizontal += 2;
		for (int Y = 0; Y < (cols - stepper); Y += stepper) {

			Triangle t1;
			all_triangles[1].push_back(t1);
			Triangle t2;
			all_triangles[1].push_back(t2);
			if (X == 0)
				verticaler += 2;
		}
	}

	int pos = 0;

	for (int X = 0; X <(rows - stepper); X += stepper) {
		for (int Y = 0; Y < (cols - stepper); Y += stepper) {

			Triangles = Triangles + 2;
			Lines = Lines + 6;

			float H1 = Height(X, Y);
			float H2 = Height(X, Y + stepper);
			float H3 = Height(X + stepper, Y + stepper);
			float H4 = Height(X + stepper, Y);

			Vec3f C1 = setVertexColor(X, Y);
			Vec3f C2 = setVertexColor(X, Y + stepper);
			Vec3f C3 = setVertexColor(X + stepper, Y + stepper);
			Vec3f C4 = setVertexColor(X + stepper, Y);

			all_triangles[1][pos].x = Vec3f(float(X), H1, float(Y));
			all_triangles[1][pos].y = Vec3f(float(X) + stepper, H4, float(Y));
			all_triangles[1][pos].z = Vec3f(float(X), H2, float(Y) + stepper);

			all_triangles[1][pos].X_color = C1;
			all_triangles[1][pos].Z_color = C2;
			all_triangles[1][pos].Y_color = C4;


			if (Y != 0)
			{
				all_triangles[1][pos].neighbor_under = pos - 1;
				all_triangles[1][pos - 1].neighbor_above = pos;
			}
			else
				all_triangles[1][pos].neighbor_under = -1;



			if (X != 0)
			{
				all_triangles[1][pos].neighbor_right = pos - verticaler + 1;
				all_triangles[1][pos - verticaler + 1].neighbor_left = pos;
			}
			else
				all_triangles[1][pos].neighbor_right = -1;


			edge_line l1;
			l1.x = Vec3f(float(X), H1, float(Y));
			l1.y = Vec3f(float(X) + stepper, H4, float(Y));
			all_lines[1].push_back(l1);

			edge_line l2;
			l2.x = Vec3f(float(X) + stepper, H4, float(Y));
			l2.y = Vec3f(float(X), H2, float(Y) + stepper);
			all_lines[1].push_back(l2);

			edge_line l3; 
			l3.x = Vec3f(float(X) + stepper, H4, float(Y));
			l3.y = Vec3f(float(X), H2, float(Y) + stepper);
			all_lines[1].push_back(l3);

			pos++;

			all_triangles[1][pos].x = Vec3f(float(X), H2, float(Y) + stepper);
			all_triangles[1][pos].y = Vec3f(float(X) + stepper, H4, float(Y));
			all_triangles[1][pos].z = Vec3f(float(X) + stepper, H3, float(Y) + stepper);

			all_triangles[1][pos].X_color = C2;
			all_triangles[1][pos].Z_color = C3;
			all_triangles[1][pos].Y_color = C4;

			all_triangles[1][pos].neighbor_right = pos - 1;
			all_triangles[1][pos - 1].neighbor_left = pos;

			all_lines[1].push_back(l3);

			pos++;

			edge_line l4;
			l4.x = Vec3f(float(X) + stepper, H4, float(Y));
			l4.y = Vec3f(float(X) + stepper, H3, float(Y) + stepper);
			all_lines[1].push_back(l4);

			edge_line l5;
			l5.x = Vec3f(float(X) + stepper, H3, float(Y) + stepper);
			l5.y = Vec3f(float(X), H2, float(Y) + stepper);
			all_lines[1].push_back(l5);
		}
	}
}

void init_big_Triangles() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	int rows = global_HMap.rows;
	int cols = global_HMap.cols;

	int big_step_size = 8;
	for (int X = 0; X < (rows - big_step_size); X += big_step_size) {
		triangle_horizontal_big += 2;
		for (int Y = 0; Y < (cols - big_step_size); Y += big_step_size) {
			Triangle t1;
			all_triangles[0].push_back(t1);
			Triangle t2;
			all_triangles[0].push_back(t2);
			if (X == 0)
				big_verticaler += 2;
		}
	}

	int pos = 0;
	for (int X = 0; X < (rows - big_step_size); X += big_step_size) {
		for (int Y = 0; Y < (cols - big_step_size); Y += big_step_size) {
			numOfBig_Tringles += 2;
			numOfBig_Lines += 6;
			
			float H1 = Height(X, Y);
			float H2 = Height(X, Y + big_step_size);
			float H3 = Height(X + big_step_size, Y + big_step_size);
			float H4 = Height(X + big_step_size, Y);

			Vec3f C1 = setVertexColor(X, Y);
			Vec3f C2 = setVertexColor(X, Y + big_step_size);
			Vec3f C3 = setVertexColor(X + big_step_size, Y + big_step_size);
			Vec3f C4 = setVertexColor(X + big_step_size, Y);

			all_triangles[0][pos].x = Vec3f(float(X), H1, float(Y));
			all_triangles[0][pos].y = Vec3f(float(X) + big_step_size, H4, float(Y));
			all_triangles[0][pos].z = Vec3f(float(X), H2, float(Y) + big_step_size);

			all_triangles[0][pos].X_color = C1;
			all_triangles[0][pos].Z_color = C2;
			all_triangles[0][pos].Y_color = C4;

			if (Y != 0)
			{
				all_triangles[0][pos].neighbor_under = pos - 1;
				all_triangles[0][pos - 1].neighbor_above = pos;
			}
			else
				all_triangles[0][pos].neighbor_under = -1;



			if (X != 0)
			{
				all_triangles[0][pos].neighbor_right = pos - big_verticaler + 1;
				all_triangles[0][pos - big_verticaler + 1].neighbor_left = pos;
			}
			else
				all_triangles[0][pos].neighbor_right = -1;

			edge_line l1;
			l1.x = Vec3f(float(X), H1, float(Y));
			l1.y = Vec3f(float(X) + big_step_size, H4, float(Y));
			all_lines[0].push_back(l1);

			edge_line l2;
			l2.x = Vec3f(float(X) + big_step_size, H4, float(Y));
			l2.y = Vec3f(float(X), H2, float(Y) + big_step_size);
			all_lines[0].push_back(l2);

			edge_line l3;
			l3.x = Vec3f(float(X) + big_step_size, H4, float(Y));
			l3.y = Vec3f(float(X), H2, float(Y) + big_step_size);
			all_lines[0].push_back(l3);

			pos++;

			all_triangles[0][pos].x = Vec3f(float(X), H2, float(Y) + big_step_size);
			all_triangles[0][pos].y = Vec3f(float(X) + big_step_size, H4, float(Y));
			all_triangles[0][pos].z = Vec3f(float(X) + big_step_size, H3, float(Y) + big_step_size);

			all_triangles[0][pos].X_color = C2;
			all_triangles[0][pos].Z_color = C3;
			all_triangles[0][pos].Y_color = C4;

			all_triangles[0][pos].neighbor_right = pos - 1;
			all_triangles[0][pos - 1].neighbor_left = pos;

			all_lines[0].push_back(l3);

			pos++;

			edge_line l4;
			l4.x = Vec3f(float(X) + big_step_size, H4, float(Y));
			l4.y = Vec3f(float(X) + big_step_size, H3, float(Y) + big_step_size);
			all_lines[0].push_back(l4);

			edge_line l5;
			l5.x = Vec3f(float(X) + big_step_size, H3, float(Y) + big_step_size);
			l5.y = Vec3f(float(X), H2, float(Y) + big_step_size);
			all_lines[0].push_back(l5);
		}
	}
}
int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glShadeModel(GL_SMOOTH);                // Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);           // Black Background
	glClearDepth(1.0f);                 // Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);                // Enables Depth Testing
	glDepthFunc(GL_LEQUAL);                 // The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Really Nice Perspective Calculations
											// g_HeightMap array.  We also pass in the size of the .raw file (1024).

	global_HMap = imread("C:\\Users\\User\\Desktop\\image4.png", IMREAD_COLOR);

	/*
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	*/
	vector<Triangle> v1;
	vector<Triangle> v2;
	vector<edge_line> v3;
	vector<edge_line> v4;

	all_triangles.push_back(v1);
	all_triangles.push_back(v2);
	all_lines.push_back(v3);
	all_lines.push_back(v4);

	
	draweTriangles(4);
	init_big_Triangles();

	//printf("%d \n", all_triangles[0].size());
	//printf("%d \n", all_triangles[1].size());

	MakeRainDrops();

	return TRUE;                        // Initialization Went OK
}

int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{
	if (!keys['Z']) {
		MakeFog();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		gluLookAt(10 + lookRightLeft, 20 + lookUpDown, -35 + scaleValue, 20 + lookRightLeft, 10 + lookUpDown, 20.0 + scaleValue, 0, 1, 0);
		glScalef(0.25, 0.25*HEIGHT_RATIO, 0.25);
		glTranslatef(256, 0.0, 256);
		glRotatef(leftRightRotate, 0, 10, 0);
		glRotatef(upDownRotate, 0, 0, 10);
		glTranslatef(-256, 0.0, -256);

		glBindTexture(GL_TEXTURE_2D, txtr[screener]);


		int rows = global_HMap.rows;
		int cols = global_HMap.cols;

		if (isClicked) {
			glDrawBuffer(GL_BACK);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			for (int index = 0; index < Triangles; index++) {
				Triangle trng = all_triangles[1][index];

				Vec3f vertex_x = trng.x;
				Vec3f vertex_y = trng.y;
				Vec3f vertex_z = trng.z;

				float r = float((index & 0x000000FF) >> 0);
				float g = float((index & 0x0000FF00) >> 8);
				float b = float((index & 0x00FF0000) >> 16);

				r = r / 255.0f;
				g = g / 255.0f;
				b = b / 255.0f;

				glDisable(GL_FOG);
				glBegin(GL_TRIANGLES);

				glColor3f(r, g, b);
				glVertex3f(vertex_x[0], vertex_x[1], vertex_x[2]);

				glColor3f(r, g, b);
				glVertex3f(vertex_y[0], vertex_y[1], vertex_y[2]);

				glColor3f(r, g, b);
				glVertex3f(vertex_z[0], vertex_z[1], vertex_z[2]);

				glEnd();
			}
		}
		else {

			glDrawBuffer(GL_FRONT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			for (int index = 0; index < Triangles; index++) {

				Triangle trng = all_triangles[1][index];
				Vec3f vertex_x = trng.x;
				Vec3f vertex_y = trng.y;
				Vec3f vertex_z = trng.z;
				Vec3f colorx = trng.X_color;
				Vec3f colory = trng.Y_color;
				Vec3f colorz = trng.Z_color;

				glBegin(GL_TRIANGLES);

				if (trng.isPicked) {

					glColor3f(1.0f, 1.0f, 1.0f);
					glVertex3f(vertex_x[0], vertex_x[1], vertex_x[2]);

					glColor3f(1.0f, 1.0f, 1.0f);
					glVertex3f(vertex_y[0], vertex_y[1], vertex_y[2]);

					glColor3f(1.0f, 1.0f, 1.0f);
					glVertex3f(vertex_z[0], vertex_z[1], vertex_z[2]);
				}
				else {
					glColor3f(colorx[0], colorx[1], colorx[2]);
					glVertex3f(vertex_x[0], vertex_x[1], vertex_x[2]);

					glColor3f(colory[0], colory[1], colory[2]);
					glVertex3f(vertex_y[0], vertex_y[1], vertex_y[2]);

					glColor3f(colorz[0], colorz[1], colorz[2]);
					glVertex3f(vertex_z[0], vertex_z[1], vertex_z[2]);
				}

				glEnd();
			}

		}
		for (int i = 0; i < Lines; i++) {
			edge_line trng = all_lines[1][i];

			Vec3f vertex_x = trng.x;
			Vec3f vertex_y = trng.y;

			glBegin(GL_LINES);

			glColor3f(0.0f, 0.0f, 0.0f);
			glVertex3f(vertex_x[0], vertex_x[1], vertex_x[2]);

			glColor3f(0.0f, 0.0f, 0.0f);
			glVertex3f(vertex_y[0], vertex_y[1], vertex_y[2]);

			glEnd();
		}

		if (isRes) {
			for (int i = 0; i < numOfBig_Lines; i++) {
				edge_line trng = all_lines[0][i];

				Vec3f vertex_x = trng.x;
				Vec3f vertex_y = trng.y;

				glBegin(GL_LINES);

				glColor3f(1.0f, 1.0f, 1.0f);
				glVertex3f(vertex_x[0], vertex_x[1], vertex_x[2]);

				glColor3f(1.0f, 1.0f, 1.0f);
				glVertex3f(vertex_y[0], vertex_y[1], vertex_y[2]);

				glEnd();
			}
		}

		if (isRainning)
			MakeRain();

		glEnable(GL_FOG);

		return TRUE;
	}
else {
	MakeFog();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(10 + lookRightLeft, 20 + lookUpDown, -35 + scaleValue, 20 + lookRightLeft, 10 + lookUpDown, 20.0 + scaleValue, 0, 1, 0);
	glScalef(0.25, 0.25*HEIGHT_RATIO, 0.25);
	glTranslatef(256, 0.0, 256);
	glRotatef(leftRightRotate, 0, 10, 0);
	glRotatef(upDownRotate, 0, 0, 10);
	glTranslatef(-256, 0.0, -256);

	glBindTexture(GL_TEXTURE_2D, txtr[screener]);


	int rows = global_HMap.rows;
	int cols = global_HMap.cols;

	if (isClicked) {
		glDrawBuffer(GL_BACK);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (int index = 0; index < numOfBig_Tringles; index++) {
			Triangle trng = all_triangles[0][index];

			Vec3f vertex_x = trng.x;
			Vec3f vertex_y = trng.y;
			Vec3f vertex_z = trng.z;

			float r = float((index & 0x000000FF) >> 0);
			float g = float((index & 0x0000FF00) >> 8);
			float b = float((index & 0x00FF0000) >> 16);

			r = r / 255.0f;
			g = g / 255.0f;
			b = b / 255.0f;

			glDisable(GL_FOG);
			glBegin(GL_TRIANGLES);

			glColor3f(r, g, b);
			glVertex3f(vertex_x[0], vertex_x[1], vertex_x[2]);

			glColor3f(r, g, b);
			glVertex3f(vertex_y[0], vertex_y[1], vertex_y[2]);

			glColor3f(r, g, b);
			glVertex3f(vertex_z[0], vertex_z[1], vertex_z[2]);

			glEnd();
		}
	}
	else {

		glDrawBuffer(GL_FRONT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (int index = 0; index < numOfBig_Tringles; index++) {

			Triangle trng = all_triangles[0][index];
			Vec3f vertex_x = trng.x;
			Vec3f vertex_y = trng.y;
			Vec3f vertex_z = trng.z;
			Vec3f colorx = trng.X_color;
			Vec3f colory = trng.Y_color;
			Vec3f colorz = trng.Z_color;

			glBegin(GL_TRIANGLES);

			if (trng.isPicked) {

				glColor3f(1.0f, 1.0f, 1.0f);
				glVertex3f(vertex_x[0], vertex_x[1], vertex_x[2]);

				glColor3f(1.0f, 1.0f, 1.0f);
				glVertex3f(vertex_y[0], vertex_y[1], vertex_y[2]);

				glColor3f(1.0f, 1.0f, 1.0f);
				glVertex3f(vertex_z[0], vertex_z[1], vertex_z[2]);
			}
			else {
				glColor3f(colorx[0], colorx[1], colorx[2]);
				glVertex3f(vertex_x[0], vertex_x[1], vertex_x[2]);

				glColor3f(colory[0], colory[1], colory[2]);
				glVertex3f(vertex_y[0], vertex_y[1], vertex_y[2]);

				glColor3f(colorz[0], colorz[1], colorz[2]);
				glVertex3f(vertex_z[0], vertex_z[1], vertex_z[2]);
			}

			glEnd();
		}

	}
	for (int i = 0; i < numOfBig_Lines; i++) {
		edge_line trng = all_lines[0][i];

		Vec3f vertex_x = trng.x;
		Vec3f vertex_y = trng.y;

		glBegin(GL_LINES);

		glColor3f(0.0f, 0.0f, 0.0f);
		glVertex3f(vertex_x[0], vertex_x[1], vertex_x[2]);

		glColor3f(0.0f, 0.0f, 0.0f);
		glVertex3f(vertex_y[0], vertex_y[1], vertex_y[2]);

		glEnd();
	}

	if (isRainning)
		MakeRain();

	glEnable(GL_FOG);

	return TRUE;
}
}

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL, 0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL, NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd, hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL, "Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL", hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;									// Set hInstance To NULL
	}
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
*	title			- Title To Appear At The Top Of The Window				*
*	width			- Width Of The GL Window Or Fullscreen Mode				*
*	height			- Height Of The GL Window Or Fullscreen Mode			*
*	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
*	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/

BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = (long)0;			// Set Left Value To 0
	WindowRect.right = (long)width;		// Set Right Value To Requested Width
	WindowRect.top = (long)0;				// Set Top Value To 0
	WindowRect.bottom = (long)height;		// Set Bottom Value To Requested Height

	fullscreen = fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance = GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc = (WNDPROC)WndProc;					// WndProc Handles Messages
	wc.cbClsExtra = 0;									// No Extra Window Data
	wc.cbWndExtra = 0;									// No Extra Window Data
	wc.hInstance = hInstance;							// Set The Instance
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground = NULL;									// No Background Required For GL
	wc.lpszMenuName = NULL;									// We Don't Want A Menu
	wc.lpszClassName = "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL, "Failed To Register The Window Class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}

	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth = width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight = height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel = bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL, "The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", "NeHe GL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				fullscreen = FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL, "Program Will Now Close.", "ERROR", MB_OK | MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle = WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle = WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle = WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

																	// Create The Window
	if (!(hWnd = CreateWindowEx(dwExStyle,							// Extended Style For The Window
		"OpenGL",							// Class Name
		title,								// Window Title
		dwStyle |							// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		0, 0,								// Window Position
		WindowRect.right - WindowRect.left,	// Calculate Window Width
		WindowRect.bottom - WindowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		hInstance,							// Instance
		NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Window Creation Error.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if (!(hDC = GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC = wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!wglMakeCurrent(hDC, hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd, SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Initialization Failed.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

LRESULT CALLBACK WndProc(HWND    hWnd,           // Handle For This Window
	UINT    uMsg,           // Message For This Window
	WPARAM  wParam,         // Additional Message Information
	LPARAM  lParam)         // Additional Message Information
{
	switch (uMsg)                       // Check For Windows Messages
	{
	case WM_ACTIVATE:               // Watch For Window Activate Message
	{
		if (!HIWORD(wParam))            // Check Minimization State
		{
			active = TRUE;            // Program Is Active
		}
		else
		{
			active = FALSE;           // Program Is No Longer Active
		}

		return 0;               // Return To The Message Loop
	}

	case WM_SYSCOMMAND:             // Intercept System Commands
	{
		switch (wParam)             // Check System Calls
		{
		case SC_SCREENSAVE:     // Screensaver Trying To Start?
		case SC_MONITORPOWER:       // Monitor Trying To Enter Powersave?
			return 0;           // Prevent From Happening
		}
		break;                  // Exit
	}

	case WM_CLOSE:                  // Did We Receive A Close Message?
	{
		PostQuitMessage(0);         // Send A Quit Message
		return 0;               // Jump Back
	}

	case WM_LBUTTONDOWN:                // Did We Receive A Left Mouse Click?
	{
		glDisable(GL_FOG);
		if (isRainning) {
			rained = true;
		}
		isRainning = false;
		POINT position;
		GetCursorPos(&position);
		ScreenToClient(hWnd, &position);

		//printf("%d %d \n", position.x, position.y);
		mouseX = position.x;
		int viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		mouseY = viewport[3] - position.y;
		
		return 0;               // Jump Back
	}

	case WM_LBUTTONUP:
	{
		unsigned char info[3];
		isClicked = true;

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glReadBuffer(GL_BACK);

		DrawGLScene();
		glReadPixels(mouseX, mouseY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, info);
		isClicked = false;
		
		
		int id = info[0] + (info[1] * 256) + (info[2] * 256 * 256);

		printf("%d %d \n", id, 0);
		if (id != 0) {
			if (keys['Z']) {
				all_triangles[0][id].isPicked = !all_triangles[0][id].isPicked;

			}
			else {
				all_triangles[1][id].isPicked = !all_triangles[1][id].isPicked;
			}
		}
		if (rained) {
			isRainning = true;
			rained = false;
		}

		glEnable(GL_FOG);
		return 0;
	}

	case WM_KEYDOWN:                // Is A Key Being Held Down?
	{
		keys[wParam] = TRUE;            // If So, Mark It As TRUE
		return 0;               // Jump Back
	}

	case WM_KEYUP:                  // Has A Key Been Released?
	{
		keys[wParam] = FALSE;           // If So, Mark It As FALSE
		return 0;               // Jump Back
	}

	case WM_SIZE:                   // Resize The OpenGL Window
	{
		ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));   // LoWord=Width, HiWord=Height
		return 0;               // Jump Back
	}
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE   hInstance,
	HINSTANCE   hPrevInstance,
	LPSTR       lpCmdLine,
	int     nCmdShow)

{
	MSG     msg;
	BOOL    done = FALSE;
	fullscreen = FALSE;

	if (!CreateGLWindow((char*)"NeHe & Ben Humphrey's Height Map Tutorial", 640, 480, 16, fullscreen))
	{
		return 0;
	}

	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				done = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else


		{

			if ((active && !DrawGLScene()) || keys[VK_ESCAPE])
			{
				done = TRUE;
			}
			else if (active)
			{
				SwapBuffers(hDC);
			}

			if (keys[VK_F1])
			{
				keys[VK_F1] = FALSE;
				KillGLWindow();
				fullscreen = !fullscreen;

				if (!CreateGLWindow((char*)"NeHe & Ben Humphrey's Height Map Tutorial", 640, 480, 16, fullscreen))
				{
					return 0;
				}
			}

			if (keys[VK_CONTROL] && keys['R'] && isRainning && keys['W']) { // move rain inwards
				rainMovement += 0.5f;
			}

			if (!(keys[VK_CONTROL] && keys['R'] && isRainning && keys['W']) && !(keys[VK_CONTROL] && keys['R'] && isRainning && keys['W'])) { // stop rain movement forward/backwards
				rainMovement = 0.0f;
			}

			if (!(keys[VK_CONTROL] && keys['R'] && isRainning && keys['A']) && !(keys[VK_CONTROL] && keys['R'] && isRainning && keys['D'])) { // stop rain movement left/right
				rainMovement_leftRight = 0.0f;
			}

			if (keys[VK_CONTROL] && keys['R'] && isRainning && keys['S']) { // move rain forward
				rainMovement -= 1.5f;
			}

			if (keys[VK_CONTROL] && keys['R'] && isRainning && keys['D'] && !isWindy) { // move rain to the right
				rainMovement_leftRight += 0.5f;
			}

			if (keys[VK_CONTROL] && keys['R'] && isRainning && keys['A'] && !isWindy) { // move rain to the left
				rainMovement_leftRight -= 0.5f;
			}

			if (keys[VK_CONTROL] && keys['R']&& isRainning && keys[VK_UP]) { // more redness to the raindrop
				color_rain_r += 0.01;
			}

			if (keys[VK_CONTROL] && keys['R'] && isRainning && keys[VK_DOWN]) { // less redness to the raindrop
				color_rain_r -= 0.01;
			}

			if (keys[VK_CONTROL] && keys['G'] && isRainning && keys[VK_UP]) { // more greennes to the raindrop
				color_rain_g += 0.01;
			}

			if (keys[VK_CONTROL] && keys['G'] && isRainning && keys[VK_DOWN]) { // less greenness to the raindrop
				color_rain_g -= 0.01;
			}

			if (keys[VK_CONTROL] && keys['B'] && isRainning && keys[VK_UP]) { // more blueness to the raindrop
				color_rain_b += 0.01;
			}

			if (keys[VK_CONTROL] && keys['B'] && isRainning && keys[VK_DOWN]) { // less blueness to the raindrop
				color_rain_b -= 0.01;
			}

			if (keys[VK_CONTROL] && keys['R'] && keys[VK_SHIFT] && keys[VK_UP]) { // more redness to the fog
				color_fog_r += 0.01;
			}

			if (keys[VK_CONTROL] && keys['R'] && keys[VK_SHIFT] && keys[VK_DOWN]) { // less redness to the fog
				color_fog_r -= 0.01;
			}

			if (keys[VK_CONTROL] && keys['G'] && keys[VK_SHIFT] && keys[VK_UP]) { // more greennes to the fog
				color_fog_g += 0.01;
			}

			if (keys[VK_CONTROL] && keys['G'] && keys[VK_SHIFT] && keys[VK_DOWN]) { // less greenness to the fog
				color_fog_g -= 0.01;
			}

			if (keys[VK_CONTROL] && keys['B'] && keys[VK_SHIFT] && keys[VK_UP]) { // more blueness to the fog
				color_fog_b += 0.01;
			}

			if (keys[VK_CONTROL] && keys['B'] && keys[VK_SHIFT] && keys[VK_DOWN]) { // less blueness to the fog
				color_fog_b -= 0.01;
			}

			if (keys['U'] && isRainning) { // slows down the rain  speed
				gravity += 0.005f;
			}

			if (keys['I'] && isRainning) { //  fasten the rain speed
				gravity -= 0.001f;
			}

			if (keys['J'] && isRainning ) { //  stop the wind
				wind_left_right = 0.0f;
				isWindy = false;
			}

			if (keys['L'] && isRainning && !keys[VK_CONTROL] ) { //  wind to the left
				isWindy = true;
				wind_left_right += 0.1f;
			}

			if (keys['K'] && isRainning && !keys[VK_CONTROL] ) { //  wind to the right
				isWindy = true;
				wind_left_right -= 0.1f;
			}


			if (keys['R'] && !keys[VK_CONTROL]) { //  rain
				isRainning = !isRainning;
			}

			if (keys['F'] && !keys[VK_CONTROL]) { //add fog
				fogScale += 0.001f;
			}

			if (keys['G'] && !keys[VK_CONTROL]) { // remove fog
				fogScale -= 0.001f;
			}

			if (keys[VK_RIGHT]) { //look right
				leftRightRotate += 0.5f;
			}
			if (keys[VK_LEFT]) { // look left
				leftRightRotate -= 0.5f;
			}
			if (keys[VK_UP] && !keys[VK_CONTROL]) { //look up
				upDownRotate += 0.8f;
			}
			if (keys[VK_DOWN] && !keys[VK_CONTROL]) { // look down
				upDownRotate -= 0.8f;
			}

			if (keys['D'] && !keys[VK_CONTROL]) { //move right
				lookRightLeft -= 1.5;
			}
			if (keys['A'] && !keys[VK_CONTROL]) { // move left
				lookRightLeft += 1.5;
			}
			if (keys['W'] && !keys[VK_CONTROL]) { // move up
				lookUpDown += 1.5;
			}
			if (keys['S'] && !keys[VK_CONTROL]) { // move down
				lookUpDown -= 1.5;
			}

			if (keys['Q']) { //move farward
				scaleValue -= 1.0f;
			}
			if (keys['E']) { // move backword
				scaleValue += 1.0f;
			}

			if (keys['P']) { //resolution line
				isRes = !isRes;
			}

			if (keys[VK_F1])
			{
				keys[VK_F1] = FALSE;
				KillGLWindow();
				fullscreen = !fullscreen;

				if (!CreateGLWindow((char*)"the best map", 640, 480, 16, fullscreen))
				{
					return 0;
				}
			}
		}
	}

	KillGLWindow();
	return (msg.wParam);
}