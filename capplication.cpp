#include"main.h"
#include "game_state_manager.h"
#include "d3ddefs.h"
#include "globals.h"

CApplication::CApplication(void)
{
	//define member variables
	m_pDirect3DObject = NULL;
	m_pDirect3DDevice = NULL;
	//m_pDirect3DSwapChain = NULL;

	m_dwFullScreen = 0;
	m_dwWidth = 800;
	m_dwHeight = 600;
	m_ColorFormat = D3DFMT_A8R8G8B8;
	m_DepthStencilFormat = D3DFMT_D24S8; 
	m_dwVertexProcessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	m_dwAnisotropy = 0;

	m_bRunningLauncherWindow = true;

	m_bRunningD3D = false;
	m_subtitles = 0;
	m_invertX = 0;
	m_invertY = 0;
	m_level = 0;
	m_debug = 0;

	//create window
	InitWindow();
}//CApplication

/************************************************************************/
/* Name:		~CApplication											*/
/* Description:	call kill functions										*/
/************************************************************************/

CApplication::~CApplication(void)
{
	KillD3D();
	KillWindow();
}//~CApplication

/************************************************************************/
/* Name:		InitWindow												*/
/* Description:	create	window class, window and child elements			*/
/************************************************************************/

void CApplication::InitFonts( ) {
		// Create a debug font
	HRESULT hr = D3DXCreateFont( g_App.GetDevice()
		          , 20
				  , 10
				  , FW_NORMAL
				  , 1
				  , FALSE
				  , DEFAULT_CHARSET
				  , OUT_DEFAULT_PRECIS
				  , DEFAULT_QUALITY
				  , DEFAULT_PITCH|FF_DONTCARE
				  , "Arial"
				  , &m_dbgFont
				  );
	assert( hr == D3D_OK );

	hr = D3DXCreateFont( g_App.GetDevice()
		          , 60
				  , 30
				  , FW_NORMAL
				  , 1
				  , FALSE
				  , DEFAULT_CHARSET
				  , OUT_DEFAULT_PRECIS
				  , DEFAULT_QUALITY
				  , DEFAULT_PITCH|FF_DONTCARE
				  , "Arial"
				  , &m_dbgFont_big
				  );
	assert( hr == D3D_OK );
}

void CApplication::DestroyFonts(void)
{
	if( m_dbgFont ) {
		m_dbgFont->Release();
		m_dbgFont = NULL;
	}
	
	if( m_dbgFont_big ) {
		m_dbgFont_big->Release();
		m_dbgFont_big = NULL;
	}
}

