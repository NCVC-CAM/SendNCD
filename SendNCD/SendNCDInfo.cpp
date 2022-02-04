// SendNCDInfo.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "SendNCD.h"
#include "SendNCDInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const DWORD	WRITETIMEOUTMULTIPLIER = 10;		// 1ﾊﾞｲﾄ当たりの処理時間(乗数)
static const DWORD	WRITETIMEOUTCONSTANT   = 100;		// 最低ﾀｲﾑｱｳﾄしきい値
static const DWORD	SENDCANCELTIMEOUT      = 5000;		// Cancelﾎﾞﾀﾝ押したときのｽﾚｯﾄﾞ待ち時間

BEGIN_MESSAGE_MAP(CSendNCDInfo, CDialog)
	//{{AFX_MSG_MAP(CSendNCDInfo)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSendNCDInfo ダイアログ

CSendNCDInfo::CSendNCDInfo(NCVCHANDLE hDoc, LPCTSTR lpszCom)
	: CDialog(CSendNCDInfo::IDD)
{
	//{{AFX_DATA_INIT(CSendNCDInfo)
	//}}AFX_DATA_INIT
	m_hDoc = hDoc;
	TCHAR	szPath[_MAX_PATH];
	if ( NCVC_GetDocumentFileName(hDoc, szPath, _MAX_PATH) > 0 )
		m_strFileName = ::PathFindFileName(szPath);		// Shell Utility API
	// ｼﾘｱﾙﾎﾟｰﾄの名前文字列
	m_strComPort = lpszCom;

	m_hCom    = NULL;
	m_pThread = NULL;
	m_fThread = FALSE;
}

void CSendNCDInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendNCDInfo)
	DDX_Control(pDX, IDC_SENDNCD_PROGRESS, m_ctSendProgress);
	DDX_Control(pDX, IDC_SENDNCD_AVI, m_ctSendAvi);
	DDX_Text(pDX, IDC_SENDNCD_FILENAME, m_strFileName);
	DDX_Text(pDX, IDC_SENDNCD_COMPORT, m_strComPort);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CSendNCDInfo メッセージ ハンドラ

int CSendNCDInfo::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// NCｺｰﾄﾞの取得
#ifdef _DEBUG
	printf("CSendNCDInfo::OnCreate() NCVC_GetNCStrData() start\n");
#endif
	m_strGcode.SetSize(0, 1024);
	char	szBuf[256];
	int		nLen, nMax = NCVC_GetNCBlockDataSize(m_hDoc);
	DWORD	dwFlag = AfxGetSendNCDApp()->GetAutoRead() ? 0 : NCF_AUTOREAD;

	try {
		for ( int i=0; i<nMax; i++ ) {
			if ( dwFlag & NCVC_GetNCBlockFlag(m_hDoc, i) )
				continue;
			nLen = NCVC_GetNCBlockData(m_hDoc, i, szBuf, sizeof(szBuf));
			if ( nLen < 0 ) {
				AfxMessageBox(IDS_ERR_GETNCCODE, MB_OK|MB_ICONSTOP);
				return -1;
			}
			m_strGcode.Add(szBuf);
		}
	}
	catch (CMemoryException* e) {
		e->Delete();
		AfxMessageBox(IDS_ERR_OUTOFMEM, MB_OK|MB_ICONSTOP);
		return -1;
	}

	// ｼﾘｱﾙﾎﾟｰﾄのｵｰﾌﾟﾝ
#ifdef _DEBUG
	printf("OpenCom = %s\n", LPCTSTR(m_strComPort));
#endif
	if ( (m_hCom=::CreateFile("\\\\.\\"+m_strComPort, GENERIC_WRITE, 0, NULL,
					OPEN_EXISTING, NULL, NULL)) == INVALID_HANDLE_VALUE ) {
		AfxMessageBox(IDS_ERR_OPENCOM, MB_OK|MB_ICONSTOP);
		return -1;
	}
	// ｼﾘｱﾙﾎﾟｰﾄｴﾗｰﾘｾｯﾄ
#ifdef _DEBUG
	printf("ClearCommError start\n");
