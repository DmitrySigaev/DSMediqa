// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Osmo4.h"

#include "MainFrm.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_INITMENUPOPUP()
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_MESSAGE(WM_SETSIZE,OnSetSize)
	ON_MESSAGE(WM_NAVIGATE,OnNavigate)
	ON_MESSAGE(WM_OPENURL, Open)
	ON_MESSAGE(WM_SETTIMING, SetTiming)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_CHAR()
	ON_WM_SYSKEYDOWN()
	ON_WM_SYSKEYUP()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_DROPFILES()
	ON_MESSAGE(WM_CONSOLEMSG, OnConsoleMessage)
	ON_COMMAND(ID_VIEW_ORIGINAL, OnViewOriginal)
	ON_COMMAND(ID_VIEW_FULLSCREEN, OnViewFullscreen)
	ON_COMMAND(ID_AR_KEEP, OnArKeep)
	ON_COMMAND(ID_AR_FILL, OnArFill)
	ON_COMMAND(ID_AR_43, OnAr43)
	ON_COMMAND(ID_AR_169, OnAr169)
	ON_UPDATE_COMMAND_UI(ID_AR_169, OnUpdateAr169)
	ON_UPDATE_COMMAND_UI(ID_AR_43, OnUpdateAr43)
	ON_UPDATE_COMMAND_UI(ID_AR_FILL, OnUpdateArFill)
	ON_UPDATE_COMMAND_UI(ID_AR_KEEP, OnUpdateArKeep)
	ON_COMMAND(ID_NAVIGATE_NONE, OnNavigateNone)
	ON_COMMAND(ID_NAVIGATE_WALK, OnNavigateWalk)
	ON_COMMAND(ID_NAVIGATE_FLY, OnNavigateFly)
	ON_COMMAND(ID_NAVIGATE_EXAM, OnNavigateExam)
	ON_COMMAND(ID_NAVIGATE_SLIDE, OnNavigateSlide)
	ON_COMMAND(ID_NAVIGATE_PAN, OnNavigatePan)
	ON_COMMAND(ID_NAVIGATE_ORBIT, OnNavigateOrbit)
	ON_COMMAND(ID_NAVIGATE_GAME, OnNavigateGame)
	ON_COMMAND(ID_NAV_RESET, OnNavigateReset)
	ON_COMMAND(ID_SHORTCUTS, OnShortcuts)
	ON_COMMAND(IDD_CONFIGURE, OnConfigure)
	ON_COMMAND(ID_FILE_PROP, OnFileProp)
	ON_COMMAND(ID_VIEW_PL, OnViewPlaylist)
	ON_UPDATE_COMMAND_UI(ID_FILE_PROP, OnUpdateFileProp)
	ON_UPDATE_COMMAND_UI(ID_NAVIGATE_NONE, OnUpdateNavigate)
	ON_COMMAND(ID_COLLIDE_DISP, OnCollideDisp)
	ON_UPDATE_COMMAND_UI(ID_COLLIDE_DISP, OnUpdateCollideDisp)
	ON_COMMAND(ID_COLLIDE_NONE, OnCollideNone)
	ON_UPDATE_COMMAND_UI(ID_COLLIDE_NONE, OnUpdateCollideNone)
	ON_COMMAND(ID_COLLIDE_REG, OnCollideReg)
	ON_UPDATE_COMMAND_UI(ID_COLLIDE_REG, OnUpdateCollideReg)
	ON_COMMAND(ID_HEADLIGHT, OnHeadlight)
	ON_UPDATE_COMMAND_UI(ID_HEADLIGHT, OnUpdateHeadlight)
	ON_COMMAND(ID_GRAVITY, OnGravity)
	ON_UPDATE_COMMAND_UI(ID_GRAVITY, OnUpdateGravity)
	ON_COMMAND(ID_NAV_INFO, OnNavInfo)
	ON_COMMAND(ID_NAV_NEXT, OnNavNext)
	ON_COMMAND(ID_NAV_PREV, OnNavPrev)
	ON_UPDATE_COMMAND_UI(ID_NAV_NEXT, OnUpdateNavNext)
	ON_UPDATE_COMMAND_UI(ID_NAV_PREV, OnUpdateNavPrev)
	ON_COMMAND(ID_CLEAR_NAV, OnClearNav)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PL, OnUpdateViewPlaylist)
	ON_COMMAND(ID_PLAY_LOOP, OnPlayLoop)
	ON_UPDATE_COMMAND_UI(ID_PLAY_LOOP, OnUpdatePlayLoop)
	ON_UPDATE_COMMAND_UI(ID_NAVIGATE_WALK, OnUpdateNavigate)
	ON_UPDATE_COMMAND_UI(ID_NAVIGATE_FLY, OnUpdateNavigate)
	ON_UPDATE_COMMAND_UI(ID_NAVIGATE_EXAM, OnUpdateNavigate)
	ON_UPDATE_COMMAND_UI(ID_NAVIGATE_PAN, OnUpdateNavigate)
	ON_UPDATE_COMMAND_UI(ID_NAVIGATE_SLIDE, OnUpdateNavigate)
	ON_UPDATE_COMMAND_UI(ID_NAVIGATE_ORBIT, OnUpdateNavigate)
	ON_UPDATE_COMMAND_UI(ID_NAVIGATE_GAME, OnUpdateNavigate)
	ON_COMMAND(ID_ADD_SUBTITLE, OnAddSubtitle)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_icoerror = AfxGetApp()->LoadIcon(IDI_ERR);
	m_icomessage = AfxGetApp()->LoadIcon(IDI_MESSAGE);
	m_bFullScreen = 0;	
	m_aspect_ratio = M4_AR_Keep;
	m_pProps = NULL;
	m_pOpt = NULL;
	m_pPlayList = NULL;
	m_pWndView = new CChildView();
	m_bInitShow = TRUE;
}

