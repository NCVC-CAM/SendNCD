// Thread Function
// ｼﾘｱﾙ送信
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SendNCD.h"
#include "SendNCDInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*
	本来なら SendThread() から直接 NCVC_GetNCStrData() を呼び出し
	NCｺｰﾄﾞを取得しながら送信する予定だったが，SendThread() から
	NCVC_GetNCStrData() を呼ぶと，アクセス違反のエラーになるので，
	CSendNCDInfo ｸﾗｽ構築時に m_strGcode へ登録し，それを参照する．
*/
UINT SendThread(LPVOID pParam)
{
#ifdef _DEBUG
	printf("SendThread() Start\n");
#endif
	CSendNCDInfo*	pParent = (CSendNCDInfo *)pParam;
	ASSERT(pParent);
//	NCVCHANDLE		hDoc    = pParent->m_hDoc;
	HANDLE			hCom    = pParent->m_hCom;
//	ASSERT(hDoc);
	ASSERT(hCom);
	DWORD	dwWrite;
	BOOL	fWrite;
//	char	szBuf[256];
//	int		nLen;

	::PurgeComm(hCom, PURGE_TXCLEAR);	// ﾊﾞｯﾌｧｸﾘｱ
	// 全て送信し終わるか親ｽﾚｯﾄﾞの継続ﾌﾗｸﾞが真のあいだ繰り返す
	for ( int i=0; i<pParent->m_strGcode.GetSize() && pParent->m_fThread; i++ ) {
		// NCｺｰﾄﾞ受け取り
/*
		nLen = NCVC_GetNCStrData(hDoc, i, szBuf, sizeof(szBuf));
		if ( nLen < 0 ) {
			AfxMessageBox(IDS_ERR_WRITECOM, MB_OK|MB_ICONSTOP);
			fWrite = FALSE;
			break;
		}
*/
#ifdef _DEBUG
		printf("Line=%d Gcode=%s\n", i, LPCTSTR(pParent->m_strGcode[i]));
#endif
		// ｼﾘｱﾙ送信
		fWrite = ::WriteFile(hCom, pParent->m_strGcode[i], pParent->m_strGcode[i].GetLength(),
						&dwWrite, NULL);
		if ( !fWrite || dwWrite != (DWORD)(pParent->m_strGcode[i].GetLength()) ) {
			AfxMessageBox(IDS_ERR_WRITECOM, MB_OK|MB_ICONSTOP);
			fWrite = FALSE;
			break;
		}
		// 改行
		fWrite = ::WriteFile(hCom, "\x0d\x0a", 2, &dwWrite, NULL);
		if ( !fWrite || dwWrite != 2 ) {
			AfxMessageBox(IDS_ERR_WRITECOM, MB_OK|MB_ICONSTOP);
			fWrite = FALSE;
			break;
		}
		// ﾌﾟﾛｸﾞﾚｽﾊﾞｰ
		pParent->m_ctSendProgress.SetPos(i+1);
		Sleep(0);
	}
	// EOF
	if ( fWrite ) {
		::WriteFile(hCom, "\x1a", 1, &dwWrite, NULL);
		::FlushFileBuffers(hCom);
#ifdef _DEBUG
		printf("WriteFile success\n");
#endif
	}
	else {
		DWORD	dwErrorMask;
		COMSTAT	comstat;
		::ClearCommError(hCom, &dwErrorMask, &comstat);
#ifdef _DEBUG
		printf("WriteFile NG\n");
#endif
	}
	::CloseHandle(hCom);

	// ﾙｰﾌﾟ正常終了なら
	if ( pParent->m_fThread ) {
		pParent->SendMessage(WM_CLOSE);	// pParent->DestroyWindow() ではｱｻｰｼｮﾝｴﾗｰ
#ifdef _DEBUG
		printf("SendMessage(WM_CLOSE) End\n");
#endif
	}

	return 0;	// AfxEndThread(0);
}
