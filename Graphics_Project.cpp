#include <windows.h>		// Header File For Windows
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>
#pragma comment(lib, "opengl32.lib")                // Link OpenGL32.lib
#pragma comment(lib, "glu32.lib")               // Link Glu32.lib
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define     MAP_SIZE    1024                // Size Of Our .RAW Height Map ( NEW )
#define     STEP_SIZE   1              // Width And Height Of Each Quad ( NEW )
#define     HEIGHT_RATIO    1.5f                // Ratio That The Y Is Scaled According To The X And Z ( NEW )

HDC     hDC = NULL;                   // Private GDI Device Context
HGLRC       hRC = NULL;                   // Permanent Rendering Context
HWND        hWnd = NULL;                  // Holds Our Window Handle
HINSTANCE   hInstance;                  // Holds The Instance Of The Application

bool        keys[256];                  // Array Used For The Keyboard Routine
bool        fullscreen = TRUE;                // Fullscreen Flag Set To TRUE By Default
bool        bRender = TRUE;                 // Polygon Flag Set To TRUE By Default ( NEW )

Mat global_HMap;
float scaleValue = 0.25f;                   // Scale Value For The Terrain ( NEW )
float leftRightRotate = 0.25f;
float upDownRotate = 0.25f;
float lookRightLeft = 0.25f;
float lookUpDown = 0.25f;


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
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 500.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

// Loads The .RAW File And Stores It In pHeightMap
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

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	glShadeModel(GL_SMOOTH);                // Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);           // Black Background
	glClearDepth(1.0f);                 // Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);                // Enables Depth Testing
	glDepthFunc(GL_LEQUAL);                 // The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Really Nice Perspective Calculations

	// Here we read read in the height map from the .raw file and put it in our
	// g_HeightMap array.  We also pass in the size of the .raw file (1024).

	global_HMap = imread("C:\\Users\\User\\Desktop\\image.jfif", IMREAD_COLOR);
	return TRUE;                        // Initialization Went OK
}

float Height(int X, int Y)          // This Returns The Height From A Height Map Index
{
	int x = X % global_HMap.rows;                   // Error Check Our x Value
	int y = Y % global_HMap.cols;                   // Error Check Our y Value

	if (! &global_HMap) return 0;               // Make Sure Our Data Is Valid
	return 61 *global_HMap.at<Vec3b>(Point(y, x)).val[0] / 256;       // Index Into Our Height Array And Return The Height
} 

void setVerteces() {
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(0.0,0.0,0.0);
	glVertex3f(700.0,0.0,0.0);
	glEnd();

	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 150.0, 0.0);
	glEnd();

	glColor3f(1.0, 0.0, 1.0);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(700.0, 0.0, 700.0);
	glEnd();
}

void setVertexColor(int x, int y)     // This Sets The Color Value For A Particular Index
{                               // Depending On The Height Index
	if (!&global_HMap) return;                 // Make Sure Our Height Data Is Valid

	float fColor = Height(x,y);

	if(fColor == 0)
		glColor3f(0.4, 0.4, 0.4);
	else if (fColor < 15.8)
		glColor4f(0, 0, 255, 0.8);
	else if (fColor < 21.45)
		glColor4f(0, 128, 255, 0.8);
	else if (fColor < 27.1)
		glColor4f(0, 255, 128, 0.8);
	else if (fColor < 28.4)
		glColor4f(0, 255, 255, 0.8);
	else if (fColor < 32.75)
		glColor4f(0, 255, 0, 0.8);
	else if (fColor < 44.05)
		glColor4f(255, 255, 0, 0.8);
	else if (fColor < 49.7)
		glColor4f(255, 128, 0, 0.8);
	else if (fColor < 55.35)
		glColor4f(255, 0, 0, 0.8);
	else if (fColor < 61)
		glColor4f(255, 0, 0, 0.8);
	else
		glColor3f(255, 255, 255);

	/*
	if (fColor < 15.8)
	glColor3f(0.4, 0.4, 0.4);
	else if (fColor < 21.45)
	glColor4f(0, 0, 255, 0.8);
	else if (fColor < 27.1)
	glColor4f(0, 128, 255, 0.8);
	else if (fColor < 28.4)
	glColor4f(0, 255, 128, 0.8);
	else if (fColor < 32.75)
	glColor4f(0, 255, 255, 0.8);
	else if (fColor < 44.05)
	glColor4f(0, 255, 0, 0.8);
	else if (fColor < 49.7)
	glColor4f(255, 255, 0, 0.8);
	else if (fColor < 55.35)
	glColor4f(255, 128, 0, 0.8);
	else if (fColor < 61)
	glColor4f(255, 0, 0, 0.8);
	else
	glColor3f(255, 255, 255);
	*/
}

