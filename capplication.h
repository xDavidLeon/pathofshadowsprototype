#ifndef capplication_h
#define capplication_h

#define IDI_ICON 101
class GameStateManager;

class CApplication
{
public:
								CApplication(void);
								~CApplication(void);

	GameStateManager*			gameStateManager;
	
	void						render();
	void						update( float delta_time );

	void						InitFonts(void);
	void						DestroyFonts(void);
	void						InitWindow(void);
	bool						LoadShader( const char *filename);
	void						DestroyShaders(void);
	//void						InitD3DWindow(void);

	void						InitD3D(void);
	void						InitScene(void);
	void						CheckDeviceCaps(void);
	void						SaveSettings(void);
	void						LoadSettings(void);

	bool						CheckDevice(void);

	void						KillWindow(void);
	void						KillD3D(void);
	void						ChangeDisplay(void);

	LPD3DXEFFECT				effect;

	inline bool					GetLauncherWindowStatus(void)						{ return m_bRunningLauncherWindow; }
	//inline bool					GetGameWindowStatus(void)						{ return m_bRunningGameWindow; }

	inline bool					GetD3DStatus(void)							{ return m_bRunningD3D; }
	inline LPDIRECT3DDEVICE9	GetDevice(void)								{ return m_pDirect3DDevice; }
	inline HWND					GetLauncherWindowHandle(void)				{ return m_hWindowLauncher; }
	inline HWND					GetGameWindowHandle(void)					{ return m_hWindowGame; }
	inline DWORD				GetWidth(void)								{ return m_dwWidth; }
	inline DWORD				GetHeight(void)								{ return m_dwHeight; }
	inline LPD3DXFONT					GetDebugFont(void)							{ return m_dbgFont; }
	inline LPD3DXFONT					GetDebugFontBig(void)							{ return m_dbgFont_big; }
	inline DWORD				GetLanguage(void)									{ return m_subtitles; }
	inline DWORD				GetDebug(void)									{ return m_debug; }

	inline D3DPRESENT_PARAMETERS GetParameters(void)						{ return m_PresentParameters; }

	inline void					SetLauncherWindowStatus(bool bRunningWindow)		{ m_bRunningLauncherWindow = bRunningWindow; }
	//inline void					SetGameWindowStatus(bool bRunningWindow)		{ m_bRunningGameWindow = bRunningWindow; }

	inline void					SetD3DStatus(bool bRunningD3D)				{ m_bRunningD3D = bRunningD3D; }

	void						onKeyDown(WPARAM key);

	inline bool						isUsingSubtitles(void) { return m_subtitles > 0; };
private:

	bool						m_bRunningLauncherWindow,
								m_bRunningGameWindow,
								m_bRunningD3D;

	HWND						m_hWindowLauncher,
								m_hWindowGame,
								m_hBtnStart,
								m_hBtnCancel,
								m_hLblFullScreen,
								m_hCbFullScreen,
								m_hLblResolution,
								m_hCbResolution,
								m_hLbSubsEnabled,
								m_hTrgSubsEnabled,
								m_hLblDebug,
								m_hCbDebug,
								m_hLblAnisotropy,
								m_hCbAnisotropy;

	LPDIRECT3D9					m_pDirect3DObject;
	LPDIRECT3DDEVICE9			m_pDirect3DDevice;
	//LPDIRECT3DSWAPCHAIN9		m_pDirect3DSwapChain;
	D3DPRESENT_PARAMETERS		m_PresentParameters;
	D3DCAPS9					m_DeviceCaps;

	DWORD						m_dwFullScreen,
								m_dwWidth,
								m_dwHeight,
								m_dwVertexProcessing,
								m_dwAnisotropy;
	D3DFORMAT					m_ColorFormat,
								m_DepthStencilFormat;

	LPD3DXFONT					m_dbgFont;
	LPD3DXFONT					m_dbgFont_big;
	DWORD						m_subtitles;
	DWORD						m_debug;

};

#endif
