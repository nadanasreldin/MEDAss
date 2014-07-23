
// ServerAppDlg.h : header file
//

#pragma once

#include <fstream>

// CServerAppDlg dialog
class CServerAppDlg : public CDialogEx
{
private:

	// abstract class representing an output destination
	class ServerOutput {
	private:
		bool enabled;
	public:
		ServerOutput() { enabled = false; }
		bool isEnabled() { return enabled; }
		void enable() { enabled = true; }
		void disable() { enabled = false; }
		virtual void update(LPTSTR) = 0;
	};

	// class for representing and keeping state of
	// output on status bar of gui
	class StatusBarOutput : public ServerOutput {
	private:
		HWND* handle;
	public:
		StatusBarOutput(HWND* handle)
			:handle(handle){};
		void update(LPTSTR);
	};

	// class for representing and keeping state of
	// output on log of gui
	class GuiLogOutput : public ServerOutput {
	private:
		HWND* handle;
	public:
		GuiLogOutput(HWND* handle)
			:handle(handle){};
		void update(LPTSTR);
	};

	// class for representing and keeping state of
	// output to logfile
	class LogFileOutput : public ServerOutput {
	private:
		std::wofstream file;
	public:
		LogFileOutput(char* filename){ this->file.open(filename); }
		~LogFileOutput() { this->file.close(); }
		void update(LPTSTR);
	};

// Construction
public:
	CServerAppDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SERVERAPP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	static DWORD WINAPI runServer(void *);

	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedStatusbar();
	afx_msg void OnBnClickedGuilog();
	afx_msg void OnBnClickedLogfile();

	LRESULT CServerAppDlg::OnNewMessage(WPARAM, LPARAM);
	LRESULT CServerAppDlg::OnUpdateStatusBar(WPARAM, LPARAM);
	LRESULT CServerAppDlg::OnUpdateGuiLog(WPARAM, LPARAM);

private:
	StatusBarOutput* statusBarOutput;
	GuiLogOutput* guiLogOutput;
	LogFileOutput* logFileOutput;

	CStatusBar statusBar;
	
	// checkboxes
	CButton* statusBarChk;
	CButton* guiLogChk;
	CButton* logFileChk;

	CEdit* guiLog;	// log of gui window

	HWND* handle;	// window handle
	HANDLE serverThread;	// server thread handle
	HANDLE shutdownEvent;	// handle to event that shuts down the server
};
