
// passwordDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "password.h"
#include "passwordDlg.h"
#include "afxdialogex.h"
#include "DecodeEncode.h"
#include <string>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CpasswordDlg 对话框



CpasswordDlg::CpasswordDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PASSWORD_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CpasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_edit);
	DDX_Control(pDX, IDC_EDIT2, m_password);
}

BEGIN_MESSAGE_MAP(CpasswordDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZING()
	ON_COMMAND(ID_FILE_OPEN, &CpasswordDlg::OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE, &CpasswordDlg::OnFileSave)
	ON_WM_HOTKEY()
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_EDIT1, &CpasswordDlg::OnEnChangeEdit1)
END_MESSAGE_MAP()


// CpasswordDlg 消息处理程序

void CpasswordDlg::Resize()
{
	CRect rect;
	this->GetClientRect(&rect);
	CRect passClient;
	m_password.GetClientRect(&passClient);
	CRect passRect;
	static int x;
	if(x == 0)
		x = rect.Width() / 2 - passClient.Width() / 2;
	static int w;
	if(w == 0)
		w = passClient.BottomRight().x;
	passRect.SetRect(x, 0, x + w, passClient.BottomRight().y-8);
	m_password.MoveWindow(passRect);
	CRect editRect;
	editRect.SetRect(0, passClient.Height(), rect.Width(), rect.Width() - passClient.Height());
	m_edit.MoveWindow(editRect, TRUE);
}

BOOL CpasswordDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	Resize();

	RegisterHotKey(this->GetSafeHwnd(), 1001, MOD_CONTROL| MOD_NOREPEAT, 'S');
	RegisterHotKey(this->GetSafeHwnd(), 1002, MOD_CONTROL| MOD_NOREPEAT, 'O');
	//注册热键 Ctrl + F1(不重复，不会连续收到多次)
	RegisterHotKey(this->GetSafeHwnd(), 1003, MOD_CONTROL | MOD_NOREPEAT, VK_F1);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CpasswordDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CpasswordDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CpasswordDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialogEx::OnSizing(fwSide, pRect);
	Resize();
}


void CpasswordDlg::OnFileOpen()
{
	// TODO: 在此添加命令处理程序代码
	CString str;
	m_password.GetWindowText(str);
	DecodeEncode decode;
	std::string key = str.GetString();
	decode.Init((char*)key.c_str());

	BOOL isOpen = TRUE;		//是否打开(否则为保存)
	CString filter = "";	//文件过虑的类型
	CFileDialog openFileDlg(isOpen, "", "", OFN_HIDEREADONLY | OFN_READONLY, filter, NULL);
	INT_PTR result = openFileDlg.DoModal();
	if (result != IDOK) {
		return;
	}
	auto path = openFileDlg.GetPathName();
	char *fileName = (char*)path.GetString();
	FILE* file = fopen(fileName, "rb");
	if (file == nullptr)
	{
		char buf[1024];
		snprintf(buf, sizeof(buf), "无法打开文件：%s", fileName);
		MessageBox(buf);
		return;
	}
	const int bufLen = 102400;
	char buf[bufLen] = { 0 };
	auto len = fread(buf, 1, bufLen, file);
	fclose(file);

	/*std::string out;
	out.resize(strlen(buf) / 2);
	decode.ToBuf(buf, (char*)out.c_str());
	int len = strlen(buf);
	decode.DoDecodeEncode((char*)out.c_str(),out.length());
	m_edit.SetWindowText(out.c_str());*/
	decode.DoDecodeEncode(buf,len);
	m_edit.SetWindowText(buf); 
}


void CpasswordDlg::OnFileSave()
{
	// TODO: 在此添加命令处理程序代码
	CFileDialog openFileDlg(FALSE, "", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "", NULL);
	INT_PTR result = openFileDlg.DoModal();
	if (result != IDOK) {
		return;
	}
	
	CString str;
	m_password.GetWindowText(str);
	DecodeEncode decode;
	std::string key = str.GetString();
	decode.Init((char*)key.c_str());

	m_edit.GetWindowText(str);
	std::string editStr = str.GetString();

	decode.DoDecodeEncode((char*)editStr.c_str(), editStr.length());
	/*std::string outStr;
	decode.ToStr(editStr.c_str(), editStr.length(), outStr);*/

	auto path = openFileDlg.GetPathName();
	char *fileName = (char*)path.GetString();
	FILE* file = fopen(fileName, "wb");
	if (file == nullptr)
	{
		char buf[1024];
		snprintf(buf, sizeof(buf), "无法打开文件：%s", fileName);
		MessageBox(buf);
		return;
	}
	//fwrite(outStr.c_str(), outStr.length(), 1, file);
	fwrite(editStr.c_str(), editStr.length(), 1, file);
	fclose(file);
}


void CpasswordDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnHotKey(nHotKeyId, nKey1, nKey2);
	if (nKey2 == 'S')
	{
		OnFileSave();
	}
	else if (nKey2 == 'O')
	{
		OnFileOpen();
	}
}


void CpasswordDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	UnregisterHotKey(this->GetSafeHwnd(), 1001);
	UnregisterHotKey(this->GetSafeHwnd(), 1002);
	// TODO: 在此处添加消息处理程序代码
}


void CpasswordDlg::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
