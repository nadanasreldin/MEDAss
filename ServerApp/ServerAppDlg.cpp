
// ServerAppDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Server.h"
#include "ServerApp.h"
#include "ServerAppDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CServerAppDlg dialog

static UINT indicators[] = { ID_INDICATOR_MSG };

CServerAppDlg::CServerAppDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CServerAppDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CServerAppDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CServerAppDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_CHECK_STATUSBAR, &CServerAppDlg::OnBnClickedStatusbar)
	ON_BN_CLICKED(IDC_CHECK_GUILOG, &CServerAppDlg::OnBnClickedGuilog)
	ON_BN_CLICKED(IDC_CHECK_LOGFILE, &CServerAppDlg::OnBnClickedLogfile)
	ON_MESSAGE(WM_NEW_MSG, &CServerAppDlg::OnNewMessage)
	ON_MESSAGE(WM_UPDATE_STATUSBAR, &CServerAppDlg::OnUpdateStatusBar)
	ON_MESSAGE(WM_UPDATE_GUILOG, &CServerAppDlg::OnUpdateGuiLog)
END_MESSAGE_MAP()

// the struct used to pass multiple parameters 
// to thread that will run server logic
struct ServerArgs{
	HWND* windowHandle;
	HANDLE* shutdownEvent;
};

// entry point of thread that starts the server 
// initializes server using args passed in a ServerArgs struct
DWORD WINAPI CServerAppDlg::runServer(LPVOID pParam) {
	// get server args
	struct ServerArgs* serverArgs = (ServerArgs*)pParam;
	HWND* windowHandle = serverArgs->windowHandle;
	HANDLE* shutdownEvent = serverArgs->shutdownEvent;

	// create new server object
	Server server(DEFAULT_PORT, windowHandle, shutdownEvent);

	delete serverArgs;
	
	// start the server
	if (server.init() == 0) {
		server.startComm();
	}

	return 0;
}

// CServerAppDlg message handlers

BOOL CServerAppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// create status bar
	statusBar.Create(this);
	statusBar.SetIndicators(indicators, 1);
	CRect rect;
	GetClientRect(&rect);
	statusBar.SetPaneInfo(0, ID_INDICATOR_MSG, SBPS_NORMAL, rect.Width());
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, ID_INDICATOR_MSG);

	// three checkboxes
	statusBarChk = (CButton *)GetDlgItem(IDC_CHECK_STATUSBAR);
	guiLogChk = (CButton *)GetDlgItem(IDC_CHECK_GUILOG);
	logFileChk = (CButton *)GetDlgItem(IDC_CHECK_LOGFILE);

	// gui log output
	guiLog = (CEdit*)GetDlgItem(IDC_LOG);

	// get window object handle
	handle = new HWND;
	*handle = GetSafeHwnd();

	// initialize the 3 output destinations
	statusBarOutput = new StatusBarOutput(handle);
	guiLogOutput = new GuiLogOutput(handle);
	logFileOutput = new LogFileOutput("output.log");

	// create an event that will be needed for graceful server shutdown
	shutdownEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	// args needed to instantiate server object
	struct ServerArgs* serverArgs = new ServerArgs;
	serverArgs->shutdownEvent = &shutdownEvent;
	serverArgs->windowHandle = handle;

	// launch server in new thread
	DWORD serverThrdId;
	serverThread = CreateThread(NULL, 0, runServer, serverArgs, 0, &serverThrdId);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CServerAppDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CServerAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CServerAppDlg::OnBnClickedCancel()
{
	// if server thread is still running, fire shutdown event
	if (WaitForSingleObject(serverThread, 0) != WAIT_OBJECT_0) {
		SetEvent(this->shutdownEvent);
		if (WaitForSingleObject(serverThread, 3) != WAIT_OBJECT_0) {
			TerminateThread(serverThread, 1);
		}
	}
	CloseHandle(this->shutdownEvent);
	CloseHandle(this->serverThread);
	
	delete handle;
	delete statusBarOutput;
	delete guiLogOutput;
	delete logFileOutput;

	CDialogEx::OnCancel();
}

// launched when status bar checkbox is toggled
void CServerAppDlg::OnBnClickedStatusbar()
{
	if (statusBarChk->GetCheck()) {
		statusBarOutput->enable();
	}
	else {
		statusBarOutput->disable();
	}
}

// launched when gui log checkbox is toggled
void CServerAppDlg::OnBnClickedGuilog()
{
	if (guiLogChk->GetCheck()) {
		guiLogOutput->enable();
	}
	else {
		guiLogOutput->disable();
	}
}

// launched when log file checkbox is toggled
void CServerAppDlg::OnBnClickedLogfile()
{
	if (logFileChk->GetCheck()) {
		logFileOutput->enable();
	}
	else {
		logFileOutput->disable();
	}
}

// launched when a WM_NEW_MSG is sent from the server thread
// uses parameter passed in message to update the gui accordingly
LRESULT CServerAppDlg::OnNewMessage(WPARAM wParam, LPARAM lParam) {
	LPTSTR msg = (LPTSTR)lParam;

	// update the 3 output destinations
	statusBarOutput->update(msg);
	guiLogOutput->update(msg);
	logFileOutput->update(msg);

	delete msg;

	return 0;
}

// launched when a WM_UPDATE_STATUSBAR msg is sent from the StatusBarOutput object
// changes status bar to show the string sent as a message parameter
LRESULT CServerAppDlg::OnUpdateStatusBar(WPARAM wParam, LPARAM lParam) {
	LPTSTR msg = reinterpret_cast<LPTSTR>(lParam);
	statusBar.SetPaneText(0, msg);

	return 0;
}

// launched when a WM_UPDATE_GUILOG msg is sent from the GuiLogOutput object
// appends the string sent as a message parameter to the log of the gui window
LRESULT CServerAppDlg::OnUpdateGuiLog(WPARAM wParam, LPARAM lParam) {
	// get msg and add new line
	LPTSTR msg = reinterpret_cast<LPTSTR>(lParam);
	CString line = msg;
	line.Format(_T("%s\r\n"), msg);

	// append msg to log
	int textLen = guiLog->GetWindowTextLengthW();
	guiLog->SetSel(textLen, textLen);
	guiLog->ReplaceSel(line);
	guiLog->LineScroll(guiLog->GetLineCount());

	return 0;
}

// sends message to ui to update status bar with the input string
// only if status bar output is enabled
void CServerAppDlg::StatusBarOutput::update(LPTSTR newMsg) {
	if (isEnabled()) {
		// send WM_UPDATE_STATUSBAR to update status bar with msg
		::SendMessage(*this->handle, WM_UPDATE_STATUSBAR, 0, (LPARAM)newMsg);
	}
}

// sends message to ui to update gui log with the input string
// only if gui log output is enabled
void CServerAppDlg::GuiLogOutput::update(LPTSTR newMsg) {
	if (isEnabled()) {
		// send WM_UPDATE_GUILOG to update gui log with msg
		::SendMessage(*this->handle, WM_UPDATE_GUILOG, 0, (LPARAM)newMsg);
	}
}

// appends the input string to the log file
// only if log file output is enabled
void CServerAppDlg::LogFileOutput::update(LPTSTR newMsg) {
	if (isEnabled()) {
		// get current time
		__time32_t clock;
		struct tm time;
		char timebuf[32];

		_time32(&clock);
		_localtime32_s(&time, &clock);
		asctime_s(timebuf, 32, &time);

		timebuf[strlen(timebuf) - 1] = '\0';

		// add to log file time + message
		file << timebuf << '\t' << newMsg;
		file.flush();
	}
}