CMainFrame::~CMainFrame()
{
	if (m_pProps != NULL) m_pProps->DestroyWindow();
	if (m_pOpt != NULL) m_pOpt->DestroyWindow();
	if (m_pPlayList != NULL) delete m_pPlayList;
	delete m_pWndView;
}


static UINT indicators[] =
{
	ID_TIMER,
	ID_FPS,
	ID_SEPARATOR,           // status line indicator
};



int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	UINT buttonArray[50];
	TBBUTTONINFO bi;
	u32 *ba;
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// create a view to occupy the client area of the frame
	if (!m_pWndView->CreateEx(0, NULL, NULL, WS_POPUP , 0, 0, 300, 200, m_hWnd, NULL, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	if (!m_wndToolBar.CreateEx(this, WS_CHILD | CBRS_TOP | CBRS_FLYBY) ||
		!m_wndToolBar.LoadBitmap(IDR_MAINTOOLS))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	ba = &buttonArray[0];
	*ba = ID_FILEOPEN; ba++;
	*ba = ID_SEPARATOR; ba++;
	*ba = ID_NAV_PREV; ba++;
	*ba = ID_NAV_NEXT; ba++;
	*ba = ID_SEPARATOR; ba++;
	*ba = ID_FILE_PLAY; ba++;
	*ba = ID_FILE_STEP; ba++;
	*ba = ID_FILE_STOP; ba++;
	*ba = ID_SEPARATOR; ba++;
	*ba = ID_FILE_PROP; ba++;
	*ba = ID_SEPARATOR; ba++;
	*ba = ID_FILE_PROP; ba++;
	*ba = ID_SWITCH_RENDER;
	m_wndToolBar.SetButtons(buttonArray, 13);
	m_wndToolBar.SetButtonInfo(0, ID_FILEOPEN, TBBS_BUTTON, 0);
	m_wndToolBar.SetButtonInfo(1, ID_SEPARATOR, TBBS_SEPARATOR, 0);
	m_wndToolBar.SetButtonInfo(2, ID_NAV_PREV, TBBS_DROPDOWN, 1);
	m_wndToolBar.SetButtonInfo(3, ID_NAV_NEXT, TBBS_DROPDOWN, 2);
	m_wndToolBar.SetButtonInfo(4, ID_SEPARATOR, TBBS_SEPARATOR, 0);
	m_wndToolBar.SetButtonInfo(5, ID_FILE_PLAY, TBBS_BUTTON, 3);
	m_wndToolBar.SetButtonInfo(6, ID_FILE_STEP, TBBS_BUTTON, 5);
	m_wndToolBar.SetButtonInfo(7, ID_FILE_STOP, TBBS_BUTTON, 6);
	m_wndToolBar.SetButtonInfo(8, ID_SEPARATOR, TBBS_SEPARATOR, 0);
	m_wndToolBar.SetButtonInfo(9, ID_FILE_PROP, TBBS_BUTTON, 7);
	m_wndToolBar.SetButtonInfo(10, ID_SEPARATOR, TBBS_SEPARATOR, 0);
	m_wndToolBar.SetButtonInfo(11, IDD_CONFIGURE, TBBS_BUTTON, 8);
	m_wndToolBar.SetButtonInfo(12, ID_SWITCH_RENDER, TBBS_BUTTON, 9);

	CToolBarCtrl &ctrl = m_wndToolBar.GetToolBarCtrl();
	ctrl.SetStyle(TBSTYLE_FLAT | TBSTYLE_DROPDOWN);
	ctrl.SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);

	memset(&bi, 0, sizeof(bi));
	bi.cbSize = sizeof(bi);
	ctrl.GetButtonInfo(2, &bi);
	bi.fsStyle |= TBSTYLE_DROPDOWN;
	ctrl.SetButtonInfo(ID_NAV_PREV, &bi);

	memset(&bi, 0, sizeof(bi));
	bi.cbSize = sizeof(bi);
	ctrl.GetButtonInfo(3, &bi);
	bi.fsStyle |= TBSTYLE_DROPDOWN;
	ctrl.SetButtonInfo(ID_NAV_NEXT, &bi);

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	if (!m_Address.Create(this, IDD_NAVBAR, WS_CHILD | CBRS_TOP | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, IDD_NAVBAR) ) {
		return -1;      // fail to create
	}

	if (!m_Sliders.Create(IDD_SLIDERS, this) ) {
		return -1;      // fail to create
	}

	m_pPlayList = new Playlist();
	m_pPlayList->Create();
	m_pPlayList->ShowWindow(SW_HIDE);

	m_wndStatusBar.SetPaneInfo(0, ID_TIMER, SBPS_NORMAL, 45);
	m_wndStatusBar.SetPaneInfo(1, ID_FPS, SBPS_NORMAL, 45);
	m_wndStatusBar.SetPaneInfo(2, ID_SEPARATOR, SBPS_STRETCH, 0);
	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), TRUE);
	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);
	return 0;
}


BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
void CMainFrame::OnSetFocus(CWnd* pOldWnd)
{
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_pWndView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	POINT pt;
	RECT rc2;
	u32 tool_h, slide_h, add_h, stat_h;


	if (m_bInitShow) {
		CFrameWnd::OnSize(nType, cx, cy);
		return;
	}

	m_wndToolBar.GetClientRect(&rc2);
	tool_h = rc2.bottom - rc2.top;
	m_Address.GetClientRect(&rc2);
	add_h = rc2.bottom - rc2.top;
	m_Sliders.GetClientRect(&rc2);
	slide_h = rc2.bottom - rc2.top;
	m_wndStatusBar.GetClientRect(&rc2);
	stat_h = rc2.bottom - rc2.top;
	if ((u32) cy <= tool_h+add_h+slide_h+stat_h) {
		OnSetSize(cx, 1);
		return;
	}
	CFrameWnd::OnSize(nType, cx, cy);
	cy -= tool_h + add_h + slide_h + stat_h;

	m_Address.SetWindowPos(this, 0, 0, cx, add_h, SWP_SHOWWINDOW | SWP_NOMOVE);

	/*and update pos (the view window is not a child one)*/
	pt.x = 0;
	pt.y = add_h + tool_h + cy;
	ClientToScreen(&pt);
	m_Sliders.SetWindowPos(this, pt.x, pt.y, cx, slide_h, SWP_SHOWWINDOW);
	
	/*and set size through the terminal (this takes care of threading issues with opengl)*/
	M4T_SetSize(GetApp()->m_term, cx, cy);

	SetActiveWindow();
	SetFocus();
}


