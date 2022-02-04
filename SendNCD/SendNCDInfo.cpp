// SendNCDInfo.cpp : �C���v�������e�[�V���� �t�@�C��
//

#include "stdafx.h"
#include "SendNCD.h"
#include "SendNCDInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const DWORD	WRITETIMEOUTMULTIPLIER = 10;		// 1�޲ē�����̏�������(�搔)
static const DWORD	WRITETIMEOUTCONSTANT   = 100;		// �Œ���ѱ�Ă������l
static const DWORD	SENDCANCELTIMEOUT      = 5000;		// Cancel���݉������Ƃ��̽گ�ޑ҂�����

BEGIN_MESSAGE_MAP(CSendNCDInfo, CDialog)
	//{{AFX_MSG_MAP(CSendNCDInfo)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSendNCDInfo �_�C�A���O

CSendNCDInfo::CSendNCDInfo(NCVCHANDLE hDoc, LPCTSTR lpszCom)
	: CDialog(CSendNCDInfo::IDD)
{
	//{{AFX_DATA_INIT(CSendNCDInfo)
	//}}AFX_DATA_INIT
	m_hDoc = hDoc;
	TCHAR	szPath[_MAX_PATH];
	if ( NCVC_GetDocumentFileName(hDoc, szPath, _MAX_PATH) > 0 )
		m_strFileName = ::PathFindFileName(szPath);		// Shell Utility API
	// �ر��߰Ă̖��O������
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
// CSendNCDInfo ���b�Z�[�W �n���h��

int CSendNCDInfo::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// NC���ނ̎擾
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

	// �ر��߰Ă̵����
#ifdef _DEBUG
	printf("OpenCom = %s\n", LPCTSTR(m_strComPort));
#endif
	if ( (m_hCom=::CreateFile("\\\\.\\"+m_strComPort, GENERIC_WRITE, 0, NULL,
					OPEN_EXISTING, NULL, NULL)) == INVALID_HANDLE_VALUE ) {
		AfxMessageBox(IDS_ERR_OPENCOM, MB_OK|MB_ICONSTOP);
		return -1;
	}
	// �ر��߰Ĵװؾ��
#ifdef _DEBUG
	printf("ClearCommError start\n");
#endif
	DCB		dcb;
	DWORD	dwErrorMask;
	COMSTAT	comstat;
	::ClearCommError(m_hCom, &dwErrorMask, &comstat);
	// �ر��߰Ă̐ݒ�
#ifdef _DEBUG
	printf("SetCommState start\n");
#endif
	AfxGetSendNCDApp()->GetDCB(m_strComPort, &dcb);
	if ( !::SetCommState(m_hCom, &dcb) ) {
		::CloseHandle(m_hCom);
		AfxMessageBox(IDS_ERR_SETCOM, MB_OK|MB_ICONSTOP);
		return -1;
	}
	// �ر��߰���ѱ�Đݒ�
#ifdef _DEBUG
	printf("SetCommTimeouts start\n");
#endif
	COMMTIMEOUTS	ct;
	::ZeroMemory(&ct, sizeof(COMMTIMEOUTS));
	ct.WriteTotalTimeoutMultiplier = WRITETIMEOUTMULTIPLIER;	// 1�޲ē�����̏�������(�搔)
	ct.WriteTotalTimeoutConstant   = WRITETIMEOUTCONSTANT;		// �Œ���ѱ�Ă������l
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

	// ��۸�ڽ�ް�ݒ�
#ifdef _DEBUG
	printf("CSendNCDInfo::OnInitDialog() m_nStrCnt=%Id\n", m_strGcode.GetSize()+1);
#endif
	m_ctSendProgress.SetRange32(0, m_strGcode.GetSize());
	m_ctSendProgress.SetPos(0);
	// ��Ұ��ݺ��۰�
	m_ctSendAvi.Open(IDC_SENDNCD_AVI);
	// �ر��߰đ��M�گ�ސ���
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
// Ӱ��ڽ�̂��ߊ�{���ނ͌Ăяo���Ȃ�
//	CDialog::OnCancel();
	DestroyWindow();
}
