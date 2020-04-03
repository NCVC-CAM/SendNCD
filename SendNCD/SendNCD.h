// SendNCD.h : SENDNCD アプリケーションのメイン ヘッダー ファイルです。
//

#if !defined(AFX_SENDNCD_H__1D13A3CA_11A9_4714_AB0B_D6CD5AEAE8BF__INCLUDED_)
#define AFX_SENDNCD_H__1D13A3CA_11A9_4714_AB0B_D6CD5AEAE8BF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// メイン シンボル
#include "NCVCaddin.h"

/////////////////////////////////////////////////////////////////////////////
// CSendNCDApp
// このクラスの動作の定義に関しては SendNCD.cpp ファイルを参照してください。
//

class CSendNCDApp : public CWinApp
{
	int		m_nComPort;		// 通信ﾎﾟｰﾄ(のﾚｼﾞｽﾄﾘ登録順番)
	BOOL	m_bAutoRead;	// 自動挿入されたｻﾌﾞﾌﾟﾛやﾏｸﾛも送信する
	CMapStringToPtr	m_mapDCB;	// ﾎﾟｰﾄ文字列をｷｰにｼﾘｱﾙﾎﾟｰﾄ設定情報(DCB)格納

public:
	CSendNCDApp();

public:
	int		GetComPort(void) {
		return m_nComPort;
	}
	void	SetComPort(int a) {
		m_nComPort = a>=0 ? a : 0;
	}
	void	GetDCB(LPCTSTR, LPDCB);
	void	SetDCB(LPCTSTR, LPDCB);

	BOOL	GetAutoRead(void) {
		return m_bAutoRead;
	}
	void	SetAutoRead(BOOL bAutoRead) {
		m_bAutoRead = bAutoRead;
	}

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CSendNCDApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CSendNCDApp)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SENDNCD_H__1D13A3CA_11A9_4714_AB0B_D6CD5AEAE8BF__INCLUDED_)