LONG CMainFrame::OnSetSize(WPARAM wParam, LPARAM lParam)
{
	UINT width, height;
	width = wParam;
	height = lParam;
	if (m_bInitShow) {
		m_wndToolBar.UpdateWindow();
		m_wndToolBar.ShowWindow(SW_SHOW);
		m_Address.UpdateWindow();
		m_Address.ShowWindow(SW_SHOW);
		m_Sliders.UpdateWindow();
		m_Sliders.ShowWindow(SW_SHOW);
		m_Sliders.m_PosSlider.EnableWindow(FALSE);
		m_pWndView->ShowWindow(SW_SHOW);
		ShowWindow(SW_SHOW);
		m_bInitShow = FALSE;
	}

	RECT winRect;
	winRect.left = 0;
	winRect.right = width;
	winRect.top = 0;
	winRect.bottom = height;
	AdjustWindowRectEx(&winRect, GetStyle(), TRUE, GetExStyle());
	winRect.bottom -= winRect.top;
	winRect.right -= winRect.left;

	RECT rc2;
	m_Address.GetClientRect(&rc2);
	winRect.bottom += rc2.bottom;
	m_wndToolBar.GetClientRect(&rc2);
	winRect.bottom += rc2.bottom;
	m_Sliders.GetClientRect(&rc2);
	winRect.bottom += rc2.bottom;
	m_wndStatusBar.GetClientRect(&rc2);
	winRect.bottom += rc2.bottom;

	SetWindowPos(NULL, 0, 0, winRect.right, winRect.bottom, SWP_NOZORDER | SWP_NOMOVE | SWP_SHOWWINDOW);
	return 0;
}

