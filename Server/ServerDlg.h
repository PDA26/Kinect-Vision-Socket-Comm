
// ServerDlg.h: 头文件
//

#pragma once


// CServerDlg 对话框
class CServerDlg : public CDialogEx, public CTcpServerListener
{
// 构造
public:
	CServerDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedButtonSelAll();
	afx_msg void OnBnClickedButtonDeselAll();
	afx_msg void OnBnClickedButtonClientRefresh();
	afx_msg void OnBnClickedButtonClientCapture();
	afx_msg void OnBnClickedButtonClientShutdown();
	afx_msg void OnBnClickedButtonClientReboot();
	afx_msg void OnBnClickedButtonServerStart();
	afx_msg void OnBnClickedButtonServerStop();

private:
	/// 日志显示框
	CListBox m_cListBox;
	/// 客户端列表显示框
	CMFCListCtrl m_cMFCListCtrl;
	/// 客户端关机/重启前等待时间
	CString m_csShutdownTimeout;
	/// 服务端启动按钮
	CButton m_cBtnSrvStart;
	/// 服务端关闭按钮
	CButton m_cBtnSrvStop;

	// 日志相关
	std::streambuf * cout_buff;
	std::stringstream m_ssConsoleOutput;
	void UpdateLog();

	// 通信服务端智能指针
	CTcpServerPtr m_pTcpServer;

	// 回调函数

	/// 服务端启动成功
	virtual EnHandleResult OnPrepareListen(ITcpServer* pSender,
										   SOCKET soListen);
	/// 新客户端连接
	virtual EnHandleResult OnAccept(ITcpServer* pSender,
									CONNID dwConnID,
									SOCKET soClient);
	/// 接收到数据
	virtual EnHandleResult OnReceive(ITcpServer* pSender,
									 CONNID dwConnID,
									 const BYTE* pData,
									 int iLength);
	/// 数据发送成功
	virtual EnHandleResult OnSend(ITcpServer* pSender,
								  CONNID dwConnID,
								  const BYTE* pData,
								  int iLength);
	/// 服务端关闭成功
	virtual EnHandleResult OnShutdown(ITcpServer* pSender);
	/// 客户端断开连接
	virtual EnHandleResult OnClose(ITcpServer* pSender,
								   CONNID dwConnID,
								   EnSocketOperation enOperation,
								   int iErrorCode);
};