void CApplication::InitWindow(void)
{
	WNDCLASSEX WindowClass;
	RECT DesktopSize;

	InitCommonControls();

	//Get desktop resolution
	GetClientRect(GetDesktopWindow(),&DesktopSize);

	//fill window class and register class
	WindowClass.cbSize = sizeof(WNDCLASSEX);
	WindowClass.style = CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc = WindowProcedure;
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = GetModuleHandle(NULL);
	WindowClass.hIcon = LoadIcon(WindowClass.hInstance,MAKEINTRESOURCE(IDI_ICON));
	WindowClass.hIconSm = LoadIcon(WindowClass.hInstance,MAKEINTRESOURCE(IDI_ICON));
	WindowClass.hCursor = NULL;
	WindowClass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	WindowClass.lpszMenuName = NULL;
	WindowClass.lpszClassName = "Launcher";

	RegisterClassEx(&WindowClass);

	//create window
	m_hWindowLauncher = CreateWindowEx(WS_EX_CONTROLPARENT,"Launcher",TITLE,WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,(DesktopSize.right - WINDOW_X) / 2,(DesktopSize.bottom - WINDOW_Y) / 2,WINDOW_X,WINDOW_Y,NULL,(HMENU) NULL,GetModuleHandle(NULL),NULL);

	//create window elements

	m_hLblResolution = CreateWindow("static","Resolution:",WS_CHILD | WS_VISIBLE | SS_LEFT,20,24,200,18,m_hWindowLauncher,NULL,(HINSTANCE)GetWindowLong(m_hWindowLauncher,GWL_HINSTANCE),NULL);
	m_hCbResolution = CreateWindow("combobox", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,160,20,160,360,m_hWindowLauncher,NULL,(HINSTANCE)GetWindowLong(m_hWindowLauncher,GWL_HINSTANCE),NULL);
	m_hLblAnisotropy = CreateWindow("static","Anisotropy:",WS_CHILD | WS_VISIBLE | SS_LEFT,20,54,200,18,m_hWindowLauncher,NULL,(HINSTANCE)GetWindowLong(m_hWindowLauncher,GWL_HINSTANCE),NULL);
	m_hCbAnisotropy = CreateWindow("combobox", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,160,50,160,360,m_hWindowLauncher,NULL,(HINSTANCE)GetWindowLong(m_hWindowLauncher,GWL_HINSTANCE),NULL);
	
	m_hLblFullScreen = CreateWindow("static","Display Mode:",WS_CHILD | WS_VISIBLE | SS_LEFT,20,84,200,18,m_hWindowLauncher,NULL,(HINSTANCE)GetWindowLong(m_hWindowLauncher,GWL_HINSTANCE),NULL);
	m_hCbFullScreen = CreateWindow("combobox", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,160,80,160,360,m_hWindowLauncher,NULL,(HINSTANCE)GetWindowLong(m_hWindowLauncher,GWL_HINSTANCE),NULL);
	
	m_hLbSubsEnabled = CreateWindow("static","Subtitles:",WS_CHILD | WS_VISIBLE | SS_LEFT,20,114,200,18,m_hWindowLauncher,NULL,(HINSTANCE)GetWindowLong(m_hWindowLauncher,GWL_HINSTANCE),NULL);
	m_hTrgSubsEnabled = CreateWindow("combobox", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,160,110,160,360,m_hWindowLauncher,NULL,(HINSTANCE)GetWindowLong(m_hWindowLauncher,GWL_HINSTANCE),NULL);
	
	m_hLbInvertX = CreateWindow("static","Invert X axis:",WS_CHILD | WS_VISIBLE | SS_LEFT,20,144,200,18,m_hWindowLauncher,NULL,(HINSTANCE)GetWindowLong(m_hWindowLauncher,GWL_HINSTANCE),NULL);
	m_hTrgInvertX = CreateWindow("combobox", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,160,140,160,360,m_hWindowLauncher,NULL,(HINSTANCE)GetWindowLong(m_hWindowLauncher,GWL_HINSTANCE),NULL);
	
	m_hLbInvertY = CreateWindow("static","Invert Y axis:",WS_CHILD | WS_VISIBLE | SS_LEFT,20,174,200,18,m_hWindowLauncher,NULL,(HINSTANCE)GetWindowLong(m_hWindowLauncher,GWL_HINSTANCE),NULL);
	m_hTrgInvertY = CreateWindow("combobox", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,160,170,160,360,m_hWindowLauncher,NULL,(HINSTANCE)GetWindowLong(m_hWindowLauncher,GWL_HINSTANCE),NULL);
	
	m_hLbLevel = CreateWindow("static","Level selector:",WS_CHILD | WS_VISIBLE | SS_LEFT,20,204,200,18,m_hWindowLauncher,NULL,(HINSTANCE)GetWindowLong(m_hWindowLauncher,GWL_HINSTANCE),NULL);
	m_hTrgLevel = CreateWindow("combobox", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,160,200,160,360,m_hWindowLauncher,NULL,(HINSTANCE)GetWindowLong(m_hWindowLauncher,GWL_HINSTANCE),NULL);
	
	m_hLblDebug = CreateWindow("static","Debug:",WS_CHILD | WS_VISIBLE | SS_LEFT,20,234,200,18,m_hWindowLauncher,NULL,(HINSTANCE)GetWindowLong(m_hWindowLauncher,GWL_HINSTANCE),NULL);
	m_hCbDebug = CreateWindow("combobox", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,160,230,160,360,m_hWindowLauncher,NULL,(HINSTANCE)GetWindowLong(m_hWindowLauncher,GWL_HINSTANCE),NULL);
	
	m_hBtnStart = CreateWindow("button","Start",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,65,260,80,24,m_hWindowLauncher,(HMENU)ID_START,(HINSTANCE)GetWindowLong(m_hWindowLauncher,GWL_HINSTANCE),NULL);
	m_hBtnCancel = CreateWindow("button","Cancel",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,175,260,80,24,m_hWindowLauncher,(HMENU)ID_CANCEL,(HINSTANCE)GetWindowLong(m_hWindowLauncher,GWL_HINSTANCE),NULL);

	//fill combo boxes
	SendMessage(m_hCbResolution,CB_ADDSTRING,0,(long)"640 x 480");
	SendMessage(m_hCbResolution,CB_ADDSTRING,0,(long)"800 x 600");
	SendMessage(m_hCbResolution,CB_ADDSTRING,0,(long)"1024 x 768");
	SendMessage(m_hCbResolution,CB_ADDSTRING,0,(long)"1152 x 864");
	SendMessage(m_hCbResolution,CB_ADDSTRING,0,(long)"1280 x 720");
	SendMessage(m_hCbResolution,CB_ADDSTRING,0,(long)"1280 x 960");
	SendMessage(m_hCbResolution,CB_ADDSTRING,0,(long)"1280 x 1024");
	SendMessage(m_hCbResolution,CB_ADDSTRING,0,(long)"1366 x 768");
	SendMessage(m_hCbResolution,CB_ADDSTRING,0,(long)"1440 x 900");
	SendMessage(m_hCbResolution,CB_ADDSTRING,0,(long)"1440 x 1080");
	SendMessage(m_hCbResolution,CB_ADDSTRING,0,(long)"1600 x 900");
	SendMessage(m_hCbResolution,CB_ADDSTRING,0,(long)"1600 x 1200");
	SendMessage(m_hCbResolution,CB_ADDSTRING,0,(long)"1680 x 1050");
	SendMessage(m_hCbResolution,CB_ADDSTRING,0,(long)"1920 x 1080");
	SendMessage(m_hCbResolution,CB_ADDSTRING,0,(long)"1920 x 1200");
	SendMessage(m_hCbResolution,CB_ADDSTRING,0,(long)"2048 x 1152");
	SendMessage(m_hCbResolution,CB_ADDSTRING,0,(long)"4096 x 2304");
	SendMessage(m_hCbResolution,CB_ADDSTRING,0,(long)"7680 x 4320");
	SendMessage(m_hCbResolution,CB_ADDSTRING,0,(long)"8192 x 4608");

	SendMessage(m_hCbAnisotropy,CB_ADDSTRING,0,(long)"No AF");
	SendMessage(m_hCbAnisotropy,CB_ADDSTRING,0,(long)"2x AF");
	SendMessage(m_hCbAnisotropy,CB_ADDSTRING,0,(long)"4x AF");
	SendMessage(m_hCbAnisotropy,CB_ADDSTRING,0,(long)"8x AF");
	SendMessage(m_hCbAnisotropy,CB_ADDSTRING,0,(long)"16x AF");

	SendMessage(m_hCbFullScreen,CB_ADDSTRING,0,(long)"Windowed Borderless");
	SendMessage(m_hCbFullScreen,CB_ADDSTRING,0,(long)"Full Screen");

	SendMessage(m_hTrgSubsEnabled,CB_ADDSTRING,0,(long)"Disabled");
	SendMessage(m_hTrgSubsEnabled,CB_ADDSTRING,0,(long)"English");
	SendMessage(m_hTrgSubsEnabled,CB_ADDSTRING,0,(long)"Spanish");

	SendMessage(m_hTrgInvertX,CB_ADDSTRING,0,(long)"No");
	SendMessage(m_hTrgInvertX,CB_ADDSTRING,0,(long)"Yes");

	SendMessage(m_hTrgInvertY,CB_ADDSTRING,0,(long)"No");
	SendMessage(m_hTrgInvertY,CB_ADDSTRING,0,(long)"Yes");

	SendMessage(m_hTrgLevel,CB_ADDSTRING,0,(long)"1");
	SendMessage(m_hTrgLevel,CB_ADDSTRING,0,(long)"2");
	SendMessage(m_hTrgLevel,CB_ADDSTRING,0,(long)"3");


	SendMessage(m_hCbDebug,CB_ADDSTRING,0,(long)"Off");
	SendMessage(m_hCbDebug,CB_ADDSTRING,0,(long)"On (Press TAB)");

	//load settings from file
	LoadSettings();
}//InitWindow

