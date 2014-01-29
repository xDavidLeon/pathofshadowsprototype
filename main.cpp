#include"main.h"

//globals
CApplication	g_App;

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPreviousInstance,LPSTR lpcmdline,int nCmdShow)
{
	/*if (SetThreadAffinityMask(GetCurrentThread(),1) < 0)
		std::printf("Error: SetThreadAffinityMask returned an error.\n");*/

	MSG Message;

	//window loop
	while(g_App.GetLauncherWindowStatus())
	{
		if(GetMessage(&Message,NULL,0,0))
		{
			if(!IsDialogMessage(g_App.GetLauncherWindowHandle(),&Message))
			{
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}
		}
	}

	//switch to D3D loop or quit
	if(!g_App.GetD3DStatus())
		return 0;

	const double FPS = 60.0f;
	const double TicksPerFrame = 1000.0f / FPS;
	double time1 = GetTickCount();
	double time2;
	double rest = 0;
	const int MAX_LOOPS = 50;

	//D3D loop
	while(g_App.GetD3DStatus())
	{
		if(PeekMessage(&Message,NULL,0,0,PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		else if(g_App.CheckDevice())
		{
			time2 = GetTickCount();
			double dt = time2 - time1;
			time1 = time2;

			if (dt > 0.0f)
			{
				double num_updates = (dt / TicksPerFrame) + rest;
				int temp = floor(num_updates);
				rest = num_updates - temp;

				for (int i = 0; i < temp && i < MAX_LOOPS; i++)
				{
					g_App.update(1.0f/FPS);
				}
			}

			g_App.render();
		}
	}

	g_App.KillD3D();

	return 0;
}//WinMain
