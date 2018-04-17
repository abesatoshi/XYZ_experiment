#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#define INITGUID
#define WIDTH 1920
#define HEIGHT 1200
#define LEFT_SIDE 1
#define RIGHT_SIDE 2
#define COLOR_GRAY cv::Point3i(127, 127, 127)
#define COLOR_LIGHTGRAY cv::Point3i(191, 191, 191)
#define COLOR_DARKGRAY cv::Point3i(63, 63, 63)
#define COLOR_RED cv::Point3i(191, 64, 64)
#define COLOR_GREEN cv::Point3i(64, 191, 64)
#define COLOR_BLUE cv::Point3i(64, 64, 191)
#define TEST_FLICKER 1
#define TEST_COLOR 2

#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <dinput.h>
#include <modulate.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "opencv_world320.lib")
#pragma comment(lib, "modulate.lib")

cv::Point3i origin;
colorPair mod;
int r, t;
FILE *fp;
int conditionID, initialID, randSeed, number;
int type;

DWORD RGB2DWORD(cv::Point3i RGB) {
	return (DWORD)(RGB.x * 256 * 256 + RGB.y * 256 + RGB.z);
}

struct condition {
	int r;
	int t;
	int c;
	int n;
};


//-----------------------------------------------------------------
//    Grobal Variables.
//-----------------------------------------------------------------
LPDIRECT3D9        g_pD3D = NULL;
LPDIRECT3DDEVICE9  g_pd3dDevice = NULL;
LPD3DXSPRITE       g_pSprite = NULL;
LPDIRECT3DTEXTURE9 g_pTexture1 = NULL;
LPDIRECT3DTEXTURE9 g_pTexture2 = NULL;
LPDIRECTINPUT8       g_lpDI;
LPDIRECTINPUTDEVICE8 g_lpDIDevice;

HWND g_hWnd;

bool isPlaying = true, parameterChanged = false;

//-----------------------------------------------------------------
//    Prototypes.
//-----------------------------------------------------------------
HWND    InitApp(HINSTANCE, int);
BOOL    InitDirect3D(HWND);
BOOL    CleanupDirect3D();
BOOL    RenderDirect3D();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL    InitDirectInput(HWND);
BOOL    ReadInput();
BOOL    CleanupDirectInput();

void setColor();
void randomize();
void printAns(int);

//-----------------------------------------------------------------
//    Main.
//-----------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevinst, LPSTR nCmdLine, int nCmdShow)
{
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	g_hWnd = InitApp(hInst, nCmdShow);
	if (!g_hWnd) return FALSE;

	if (!InitDirect3D(g_hWnd)) return FALSE;
	if (!InitDirectInput(g_hWnd)) return FALSE;

	char* cmd = nCmdLine;
	char *tp = strtok(cmd, " ");
	if (tp == NULL)
		randSeed = 0;
	else
		randSeed = atoi(tp);
	tp = strtok(NULL, " ");
	if (tp == NULL)
		initialID = -1;
	else
		initialID = atoi(tp);

	if (initialID == -1)
		fp = fopen("log.txt", "w");
	else
		fp = fopen("log.txt", "a");
	if (fp == NULL)
		return FALSE;

	tp = strtok(NULL, " ");
	if (tp == NULL) {
		fprintf(fp, "usage: .exe seed initialID type(1:flicker, 2:color)");
		fflush(fp);
		return FALSE;
	}
	else
		type = atoi(tp);

	if (initialID == -1)
		fprintf(fp, "origin, r, theta, ans, id, R1, G1, B1, R2, G2, B2\n");
	conditionID = initialID;

	origin = COLOR_GRAY;
	r = t = 0;
	number = -1;
	setColor();
	char    titlebar[32];
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			RenderDirect3D();
			ReadInput();

			sprintf(titlebar, "(%d, %d, %d) : (%d, %d, %d) <-> (%d, %d, %d), r = %d (%d), t = %d, ID = %d",
				origin.x, origin.y, origin.z,
				mod.getRGB1().x, mod.getRGB1().y, mod.getRGB1().z,
				mod.getRGB2().x, mod.getRGB2().y, mod.getRGB2().z,
				r, mod.eval(), t, conditionID);

			SetWindowText(g_hWnd, titlebar);
		}
		Sleep(1);
	}

	return msg.wParam;
}



