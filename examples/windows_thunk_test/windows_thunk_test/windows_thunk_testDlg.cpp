
// windows_thunk_testDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "windows_thunk_test.h"
#include "windows_thunk_testDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CwindowsthunktestDlg dialog



CwindowsthunktestDlg::CwindowsthunktestDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WINDOWS_THUNK_TEST_DIALOG, pParent)
    , m_thunk_hook_mouse{ this }
    , m_thunk_enum_child_proc{ this }
    , m_thunk_timer_proc{ this }
    , m_thunk_timer_proc_wrong_x64_signature{ this }
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CwindowsthunktestDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STC_MOUSE_BTN, m_stc_mouse_btn);
    DDX_Control(pDX, IDC_STC_MOUSE_POS, m_stc_mouse_pos);
    DDX_Control(pDX, IDC_STC_TIMER_X86_x64, m_stc_timer_x86_x64);
    DDX_Control(pDX, IDC_STC_TIMER_X86, m_stc_timer_x86_only);
}

BEGIN_MESSAGE_MAP(CwindowsthunktestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_LIST_CHILD_WINDOWS, &CwindowsthunktestDlg::OnBnClickedBtnListChildWindows)
END_MESSAGE_MAP()


// CwindowsthunktestDlg message handlers

BOOL CwindowsthunktestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon


    m_thunk_hook_mouse.hook();

    SetTimer(static_cast<UINT_PTR>(0xabcdef1234567890), 1000, m_thunk_timer_proc.func());

    // before x64 was introduced, the timer-callback signatuer was different (used UINT for the id, now it's UINT_PTR)
    // but the good news is: the compiler will error when compiling for x64
#ifdef _M_IX86
    
    SetTimer(static_cast<UINT_PTR>(0x0987654321fedcba), 1000, m_thunk_timer_proc_wrong_x64_signature.func());

#elif defined(_M_X64)

    // remove the comment to see the compiler error!
    //SetTimer(static_cast<UINT_PTR>(0x0987654321fedcba), 1000, m_thunk_timer_proc_wrong_x64_signature.func());

#endif

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CwindowsthunktestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CwindowsthunktestDlg::OnPaint()
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
HCURSOR CwindowsthunktestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



// Mouse hook: used to implement un-obtrusive mouse tracking
LRESULT CwindowsthunktestDlg::MouseHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (code >= 0)
    {
        switch ((UINT)wParam)
        {
        case WM_NCMOUSEMOVE:
        case WM_MOUSEMOVE:
            {
                LPMOUSEHOOKSTRUCT mhs = reinterpret_cast<LPMOUSEHOOKSTRUCT>(lParam);
                CString  text;
                text.Format(_T("mouse-position: [%d, %d]"), mhs->pt.x, mhs->pt.y);
                m_stc_mouse_pos.SetWindowText(text);
            }
            break;

        case WM_NCLBUTTONDOWN:
            {
                m_stc_mouse_btn.SetWindowText(_T("nc left button down"));
            }
            break;

        case WM_LBUTTONDOWN:
            {
                m_stc_mouse_btn.SetWindowText(_T("left button down"));
            }
            break;

        case WM_NCMBUTTONDOWN:
            {
                m_stc_mouse_btn.SetWindowText(_T("nc middle button down"));
            }
            break;

        case WM_MBUTTONDOWN:
            {
                m_stc_mouse_btn.SetWindowText(_T("middle button down"));
            }
            break;

        case WM_NCRBUTTONDOWN:
            {
                m_stc_mouse_btn.SetWindowText(_T("nc right button down"));
            }
            break;

        case WM_RBUTTONDOWN:
            {
                m_stc_mouse_btn.SetWindowText(_T("right button down"));
            }
            break;


        case WM_NCLBUTTONUP:
            {
                m_stc_mouse_btn.SetWindowText(_T("nc left button up"));
            }
            break;

        case WM_LBUTTONUP:
            {
                m_stc_mouse_btn.SetWindowText(_T("left button up"));
            }
            break;

        case WM_NCMBUTTONUP:
            {
                m_stc_mouse_btn.SetWindowText(_T("nc middle button up"));
            }
            break;

        case WM_MBUTTONUP:
            {
                m_stc_mouse_btn.SetWindowText(_T("middle button up"));
            }
            break;

        case WM_NCRBUTTONUP:
            {
                m_stc_mouse_btn.SetWindowText(_T("nc right button up"));
            }
            break;

        case WM_RBUTTONUP:
            {
                m_stc_mouse_btn.SetWindowText(_T("right button up"));
            }
            break;



        }
    }
    return ::CallNextHookEx(m_thunk_hook_mouse.get_hook(), code, wParam, lParam);
}

BOOL CwindowsthunktestDlg::EnumChildProc(HWND hwnd, LPARAM lParam)
{
    if (auto wnd = CWnd::FromHandle(hwnd)) {
        CString name;
        wnd->GetWindowText(name);
        m_child_windows_names.push_back(name);
    }

    return TRUE;
}

void CwindowsthunktestDlg::TimerProc(HWND hwnd, UINT msg, UINT_PTR id, DWORD time)
{
    CString  text;
    text.Format(_T("timer x86 and x64: id %p, called at %u"), id, time);
    m_stc_timer_x86_x64.SetWindowText(text);
}



void CwindowsthunktestDlg::TimerProcWrongx64Signature(HWND hwnd, UINT msg, UINT id, DWORD time)
{
    CString  text;
    text.Format(_T("timer x86 only: id %p, called at %u"), id, time);
    m_stc_timer_x86_only.SetWindowText(text);
}

void CwindowsthunktestDlg::OnBnClickedBtnListChildWindows()
{
    m_child_windows_names.clear();
    ::EnumChildWindows(GetSafeHwnd(), m_thunk_enum_child_proc.func(), 0);
    
    CString msg = _T("Found the following child windows:");
    for (auto&& name : m_child_windows_names) {
        msg += _T('\n') + name;
    }

    AfxMessageBox(msg);


}