void CMainFrame::OnMove(int x, int y) 
{
	CFrameWnd::OnMove(x, y);
	RECT rc;
	
	m_wndToolBar.GetClientRect(&rc);
	m_wndToolBar.SetWindowPos(this, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
	y += rc.bottom - rc.top;
	m_Address.SetWindowPos(this, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
	m_Address.GetClientRect(&rc);
	y += rc.bottom - rc.top;
	m_pWndView->SetWindowPos(this, x, y, 0, 0, SWP_NOSIZE);
	m_pWndView->GetClientRect(&rc);
	y += rc.bottom;
	m_Sliders.SetWindowPos(this, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
}


#define PROGRESS_TIMER	20
#define PROGRESS_REFRESH_MS		100

void CALLBACK EXPORT ProgressTimer(HWND , UINT , UINT nID , DWORD )
{
	u32 now;
	if (nID != PROGRESS_TIMER) return;
	WinGPAC *app = GetApp();
	CMainFrame *pFrame = (CMainFrame *) app->m_pMainWnd;
	/*shutdown*/
	if (!pFrame) return;

	now = M4T_GetCurrentTimeInMS(app->m_term);
	if (!now) return;

	if (app->max_duration && !pFrame->m_Sliders.m_grabbed) {
		if (now >= app->max_duration + 100) {
			if (M4T_GetOption(app->m_term, M4O_IsOver)) {
				pFrame->m_pPlayList->PlayNext();
		}
			/*if no IsOver go on forever*/
		} else {
			if (!app->m_reset)
				pFrame->m_Sliders.m_PosSlider.SetPos(now);
		}
	}

	if (!app->m_prev_time || (app->m_prev_time + 500 <= now)) {
		app->current_FPS = M4T_GetCurrentFPS(app->m_term, 0);
		app->current_time_ms = now;
		pFrame->PostMessage(WM_SETTIMING, 0, 0);
	}

}

void CMainFrame::SetProgTimer(Bool bOn) 
{
	if (bOn) 
		SetTimer(PROGRESS_TIMER, PROGRESS_REFRESH_MS, ProgressTimer); 
	else
		KillTimer(PROGRESS_TIMER);
}

LONG CMainFrame::Open(WPARAM wParam, LPARAM lParam)
{
	WinGPAC *app = GetApp();
	CString txt, url;
	txt = "Osmo4 - ";
	txt += m_pPlayList->GetDisplayName();

	url = m_pPlayList->GetURL();
	m_Address.m_Address.SetWindowText(url);

	SetWindowText(txt);
	
	if (app->m_reconnect_time) {
		M4T_ConnectURLWithStartTime(app->m_term, (LPCSTR) url, app->m_reconnect_time);
		app->m_reconnect_time = 0;
	} else {
		M4T_ConnectURL(app->m_term, (LPCSTR) url);
	}
	return 1;	
}

void CMainFrame::ForwardMessage()
{
	const MSG *msg = GetCurrentMessage();
	m_pWndView->SendMessage(msg->message, msg->wParam, msg->lParam);
}
void CMainFrame::OnSysKeyUp(UINT , UINT , UINT ) { ForwardMessage(); }
void CMainFrame::OnSysKeyDown(UINT , UINT , UINT ) { ForwardMessage(); }
void CMainFrame::OnChar(UINT , UINT , UINT ) { ForwardMessage(); }
void CMainFrame::OnKeyDown(UINT , UINT , UINT ) { ForwardMessage(); }
void CMainFrame::OnKeyUp(UINT , UINT , UINT ) { ForwardMessage(); }
void CMainFrame::OnLButtonDown(UINT , CPoint ) { ForwardMessage(); }
void CMainFrame::OnLButtonDblClk(UINT , CPoint ) { ForwardMessage(); }
void CMainFrame::OnLButtonUp(UINT , CPoint ) { ForwardMessage(); }

void CMainFrame::OnDropFiles(HDROP hDropInfo) 
{
	u32 i, count;
	WinGPAC *app = GetApp();
	char fileName[MAX_PATH];

	count = ::DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
	if (!count) return;
	
	m_pPlayList->Clear();

	for (i=0; i<count; i++) {
		::DragQueryFile (hDropInfo, i, fileName, MAX_PATH);
		m_pPlayList->QueueURL(fileName);
	}
	m_pPlayList->PlayNext();
}

LONG CMainFrame::SetTiming(WPARAM wParam, LPARAM lParam)
{
	char sTime[75];
	WinGPAC *gpac = GetApp();
	u32 ms = gpac->current_time_ms;
	u32 h = ms / 1000 / 3600;
	u32 m = ms / 1000 / 60 - h*60;
	u32 s = ms / 1000 - h*3600 - m*60;
	
	sprintf(sTime, "%02d:%02d.%02d", h, m, s);
	m_wndStatusBar.SetPaneText(0, sTime);
	
	sprintf(sTime, "FPS %.1f", gpac->current_FPS);
	m_wndStatusBar.SetPaneText(1, sTime);
	return 1;
}



void CALLBACK EXPORT ConsoleTimer(HWND , UINT , UINT , DWORD )
{
	CMainFrame *pFrame = (CMainFrame *) GetApp()->m_pMainWnd;
	
	pFrame->m_wndStatusBar.GetStatusBarCtrl().SetIcon(2, NULL);
	pFrame->KillTimer(pFrame->m_timer_on);
	pFrame->m_timer_on = 0;
	pFrame->m_wndStatusBar.SetPaneText(2, "Ready");
}

#define CONSOLE_DISPLAY_TIME	1000

LONG CMainFrame::OnConsoleMessage(WPARAM wParam, LPARAM lParam)
{
	if (m_timer_on) KillTimer(m_timer_on);
	
	if (console_err>=0) {
		m_wndStatusBar.GetStatusBarCtrl().SetIcon(2, m_icomessage);
		m_wndStatusBar.SetPaneText(2, console_message);
	} else {
		char msg[5000];
		m_wndStatusBar.GetStatusBarCtrl().SetIcon(2, m_icoerror);
		sprintf(msg, "%s (%s)", console_message, console_service);
		m_wndStatusBar.SetPaneText(2, msg);
	}
	m_timer_on = SetTimer(10, CONSOLE_DISPLAY_TIME, ConsoleTimer);
	return 0;
}

BOOL CMainFrame::DestroyWindow() 
{
	if (GetApp()->m_isopen) KillTimer(PROGRESS_TIMER);
	/*signal close to prevent callbacks but don't close, this is done in ExitInstance (otherwise there's a
	deadlock happening not sure why yet)*/
//	GetApp()->m_open = 0;
	return CFrameWnd::DestroyWindow();
}


void CMainFrame::OnViewOriginal() 
{
	WinGPAC *gpac = GetApp();
	M4T_SetOption(gpac->m_term, M4O_OriginalView, 1);	
	OnSetSize(gpac->orig_width, gpac->orig_height);
}

void CMainFrame::SetFullscreen() 
{
	WinGPAC *gpac = GetApp();
	if (!m_bFullScreen) {
		GetWindowRect(&backup_wnd_rc);
		if (M4T_SetOption(gpac->m_term, M4O_Fullscreen, 1) == M4OK) 
			m_bFullScreen = 1;
	} else {
		if (M4T_SetOption(gpac->m_term, M4O_Fullscreen, 0) == M4OK) 
			m_bFullScreen = 0;
		SetWindowPos(NULL, backup_wnd_rc.left, backup_wnd_rc.top, backup_wnd_rc.right-backup_wnd_rc.left, backup_wnd_rc.bottom-backup_wnd_rc.top, SWP_NOZORDER);
	}
}

void CMainFrame::OnViewFullscreen() 
{
	SetFullscreen();
}

void CMainFrame::OnArKeep() 
{
	M4T_SetOption(GetApp()->m_term, M4O_AspectRatio, M4_AR_Keep);	
	m_aspect_ratio = M4_AR_Keep;
}

void CMainFrame::OnArFill() 
{
	M4T_SetOption(GetApp()->m_term, M4O_AspectRatio, M4_AR_None);	
	m_aspect_ratio = M4_AR_None;
}

void CMainFrame::OnAr43() 
{
	M4T_SetOption(GetApp()->m_term, M4O_AspectRatio, M4_AR_4_3);	
	m_aspect_ratio = M4_AR_4_3;
}

void CMainFrame::OnAr169() 
{
	M4T_SetOption(GetApp()->m_term, M4O_AspectRatio, M4_AR_16_9);	
	m_aspect_ratio = M4_AR_16_9;
}

void CMainFrame::OnUpdateAr169(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_aspect_ratio == M4_AR_16_9);
}

void CMainFrame::OnUpdateAr43(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_aspect_ratio == M4_AR_4_3);
}

void CMainFrame::OnUpdateArFill(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_aspect_ratio == M4_AR_None);
}

void CMainFrame::OnUpdateArKeep(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_aspect_ratio == M4_AR_Keep);
}

