
// AppDialog.h : header file
//

#pragma once

#include <fstream>

// CAppDialog dialog
class CAppDialog : public CDialogEx
{
private:
	class ServerOutput {
	private:
		bool enabled;
	public:
		ServerOutput() { enabled = false; }
		bool isEnabled() { return enabled; }
		void enable() { enabled = true; }
		void disable() { enabled = false; }
		virtual void update(LPTSTR newString) = 0;
	};
	class StatusBarOutput : public ServerOutput {
	private:
		HWND* handle;
	public:
		StatusBarOutput(HWND*);
		void update(LPTSTR newString);
	};

	class GuiLogOutput : public ServerOutput {
	private:
		HWND* handle;
	public:
		GuiLogOutput(HWND*);
		void update(LPTSTR newString);
	};

	class LogFileOutput : public ServerOutput {
	private:
		std::wofstream file;
	public:
		LogFileOutput(char* filename);
		~LogFileOutput();
		void update(LPTSTR newString);
	};

// Construction
public:
	CAppDialog(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SERVER_DIALOG };

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
	static DWORD WINAPI serverThrd(void *);

	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedStatusbar();
	afx_msg void OnBnClickedGuilog();
	afx_msg void OnBnClickedLogfile();

	LRESULT CAppDialog::OnNewMessage(WPARAM, LPARAM);
	LRESULT CAppDialog::OnUpdateStatusBar(WPARAM, LPARAM);
	LRESULT CAppDialog::OnUpdateGuiLog(WPARAM, LPARAM);

private:
	StatusBarOutput* statusBarOutput;
	GuiLogOutput* guiLogOutput;
	LogFileOutput* logFileOutput;

	CStatusBar statusBar;
};