//-----------------------------------------------------------------
//    Initialize Application.
//-----------------------------------------------------------------
HWND InitApp(HINSTANCE hInst, int nCmdShow)
{
	HWND hWnd;
	WNDCLASS wc;
	char szClassName[] = "Direct3D Test";

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = hInst;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = szClassName;
	wc.lpszMenuName = NULL;
	wc.lpfnWndProc = WndProc;
	wc.cbWndExtra = 0;
	wc.cbClsExtra = 0;
	if (!RegisterClass(&wc)) return FALSE;

	hWnd = CreateWindow(szClassName, "Direct3D Test", WS_POPUP,//WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT,
		NULL, NULL, hInst, NULL);
	if (!hWnd) return FALSE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

//-----------------------------------------------------------------
//    Initialize Direct3D.
//-----------------------------------------------------------------
BOOL InitDirect3D(HWND hWnd)
{
	D3DPRESENT_PARAMETERS d3dpp;
	HRESULT hr;

	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION))) {
		MessageBox(hWnd, "Can't create Direct3D.", "Error", MB_OK);
		return FALSE;
	}
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	hr = g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice);
	if (FAILED(hr)) {
		MessageBox(hWnd, "Can't create device.", "Error", MB_OK);
		return FALSE;
	}

	D3DXCreateSprite(g_pd3dDevice, &g_pSprite);

	g_pd3dDevice->CreateTexture(WIDTH, HEIGHT, 1,
		D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT,
		&g_pTexture1, NULL);
	g_pd3dDevice->CreateTexture(WIDTH, HEIGHT, 1,
		D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT,
		&g_pTexture2, NULL);
	return TRUE;
}



//-----------------------------------------------------------------
//    Cleanup Direct3D.
//-----------------------------------------------------------------
BOOL CleanupDirect3D()
{
	if (g_pTexture1 != NULL)
		g_pTexture1->Release();

	if (g_pTexture2 != NULL)
		g_pTexture2->Release();

	if (g_pSprite != NULL)
		g_pSprite->Release();

	if (g_pd3dDevice != NULL)
		g_pd3dDevice->Release();

	if (g_pD3D != NULL)
		g_pD3D->Release();

	return TRUE;
}



//-----------------------------------------------------------------
//    Initialize DirectInput.
//-----------------------------------------------------------------
BOOL InitDirectInput(HWND hWnd)
{
	HRESULT   hr;

	hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&g_lpDI, NULL);
	if (FAILED(hr)) {
		MessageBox(hWnd, "Can't create DirectInput object.", "Error", MB_OK);
		return FALSE;
	}

	hr = g_lpDI->CreateDevice(GUID_SysKeyboard, &g_lpDIDevice, NULL);
	if (FAILED(hr)) {
		CleanupDirectInput();
		MessageBox(hWnd, "Can't create DirectInput device.", "Error", MB_OK);
		return FALSE;
	}

	hr = g_lpDIDevice->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(hr)) {
		CleanupDirectInput();
		MessageBox(hWnd, "Can't set data format.", "Error", MB_OK);
		return FALSE;
	}

	hr = g_lpDIDevice->SetCooperativeLevel(hWnd,
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) {
		CleanupDirectInput();
		MessageBox(hWnd, "Can't set cooperative level.", "Error", MB_OK);
		return FALSE;
	}

	if (g_lpDIDevice) g_lpDIDevice->Acquire();

	return TRUE;
}



//-----------------------------------------------------------------
//    Cleanup DirectInput.
//-----------------------------------------------------------------
BOOL CleanupDirectInput()
{
	g_lpDIDevice->Unacquire();

	if (g_lpDIDevice != NULL)
		g_lpDIDevice->Release();

	if (g_lpDI != NULL)
		g_lpDI->Release();

	return TRUE;
}