int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(212 +lookRightLeft, 60+lookUpDown, 194,186 + lookRightLeft, 55 + lookUpDown, 171,0, 1, 0);
	glScalef(scaleValue, scaleValue*HEIGHT_RATIO, scaleValue);
	glTranslatef(256,0.0,256);
	setVerteces();
	glRotatef(leftRightRotate *10, 0 ,10, 0);
	glRotatef(upDownRotate * 10, 0, 0, 10);
	glTranslatef(-256, 0.0, -256);

	//gluLookAt(20.0 +rotateRight+rotateAngle, 10.0+rotateUp, -40.0 +scaleValue, 10 + rotateRight, 0.0+rotateUp, 20.0 + scaleValue, 0, 1, 0);
	//glTranslatef(global_HMap.rows/256, global_HMap.cols/256, 0);
	//glScalef(scaleValue, scaleValue*HEIGHT_RATIO, scaleValue);
	//glRotatef(rotateRight, 0, 0, 0);
	//glRotatef(rotateUp, 0, 0, 0);

	float cols = global_HMap.cols;
	float rows = global_HMap.rows;

	for (float x = 0; x < (rows - STEP_SIZE); x += STEP_SIZE) {
		for (float y = 0; y < (cols - STEP_SIZE); y += STEP_SIZE) {
			float h1 = Height(x, y);
			float h2 = Height(x, y + STEP_SIZE);
			float h3 = Height(x + STEP_SIZE, y + STEP_SIZE);
			float h4 = Height(x + STEP_SIZE, y);

			glBegin(GL_TRIANGLES);
			setVertexColor(float(x), float(y));
			glVertex3f(float(x), h1, float(y));
			setVertexColor(float(x) + STEP_SIZE, float(y));
			glVertex3f(float(x) + STEP_SIZE, h4, float(y));
			setVertexColor(float(x), float(y) + STEP_SIZE);
			glVertex3f(float(x), h2, float(y)+STEP_SIZE);
			glEnd();

			glBegin(GL_TRIANGLES);
			setVertexColor(float(x), float(y) + STEP_SIZE);
			glVertex3f(float(x), h2, float(y) + STEP_SIZE);
			setVertexColor(float(x) + STEP_SIZE, float(y));
			glVertex3f(float(x) + STEP_SIZE, h4, float(y));
			setVertexColor(float(x) + STEP_SIZE, float(y) + STEP_SIZE);
			glVertex3f(float(x) + STEP_SIZE, h3, float(y) + STEP_SIZE);
			glEnd();

			glColor3f(0, 0, 0);
			glBegin(GL_LINES);
			glVertex3f(float(x), h1, float(y));
			glVertex3f(float(x) + STEP_SIZE, h4, float(y));
			glEnd();
			
			glBegin(GL_LINES);
			glVertex3f(float(x), h2, float(y) + STEP_SIZE);
			glVertex3f(float(x) + STEP_SIZE, h4, float(y));
			glEnd();

			glBegin(GL_LINES);
			glVertex3f(float(x), h2, float(y) + STEP_SIZE);
			glVertex3f(float(x), h1, float(y));
			glEnd();
			//
			glBegin(GL_LINES);
			glVertex3f(float(x), h2, float(y) + STEP_SIZE);
			glVertex3f(float(x) + STEP_SIZE, h4, float(y));
			glEnd();

			glBegin(GL_LINES);
			glVertex3f(float(x) + STEP_SIZE, h4, float(y));
			glVertex3f(float(x) + STEP_SIZE, h3, float(y) + STEP_SIZE);
			glEnd();

			glBegin(GL_LINES);
			glVertex3f(float(x), h2, float(y) + STEP_SIZE);
			glVertex3f(float(x) + STEP_SIZE, h3, float(y) + STEP_SIZE);
			glEnd();
		}
	}
	return TRUE;
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
		bRender = !bRender;         // Change Rendering State Between Fill/Wire Frame
		return 0;               // Jump Back
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

