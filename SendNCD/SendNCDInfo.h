// SendNCDInfo.h : ヘッダー ファイル
//
#if !defined(AFX_SENDNCDINFO_H__4BA86C2B_5BC7_45AD_865E_64F158D1CD99__INCLUDED_)
#define AFX_SENDNCDINFO_H__4BA86C2B_5BC7_45AD_865E_64F158D1CD99__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ｼﾘｱﾙ送信ｽﾚｯﾄﾞ
UINT SendThread(LPVOID);

/////////////////////////////////////////////////////////////////////////////
// CSendNCDInfo ダイアログ

class CSendNCDInfo : public CDialog
{
// ｼﾘｱﾙ送信ｽﾚｯﾄﾞは友達
friend UINT SendThread(LPVOID);

	NCVCHANDLE	m_hDoc;		// NCVCﾊﾝﾄﾞﾙ
	HANDLE		m_hCom;		// ｼﾘｱﾙﾎﾟｰﾄOPENﾊﾝﾄﾞﾙ
	CWinThread*	m_pThread;	// ｼﾘｱﾙ送信ｽﾚｯﾄﾞﾎﾟｲﾝﾀ
	BOOL		m_fThread;	// ｼﾘｱﾙ送信ｽﾚｯﾄﾞ継続ﾌﾗｸﾞ

/*
	本来なら SendThread() から直接 NCVC_GetNCStrData() を呼び出し
	NCｺｰﾄﾞを取得しながら送信する予定だったが，SendThread() から
	NCVC_GetNCStrData() を呼ぶと，アクセス違反のエラーになるので，
	CSendNCDInfo ｸﾗｽ構築時に m_strGcode へ登録
	(処理途中にﾄﾞｷｭﾒﾝﾄが閉じられても困るしネ)
*/
	CStringArray	m_strGcode;	// NCｺｰﾄﾞ

// コンストラクション
public:
	CSendNCDInfo(NCVCHANDLE, LPCTSTR);   // 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CSendNCDInfo)
	enum { IDD = IDD_SENDNCD };
	CProgressCtrl	m_ctSendProgress;
	CAnimateCtrl	m_ctSendAvi;
	CString	m_strFileName;
	CString	m_strComPort;
	//}}AFX_DATA

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CSendNCDInfo)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CSendNCDInfo)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	virtual void OnCancel();
	//}}AFX_MSG
	afx_msg LRESULT OnNcHitTest(CPoint);	// ｸﾗｲｱﾝﾄ領域でｳｨﾝﾄﾞｳをﾄﾞﾗｯｸﾞ
	
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SENDNCDINFO_H__4BA86C2B_5BC7_45AD_865E_64F158D1CD99__INCLUDED_)