void CMainFrame::OnUpdateNavigate(CCmdUI* pCmdUI)
{
	BOOL enable;
	WinGPAC *app = GetApp();
	pCmdUI->Enable(FALSE);
	if (!app->m_isopen) return;
	
	u32 type = M4T_GetOption(app->m_term, M4O_NavigationType);
	enable = type ? TRUE : FALSE;

	if (pCmdUI->m_nID==ID_NAV_RESET) {
		pCmdUI->Enable(TRUE);
		return;
	}

	u32 mode = M4T_GetOption(app->m_term, M4O_NavigationMode);
	/*common 2D/3D modes*/
	if (pCmdUI->m_nID==ID_NAVIGATE_NONE) { pCmdUI->Enable(enable); pCmdUI->SetCheck(mode ? 0 : 1); }
	else if (pCmdUI->m_nID==ID_NAVIGATE_EXAM) { pCmdUI->Enable(enable); pCmdUI->SetCheck((mode==M4_NavExamine) ? 1 : 0); }
	else if (pCmdUI->m_nID==ID_NAVIGATE_SLIDE) { pCmdUI->Enable(enable); pCmdUI->SetCheck((mode==M4_NavSlide) ? 1 : 0); }

	if (type==M4_Navigation2D) return;
	pCmdUI->Enable(enable); 	
	if (pCmdUI->m_nID==ID_NAVIGATE_WALK) pCmdUI->SetCheck((mode==M4_NavWalk) ? 1 : 0);
	else if (pCmdUI->m_nID==ID_NAVIGATE_FLY) pCmdUI->SetCheck((mode==M4_NavFly) ? 1 : 0);
	else if (pCmdUI->m_nID==ID_NAVIGATE_PAN) pCmdUI->SetCheck((mode==M4_NavPan) ? 1 : 0);
	else if (pCmdUI->m_nID==ID_NAVIGATE_GAME) pCmdUI->SetCheck((mode==M4_NavGame) ? 1 : 0);
}


void CMainFrame::SetNavigate(u32 mode)
{
	WinGPAC *app = GetApp();
	M4T_SetOption(app->m_term, M4O_NavigationMode, mode);
}
void CMainFrame::OnNavigateNone() { SetNavigate(M4_NavNone); }
void CMainFrame::OnNavigateWalk() { SetNavigate(M4_NavWalk); }
void CMainFrame::OnNavigateFly() { SetNavigate(M4_NavFly); }
void CMainFrame::OnNavigateExam() { SetNavigate(M4_NavExamine); }
void CMainFrame::OnNavigateSlide() { SetNavigate(M4_NavSlide); }
void CMainFrame::OnNavigatePan() { SetNavigate(M4_NavPan); }
void CMainFrame::OnNavigateOrbit() { SetNavigate(M4_NavOrbit); }
void CMainFrame::OnNavigateGame() { SetNavigate(M4_NavGame); }

void CMainFrame::OnNavigateReset()
{
	WinGPAC *app = GetApp();
	M4T_SetOption(app->m_term, M4O_NavigationType, 0);
}


LONG CMainFrame::OnNavigate(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	WinGPAC *gpac = GetApp();

	if (is_supported_file(gpac->m_config, gpac->m_navigate_url, 0)) {
		char *str = URL_Concatenate(m_pPlayList->GetURL(), gpac->m_navigate_url);
		if (str) {
			m_pPlayList->Truncate();
			m_pPlayList->QueueURL(str);
			free(str);
			m_pPlayList->PlayNext();
			return 0;
		}
	}
	
	console_message = gpac->m_navigate_url;
	console_err = M4OK;
	PostMessage(WM_CONSOLEMSG);
	ShellExecute(NULL, "open", (LPCSTR) gpac->m_navigate_url, NULL, NULL, SW_SHOWNORMAL);
	return 0;
}

void CMainFrame::OnFileProp() 
{
	if (!m_pProps) {
		m_pProps = new CFileProps(this);
		m_pProps->Create(this);
	}
	m_pProps->ShowWindow(SW_SHOW);
}

void CMainFrame::OnUpdateFileProp(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetApp()->m_isopen);	
}

void CMainFrame::OnConfigure() 
{
	if (!m_pOpt) {
		m_pOpt = new COptions(this);
		m_pOpt->Create(this);
	}
	m_pOpt->ShowWindow(SW_SHOW);
}

void CMainFrame::OnShortcuts() 
{
	MessageBox(
		"Open File: Ctrl + O\n"
		"Open URL: Ctrl + U\n"
		"Reload File: Ctrl + R\n"
		"Pause/Resume File: Ctrl + P\n"
		"Step by Step: Ctrl + S\n"
		"Seek +5%: Alt + left arrow\n"
		"Seek -5%: Alt + right arrow\n"
		"Seek +1min: Alt + up arrow\n"
		"Seek -1min: Alt + down arrow\n"
		"Fullscreen On/Off: Alt+Return or Escape\n"
		"\n"
		"Show Properties: Ctrl + I\n"
		"Show Options: Ctrl + C\n"
		"\n"
		"Aspect Ratio Normal: Ctrl + 1\n"
		"Aspect Ratio Fill: Ctrl + 2\n"
		"Aspect Ratio 4/3: Ctrl + 3\n"
		"Aspect Ratio 16/9: Ctrl + 4\n"
		
		
		, "Shortcuts Available on Osmo4", MB_OK);
}

void CMainFrame::OnNavInfo() 
{
	MessageBox(
		"* Walk & Fly modes:\n"
		"\tH move: H pan - V move: Z-translate - V move+CTRL or Wheel: V pan - Right Click (Walk only): Jump\n"
		"\tleft/right: H pan - left/right+CTRL: H translate - up/down: Z-translate - up/down+CTRL: V pan\n"
		"* Pan mode:\n"
		"\tH move: H pan - V move: V pan - V move+CTRL or Wheel: Z-translate\n"
		"\tleft/right: H pan - left/right+CTRL: H translate - up/down: V pan - up/down+CTRL: Z-translate\n"
		"* Slide mode:\n"
		"\tH move: H translate - V move: V translate - V move+CTRL or Wheel: Z-translate\n"
		"\tleft/right: H translate - left/right+CTRL: H pan - up/down: V translate - up/down+CTRL: Z-translate\n"
		"* Examine & Orbit mode:\n"
		"\tH move: Y-Axis rotate - H move+CTRL: No move - V move: X-Axis rotate - V move+CTRL or Wheel: Z-translate\n"
		"\tleft/right: Y-Axis rotate - left/right+CTRL: H translate - up/down: X-Axis rotate - up/down+CTRL: Y-translate\n"
		"* Game mode (press END to escape):\n"
		"\tH move: H pan - V move: V pan\n"
		"\tleft/right: H translate - up/down: Z-translate\n"
		"\n"
		"* All 3D modes: CTRL+PGUP/PGDOWN will zoom in/out camera (field of view) \n"

		"\n"
		"*Slide Mode in 2D:\n"
		"\tH move: H translate - V move: V translate - V move+CTRL: zoom\n"
		"\tleft/right: H translate - up/down: V translate - up/down+CTRL: zoom\n"
		"*Examine Mode in 2D (3D renderer only):\n"
		"\tH move: Y-Axis rotate - V move: X-Axis rotate\n"
		"\tleft/right: Y-Axis rotate - up/down: X-Axis rotate\n"

		"\n"
		"HOME: reset navigation to last viewpoint (2D or 3D navigation)\n"
		"SHIFT key in all modes: fast movement\n"

		, "3D navigation keys (\'H\'orizontal and \'V\'ertical) used in GPAC", MB_OK);
}