//-----------------------------------------------------------------
//    Window Proc.
//-----------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_DESTROY:
		CleanupDirect3D();
		CleanupDirectInput();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wp, lp);
	}

	return 0L;
}



//-----------------------------------------------------------------
//    Render Direct3D.
//-----------------------------------------------------------------
BOOL RenderDirect3D()
{

	static int count = 0;

	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET,
		D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	g_pd3dDevice->BeginScene();

	g_pSprite->Begin(NULL);

	if (count % 2 == 0)
		g_pSprite->Draw(g_pTexture1, NULL, NULL, NULL, 0xFFFFFFFF);
	else
		g_pSprite->Draw(g_pTexture2, NULL, NULL, NULL, 0xFFFFFFFF);
	
	g_pSprite->End();
	g_pd3dDevice->EndScene();
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

	if (isPlaying) count++;

	return TRUE;
}

//-----------------------------------------------------------------
//    Read Input.
//-----------------------------------------------------------------
BOOL ReadInput()
{
	char    buffer[256];
	HRESULT hr;
	static bool put = false;

	hr = g_lpDIDevice->GetDeviceState(sizeof(buffer), (LPVOID)&buffer);
	if (FAILED(hr)) return FALSE;

	if (buffer[DIK_SPACE] & 0x80) {
		if (!put) isPlaying = !isPlaying;
	}
	else if (buffer[DIK_RETURN] & 0x80) {
		if (!put) {
			conditionID++;
			randomize();
		}
	}
	else if (buffer[DIK_Q] & 0x80) {
		if (!put) PostQuitMessage(0);
	}
	else if ((buffer[DIK_0] & 0x80) || (buffer[DIK_NUMPAD0] & 0x80)) {
		if (!put) {
			printAns(0);
			conditionID++;
			randomize();
		}
	}
	else if ((buffer[DIK_1] & 0x80) || (buffer[DIK_NUMPAD1] & 0x80)) {
		if (!put) {
			printAns(1);
			conditionID++;
			randomize();
		}
	}
	else if (buffer[DIK_UPARROW] & 0x80) {
		if (!put) {
			r += 10;
			setColor();
		}
	}
	else if (buffer[DIK_DOWNARROW] & 0x80) {
		if (!put) {
			r -= 10;
			setColor();
		}
	}
	else if (buffer[DIK_LEFTARROW] & 0x80) {
		if (!put) {
			t = 90;
			setColor();
		}
	}
	else if (buffer[DIK_RIGHTARROW] & 0x80) {
		if (!put) {
			t = 0;
			setColor();
		}
	}
	else if (buffer[DIK_A] & 0x80) {
		if (!put) {
			origin = COLOR_GRAY;
			setColor();
		}
	}
	else if (buffer[DIK_S] & 0x80) {
		if (!put) {
			origin = COLOR_LIGHTGRAY;
			setColor();
		}
	}
	else if (buffer[DIK_D] & 0x80) {
		if (!put) {
			origin = COLOR_DARKGRAY;
			setColor();
		}
	}
	else if (buffer[DIK_F] & 0x80) {
		if (!put) {
			origin = COLOR_RED;
			setColor();
		}
	}
	else if (buffer[DIK_G] & 0x80) {
		if (!put) {
			origin = COLOR_GREEN;
			setColor();
		}
	}
	else if (buffer[DIK_H] & 0x80) {
		if (!put) {
			origin = COLOR_BLUE;
			setColor();
		}
	}
	else {
		put = false;
		return TRUE;
	}

	put = true;

	return TRUE;
}

