#pragma once

///
/// 简易序列化/逆序列化器
///

/**
 * 用于把多个变量/对象连接成字节流便于传送/储存。
 *
 * 用法：
 * 
 *		// 初始化序列化器对象
 *		SimpleSerializer serializer;
 *
 *		// 序列化变量
 *		serializer << a << b << c;
 *
 *		// 获取序列化的字节流
 *		const char *data = nullptr;
 *		size_t length = 0;
 *		serializer.getSerialized(data, length);
 *
 *		// 清空序列化器对象
 *		serializer.clear();
 *
 *		// 逆序列化字节流
 *		serializer.setSerialized(data, length);
 *
 *		// 获取逆序列化的变量
 *		serializer >> a >> b >> c;
 */

#include <sstream>

class SimpleSerializer
{
public:
	SimpleSerializer();
	~SimpleSerializer();

	void clear();

	void getSerialized(const char * &data, size_t &length);
	void setSerialized(const char * data, size_t length);

	template<typename Type> SimpleSerializer& operator<<(const Type&);
	template<typename Type> SimpleSerializer& operator>>(Type&);

private:
	std::stringstream m_ss;
};



SimpleSerializer::SimpleSerializer() :
	m_ss(std::ios_base::binary | std::ios_base::in | std::ios_base::out)
{
}


SimpleSerializer::~SimpleSerializer()
{
}

inline void SimpleSerializer::clear()
{
	m_ss.str(std::string());
	m_ss.clear();
}

inline void SimpleSerializer::getSerialized(const char *& data, size_t & length)
{
	static std::string TmpStr;
	TmpStr = m_ss.str();
	data = TmpStr.c_str();
	length = TmpStr.length();
}

inline void SimpleSerializer::setSerialized(const char * data, size_t length)
{
	m_ss.str(std::string(data, length));
}

template<typename Type>
inline SimpleSerializer & SimpleSerializer::operator<<(const Type &obj)
{
	m_ss.write(reinterpret_cast<const char*>(&obj), sizeof(obj));
	return *this;
}

template<typename Type>
inline SimpleSerializer & SimpleSerializer::operator>>(Type &obj)
{
	m_ss.read(reinterpret_cast<char*>(&obj), sizeof(obj));
	return *this;
}
