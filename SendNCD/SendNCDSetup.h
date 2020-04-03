// SendNCDSetup.h : ヘッダー ファイル
//

#if !defined(AFX_SENDNCDSETUP_H__215820C0_F598_42D8_A738_E2E0331EDF81__INCLUDED_)
#define AFX_SENDNCDSETUP_H__215820C0_F598_42D8_A738_E2E0331EDF81__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSendNCDSetup ダイアログ

class CSendNCDSetup : public CDialog
{
// コンストラクション
public:
	CSendNCDSetup(CWnd* pParent = NULL);   // 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CSendNCDSetup)
	enum { IDD = IDD_SENDNCD_SETUP };
	CComboBox	m_ctComPort;
	BOOL	m_bAutoRead;
	int		m_nComPort;
	//}}AFX_DATA
	CString	m_strSendCom;	// 通信ﾎﾟｰﾄ文字列

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CSendNCDSetup)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CSendNCDSetup)
	afx_msg void OnSendSetup();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SENDNCDSETUP_H__215820C0_F598_42D8_A738_E2E0331EDF81__INCLUDED_)