void paintTexture(cv::Point3i RGB, LPDIRECT3DTEXTURE9 pTexture) {
	D3DLOCKED_RECT lockedRect;
	pTexture->LockRect(0, &lockedRect, NULL, D3DLOCK_NO_DIRTY_UPDATE);
	for (int x = WIDTH / 2 - HEIGHT / 2; x < WIDTH / 2 + HEIGHT / 2; x++) {
		for (int y = 0; y < HEIGHT; y++) {
			DWORD color = RGB2DWORD(RGB);
			memcpy((BYTE*)lockedRect.pBits + lockedRect.Pitch*y + 4 * x, &color, sizeof(DWORD));
		}
	}
	pTexture->UnlockRect(0);
}

void paintTexture2(cv::Point3i RGB, LPDIRECT3DTEXTURE9 pTexture, int side) {

	D3DLOCKED_RECT lockedRect;
	pTexture->LockRect(0, &lockedRect, NULL, D3DLOCK_NO_DIRTY_UPDATE);
	int start, end;
	switch (side) {
	case LEFT_SIDE:
		start = WIDTH / 2 - HEIGHT / 2;
		end = WIDTH / 2;
		break;
	case RIGHT_SIDE:
		start = WIDTH / 2;
		end = WIDTH / 2 + HEIGHT / 2;
		break;
	}
	for (int x = start; x < end; x++) {
		for (int y = 0; y < HEIGHT; y++) {
			DWORD color = (x == WIDTH / 2 - 1) ? 
				((RGB2XYZ(RGB).y <= 0.5) ? 0xffffffff : 0xff000000):
				RGB2DWORD(RGB);
			memcpy((BYTE*)lockedRect.pBits + lockedRect.Pitch*y + 4 * x, &color, sizeof(DWORD));
		}
	}
	pTexture->UnlockRect(0);
}

void paint(cv::Point3i c, colorPair p) {
	if (type == TEST_FLICKER) {
		paintTexture(p.getRGB1(), g_pTexture1);
		paintTexture(p.getRGB2(), g_pTexture2);
	}
	else if (type == TEST_COLOR) {
		paintTexture2(p.getRGB1(), g_pTexture1, RIGHT_SIDE);
		paintTexture2(p.getRGB2(), g_pTexture2, RIGHT_SIDE);
		paintTexture2(c, g_pTexture1, LEFT_SIDE);
		paintTexture2(c, g_pTexture2, LEFT_SIDE);
	}
}

colorPair modulate(cv::Point3i RGB, int r, double t) {
	colorPair p(RGB, RGB);
	cv::Point3d XYZ = RGB2XYZ(RGB);
	double X = XYZ.x, Y = XYZ.y, Z = XYZ.z;

	double R = 0.0;
	while (p.eval() < r && R <= 1.0) {
		R += 0.0001;
		cv::Point3i RGB1 = XYZ2RGB(cv::Point3d(X + R * cos(t), Y, Z + R * sin(t)));
		cv::Point3i RGB2 = XYZ2RGB(cv::Point3d(X - R * cos(t), Y, Z - R * sin(t)));
		p = colorPair(RGB1, RGB2);
	}
	return p;
}

void setColor() {
	mod = modulate(origin, r, t / 180.0 * M_PI);
	paint(origin, mod);
}

void setCondition(struct condition cond) {
	switch (cond.c) {
	case 0:
		origin = COLOR_GRAY;
		break;
	case 1:
		origin = COLOR_LIGHTGRAY;
		break;
	case 2:
		origin = COLOR_DARKGRAY;
		break;
	case 3:
		origin = COLOR_RED;
		break;
	case 4:
		origin = COLOR_GREEN;
		break;
	case 5:
		origin = COLOR_BLUE;
		break;
	}
	switch (cond.t) {
	case 0:
		t = 0;
		break;
	case 1:
		t = 90;
		break;
	}
	r = cond.r;
	number = cond.n;
	setColor();
}

