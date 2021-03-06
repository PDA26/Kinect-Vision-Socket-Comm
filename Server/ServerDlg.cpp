
// ServerDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "ServerDlg.h"
#include "afxdialogex.h"

#include "Protocol.h"
#include "SimpleSerializer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 客户端列表的每列
#define LIST_COLUMN_CAM_ID   0
#define LIST_COLUMN_IP_ADDR     1
#define LIST_COLUMN_CONN_STAT   2
#define LIST_COLUMN_CAM_STAT    3

// CServerDlg 对话框

CServerDlg::CServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SERVER_DIALOG, pParent)
	, m_pTcpServer(this)
	, m_csShutdownTimeout(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LOG, m_cListBox);
	DDX_Control(pDX, IDC_LIST_CLIENTS, m_cMFCListCtrl);
	DDX_Control(pDX, IDC_BUTTON_SERVER_START, m_cBtnSrvStart);
	DDX_Control(pDX, IDC_BUTTON_SERVER_STOP, m_cBtnSrvStop);
	DDX_Text(pDX, IDC_EDIT_SHUTDOWN_TIMEOUT, m_csShutdownTimeout);
}

BEGIN_MESSAGE_MAP(CServerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SEL_ALL, &CServerDlg::OnBnClickedButtonSelAll)
	ON_BN_CLICKED(IDC_BUTTON_DESEL_ALL, &CServerDlg::OnBnClickedButtonDeselAll)
	ON_BN_CLICKED(IDC_BUTTON_CLIENT_REFRESH, &CServerDlg::OnBnClickedButtonClientRefresh)
	ON_BN_CLICKED(IDC_BUTTON_CLIENT_CAPTURE, &CServerDlg::OnBnClickedButtonClientCapture)
	ON_BN_CLICKED(IDC_BUTTON_CLIENT_SHUTDOWN, &CServerDlg::OnBnClickedButtonClientShutdown)
	ON_BN_CLICKED(IDC_BUTTON_CLIENT_REBOOT, &CServerDlg::OnBnClickedButtonClientReboot)
	ON_BN_CLICKED(IDC_BUTTON_SERVER_START, &CServerDlg::OnBnClickedButtonServerStart)
	ON_BN_CLICKED(IDC_BUTTON_SERVER_STOP, &CServerDlg::OnBnClickedButtonServerStop)
END_MESSAGE_MAP()


// CServerDlg 消息处理程序