/************************************************************************/
/* Name:		InitD3D													*/
/* Description:	create Direct3D object and device						*/
/************************************************************************/

void CApplication::InitD3D(void)
{
	if (m_hWindowLauncher) DestroyWindow(m_hWindowLauncher);
	SetD3DStatus(true);

	WNDCLASSEX WindowClass;
	RECT DesktopSize;

	InitCommonControls();

	//Get desktop resolution
	GetClientRect(GetDesktopWindow(),&DesktopSize);

	//fill window class and register class
	WindowClass.cbSize = sizeof(WNDCLASSEX);
	WindowClass.style = CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc = WindowProcedure;
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = GetModuleHandle(NULL);
	WindowClass.hIcon = LoadIcon(WindowClass.hInstance,MAKEINTRESOURCE(IDI_ICON));
	WindowClass.hIconSm = LoadIcon(WindowClass.hInstance,MAKEINTRESOURCE(IDI_ICON));
	WindowClass.hCursor = NULL;
	WindowClass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	WindowClass.lpszMenuName = NULL;
	WindowClass.lpszClassName = "Game";

	RegisterClassEx(&WindowClass);

	//create window
	RECT r;
	r.top = 0;
	r.left = 0;
	r.right = m_dwWidth;
	r.bottom = m_dwHeight;
	::AdjustWindowRect(&r,WS_POPUP|WS_VISIBLE,false);

	int w = r.right - r.left;
	int h = r.bottom - r.top;
	m_hWindowGame = CreateWindow("Game",TITLE,WS_POPUP|WS_VISIBLE,0,0,w,h,(HWND) NULL,(HMENU) NULL,GetModuleHandle(NULL),(LPVOID) NULL);
	
	//create Direct3D object
	if((m_pDirect3DObject = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
	{
		MessageBox(m_hWindowGame,"Direct3DCreate9() failed!","InitD3D()",MB_OK);
		m_bRunningD3D = false;
	}

	ZeroMemory(&m_PresentParameters,sizeof(m_PresentParameters));
	m_PresentParameters.Windowed = !m_dwFullScreen;
	m_PresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_PresentParameters.EnableAutoDepthStencil = true;
	m_PresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	m_PresentParameters.AutoDepthStencilFormat = m_DepthStencilFormat;
	m_PresentParameters.hDeviceWindow = m_hWindowGame;
	m_PresentParameters.BackBufferWidth = m_dwWidth;
	m_PresentParameters.BackBufferHeight = m_dwHeight;
	m_PresentParameters.BackBufferFormat = m_ColorFormat;
	m_PresentParameters.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	m_PresentParameters.MultiSampleType = D3DMULTISAMPLE_NONE;

	if(FAILED(m_pDirect3DObject->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,m_hWindowGame,m_dwVertexProcessing,&m_PresentParameters,&m_pDirect3DDevice)))
	{
		MessageBox(m_hWindowGame,"CreateDevice() failed!","InitD3D()",MB_OK);
		m_bRunningD3D = false;
		return;
	}

	ShowCursor(false);

	InitFonts();
	InitScene();
	CheckDeviceCaps();
}//InitD3D


void CApplication::ChangeDisplay()
{
	HRESULT hr;
	if(!m_dwFullScreen)
	{
		m_dwFullScreen=true;
	}
	else
	{
		m_dwFullScreen=false;
	}
	m_PresentParameters.Windowed=!m_dwFullScreen;
	hr=m_pDirect3DDevice->Reset(&m_PresentParameters);
	if(FAILED(hr))
	{if(hr==D3DERR_DEVICELOST)
	{MessageBox(0,"D3DERR_DEVICELOST:device cannot be reset","DirectX ERROR",MB_OK);
	}
	else if(hr==D3DERR_DEVICENOTRESET)
	{MessageBox(0,"D3DERR_DEVICENOTRESET","DirectX ERROR",MB_OK);
	}
	else if(hr==D3DERR_DRIVERINTERNALERROR)
	{MessageBox(0,"D3DERR_INTERNALDRIVERERROR","DirectX ERROR",MB_OK);
	}
	}
}
/************************************************************************/
/* Name:		InitScene												*/
/* Description:	set projection matrix, render states and texture stage	*/
/*				states													*/
/************************************************************************/

void CApplication::InitScene(void)
{
	//m_pDirect3DDevice->SetRenderState(D3DRS_AMBIENT,RGB(255,255,255));
	m_pDirect3DDevice->SetRenderState(D3DRS_LIGHTING,false);
	m_pDirect3DDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);
	m_pDirect3DDevice->SetRenderState(D3DRS_ZENABLE,D3DZB_TRUE);
	m_pDirect3DDevice->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID);
	m_pDirect3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_pDirect3DDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	m_pDirect3DDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
	m_pDirect3DDevice->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);
	m_pDirect3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE); 

	for(unsigned i = 0;i < 8;++i)
	{
		m_pDirect3DDevice->SetSamplerState(i,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
		m_pDirect3DDevice->SetSamplerState(i,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
		m_pDirect3DDevice->SetSamplerState(i,D3DSAMP_MIPFILTER,D3DTEXF_ANISOTROPIC);
		m_pDirect3DDevice->SetSamplerState(i,D3DSAMP_MAXANISOTROPY,m_dwAnisotropy);
	}

	m_pDirect3DDevice->SetFVF(FVF_FLAGS);

	GameStateManager::get().setGameState(GameStateManager::FIRSTLOAD_SCREEN);
}//InitScene

/************************************************************************/
/* Name:		CheckDeviceCaps											*/
/* Description:	check for necessary device caps							*/
/************************************************************************/

void CApplication::CheckDeviceCaps(void)
{
}//CheckDeviceCaps

/************************************************************************/
/* Name:		LoadSettings											*/
/* Description:	load settings and fill combo boxes						*/
/************************************************************************/

void CApplication::LoadSettings(void)
{
	ifstream File("settings.cfg");
	int iColorFormat,iDepthStencilFormat;

	//load settings from file
	File >> m_dwWidth;
	File >> m_dwHeight;
	File >> iColorFormat;
	File >> iDepthStencilFormat;
	File >> m_dwVertexProcessing;
	//File >> iMultiSampling;
	File >> m_dwAnisotropy;
	File >> m_dwFullScreen;
	File >> m_subtitles;
	File >> m_invertX;
	File >> m_invertY;
	File >> m_level;
	File >> m_debug;

	m_ColorFormat = D3DFMT_A8R8G8B8;
	m_DepthStencilFormat = D3DFMT_D24S8;

	//select loaded settings in combo boxes
	switch(m_dwHeight)
	{
	case 480: SendMessage(m_hCbResolution,CB_SETCURSEL,0,0); break;
	case 600: SendMessage(m_hCbResolution,CB_SETCURSEL,1,0); break;
	case 720: SendMessage(m_hCbResolution,CB_SETCURSEL,4,0); break;
	case 768: 
		if (m_dwWidth == 1024) SendMessage(m_hCbResolution,CB_SETCURSEL,2,0); 
		else if (m_dwWidth == 1366) SendMessage(m_hCbResolution,CB_SETCURSEL,7,0); 
		break;
	case 864: SendMessage(m_hCbResolution,CB_SETCURSEL,3,0); break;
	case 900: 
		if (m_dwWidth == 1440) SendMessage(m_hCbResolution,CB_SETCURSEL,8,0);
		else if (m_dwWidth == 1600) SendMessage(m_hCbResolution,CB_SETCURSEL,10,0);
		break;
	case 960: SendMessage(m_hCbResolution,CB_SETCURSEL,5,0); break;
	case 1024: SendMessage(m_hCbResolution,CB_SETCURSEL,6,0); break;
	case 1200:
		if (m_dwWidth == 1600) SendMessage(m_hCbResolution,CB_SETCURSEL,11,0);
		else if (m_dwWidth == 1920) SendMessage(m_hCbResolution,CB_SETCURSEL,14,0);
		break;
	case 1050: SendMessage(m_hCbResolution,CB_SETCURSEL,12,0); break;
	case 1080: 
		if (m_dwWidth == 1440) SendMessage(m_hCbResolution,CB_SETCURSEL,9,0);
		else if (m_dwWidth == 1920) SendMessage(m_hCbResolution,CB_SETCURSEL,13,0);
		break;
	case 1152: SendMessage(m_hCbResolution,CB_SETCURSEL,15,0); break;
	case 2304: SendMessage(m_hCbResolution,CB_SETCURSEL,16,0); break;
	case 4320: SendMessage(m_hCbResolution,CB_SETCURSEL,17,0); break;
	case 4608: SendMessage(m_hCbResolution,CB_SETCURSEL,18,0); break;
	}

	switch(m_dwAnisotropy)
	{
	case 1: SendMessage(m_hCbAnisotropy,CB_SETCURSEL,0,0); break;
	case 2: SendMessage(m_hCbAnisotropy,CB_SETCURSEL,1,0); break;
	case 4: SendMessage(m_hCbAnisotropy,CB_SETCURSEL,2,0); break;
	case 8: SendMessage(m_hCbAnisotropy,CB_SETCURSEL,3,0); break;
	case 16: SendMessage(m_hCbAnisotropy,CB_SETCURSEL,4,0); break;
	}

	switch (m_dwFullScreen)
	{
	case 0: SendMessage(m_hCbFullScreen,CB_SETCURSEL,0,0); break;
	case 1: SendMessage(m_hCbFullScreen,CB_SETCURSEL,1,0); break;
	}
	
	switch (m_subtitles)
	{
	case 0: SendMessage(m_hTrgSubsEnabled,CB_SETCURSEL,0,0); break;
	case 1: SendMessage(m_hTrgSubsEnabled,CB_SETCURSEL,1,0); break;
	case 2: SendMessage(m_hTrgSubsEnabled,CB_SETCURSEL,2,0); break;
	}

	switch (m_invertX)
	{
	case 0: SendMessage(m_hTrgInvertX,CB_SETCURSEL,0,0); break;
	case 1: SendMessage(m_hTrgInvertX,CB_SETCURSEL,1,0); break;
	}

	switch (m_invertY)
	{
	case 0: SendMessage(m_hTrgInvertY,CB_SETCURSEL,0,0); break;
	case 1: SendMessage(m_hTrgInvertY,CB_SETCURSEL,1,0); break;
	}

	switch (m_level)
	{
	case 0: SendMessage(m_hTrgLevel,CB_SETCURSEL,0,0); break;
	case 1: SendMessage(m_hTrgLevel,CB_SETCURSEL,1,0); break;
	case 2: SendMessage(m_hTrgLevel,CB_SETCURSEL,2,0); break;
	}

	switch (m_debug)
	{
	case 0: SendMessage(m_hCbDebug,CB_SETCURSEL,0,0); break;
	case 1: SendMessage(m_hCbDebug,CB_SETCURSEL,1,0); break;
	}

	File.close();
}//LoadSettings

/************************************************************************/
/* Name:		SaveSettings											*/
/* Description:	retrieve settings from window and save					*/
/************************************************************************/

void CApplication::SaveSettings(void)
{
	ofstream File("settings.cfg",ios::trunc);

	//get selected settings from combo boxes
	switch(SendMessage(m_hCbResolution,CB_GETCURSEL,0,0))
	{
	case 0: m_dwWidth = 640; m_dwHeight = 480; break;
	case 1: m_dwWidth = 800; m_dwHeight = 600; break;
	case 2: m_dwWidth = 1024; m_dwHeight = 768; break;
	case 3: m_dwWidth = 1152; m_dwHeight = 864; break;
	case 4: m_dwWidth = 1280; m_dwHeight = 720; break;
	case 5: m_dwWidth = 1280; m_dwHeight = 960; break;
	case 6: m_dwWidth = 1280; m_dwHeight = 1024; break;
	case 7: m_dwWidth = 1366; m_dwHeight = 768; break;
	case 8: m_dwWidth = 1440; m_dwHeight = 900; break;
	case 9: m_dwWidth = 1440; m_dwHeight = 1080; break;
	case 10: m_dwWidth = 1600; m_dwHeight = 900; break;
	case 11: m_dwWidth = 1600; m_dwHeight = 1200; break;
	case 12: m_dwWidth = 1680; m_dwHeight = 1050; break;
	case 13: m_dwWidth = 1920; m_dwHeight = 1080; break;
	case 14: m_dwWidth = 1920; m_dwHeight = 1200; break;
	case 15: m_dwWidth = 2048; m_dwHeight = 1152; break;
	case 16: m_dwWidth = 4096; m_dwHeight = 2304; break;
	case 17: m_dwWidth = 7680; m_dwHeight = 4320; break;
	case 18: m_dwWidth = 8192; m_dwHeight = 4608; break;

	}
		
	m_ColorFormat = D3DFMT_A8R8G8B8;
	m_DepthStencilFormat = D3DFMT_D24S8;
	m_dwVertexProcessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;

	switch(SendMessage(m_hCbAnisotropy,CB_GETCURSEL,0,0))
	{
	case 0: m_dwAnisotropy = 1; break;
	case 1: m_dwAnisotropy = 2; break;
	case 2: m_dwAnisotropy = 4; break;
	case 3: m_dwAnisotropy = 8; break;
	case 4: m_dwAnisotropy = 16; break;
	}

	switch (SendMessage(m_hCbFullScreen,CB_GETCURSEL,0,0))
	{
	case 0: m_dwFullScreen = 0; break;
	case 1: m_dwFullScreen = 1; break;
	}

	switch (SendMessage(m_hTrgSubsEnabled,CB_GETCURSEL,0,0))
	{
	case 0: m_subtitles = 0; break;
	case 1: m_subtitles = 1; break;
	case 2: m_subtitles = 2; break;
	}

	switch (SendMessage(m_hTrgInvertX,CB_GETCURSEL,0,0))
	{
	case 0: m_invertX = 0; break;
	case 1: m_invertX = 1; break;
	}

	switch (SendMessage(m_hTrgInvertY,CB_GETCURSEL,0,0))
	{
	case 0: m_invertY = 0; break;
	case 1: m_invertY = 1; break;
	}

	switch (SendMessage(m_hTrgLevel,CB_GETCURSEL,0,0))
	{
	case 0: m_level = 0; break;
	case 1: m_level = 1; break;
	case 2: m_level = 2; break;
	}

	switch (SendMessage(m_hCbDebug,CB_GETCURSEL,0,0))
	{
	case 0: m_debug = 0; break;
	case 1: m_debug = 1; break;
	}

	//save settings to file
	File << m_dwWidth << endl;
	File << m_dwHeight << endl;
	File << m_ColorFormat << endl;
	File << m_DepthStencilFormat << endl;
	File << m_dwVertexProcessing << endl;
	File << m_dwAnisotropy << endl;
	File << m_dwFullScreen << endl;
	File << m_subtitles << endl;
	File << m_invertX << endl;
	File << m_invertY << endl;
	File << m_level << endl;
	File << m_debug << endl;
	File.close();
}//SaveSettings

/************************************************************************/
/* Name:		CheckDevice												*/
/* Description:	check for lost device									*/
/************************************************************************/
#include "world.h"
bool CApplication::CheckDevice(void)
{
	switch(m_pDirect3DDevice->TestCooperativeLevel())
	{
	case D3DERR_DEVICELOST: return false;
	case D3DERR_DEVICENOTRESET:
		{
			if(FAILED(m_pDirect3DDevice->Reset(&m_PresentParameters)))
			{
				//MessageBox(m_hWindowGame,"Reset() failed!","CheckDevice()",MB_OK);
				KillD3D();
				KillWindow();
				g_App.SetD3DStatus(false);
				PostQuitMessage(0);
				return false;
			}

			InitScene();

			return true;
		}
	default: return true;
	}
}//CheckDevice

/************************************************************************/
/* Name:		KillWindow												*/
/* Description:	unregister window class									*/
/************************************************************************/

void CApplication::KillWindow(void)
{
	UnregisterClass("Launcher",GetModuleHandle(NULL));
}//KillWindow

/************************************************************************/
/* Name:		KillD3D													*/
/* Description:	release memory for Direct3D device and Direct3D object	*/
/************************************************************************/

void CApplication::KillD3D(void)
{
	DestroyFonts();

	if(m_pDirect3DDevice != NULL)
	{
		DWORD remaining_refs = m_pDirect3DDevice->Release();
		//		assert( remaining_refs == 0 ); //TODO: liberar bien las cosas.
		m_pDirect3DDevice = NULL;
	}

	if(m_pDirect3DObject != NULL)
	{
		m_pDirect3DObject->Release();
		m_pDirect3DObject = NULL;
	}
	//if (m_pDirect3DSwapChain != NULL)
	//{
	//	m_pDirect3DSwapChain->Release();
	//	m_pDirect3DSwapChain = NULL;
	//}
	UnregisterClass("Game",GetModuleHandle(NULL));

}//KillD3D

#include "iostatus.h"
#include "system_sound.h"
void CApplication::update( float delta ) {

	CIOStatus::instance()->update(delta);
	SoundSystem::get().update(delta);
	GameStateManager::get().update(delta);
}

void CApplication::render()
{
    // Clear the backbuffer to a black color
	if (!m_pDirect3DDevice) return;
   m_pDirect3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 0x00000000, 1.0f, 0 );

    // Begin the scene
    if( SUCCEEDED( m_pDirect3DDevice->BeginScene()))
	{
		GameStateManager::get().render();

		m_pDirect3DDevice->EndScene();
	}
    
    // Present the backbuffer contents to the display
    m_pDirect3DDevice->Present( NULL, NULL, NULL, NULL );
}

void CApplication::onKeyDown(WPARAM key)
{
	GameStateManager::get().onKeyDown(key);
}

bool CApplication::LoadShader(const char *filename ) {

	if( effect )
		effect->Release();

	// Mientras no consigamos compilar sin errores...
	while( true ) {
		LPD3DXBUFFER err_buffer;
		HRESULT hr = D3DXCreateEffectFromFile( 
						m_pDirect3DDevice
					  , filename
					  , NULL			// no macros
					  , NULL			// no includes
					  , 0				// no flags
					  , NULL			// no parameters shared
					  , &effect		// result received here
					  , &err_buffer			
					  );
		if( hr == D3D_OK )
			break;
		const char *buffer_text = (const char *) err_buffer->GetBufferPointer();
		dbg( "Error compiling FX %s: %s\n", filename, buffer_text );
		MessageBox( NULL, buffer_text, "Error compiling FX", MB_OK );
		err_buffer->Release();
	}
	return true;
}

void CApplication::DestroyShaders( ) {
	if( effect )
		effect->Release();
}