#endif
	DCB		dcb;
	DWORD	dwErrorMask;
	COMSTAT	comstat;
	::ClearCommError(m_hCom, &dwErrorMask, &comstat);
	// ｼﾘｱﾙﾎﾟｰﾄの設定
#ifdef _DEBUG
	printf("SetCommState start\n");
#endif
	AfxGetSendNCDApp()->GetDCB(m_strComPort, &dcb);
	if ( !::SetCommState(m_hCom, &dcb) ) {
		::CloseHandle(m_hCom);
		AfxMessageBox(IDS_ERR_SETCOM, MB_OK|MB_ICONSTOP);
		return -1;
	}
	// ｼﾘｱﾙﾎﾟｰﾄﾀｲﾑｱｳﾄ設定
#ifdef _DEBUG
	printf("SetCommTimeouts start\n");
#endif
	COMMTIMEOUTS	ct;
	::ZeroMemory(&ct, sizeof(COMMTIMEOUTS));
	ct.WriteTotalTimeoutMultiplier = WRITETIMEOUTMULTIPLIER;	// 1ﾊﾞｲﾄ当たりの処理時間(乗数)
	ct.WriteTotalTimeoutConstant   = WRITETIMEOUTCONSTANT;		// 最低ﾀｲﾑｱｳﾄしきい値
	if ( !::SetCommTimeouts(m_hCom, &ct) ) {
		::CloseHandle(m_hCom);
		AfxMessageBox(IDS_ERR_SETTIMEOUT, MB_OK|MB_ICONSTOP);
		return -1;
	}

	return 0;
}

BOOL CSendNCDInfo::OnInitDialog() 
{
	CDialog::OnInitDialog();
	CenterWindow();

	// ﾌﾟﾛｸﾞﾚｽﾊﾞｰ設定
#ifdef _DEBUG
	printf("CSendNCDInfo::OnInitDialog() m_nStrCnt=%Id\n", m_strGcode.GetSize()+1);
#endif
	m_ctSendProgress.SetRange32(0, m_strGcode.GetSize());
	m_ctSendProgress.SetPos(0);
	// ｱﾆﾒｰｼｮﾝｺﾝﾄﾛｰﾙ
	m_ctSendAvi.Open(IDC_SENDNCD_AVI);
	// ｼﾘｱﾙﾎﾟｰﾄ送信ｽﾚｯﾄﾞ生成
	m_fThread = TRUE;
	m_pThread = AfxBeginThread(SendThread, this);

	return TRUE;
}

LRESULT CSendNCDInfo::OnNcHitTest(CPoint pt)
{
	LRESULT nHitTest = CDialog::OnNcHitTest(pt);
	if ( nHitTest==HTCLIENT && ::GetAsyncKeyState(MK_LBUTTON)<0 )
		nHitTest = HTCAPTION;
	return nHitTest;
}

void CSendNCDInfo::PostNcDestroy() 
{
#ifdef _DEBUGOLD
	printf("CSendNCDInfo::PostNcDestroy() Start\n");
#endif
	delete	this;
	CDialog::PostNcDestroy();
}

void CSendNCDInfo::OnDestroy() 
{
#ifdef _DEBUGOLD
	printf("CSendNCDInfo::OnDestroy() Start\n");
#endif
	m_strGcode.RemoveAll();
	CDialog::OnDestroy();
}

void CSendNCDInfo::OnCancel() 
{
#ifdef _DEBUG
	printf("CSendNCDInfo::OnCancel() Start\n");
#endif
	if ( m_pThread ) {
		HANDLE hThread = m_pThread->m_hThread;
		m_fThread = FALSE;
		if ( ::WaitForSingleObject(hThread, SENDCANCELTIMEOUT) == WAIT_TIMEOUT ) {
			::TerminateThread(hThread, 0);
			if ( !m_hCom )
				::CloseHandle(m_hCom);
			AfxMessageBox(IDS_ERR_SENDTIMEOUT, MB_OK|MB_ICONWARNING);
		}
	}
// ﾓｰﾄﾞﾚｽのため基本ﾒﾝﾊﾞは呼び出さない
//	CDialog::OnCancel();
	DestroyWindow();
}
