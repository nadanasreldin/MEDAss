
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
	ON_MESSAGE(WM_UPDATE_STATUSBAR, &CAppDialog::OnUpdateStatusBar)
	ON_MESSAGE(WM_UPDATE_GUILOG, &CAppDialog::OnUpdateGuiLog)
	ON_MESSAGE(WM_NEW_MSG, &CAppDialog::OnNewMessage)
END_MESSAGE_MAP()

//DWORD WINAPI CAppDialog::uiThrdStatic(void *pThis) {
//	CAppDialog * pDlg = (CAppDialog*)pThis;
//	return pDlg->uiThrd();
//}
//
//DWORD CAppDialog::uiThrd() {
//	/*MSG msg;
//	bool msgReturn = GetMessage(&msg, NULL, WM_USER + 1, WM_USER + 2);
//
//	if (msgReturn) {
//	statusBar.update();
//	guiLog.update();
//	logFile.update();
//	}*/
//	statusBarOutput->update(_T("BLABALBALBA"));
//	guiLogOutput->update(_T("BLABALBALBA"));
//	return 0;
//}

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

	//setlocale(LC_CTYPE, "");

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// get window object handle
	HWND *handle = new HWND;
	*handle = GetSafeHwnd();

	// initialize the 3 output destinations

	// init guiLogOutput
	//guiLogOutput = new GuiLogOutput((CStatic*)GetDlgItem(IDC_LOG));
	guiLogOutput = new GuiLogOutput(handle);

	// create status bar and init statusBarOutput
	statusBar.Create(this);
	statusBar.SetIndicators(indicators, 1);
	CRect rect;
	GetClientRect(&rect);
	statusBar.SetPaneInfo(0, ID_INDICATOR, SBPS_NORMAL, rect.Width());
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, ID_INDICATOR);

	statusBarOutput = new StatusBarOutput(handle); 

	//HANDLE uiHandle, serverHandle;.
	//DWORD uiThrdId, serverThrdId;

	//// initialize thread that listens on msg queue (how??) and invokes
	//// the update function of the three destinations outputs accordingly
	////uiHandle = CreateThread(NULL, 0, uiThrdStatic, (void*) this, 0, &uiThrdId);


	//// initialize server in new thread
	//// pass newly created msg queue to constructor
	//serverHandle = CreateThread(NULL, 0, serverThrd, 0, 0, &serverThrdId);

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
	if (((CButton *)GetDlgItem(IDC_LOGFILE))->GetCheck() == BST_CHECKED) {
		logFileOutput->enable();
	}
	else {
		logFileOutput->disable();
	}
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
		
	/*CString previousText;
	guiLog->GetWindowText(previousText);
	guiLog->SetWindowText(previousText + "\r" + msg);*/

	
	//guiLog->SetWindowText(msg);

	return 0;
}

LRESULT CAppDialog::OnNewMessage(WPARAM wParam, LPARAM lParam) {
	/*statusBarOutput->update(_T("BLABALBALBA"));
	guiLogOutput->update(_T("BLABALBALBA"));*/

	//statusBarOutput->update((LPTSTR)wParam);
	statusBarOutput->update((LPTSTR)lParam);
	guiLogOutput->update((LPTSTR)lParam);

	return 0;
}

CAppDialog::StatusBarOutput::StatusBarOutput(HWND* handle) {
	this->handle = handle;
}

void CAppDialog::StatusBarOutput::update(LPTSTR newString) {
	if (isEnabled()) {
		::SendMessage(*this->handle, WM_UPDATE_STATUSBAR, 0, (LPARAM)newString);
	}
}

//CAppDialog::GuiLogOutput::GuiLogOutput(CStatic* guiLog) {
//	this->guiLog = guiLog;
//}

CAppDialog::GuiLogOutput::GuiLogOutput(HWND* handle) {
	this->handle = handle;
}

void CAppDialog::GuiLogOutput::update(LPTSTR newString) {
	if (isEnabled()) {
		//guiLog->SetWindowText(newString);
		::SendMessage(*this->handle, WM_UPDATE_GUILOG, 0, (LPARAM)newString);
	}
}

void CAppDialog::LogFileOutput::update(LPTSTR newString) {

}
