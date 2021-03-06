
// ClientDlg.h: 头文件
//

#pragma once


// CClientDlg 对话框
class CClientDlg : public CDialogEx, public CTcpClientListener
{
// 构造
public:
	CClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIENT_DIALOG };
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

private:
	afx_msg void OnBnClickedCheckAutostart();
	afx_msg void OnBnClickedButtonInitcamera();
	afx_msg void OnBnClickedButtonCapture();
	afx_msg void OnBnClickedButtonShutdown();
	afx_msg void OnBnClickedButtonReboot();
	afx_msg void OnBnClickedButtonAbort();
	afx_msg void OnBnClickedButtonClientStart();
	afx_msg void OnBnClickedButtonClientStop();

	/// 开机自启动选项
	BOOL m_bAutostartEnable;

	/// 客户端通讯启动按钮
	CButton m_cBtnClntStart;
	/// 客户端通讯关闭按钮
	CButton m_cBtnClntStop;

	/// 采图按钮
	CButton m_cCaptureButton;

	/// 终止关机按钮
	CButton m_cAbortButton;

	/// 日志显示框
	CListBox m_cListBox;

	// 日志相关
	std::stringstream m_ssConsoleOutput;
	std::streambuf * cout_buff;
	void UpdateLog();

	/// 通信客户端智能指针
	CTcpClientPtr m_pTcpClient;

	// 回调函数

	// 客户端连接启动
	virtual EnHandleResult OnPrepareConnect(ITcpClient* pSender,
											CONNID dwConnID,
											SOCKET socket);
	// 连接成功
	virtual EnHandleResult OnConnect(ITcpClient* pSender,
									 CONNID dwConnID);
	// 接收到数据
	virtual EnHandleResult OnReceive(ITcpClient* pSender,
									 CONNID dwConnID,
									 const BYTE* pData,
									 int iLength);
	// 数据发送成功
	virtual EnHandleResult OnSend(ITcpClient* pSender,
								  CONNID dwConnID,
								  const BYTE* pData,
								  int iLength);
	// 连接断开
	virtual EnHandleResult OnClose(ITcpClient* pSender,
								   CONNID dwConnID,
								   EnSocketOperation enOperation,
								   int iErrorCode);

	// AFX_THREADPROC 线程用
	// 声明为 friend 以便于访问 private 成员
	friend UINT __cdecl RetryConnect(LPVOID);
};
