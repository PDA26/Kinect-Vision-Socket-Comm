
// ClientDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"

#include "Protocol.h"
#include "SimpleSerializer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CClientDlg 对话框



CClientDlg::CClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLIENT_DIALOG, pParent)
	, m_bAutostartEnable(FALSE)
	, m_pTcpClient(this)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_AUTOSTART, m_bAutostartEnable);
	DDX_Control(pDX, IDC_BUTTON_CAPTURE, m_cCaptureButton);
	DDX_Control(pDX, IDC_BUTTON_ABORT, m_cAbortButton);
	DDX_Control(pDX, IDC_LIST_LOG, m_cListBox);
	DDX_Control(pDX, IDC_BUTTON_CLIENT_START, m_cBtnClntStart);
	DDX_Control(pDX, IDC_BUTTON_CLIENT_STOP, m_cBtnClntStop);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CHECK_AUTOSTART, &CClientDlg::OnBnClickedCheckAutostart)
	ON_BN_CLICKED(IDC_BUTTON_INITCAMERA, &CClientDlg::OnBnClickedButtonInitcamera)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE, &CClientDlg::OnBnClickedButtonCapture)
	ON_BN_CLICKED(IDC_BUTTON_SHUTDOWN, &CClientDlg::OnBnClickedButtonShutdown)
	ON_BN_CLICKED(IDC_BUTTON_REBOOT, &CClientDlg::OnBnClickedButtonReboot)
	ON_BN_CLICKED(IDC_BUTTON_ABORT, &CClientDlg::OnBnClickedButtonAbort)
	ON_BN_CLICKED(IDC_BUTTON_CLIENT_START, &CClientDlg::OnBnClickedButtonClientStart)
	ON_BN_CLICKED(IDC_BUTTON_CLIENT_STOP, &CClientDlg::OnBnClickedButtonClientStop)
END_MESSAGE_MAP()


// CClientDlg 消息处理程序

BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// Switch std::cout and m_ssConsoleOutput
	cout_buff = std::cout.rdbuf();
	std::cout.rdbuf(m_ssConsoleOutput.rdbuf());

	m_bAutostartEnable = theApp.QueryStartupStatus();

	OnBnClickedButtonClientStart();

	UpdateData(false);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CClientDlg::OnCancel()
{
	OnBnClickedButtonClientStop();

	UpdateLog();
	// Switch std::cout back
	std::cout.rdbuf(cout_buff);
	EndDialog(IDCLOSE);
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CClientDlg::OnPaint()
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
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CClientDlg::OnBnClickedCheckAutostart()
{
	UpdateData();
	theApp.SetStartup(m_bAutostartEnable);
	UpdateLog();
	UpdateData(false);
}


void CClientDlg::OnBnClickedButtonClientStart()
{
	Log::Info("Client starting...");
	UpdateLog();
	m_pTcpClient->Start(theApp.serverAddr, theApp.serverPort);
}


void CClientDlg::OnBnClickedButtonClientStop()
{
	Log::Info("Client stopping...");
	UpdateLog();
	m_pTcpClient->Stop();
}


void CClientDlg::OnBnClickedButtonInitcamera()
{
	m_cCaptureButton.EnableWindow(SUCCEEDED(theApp.CheckCamera()));
	UpdateLog();
	UpdateData(false);
}


void CClientDlg::OnBnClickedButtonCapture()
{
	theApp.TakePhoto();
	UpdateLog();
	UpdateData(false);
}


void CClientDlg::OnBnClickedButtonShutdown()
{
	theApp.SystemShutdown(30, false);
	m_cAbortButton.EnableWindow();
	UpdateLog();
	UpdateData(false);
}


void CClientDlg::OnBnClickedButtonReboot()
{
	theApp.SystemShutdown(30, true);
	m_cAbortButton.EnableWindow();
	UpdateLog();
	UpdateData(false);
}


void CClientDlg::OnBnClickedButtonAbort()
{
	AbortSystemShutdown(nullptr);
	m_cAbortButton.EnableWindow(false);
	UpdateLog();
	UpdateData(false);
}

/// 刷新日志显示
void CClientDlg::UpdateLog()
{
	std::string line;
	while (std::getline(m_ssConsoleOutput, line))
	{
		m_cListBox.AddString(CString(line.c_str()));
	}
	m_ssConsoleOutput.clear();
}

EnHandleResult CClientDlg::OnPrepareConnect(ITcpClient * pSender,
											CONNID dwConnID,
											SOCKET socket)
{
	Log::Info("Connecting to %ls:%hu...", theApp.serverAddr, theApp.serverPort);
	UpdateLog();
	return HR_OK;
}

EnHandleResult CClientDlg::OnConnect(ITcpClient * pSender, CONNID dwConnID)
{
	// 连接成功

	TCHAR Host[16];
	int HostLen = 16;
	USHORT Port;
	pSender->GetRemoteHost(Host, HostLen, Port);
	Log::Info("Connected to server %ls:%hu", Host, Port);

	// 更新 UI
	m_cBtnClntStart.EnableWindow(FALSE);
	m_cBtnClntStop.EnableWindow(TRUE);

	// 发送相机 ID

	namespace SP = SocketProtocol;
	SimpleSerializer serializer;
	const char * data = nullptr;
	size_t length;

	serializer << SP::Command::IDENTIFY << SP::StatCode::OK << theApp.cameraID;
	serializer.getSerialized(data, length);
	pSender->Send(reinterpret_cast<const BYTE*>(data), static_cast<int>(length));

	UpdateLog();
	return HR_OK;
}

EnHandleResult CClientDlg::OnReceive(ITcpClient * pSender,
									 CONNID dwConnID,
									 const BYTE * pData,
									 int iLength)
{
	namespace SP = SocketProtocol;
	SimpleSerializer serializer;
	serializer.setSerialized(reinterpret_cast<const char*>(pData), iLength);
	SP::Command cmd;
	serializer >> cmd;

	switch (cmd)
	{
		// 识别指令
	case SP::Command::IDENTIFY:
	{
		Log::Info("IDENTIFY command received");
		serializer.clear();

		serializer << SP::Command::IDENTIFY << SP::StatCode::OK << theApp.cameraID;

		const char * data = nullptr;
		size_t length;
		serializer.getSerialized(data, length);
		pSender->Send(reinterpret_cast<const BYTE*>(data), static_cast<int>(length));

		break;
	}
	// 关机指令
	case SP::Command::SHUTDOWN:
	{
		Log::Info("SHUTDOWN command received");
		DWORD dwDelay;
		bool bReboot;
		ASSERT(iLength >= sizeof(cmd) + sizeof(dwDelay) + sizeof(bReboot));

		serializer >> dwDelay >> bReboot;
		serializer.clear();
		serializer << cmd;

		bool ShutdownStat = theApp.SystemShutdown(dwDelay, bReboot);
		auto stat = ShutdownStat ? SP::StatCode::OK : SP::StatCode::FAIL;

		serializer << stat;

		const char * data = nullptr;
		size_t length;
		serializer.getSerialized(data, length);
		pSender->Send(reinterpret_cast<const BYTE*>(data), static_cast<int>(length));

		m_cAbortButton.EnableWindow();

		break;
	}
	// 相机状态
	case SP::Command::CAM_STAT:
	{
		Log::Info("CAM_STAT command received");
		serializer.clear();

		auto hrCode = theApp.CheckCamera();
		auto stat = SUCCEEDED(hrCode) ? SP::StatCode::OK : SP::StatCode::FAIL;
		serializer << cmd << stat << hrCode;

		const char * data = nullptr;
		size_t length;
		serializer.getSerialized(data, length);
		pSender->Send(reinterpret_cast<const BYTE*>(data), static_cast<int>(length));

		m_cCaptureButton.EnableWindow(SUCCEEDED(hrCode));

		break;
	}
	// 采图指令
	case SP::Command::CAPTURE:
	{
		Log::Info("CAPTURE command received");
		serializer.clear();

		auto hrCode = theApp.TakePhoto();
		auto stat = SUCCEEDED(hrCode) ? SP::StatCode::OK : SP::StatCode::FAIL;
		serializer << cmd << stat << hrCode;

		const char * data = nullptr;
		size_t length;
		serializer.getSerialized(data, length);
		pSender->Send(reinterpret_cast<const BYTE*>(data), static_cast<int>(length));

		break;
	}
	default:
		break;
	}
	UpdateLog();
	return HR_OK;
}

EnHandleResult CClientDlg::OnSend(ITcpClient * pSender,
								  CONNID dwConnID,
								  const BYTE * pData,
								  int iLength)
{
	// TODO: more detailed logging
	Log::Debug("Instruction sent");
	UpdateLog();
	return HR_OK;
}

EnHandleResult CClientDlg::OnClose(ITcpClient * pSender,
								   CONNID dwConnID,
								   EnSocketOperation enOperation,
								   int iErrorCode)
{
	if (iErrorCode)
	{
		Log::Error("Connection closed: Error code: %d", iErrorCode);
		UpdateLog();

		AfxBeginThread(RetryConnect, pSender);
		return HR_ERROR;
	}
	else if (pSender->GetLastError() != SE_OK)
	{
		Log::Error("%ls", pSender->GetLastErrorDesc());
		UpdateLog();

		AfxBeginThread(RetryConnect, pSender);
		return HR_ERROR;
	}
	else
	{
		TCHAR Host[16];
		int HostLen = 16;
		USHORT Port;
		pSender->GetRemoteHost(Host, HostLen, Port);
		Log::Info("Connection to server %ls:%hu closed.", Host, Port);
		UpdateLog();

		// 更新 UI
		m_cBtnClntStart.EnableWindow(TRUE);
		m_cBtnClntStop.EnableWindow(FALSE);

		AfxBeginThread(RetryConnect, pSender);
		return HR_OK;
	}
}

UINT __cdecl RetryConnect(LPVOID pITcpClient)
{
	Sleep(theApp.retryDelayMilliseconds);
	Log::Info("Retrying to connect...");
	reinterpret_cast<ITcpClient*>(pITcpClient)->Start(theApp.serverAddr, theApp.serverPort);
	return 0;
}
