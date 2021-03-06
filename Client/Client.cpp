
// Client.cpp: 定义应用程序的类行为。
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CClientApp

BEGIN_MESSAGE_MAP(CClientApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

const std::string CClientApp::logFile = "Client.log";
LPCTSTR const CClientApp::name = _T("XiRobotSocketClient");
LPCTSTR const CClientApp::configFile = _T(".\\Client.ini");
const size_t CClientApp::uCameraCheckMaxTry = 16;
TCHAR CClientApp::serverAddr[16] = _T("127.0.0.1");
USHORT CClientApp::serverPort = 5555;
WORD CClientApp::cameraID = 1;
DWORD CClientApp::retryDelayMilliseconds = 1000;

// CClientApp 构造

CClientApp::CClientApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
	camera.m_szColorOutput = "TestColor.png";
	camera.m_szInfraredOutput = "TestInfrared.png";
	camera.m_szDepthOutput = "TestDepth.png";
}


// 唯一的 CClientApp 对象

CClientApp theApp;


// CClientApp 初始化

BOOL CClientApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程

#ifndef DEBUG

	// 通过系统自启动时，工作目录与可执行文件目录不同
	ChangeWorkingDir();

#endif // !DEBUG

	Log::Initialise(logFile);

#ifdef DEBUG
	Log::SetThreshold(Log::Level::Debug);
#endif // DEBUG

	if (FAILED(camera.InitializeDefaultSensor()))
	{
		MessageBox(NULL, _T("初始化相机失败，请检查驱动安装是否正确"), NULL, MB_OK);
		return FALSE;
	}

	LoadConfig();

	CClientDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDCLOSE)
	{
		Log::Finalise();
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

void CClientApp::SetStartup(bool enable)
{
	LPTSTR programPath = GetCommandLine();
	HKEY hKey;
	RegOpenKeyEx(HKEY_CURRENT_USER,
				 _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
				 0,
				 KEY_WRITE,
				 &hKey);
	if (enable)
	{
		// 添加启动项
		RegSetValueEx(hKey,
					  name,
					  0,
					  REG_SZ,
					  reinterpret_cast<CONST BYTE*>(programPath),
					  static_cast<DWORD>((_tcslen(programPath) + 1) * sizeof(*programPath)));
		Log::Info("Startup set!");
	}
	else
	{
		// 删除启动项
		RegDeleteValue(hKey, name);
		Log::Info("Startup unset!");
	}
	RegCloseKey(hKey);
}

bool CClientApp::QueryStartupStatus()
{
	TCHAR buf[_MAX_PATH];
	DWORD bufLen = _MAX_PATH;
	RegGetValue(HKEY_CURRENT_USER,
				_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
				name,
				RRF_RT_REG_SZ,
				NULL,
				&buf,
				&bufLen);
	return _tcscmp(GetCommandLine(), buf) == 0;
}

HRESULT CClientApp::CheckCamera()
{
	HRESULT hr;

	for (size_t i = 0; i < uCameraCheckMaxTry; i++)
	{
		hr = camera.CheckAvailiable();
		if (SUCCEEDED(hr))
			break;
		else
			Sleep(100);
	}

	if (SUCCEEDED(hr))
		Log::Info("Camera ready");
	else
		Log::Error("Camera not available");

	return hr;
}

HRESULT CClientApp::TakePhoto()
{
	Log::Info("Taking Photo...");
	return camera.Capture();
}

bool CClientApp::SystemShutdown(DWORD delay, bool reboot)
{
   HANDLE hToken; 
   TOKEN_PRIVILEGES tkp; 
 
   // Get a token for this process. 
 
   if (!OpenProcessToken(GetCurrentProcess(), 
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
      return( FALSE ); 
 
   // Get the LUID for the shutdown privilege. 
 
   LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
        &tkp.Privileges[0].Luid); 
 
   tkp.PrivilegeCount = 1;  // one privilege to set    
   tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
 
   // Get the shutdown privilege for this process. 
 
   AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
        (PTOKEN_PRIVILEGES)NULL, 0); 
 
   if (GetLastError() != ERROR_SUCCESS) 
      return FALSE; 
 
   // Shut down the system and force all applications to close. 
 
   if (!InitiateSystemShutdown(nullptr, nullptr, delay, true, reboot)) 
      return FALSE; 

   //shutdown was successful
   return TRUE;
}

void CClientApp::LoadConfig()
{
	USES_CONVERSION;
	// 读取服务器 IP 地址
	GetPrivateProfileString(_T("Server"),
							_T("Addr"),
							serverAddr,
							serverAddr,
							sizeof(serverAddr) / sizeof(*serverAddr),
							configFile);

	// 读取服务器端口号
	serverPort = GetPrivateProfileInt(_T("Server"),
									  _T("Port"),
									  serverPort,
									  configFile);

	// 读取客户端失败重试前的等待时间
	retryDelayMilliseconds = GetPrivateProfileInt(_T("Client"),
												  _T("DelayBeforeRetry"),
												  retryDelayMilliseconds,
												  configFile);

	// 读取相机 ID
	cameraID = GetPrivateProfileInt(_T("Kinect"),
									_T("ID"),
									cameraID,
									configFile);

	// 读取相机图片输出路径
	TCHAR Path[MAX_PATH];
	GetPrivateProfileString(_T("Kinect"),
							_T("ColorFrameOutput"),
							nullptr,
							Path,
							MAX_PATH,
							configFile);
	if (_tcslen(Path) > 0)
		camera.m_szColorOutput = T2A(Path);
	GetPrivateProfileString(_T("Kinect"),
							_T("InfraredFrameOutput"),
							nullptr,
							Path,
							MAX_PATH,
							configFile);
	if (_tcslen(Path) > 0)
		camera.m_szInfraredOutput = T2A(Path);
	GetPrivateProfileString(_T("Kinect"),
							_T("DepthFrameOutput"),
							nullptr,
							Path,
							MAX_PATH,
							configFile);
	if (_tcslen(Path) > 0)
		camera.m_szDepthOutput = T2A(Path);
}

// 切换工作目录到此可执行文件的目录
void CClientApp::ChangeWorkingDir()
{
	TCHAR Path[MAX_PATH];
	GetModuleFileName(NULL, Path, MAX_PATH);
	PathRemoveFileSpec(Path);
	SetCurrentDirectory(Path);
}
