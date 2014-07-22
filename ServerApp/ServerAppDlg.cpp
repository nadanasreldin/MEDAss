
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

DWORD WINAPI CServerAppDlg::serverThrd(LPVOID pParam) {
	HWND* handle = (HWND*)pParam;
	Server server(DEFAULT_PORT, handle);
	if (server.init() == 0) {
		server.start_comm();
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

	statusBarChk = (CButton *)GetDlgItem(IDC_CHECK_STATUSBAR);
	guiLogChk = (CButton *)GetDlgItem(IDC_CHECK_GUILOG);
	logFileChk = (CButton *)GetDlgItem(IDC_CHECK_LOGFILE);
	guiLog = (CEdit*)GetDlgItem(IDC_LOG);

	// get window object handle
	HWND *handle = new HWND;
	*handle = GetSafeHwnd();

	// initialize the 3 output destinations
	statusBarOutput = new StatusBarOutput(handle);
	guiLogOutput = new GuiLogOutput(handle);
	logFileOutput = new LogFileOutput("output.log");

	// start server logic in another thread
	DWORD serverThrdId;
	HANDLE serverHandle = CreateThread(NULL, 0, serverThrd, handle, 0, &serverThrdId);

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
	CDialogEx::OnCancel();
}

void CServerAppDlg::OnBnClickedStatusbar()
{
	if (statusBarChk->GetCheck()) {
		statusBarOutput->enable();
	}
	else {
		statusBarOutput->disable();
	}
}


void CServerAppDlg::OnBnClickedGuilog()
{
	if (guiLogChk->GetCheck()) {
		guiLogOutput->enable();
	}
	else {
		guiLogOutput->disable();
	}
}

void CServerAppDlg::OnBnClickedLogfile()
{
	if (logFileChk->GetCheck()) {
		logFileOutput->enable();
	}
	else {
		logFileOutput->disable();
	}
}

LRESULT CServerAppDlg::OnNewMessage(WPARAM wParam, LPARAM lParam) {
	LPTSTR msg = (LPTSTR)lParam;

	// update the 3 output destinations
	statusBarOutput->update(msg);
	guiLogOutput->update(msg);
	logFileOutput->update(msg);

	return 0;
}

LRESULT CServerAppDlg::OnUpdateStatusBar(WPARAM wParam, LPARAM lParam) {
	LPTSTR msg = reinterpret_cast<LPTSTR>(lParam);
	statusBar.SetPaneText(0, msg);

	return 0;
}

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

CServerAppDlg::StatusBarOutput::StatusBarOutput(HWND* handle) {
	this->handle = handle;
}

CServerAppDlg::GuiLogOutput::GuiLogOutput(HWND* handle) {
	this->handle = handle;
}

CServerAppDlg::LogFileOutput::LogFileOutput(char* filename) {
	this->file.open(filename);
}

CServerAppDlg::LogFileOutput::~LogFileOutput() {
	this->file.close();
}

void CServerAppDlg::StatusBarOutput::update(LPTSTR newMsg) {
	if (isEnabled()) {
		// send WM_UPDATE_STATUSBAR to update status bar with msg
		::SendMessage(*this->handle, WM_UPDATE_STATUSBAR, 0, (LPARAM)newMsg);
	}
}

void CServerAppDlg::GuiLogOutput::update(LPTSTR newMsg) {
	if (isEnabled()) {
		// send WM_UPDATE_GUILOG to update guilog with msg
		::SendMessage(*this->handle, WM_UPDATE_GUILOG, 0, (LPARAM)newMsg);
	}
}

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

