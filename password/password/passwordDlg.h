
// passwordDlg.h: 头文件
//

#pragma once

#include <string>

// CpasswordDlg 对话框
class CpasswordDlg : public CDialogEx
{
// 构造
public:
	CpasswordDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PASSWORD_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_edit;
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
	afx_msg void OnDestroy();
public:
	afx_msg void OnEnChangeEdit1();
private:
	CEdit m_password;
	void Resize();
};
