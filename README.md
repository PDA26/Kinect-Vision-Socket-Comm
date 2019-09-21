# XiVisionSocketComm

## 项目介绍
行健视觉 Socket 通信

## 软件架构

服务端运行于工控机，每个客户端连接一个 Kinect 相机；
客户端与服务端之间通过 socket 通信。

## 编译说明

### 依赖项

1. [HP-Socket](https://github.com/ldcsaa/HP-Socket)
1. [Log](https://github.com/Sam0523/Log)
1. [KinectCamera](https://gitee.com/Sam0523/KinectCamera)
1. [Kinect SDK](https://www.microsoft.com/en-us/download/details.aspx?id=44561)
1. [OpenCV](https://opencv.org/)

安装 Kinect SDK 与 OpenCV ，并将 HP-Socket, Log, KinectCamera 放在与本项目同一目录下并分别编译（或修改 `.props` 文件指向正确的目录），
然后编译本项目。
