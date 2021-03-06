
// Client.h: PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号

// CClientApp:
// 有关此类的实现，请参阅 Client.cpp
//

class CClientApp : public CWinApp
{
public:
	CClientApp();

	static LPCTSTR const name;

// 重写
public:
	virtual BOOL InitInstance();

// 实现
public:
	void SetStartup(bool enable);
	bool QueryStartupStatus();
	HRESULT CheckCamera();
	HRESULT TakePhoto();
	bool SystemShutdown(DWORD delay, bool reboot);

	static TCHAR serverAddr[16];
	static USHORT serverPort;
	static WORD cameraID;
	static DWORD retryDelayMilliseconds;

private:
	static const std::string logFile;
	static LPCTSTR const configFile;
	static const size_t uCameraCheckMaxTry;
	KinectCamera camera;

	void LoadConfig();
	void ChangeWorkingDir();

	DECLARE_MESSAGE_MAP()
};

extern CClientApp theApp;
