
// AppDialog.cpp : implementation file
//

#include "stdafx.h"
#include "App.h"
#include "AppDialog.h"
#include "afxdialogex.h"
#include "windows.h"
#include "server.h"
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAppDialog dialog

static UINT BASED_CODE indicators[] = { ID_INDICATOR };

CAppDialog::CAppDialog(CWnd* pParent /*=NULL*/)
: CDialogEx(CAppDialog::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAppDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAppDialog, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CAppDialog::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_STATUSBAR, &CAppDialog::OnBnClickedStatusbar)
	ON_BN_CLICKED(IDC_GUILOG, &CAppDialog::OnBnClickedGuilog)
	ON_BN_CLICKED(IDC_LOGFILE, &CAppDialog::OnBnClickedLogfile)
	ON_MESSAGE(WM_NEW_MSG, &CAppDialog::OnNewMessage)
	ON_MESSAGE(WM_UPDATE_STATUSBAR, &CAppDialog::OnUpdateStatusBar)
	ON_MESSAGE(WM_UPDATE_GUILOG, &CAppDialog::OnUpdateGuiLog)
END_MESSAGE_MAP()

DWORD WINAPI CAppDialog::serverThrd(LPVOID pParam) {
	HWND* handle = (HWND*)pParam;
	Server server(DEFAULT_PORT, handle);
	if (server.init() == 0) {
		server.start_comm();
	}
	return 0;
}

// CMFCApplication1Dlg message handlers

BOOL CAppDialog::OnInitDialog()
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
	statusBar.SetPaneInfo(0, ID_INDICATOR, SBPS_NORMAL, rect.Width());
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, ID_INDICATOR);

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

void CAppDialog::OnPaint()
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
HCURSOR CAppDialog::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CAppDialog::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}


void CAppDialog::OnBnClickedStatusbar()
{
	if (((CButton *)GetDlgItem(IDC_STATUSBAR))->GetCheck()) {
		statusBarOutput->enable();
	}
	else {
		statusBarOutput->disable();
	}
}


void CAppDialog::OnBnClickedGuilog()
{
	if (((CButton *)GetDlgItem(IDC_GUILOG))->GetCheck()) {
		guiLogOutput->enable();
	}
	else {
		guiLogOutput->disable();
	}
}

void CAppDialog::OnBnClickedLogfile()
{
	if (((CButton *)GetDlgItem(IDC_LOGFILE))->GetCheck()) {
		logFileOutput->enable();
	}
	else {
		logFileOutput->disable();
	}
}

LRESULT CAppDialog::OnNewMessage(WPARAM wParam, LPARAM lParam) {
	LPTSTR msg = (LPTSTR)lParam;

	statusBarOutput->update(msg);
	guiLogOutput->update(msg);
	logFileOutput->update(msg);

	return 0;
}

LRESULT CAppDialog::OnUpdateStatusBar(WPARAM wParam, LPARAM lParam) {
	LPTSTR msg = reinterpret_cast<LPTSTR>(lParam);
	statusBar.SetPaneText(0, msg);

	return 0;
}

LRESULT CAppDialog::OnUpdateGuiLog(WPARAM wParam, LPARAM lParam) {
	CEdit* guiLog = (CEdit*)GetDlgItem(IDC_LOG);

	LPTSTR msg = reinterpret_cast<LPTSTR>(lParam);
	CString line = msg;
	line.Format(_T("%s\r\n"), msg);
	int textLen = guiLog->GetWindowTextLengthW();
	guiLog->SetSel(textLen, textLen);
	guiLog->ReplaceSel(line);
	guiLog->LineScroll(guiLog->GetLineCount());

	return 0;
}

CAppDialog::StatusBarOutput::StatusBarOutput(HWND* handle) {
	this->handle = handle;
}

CAppDialog::GuiLogOutput::GuiLogOutput(HWND* handle) {
	this->handle = handle;
}

CAppDialog::LogFileOutput::LogFileOutput(char* filename) {
	this->file.open(filename);
}

CAppDialog::LogFileOutput::~LogFileOutput() {
	this->file.close();
}

void CAppDialog::StatusBarOutput::update(LPTSTR newString) {
	if (isEnabled()) {
		::SendMessage(*this->handle, WM_UPDATE_STATUSBAR, 0, (LPARAM)newString);
	}
}

void CAppDialog::GuiLogOutput::update(LPTSTR newString) {
	if (isEnabled()) {
		::SendMessage(*this->handle, WM_UPDATE_GUILOG, 0, (LPARAM)newString);
	}
}

void CAppDialog::LogFileOutput::update(LPTSTR newMsg) {
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