int WINAPI WinMain(HINSTANCE   hInstance,      // Instance
	HINSTANCE   hPrevInstance,      // Previous Instance
	LPSTR       lpCmdLine,      // Command Line Parameters
	int     nCmdShow)       // Window Show State
{
	MSG     msg;                    // Windows Message Structure
	BOOL    done = FALSE;                 // Bool Variable To Exit Loop

										  // Ask The User Which Screen Mode They Prefer
	//if (MessageBox(NULL, "Would You Like To Run In Fullscreen Mode?", "Start FullScreen?", MB_YESNO | MB_ICONQUESTION) == IDNO)
	//{
	fullscreen = FALSE;               // Windowed Mode
	//}

	// Create Our OpenGL Window
	if (!CreateGLWindow("Roy's Triangles", 640, 480, 16, fullscreen))
	{
		return 0;                   // Quit If Window Was Not Created
	}

	while (!done)                        // Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))   // Is There A Message Waiting?
		{
			if (msg.message == WM_QUIT)       // Have We Received A Quit Message?
			{
				done = TRUE;          // If So done=TRUE
			}
			else                    // If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);     // Translate The Message
				DispatchMessage(&msg);      // Dispatch The Message
			}
		}
		else                        // If There Are No Messages
		{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if ((active && !DrawGLScene()) || keys[VK_ESCAPE])  // Active?  Was There A Quit Received?
			{
				done = TRUE;          // ESC or DrawGLScene Signalled A Quit
			}
			else if (active)            // Not Time To Quit, Update Screen
			{
				SwapBuffers(hDC);       // Swap Buffers (Double Buffering)
			}

			if (keys[VK_F1])            // Is F1 Being Pressed?
			{
				keys[VK_F1] = FALSE;      // If So Make Key FALSE
				KillGLWindow();         // Kill Our Current Window
				fullscreen = !fullscreen;     // Toggle Fullscreen / Windowed Mode
											  // Recreate Our OpenGL Window
				if (!CreateGLWindow("NeHe & Ben Humphrey's Height Map Tutorial", 640, 480, 16, fullscreen))
				{
					return 0;       // Quit If Window Was Not Created
				}
			}
			if (keys['E'])
				scaleValue += 0.01f;

			if (keys['Q'])
				scaleValue -= 0.01f;

			if (keys[VK_RIGHT])
				leftRightRotate += 0.1f;

			if (keys[VK_LEFT])
				leftRightRotate -= 0.1f;

			if (keys[VK_UP])
				upDownRotate += 0.1f;

			if (keys[VK_DOWN])
				upDownRotate -= 0.1f;

			if (keys['W'])
				lookUpDown += 3;

			if (keys['S'])
				lookUpDown -= 3;

			if (keys['D'])
				lookRightLeft += 3;
			
			if (keys['A'])
				lookRightLeft -= 3;
		}
	}

	// Shutdown
	KillGLWindow();                     // Kill The Window
	return (msg.wParam);                    // Exit The Program
}

	/*
	Mat img_color = imread("C:\\Users\\User\\Desktop\\chewbacca.png", 1);
	namedWindow("color image", WINDOW_AUTOSIZE);
	imshow("color image", img_color);
	imwrite("C:\\Users\\User\\Desktop\\chewbaccaa.png", img_color);
	waitKey(0);
	destroyAllWindows();
	return 0;
	*/