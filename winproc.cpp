#include"main.h"
#include "iostatus.h"
#include "globals.h"
#include "world.h"
#include "console_lua.h"

/************************************************************************/
/* Name:		WindowProcedure											*/
/* Description:	handles windows messages								*/
/************************************************************************/

LRESULT CALLBACK WindowProcedure(HWND hWindow,UINT uMessage,WPARAM wparam,LPARAM lparam)
{
	switch(uMessage)
	{
		//user command on window
	case WM_COMMAND:
		{
			switch(LOWORD(wparam))
			{
				//start button pressed
			case ID_START:
				{
					//switch to D3D loop and init D3D with settings

					g_App.SaveSettings();
					g_App.SetLauncherWindowStatus(false);
					g_App.SetD3DStatus(true);
					SetFocus(hWindow);
					g_App.InitD3D();
					break;
				}
				//cancel button pressed
			case ID_CANCEL:
				{
					DestroyWindow(hWindow);
					break;
				}
			}
			return 0;
		}
	case WM_KEYDOWN:
		{
			switch(wparam)
			{
			case VK_ESCAPE:	
				{
					DestroyWindow(hWindow);
					break;
				}
			}
			return 0;
		}
	case WM_DESTROY:
		{
			//PostQuitMessage ( WM_QUIT ) ;
			g_App.SetLauncherWindowStatus(false);
			g_App.SetD3DStatus(false);
			
			break;
		}
	case WM_LBUTTONDOWN:
		CIOStatus::instance()->current_mouse_left = true;
		break;
	case WM_LBUTTONUP:			// released left button
		CIOStatus::instance()->current_mouse_left = false;
		break;
	case WM_RBUTTONDOWN:		// press right
		CIOStatus::instance()->current_mouse_right = true;
		break;
	case WM_RBUTTONUP:			// released right button
		CIOStatus::instance()->current_mouse_right = false;
		break;
	case WM_MOUSEWHEEL: {
		//DWORD fwKeys = GET_KEYSTATE_WPARAM(wParam);
		DWORD zDelta = GET_WHEEL_DELTA_WPARAM(wparam);
		dbg( "current_mouse_wheel = %d\n", zDelta);
		CIOStatus::instance()->current_mouse_wheel = zDelta;
		break;}
	case WM_KILLFOCUS:
		CIOStatus::instance()->releaseMouse();
		break;

	case WM_SETFOCUS:
		CIOStatus::instance()->adquireMouse();
		break;
	case WM_CHAR:
		g_App.onKeyDown(wparam);
		ConsoleLua::get().setCurrentKey(wparam);
		break;
	case WM_KEYUP:
		if(wparam == 38 || wparam == 40) ConsoleLua::get().setCurrentKey(wparam);
		break;
	}

	//Para identificar teclas y su tipo
	//if(wparam>10 && wparam<510)
	//	int kk=23;

	return DefWindowProc(hWindow,uMessage,wparam,lparam);
}//WindowProcedure