BOOL CServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// 将 std::cout 重定向到 m_ssConsoleOutput
	cout_buff = std::cout.rdbuf();
	std::cout.rdbuf(m_ssConsoleOutput.rdbuf());

	// 设置客户端列表的列标题
	m_cMFCListCtrl.InsertColumn(LIST_COLUMN_CAM_ID, _T("相机编号"), LVCFMT_RIGHT);
	m_cMFCListCtrl.InsertColumn(LIST_COLUMN_IP_ADDR, _T("IP 地址"), LVCFMT_CENTER);
	m_cMFCListCtrl.InsertColumn(LIST_COLUMN_CONN_STAT, _T("连接状态"), LVCFMT_LEFT);
	m_cMFCListCtrl.InsertColumn(LIST_COLUMN_CAM_STAT, _T("相机状态"), LVCFMT_LEFT);

	// 设置客户端列表列宽
	m_cMFCListCtrl.SetColumnWidth(LIST_COLUMN_CAM_ID, LVSCW_AUTOSIZE_USEHEADER);
	m_cMFCListCtrl.SetColumnWidth(LIST_COLUMN_IP_ADDR, 150);
	m_cMFCListCtrl.SetColumnWidth(LIST_COLUMN_CONN_STAT, LVSCW_AUTOSIZE_USEHEADER);
	m_cMFCListCtrl.SetColumnWidth(LIST_COLUMN_CAM_STAT, LVSCW_AUTOSIZE_USEHEADER);

	// 初始化客户端列表项
	for (UINT i = 0; i < CServerApp::cameraNum; i++)
	{
		TCHAR buf[4];
		_itot_s(i + 1, buf, 4, 10);
		int nIndex = m_cMFCListCtrl.InsertItem(i, buf);
		m_cMFCListCtrl.SetItemText(nIndex, LIST_COLUMN_CONN_STAT, _T("未连接"));
		m_cMFCListCtrl.SetItemData(nIndex, 0);
	}

	// 启动服务端
	OnBnClickedButtonServerStart();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CServerDlg::OnCancel()
{
	// 关闭服务端
	OnBnClickedButtonServerStop();

	UpdateLog();
	// 将 std::cout 切换回默认
	std::cout.rdbuf(cout_buff);

	EndDialog(IDCLOSE);
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CServerDlg::OnPaint()
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
HCURSOR CServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CServerDlg::UpdateLog()
{
	std::string line;
	while (std::getline(m_ssConsoleOutput, line))
	{
		m_cListBox.AddString(CString(line.c_str()));
	}
	m_ssConsoleOutput.clear();
}

EnHandleResult CServerDlg::OnPrepareListen(ITcpServer * pSender,
										   SOCKET soListen)
{
	// 日志
	TCHAR Addr[16];
	int AddrLen = 16;
	USHORT Port;
	pSender->GetListenAddress(Addr, AddrLen, Port);
	Log::Info("Server started: Listening %ls:%hu", Addr, Port);
	UpdateLog();

	// 更新 UI
	m_cBtnSrvStart.EnableWindow(FALSE);
	m_cBtnSrvStop.EnableWindow(TRUE);

	return HR_OK;
}

EnHandleResult CServerDlg::OnAccept(ITcpServer * pSender,
									CONNID dwConnID,
									SOCKET soClient)
{
	TCHAR Addr[32];
	int AddrLen = 32;
	USHORT Port;
	if (!pSender->GetRemoteAddress(dwConnID, Addr, AddrLen, Port))
	{
		// 获取客户端 IP 地址失败
		Log::Error("Failed to get remote address: Error Code: %d", WSAGetLastError());
	}
	Log::Info("New client [%lu] connected: %ls:%hu", dwConnID, Addr, Port);
	UpdateLog();
	return HR_OK;
}

EnHandleResult CServerDlg::OnReceive(ITcpServer * pSender,
									 CONNID dwConnID,
									 const BYTE * pData,
									 int iLength)
{
	namespace SP = SocketProtocol;
	SimpleSerializer serializer;

	// 逆序列化
	serializer.setSerialized(reinterpret_cast<const char*>(pData), iLength);

	SP::Command cmd;
	SP::StatCode stat;
	serializer >> cmd >> stat;

	switch (cmd)
	{
		// 识别指令
	case SP::Command::IDENTIFY:
	{
		WORD &wCameraID = *new WORD;
		ASSERT(iLength >= sizeof(cmd) + sizeof(stat) + sizeof(wCameraID));
		serializer >> wCameraID;
		Log::Info("Client [%lu] identified as camera [%hu]", dwConnID, wCameraID);

		// 清理旧连接
		if (CONNID dwOldConnID = m_cMFCListCtrl.GetItemData(wCameraID - 1))
		{
			pSender->Disconnect(dwOldConnID);
		}
		// 设置 CameraID <-> ConnID 对应关系
		m_cMFCListCtrl.SetItemData(wCameraID - 1, dwConnID);
		pSender->SetConnectionExtra(dwConnID, &wCameraID);

		// 更新 UI
		TCHAR Addr[32];
		int AddrLen = 32;
		USHORT Port;
		if (!pSender->GetRemoteAddress(dwConnID, Addr, AddrLen, Port))
		{
			Log::Error("Error Code: %d", WSAGetLastError());
		}
		m_cMFCListCtrl.SetItemText(wCameraID - 1, LIST_COLUMN_IP_ADDR, Addr);
		m_cMFCListCtrl.SetItemText(wCameraID - 1, LIST_COLUMN_CONN_STAT, _T("已连接"));
		break;
	}
	// 关机指令
	case SP::Command::SHUTDOWN:
	{
		if (stat == SP::StatCode::OK)
		{
			Log::Info("Client [%lu] initiated shutdown", dwConnID);
		}
		else
		{
			Log::Error("Client [%lu] failed to initiate shutdown", dwConnID);
		}
		break;
	}
	// 相机状态
	case SP::Command::CAM_STAT:
	{
		HRESULT hrCode;
		ASSERT(iLength >= sizeof(cmd) + sizeof(stat) + sizeof(hrCode));
		serializer >> hrCode;

		// 获取相机 ID
		void *pwCameraID;
		pSender->GetConnectionExtra(dwConnID, &pwCameraID);
		WORD &wCameraID = *reinterpret_cast<WORD*>(pwCameraID);

		if (stat == SP::StatCode::OK)
		{
			m_cMFCListCtrl.SetItemText(wCameraID - 1, LIST_COLUMN_CAM_STAT, _T("就绪"));
			Log::Info("Camera [%hu] at client [%lu] ready", wCameraID, dwConnID);
		}
		else
		{
			m_cMFCListCtrl.SetItemText(wCameraID - 1, LIST_COLUMN_CAM_STAT, _T("错误"));
			Log::Error("Camera [%hu] at client [%lu] failed: Error Code 0x%X",
					   wCameraID,
					   dwConnID,
					   hrCode);
		}
		break;
	}
	// 采图指令
	case SP::Command::CAPTURE:
	{
		HRESULT hrCode;
		ASSERT(iLength >= sizeof(cmd) + sizeof(stat) + sizeof(hrCode));
		serializer >> hrCode;

		// 获取相机 ID
		void *pwCameraID;
		pSender->GetConnectionExtra(dwConnID, &pwCameraID);
		WORD &wCameraID = *reinterpret_cast<WORD*>(pwCameraID);

		if (stat == SP::StatCode::OK)
		{
			Log::Info("Camera [%hu] at client [%lu] capture succeeded", wCameraID, dwConnID);
		}
		else
		{
			Log::Error("Camera [%hu] at client [%lu] capture failed: Error Code 0x%X",
					   wCameraID,
					   dwConnID,
					   hrCode);
		}
		break;
	}
	default:
		break;
	}
	UpdateLog();
	return HR_OK;
}

EnHandleResult CServerDlg::OnSend(ITcpServer * pSender,
								  CONNID dwConnID,
								  const BYTE * pData,
								  int iLength)
{
	// TODO: more detailed logging
	Log::Debug("Instruction sent");
	return HR_OK;
}

EnHandleResult CServerDlg::OnShutdown(ITcpServer * pSender)
{
	Log::Info("Server shutdown.");
	UpdateLog();

	// 更新 UI
	m_cBtnSrvStart.EnableWindow(TRUE);
	m_cBtnSrvStop.EnableWindow(FALSE);

	return HR_OK;
}

EnHandleResult CServerDlg::OnClose(ITcpServer * pSender,
								   CONNID dwConnID,
								   EnSocketOperation enOperation,
								   int iErrorCode)
{
	// 获取断开的客户端的相机 ID
	void *pwCameraID;
	pSender->GetConnectionExtra(dwConnID, &pwCameraID);

	if (pwCameraID != nullptr)
	{
		WORD &wCameraID = *reinterpret_cast<WORD*>(pwCameraID);

		// 更新客户端列表
		m_cMFCListCtrl.SetItemText(wCameraID - 1, LIST_COLUMN_CONN_STAT, _T("已断开"));
		m_cMFCListCtrl.SetItemText(wCameraID - 1, LIST_COLUMN_CAM_STAT, _T(""));

		Log::Info("Connection to client [%lu] with camera [%hu] closed.", dwConnID, wCameraID);

		// 释放内存
		delete &wCameraID;
		pSender->SetConnectionExtra(dwConnID, nullptr);
	}
	else
	{
		// 客户端相机 ID 未知
		Log::Info("Connection to client [%lu] closed.", dwConnID);
	}

	UpdateLog();
	return HR_OK;
}


void CServerDlg::OnBnClickedButtonSelAll()
{
	// 全选客户端
	m_cMFCListCtrl.SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
	UpdateData(FALSE);
}


void CServerDlg::OnBnClickedButtonDeselAll()
{
	// 全不选客户端
	m_cMFCListCtrl.SetItemState(-1, 0, LVIS_SELECTED);
	UpdateData(FALSE);
}


void CServerDlg::OnBnClickedButtonClientRefresh()
{
	namespace SP = SocketProtocol;
	SimpleSerializer serializer;
	const char * data = nullptr;
	size_t length;

	serializer << SP::Command::CAM_STAT;
	serializer.getSerialized(data, length);

	// 遍历被选中的客户端
	POSITION pos = m_cMFCListCtrl.GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		int nItem = m_cMFCListCtrl.GetNextSelectedItem(pos);
		CONNID dwConnID = m_cMFCListCtrl.GetItemData(nItem);
		if (dwConnID != 0)
			m_pTcpServer->Send(dwConnID,
							   reinterpret_cast<const BYTE*>(data),
							   static_cast<int>(length));
	}
}


