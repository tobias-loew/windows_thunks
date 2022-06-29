
// windows_thunk_testDlg.h : header file
//

#pragma once

#include <vector>
#include <thunks.hpp>
#include <thunk_hook.hpp>

// CwindowsthunktestDlg dialog
class CwindowsthunktestDlg : public CDialogEx
{
// Construction
public:
	CwindowsthunktestDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WINDOWS_THUNK_TEST_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

    // Mouse hook callback
    LRESULT MouseHookProc(int code, WPARAM wParam, LPARAM lParam);

    lunaticpp::thunk_hook<&MouseHookProc, WH_MOUSE>   m_thunk_hook_mouse;


    BOOL EnumChildProc(
        _In_ HWND   hwnd,
        _In_ LPARAM lParam
    );
    lunaticpp::thunk<&EnumChildProc> m_thunk_enum_child_proc;

    std::vector<CString> m_child_windows_names;



    void TimerProc(HWND hwnd, UINT msg, UINT_PTR id, DWORD time);
    lunaticpp::thunk<&TimerProc> m_thunk_timer_proc;

    // before x64 was introduced, the timer-callback signatuer was different
    // but the good news is: the compiler will error when compiling for x64 on that! 
    void TimerProcWrongx64Signature(HWND hwnd, UINT msg, UINT id, DWORD time);
    lunaticpp::thunk<&TimerProcWrongx64Signature> m_thunk_timer_proc_wrong_x64_signature;

public:
    afx_msg void OnBnClickedBtnListChildWindows();
    CStatic m_stc_mouse_btn;
    CStatic m_stc_mouse_pos;
    CStatic m_stc_timer_x86_x64;
    CStatic m_stc_timer_x86_only;
};
