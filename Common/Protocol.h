#pragma once

/// 通信协议

/**
 * 指令格式：
 *
 *		偏移量		长度		类型
 *		0			1			SocketProtocol::Command
 *		1			1			SocketProtocol::StatCode
 *		2			可变		自定义（可无）
 *
 */

namespace SocketProtocol
{
	enum class Command : uint8_t
	{
		IDENTIFY,
		SHUTDOWN,
		CAM_STAT,
		CAPTURE,
	};

	enum class StatCode : uint8_t
	{
		OK,
		FAIL,
	};
}

inline std::ostream& operator<<(std::ostream& os, SocketProtocol::Command cmd)
{
	switch (cmd)
	{
	case SocketProtocol::Command::IDENTIFY:
		os << "IDENTIFY";
		break;
	case SocketProtocol::Command::SHUTDOWN:
		os << "SHUTDOWN";
		break;
	case SocketProtocol::Command::CAM_STAT:
		os << "CAM_STAT";
		break;
	case SocketProtocol::Command::CAPTURE:
		os << "CAPTURE";
		break;
	default:
		os.setstate(std::ios_base::failbit);
		break;
	}
	return os;
}

inline std::ostream& operator<<(std::ostream& os, SocketProtocol::StatCode stat)
{
	switch (stat)
	{
	case SocketProtocol::StatCode::OK:
		os << "OK";
		break;
	case SocketProtocol::StatCode::FAIL:
		os << "FAIL";
		break;
	default:
		os.setstate(std::ios_base::failbit);
		break;
	}
	return os;
}