void CMainFrame::BuildViewList()
{
	WinGPAC *app = GetApp();
	if (!app->m_isopen) return;

	/*THIS IS HARCODED FROM THE MENU LAYOUT */
	CMenu *pMenu = GetMenu()->GetSubMenu(1)->GetSubMenu(0);
	while (pMenu->GetMenuItemCount()) pMenu->DeleteMenu(0, MF_BYPOSITION);

	s32 id = ID_VP_0;
	nb_viewpoints = 0;
	while (1) {
		const char *szName;
		Bool bound;
		M4Err e = M4T_GetViewpoint(app->m_term, nb_viewpoints+1, &szName, &bound);
		if (e) break;
		if (szName) {
			pMenu->AppendMenu(MF_ENABLED, id+nb_viewpoints, szName);
		} else {
			char szLabel[1024];
			sprintf(szLabel, "Viewpoint #%d", nb_viewpoints+1);
			pMenu->AppendMenu(MF_ENABLED, id+nb_viewpoints, szLabel);
		}
		nb_viewpoints++;
		if (nb_viewpoints==ID_VP_19-ID_VP_0) break;
	}
}


void CMainFrame::BuildStreamList(Bool reset_only)
{
	u32 nb_subs;
	CMenu *pSelect;
	WinGPAC *app = GetApp();

	pSelect = GetMenu()->GetSubMenu(2)->GetSubMenu(0);
	/*THIS IS HARCODED FROM THE MENU LAYOUT */
	CMenu *pMenu = pSelect->GetSubMenu(0);
	while (pMenu->GetMenuItemCount()) pMenu->DeleteMenu(0, MF_BYPOSITION);
	pMenu = pSelect->GetSubMenu(1);
	while (pMenu->GetMenuItemCount()) pMenu->DeleteMenu(0, MF_BYPOSITION);
	pMenu = pSelect->GetSubMenu(2);
	while (pMenu->GetMenuItemCount()) pMenu->DeleteMenu(0, MF_BYPOSITION);

	if (reset_only) {
		m_bFirstStreamQuery = 1;
		return;
	}
	if (!app->m_isopen || !M4T_GetOption(app->m_term, M4O_HasSelectableStreams)) return;

	LPODMANAGER root_od = M4T_GetRootOD(app->m_term);
	if (!root_od) return;
	u32 count = M4T_GetODCount(app->m_term, root_od);
	nb_subs = 0;

	for (u32 i=0; i<count; i++) {
		char szLabel[1024];
		ODInfo info;
		LPODMANAGER odm = M4T_GetODManager(app->m_term, root_od, i);
		if (!odm) return;
		LPODMANAGER remote = odm;
		while (1) {
			remote = M4T_GetRemoteOD(app->m_term, odm);
			if (remote) odm = remote;
			else break;
		}

		if (M4T_GetODInfo(app->m_term, odm, &info) != M4OK) break;
		if (info.owns_service) {
			char *szName = strrchr(info.service_url, '\\');
			if (!szName) szName = strrchr(info.service_url, '/');
			if (!szName) szName = (char *) info.service_url;
			else szName += 1;
			strcpy(szLabel, szName);
			szName = strrchr(szLabel, '.');
			if (szName) szName[0] = 0;
		}

		switch (info.od_type) {
		case M4ST_AUDIO:
			pMenu = pSelect->GetSubMenu(0);
			if (!info.owns_service) sprintf(szLabel, "Audio #%d", pMenu->GetMenuItemCount() + 1);
			pMenu->AppendMenu(MF_ENABLED, ID_SELOBJ_0 + i, szLabel);
			break;
		case M4ST_VISUAL:
			pMenu = pSelect->GetSubMenu(1);
			if (!info.owns_service) sprintf(szLabel, "Video #%d", pMenu->GetMenuItemCount() + 1);
			pMenu->AppendMenu(MF_ENABLED, ID_SELOBJ_0 + i, szLabel);
			break;
		case M4ST_TEXT:
			nb_subs ++;
			pMenu = pSelect->GetSubMenu(2);
			if (!info.owns_service) sprintf(szLabel, "Subtitle #%d", pMenu->GetMenuItemCount() + 1);
			pMenu->AppendMenu(MF_ENABLED, ID_SELOBJ_0 + i, szLabel);
			break;
		}
	}
	if (m_bFirstStreamQuery) {
		m_bFirstStreamQuery = 0;
		if (!nb_subs && app->m_LookForSubtitles) LookForSubtitles();
	}

}

BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int ID = LOWORD(wParam);
	WinGPAC *app = GetApp();

	if ( (ID>=ID_VP_0) && (ID<=ID_VP_0+nb_viewpoints)) {
		ID -= ID_VP_0;
		M4T_SetViewpoint(app->m_term, ID+1, NULL);
		return TRUE;
	}
	if ( (ID>=ID_NAV_PREV_0) && (ID<=ID_NAV_PREV_9)) {
		ID -= ID_NAV_PREV_0;
		s32 prev = m_pPlayList->m_cur_entry - ID;
		if (prev>=0) {
			m_pPlayList->m_cur_entry = prev;
			m_pPlayList->PlayPrev();
		}
		return TRUE;
	}
	if ( (ID>=ID_NAV_NEXT_0) && (ID<=ID_NAV_NEXT_9)) {
		ID -= ID_NAV_NEXT_0;
		u32 next = m_pPlayList->m_cur_entry + ID;
		if (next < ChainGetCount(m_pPlayList->m_entries) ) {
			m_pPlayList->m_cur_entry = next;
			m_pPlayList->PlayNext();
		}
		return TRUE;
	}
	if ( (ID>=ID_SELOBJ_0) && (ID<=ID_SELOBJ_9)) {
		ID -= ID_SELOBJ_0;
		LPODMANAGER root_od = M4T_GetRootOD(app->m_term);
		if (!root_od) return TRUE;
		LPODMANAGER odm = M4T_GetODManager(app->m_term, root_od, ID);
		M4T_SelectOD(app->m_term, odm);
		return TRUE;
	}
	return CFrameWnd::OnCommand(wParam, lParam);
}

void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT ID, BOOL bSys)
{
	WinGPAC *app = GetApp();
	/*viewport list*/
	if (pPopupMenu->GetMenuItemID(0)==ID_VP_0) {
		for (int i=0; i<nb_viewpoints; i++) {
			const char *szName;
			Bool bound;
			M4Err e = M4T_GetViewpoint(app->m_term, i+1, &szName, &bound);
			pPopupMenu->EnableMenuItem(i, MF_BYPOSITION);
			if (bound) pPopupMenu->CheckMenuItem(i, MF_BYPOSITION | MF_CHECKED);
		}
		return;
	}
	/*navigation*/
	if ((pPopupMenu->GetMenuItemID(0)==ID_NAV_PREV_0) || (pPopupMenu->GetMenuItemID(0)==ID_NAV_NEXT_0)) {
		int count = pPopupMenu->GetMenuItemCount();
		for (int i=0; i<count; i++) {
			pPopupMenu->EnableMenuItem(i, MF_BYPOSITION);		
		}
		return;
	}
	if (pPopupMenu->m_hMenu == GetMenu()->GetSubMenu(2)->m_hMenu) {
		if (!app->m_isopen || !M4T_GetOption(app->m_term, M4O_HasSelectableStreams)) {
			pPopupMenu->EnableMenuItem(0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		} else {
			pPopupMenu->EnableMenuItem(0, MF_BYPOSITION | MF_ENABLED);
		}
	}
	if ((pPopupMenu->GetMenuItemID(0)>=ID_SELOBJ_0) && (pPopupMenu->GetMenuItemID(0)<=ID_SELOBJ_9)) {
		LPODMANAGER root_od = M4T_GetRootOD(app->m_term);
		if (!root_od) return;

		int count = pPopupMenu->GetMenuItemCount();
		for (int i=0; i<count; i++) {
			u32 id = pPopupMenu->GetMenuItemID(i) - ID_SELOBJ_0;
			LPODMANAGER odm = M4T_GetODManager(app->m_term, root_od, id);
			if (!odm) {
				pPopupMenu->EnableMenuItem(i, MF_DISABLED | MF_BYPOSITION);
			} else {
				ODInfo info;
				LPODMANAGER remote = odm;
				while (1) {
					remote = M4T_GetRemoteOD(app->m_term, odm);
					if (remote) odm = remote;
					else break;
				}

				M4T_GetODInfo(app->m_term, odm, &info);
				pPopupMenu->EnableMenuItem(i, MF_BYPOSITION);
				pPopupMenu->CheckMenuItem(i, MF_BYPOSITION | (info.status ? MF_CHECKED : MF_UNCHECKED) );
			}
		}
		return;
	}

	/*default*/
	CFrameWnd::OnInitMenuPopup(pPopupMenu, ID, bSys);
}

void CMainFrame::OnCollideDisp() 
{
	M4T_SetOption(GetApp()->m_term, M4O_Collision, M4_CollideDisp);
}

void CMainFrame::OnUpdateCollideDisp(CCmdUI* pCmdUI) 
{
	WinGPAC *gpac = GetApp(); 
	pCmdUI->Enable(gpac->m_isopen);	
	pCmdUI->SetCheck( (M4T_GetOption(gpac->m_term, M4O_Collision) == M4_CollideDisp) ? 1 : 0);
}

void CMainFrame::OnCollideNone() 
{
	M4T_SetOption(GetApp()->m_term, M4O_Collision, M4_CollideNone);
}

void CMainFrame::OnUpdateCollideNone(CCmdUI* pCmdUI) 
{
	WinGPAC *gpac = GetApp(); 
	pCmdUI->Enable(gpac->m_isopen);	
	pCmdUI->SetCheck( (M4T_GetOption(gpac->m_term, M4O_Collision) == M4_CollideNone) ? 1 : 0);
}

void CMainFrame::OnCollideReg() 
{
	M4T_SetOption(GetApp()->m_term, M4O_Collision, M4_Collide);
}

void CMainFrame::OnUpdateCollideReg(CCmdUI* pCmdUI) 
{
	WinGPAC *gpac = GetApp(); 
	pCmdUI->Enable(gpac->m_isopen);	
	pCmdUI->SetCheck( (M4T_GetOption(gpac->m_term, M4O_Collision) == M4_Collide) ? 1 : 0);
}

void CMainFrame::OnHeadlight() 
{
	WinGPAC *app = GetApp();
	Bool val = !M4T_GetOption(app->m_term, M4O_Headlight);
	M4T_SetOption(app->m_term, M4O_Headlight, val);
}

void CMainFrame::OnUpdateHeadlight(CCmdUI* pCmdUI) 
{
	WinGPAC *app = GetApp();
	pCmdUI->Enable(FALSE);
	if (!app->m_isopen) return;
	u32 type = M4T_GetOption(app->m_term, M4O_NavigationType);
	if (type!=M4_Navigation3D) return;

	pCmdUI->Enable(TRUE);
	pCmdUI->SetCheck(M4T_GetOption(app->m_term, M4O_Headlight) ? 1 : 0);
}

void CMainFrame::OnGravity() 
{
	WinGPAC *app = GetApp();
	Bool val = M4T_GetOption(app->m_term, M4O_Gravity) ? 0 : 1;
	M4T_SetOption(app->m_term, M4O_Gravity, val);
}

void CMainFrame::OnUpdateGravity(CCmdUI* pCmdUI) 
{
	WinGPAC *app = GetApp();
	pCmdUI->Enable(FALSE);
	if (!app->m_isopen) return;
	u32 type = M4T_GetOption(app->m_term, M4O_NavigationType);
	if (type!=M4_Navigation3D) return;
	type = M4T_GetOption(app->m_term, M4O_NavigationMode);
	if (type != M4_NavWalk) return;
	pCmdUI->Enable(TRUE);
	pCmdUI->SetCheck(M4T_GetOption(app->m_term, M4O_Gravity) ? 1 : 0);
}


BOOL CMainFrame::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{

	if (((LPNMHDR)lParam)->code == TBN_DROPDOWN) {
		RECT rc;
		s32 i, count, start;
		POINT pt;
		CMenu *pPopup = new CMenu();
		pPopup->CreatePopupMenu();

		m_wndToolBar.GetWindowRect(&rc);
		pt.y = rc.bottom;
		pt.x = rc.left;
		m_wndToolBar.GetToolBarCtrl().GetItemRect(0, &rc);
		pt.x += (rc.right - rc.left);
		m_wndToolBar.GetToolBarCtrl().GetItemRect(1, &rc);
		pt.x += (rc.right - rc.left);

		count = ChainGetCount(m_pPlayList->m_entries);
		if ( ((LPNMTOOLBAR)lParam)->iItem == ID_NAV_PREV) {
			start = m_pPlayList->m_cur_entry - 1;
			for (i=0; i<10; i++) {
				if (start - i < 0) break;
				if (start - i >= count) break;
				PLEntry *ple = (PLEntry *) ChainGetEntry(m_pPlayList->m_entries, start - i);
				pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_NAV_PREV_0 + i, ple->m_disp_name);
			}
		} else {
			start = m_pPlayList->m_cur_entry + 1;
			for (i=0; i<10; i++) {
				if (start + i >= count) break;
				PLEntry *ple = (PLEntry *) ChainGetEntry(m_pPlayList->m_entries, start + i);
				pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_NAV_NEXT_0 + i, ple->m_disp_name);
			}
			m_wndToolBar.GetToolBarCtrl().GetItemRect(2, &rc);
			pt.x += (rc.right - rc.left);
		}
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
		delete pPopup;

		return FALSE;
	}
	return CFrameWnd::OnNotify(wParam, lParam, pResult);
}