void CServerDlg::OnBnClickedButtonClientCapture()
{
	namespace SP = SocketProtocol;
	SimpleSerializer serializer;
	const char * data = nullptr;
	size_t length;

	serializer << SP::Command::CAPTURE;
	serializer.getSerialized(data, length);

	// 遍历被选中的客户端
	POSITION pos = m_cMFCListCtrl.GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		int nItem = m_cMFCListCtrl.GetNextSelectedItem(pos);
		CONNID dwConnID = m_cMFCListCtrl.GetItemData(nItem);
		if (dwConnID != 0)
			m_pTcpServer->Send(dwConnID,
							   reinterpret_cast<const BYTE*>(data),
							   static_cast<int>(length));
	}
}


void CServerDlg::OnBnClickedButtonClientShutdown()
{
	namespace SP = SocketProtocol;
	SimpleSerializer serializer;
	const char * data = nullptr;
	size_t length;

	// 获取关机前等待时间
	UpdateData();
	DWORD dwDelay = _ttoi(m_csShutdownTimeout);

	serializer << SP::Command::SHUTDOWN << dwDelay << false;
	serializer.getSerialized(data, length);

	// 遍历被选中的客户端
	POSITION pos = m_cMFCListCtrl.GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		int nItem = m_cMFCListCtrl.GetNextSelectedItem(pos);
		CONNID dwConnID = m_cMFCListCtrl.GetItemData(nItem);
		if (dwConnID != 0)
			m_pTcpServer->Send(dwConnID,
							   reinterpret_cast<const BYTE*>(data),
							   static_cast<int>(length));
	}
}


void CServerDlg::OnBnClickedButtonClientReboot()
{
	namespace SP = SocketProtocol;
	SimpleSerializer serializer;
	const char * data = nullptr;
	size_t length;

	UpdateData();
	DWORD dwDelay = _ttoi(m_csShutdownTimeout);

	serializer << SP::Command::SHUTDOWN << dwDelay << true;
	serializer.getSerialized(data, length);

	// 遍历被选中的客户端
	POSITION pos = m_cMFCListCtrl.GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		int nItem = m_cMFCListCtrl.GetNextSelectedItem(pos);
		CONNID dwConnID = m_cMFCListCtrl.GetItemData(nItem);
		if (dwConnID != 0)
			m_pTcpServer->Send(dwConnID,
							   reinterpret_cast<const BYTE*>(data),
							   static_cast<int>(length));
	}
}


void CServerDlg::OnBnClickedButtonServerStart()
{
	Log::Info("Starting server...");
	m_pTcpServer->Start(theApp.serverAddr, theApp.serverPort);
	UpdateLog();
}


void CServerDlg::OnBnClickedButtonServerStop()
{
	Log::Info("Stopping server...");
	m_pTcpServer->Stop();
	UpdateLog();
}