const int Ncolor = 6;
const int Ntheta = 2;
const int Nr1[Ncolor][Ntheta] = // n = 61
{
	{ 4, 7 },
	{ 7, 7 },
	{ 4, 4 },
	{ 6, 4 },
	{ 4, 4 },
	{ 4, 6 },
};
const int rs1[Ncolor][Ntheta][10] = 
{
	{ { 50, 100, 150, 210 }, { 20, 60, 80, 100, 120, 140, 180 } },
	{ { 50, 100, 150, 180, 200, 220, 250 }, { 10, 40, 60, 80, 100, 120, 160 } },
	{ { 10, 50, 80, 110 }, { 10, 50, 90, 120 } },
	{ { 20, 40, 60, 80, 100, 150 }, { 10, 50, 80, 100 } },
	{ { 10, 50, 70, 90 }, { 10, 50, 90, 120 } },
	{ { 10, 50, 80, 110 }, { 20, 40, 60, 80, 100, 130 } }
};
const int Nr2[Ncolor][Ntheta] = // n = 62
{
	{ 4, 7 },
	{ 8, 7 },
	{ 6, 6 },
	{ 4, 4 },
	{ 4, 4 },
	{ 4, 4 },
};
const int rs2[Ncolor][Ntheta][10] =
{
	{ { 50, 100, 150, 210 },{ 20, 60, 80, 100, 120, 140, 180 } },
	{ { 50, 100, 140, 160, 180, 200, 220, 250 },{ 40, 90, 110, 130, 150, 170, 230 } },
	{ { 10, 30, 50, 70, 90, 110 },{ 20, 50, 70, 90, 110, 120 } },
	{ { 10, 50, 100, 150 },{ 10, 50, 80, 100 } },
	{ { 10, 50, 70, 90 },{ 10, 50, 90, 120 } },
	{ { 10, 50, 80, 110 },{ 10, 50, 90, 130 } }
};

void shuffle(struct condition array[], int size) {

	srand(randSeed);
	for (int i = 0; i < size; i++) {
		int j = rand() % size;
		struct condition t = array[i];
		array[i] = array[j];
		array[j] = t;
	}
}


void randomize() {

	int Nitr = 3;
	int Ncond = 0;
	for (int i = 0; i < Ncolor; i++)
		for (int j = 0; j < Ntheta; j++)
			Ncond += (type == TEST_FLICKER) ? Nr1[i][j] : Nr2[i][j];
	int Ntotal = Ncond * Nitr;

	static bool first = true;
	static struct condition *conditions;
	if (first) {
		conditions = (struct condition*)malloc(sizeof(struct condition) * Ntotal);
		int id = 0;
		for (int i = 0; i < Ncolor; i++) {
			for (int j = 0; j < Ntheta; j++) {
				for (int k = 0; k < ((type == TEST_FLICKER) ? Nr1[i][j] : Nr2[i][j]); k++) {
					for (int l = 0; l < Nitr; l++) {
						conditions[id].r = (type == TEST_FLICKER) ? rs1[i][j][k] : rs2[i][j][k];
						conditions[id].t = j;
						conditions[id].c = i;
						conditions[id].n = (id - l) / Nitr;
						id++;
					}
				}
			}
		}
		shuffle(conditions, Ntotal);
		first = false;
	}

	if (conditionID >= Ntotal) {
		MessageBox(g_hWnd, "finished.", "Error", MB_OK);
		fclose(fp);
		PostQuitMessage(0);
	}
	else
		setCondition(conditions[conditionID]);
}

void printAns(int ans) {
	if (origin == COLOR_GRAY)
		fprintf(fp, "GRAY, ");
	else if (origin == COLOR_LIGHTGRAY)
		fprintf(fp, "LIGHTGRAY, ");
	else if (origin == COLOR_DARKGRAY)
		fprintf(fp, "DARKGRAY, ");
	else if (origin == COLOR_RED)
		fprintf(fp, "RED, ");
	else if (origin == COLOR_GREEN)
		fprintf(fp, "GREEN, ");
	else if (origin == COLOR_BLUE)
		fprintf(fp, "BLUE, ");
	fprintf(fp, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
		r, t,
		ans, number,
		mod.getRGB1().x, mod.getRGB1().y, mod.getRGB1().z, 
		mod.getRGB2().x, mod.getRGB2().y, mod.getRGB2().z);
	fflush(fp);
}