void CMainFrame::OnNavNext() 
{
	WinGPAC *app = GetApp();
	/*don't play if last could trigger playlist loop*/
	if ((m_pPlayList->m_cur_entry<0) || (ChainGetCount(m_pPlayList->m_entries) == 1 + (u32) m_pPlayList->m_cur_entry)) return;
	m_pPlayList->PlayNext();
}

void CMainFrame::OnUpdateNavNext(CCmdUI* pCmdUI) 
{
	if (m_pPlayList->m_cur_entry<0) pCmdUI->Enable(FALSE);
	else if ((u32) m_pPlayList->m_cur_entry + 1 == ChainGetCount(m_pPlayList->m_entries) ) pCmdUI->Enable(FALSE);
	else pCmdUI->Enable(TRUE);
}

void CMainFrame::OnNavPrev() 
{
	WinGPAC *app = GetApp();
	if (m_pPlayList->m_cur_entry<=0) return;
	m_pPlayList->PlayPrev();
}

void CMainFrame::OnUpdateNavPrev(CCmdUI* pCmdUI) 
{
	if (m_pPlayList->m_cur_entry<=0) pCmdUI->Enable(FALSE);
	else pCmdUI->Enable(TRUE);
}


void CMainFrame::OnClearNav() 
{
	m_pPlayList->ClearButPlaying();
}

void CMainFrame::OnViewPlaylist() 
{
	m_pPlayList->ShowWindow(m_pPlayList->IsWindowVisible() ? SW_HIDE : SW_SHOW);
}

void CMainFrame::OnUpdateViewPlaylist(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_pPlayList->IsWindowVisible() ? 1 : 0);
}
void CMainFrame::OnPlayLoop() 
{
	GetApp()->m_Loop = !GetApp()->m_Loop;
}

void CMainFrame::OnUpdatePlayLoop(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetApp()->m_Loop ? 1 : 0);
}

void CMainFrame::OnAddSubtitle() 
{
	CFileDialog fd(TRUE,NULL,NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, "All Subtitles|*.srt;*.ttxt|SRT Subtitles|*.srt|3GPP TimedText|*.ttxt|");
	if (fd.DoModal() != IDOK) return;

	AddSubtitle(fd.GetPathName(), 1);
}

void CMainFrame::AddSubtitle(const char *fileName, Bool auto_play)
{
	M4T_AddObject(GetApp()->m_term, fileName, auto_play);
}

static Bool subs_enum_dir_item(void *cbck, char *item_name, char *item_path)
{
	char *ext;
	WinGPAC *app = GetApp();
	CMainFrame *_this = (CMainFrame *)cbck;

	ext = strrchr(item_name, '.');
	if (!ext) return 0;
	ext += 1;
	if (!stricmp(ext, "srt") || !stricmp(ext, "ttxt")) 
		_this->AddSubtitle(item_path, 0);

	return 0;
}

void CMainFrame::LookForSubtitles()
{
	char dir[M4_MAX_PATH];
	CString url = m_pPlayList->GetURL();
	strcpy(dir, url);
	char *sep = strrchr(dir, '\\');
	if (!sep) ::GetCurrentDirectory(M4_MAX_PATH, dir);
	else sep[0] = 0;

	DIR_Enum(dir, 0, subs_enum_dir_item, this);
}

