#ifndef __LOG_H__
#define __LOG_H__
#include <string>
#include <map>
#include "thread.h"

namespace Infra
{

void print_backtrace();

/*
字背景颜色范围: 40--49                   字颜色: 30—39             
 40: 黑                           30: 黑                 
 41: 红                           31: 红                
 42: 绿                           32: 绿                
 43: 黄                           33: 黄                
 44: 蓝                           34: 蓝               
 45: 紫                           35: 紫                 
 46: 深绿                       36: 深绿                 
 47: 白色                       37: 白色
ANSI控制码:
 QUOTE: 
   \033[0m   关闭所有属性      
   \033[1m   设置高亮度      
   \033[4m   下划线      
   \033[5m   闪烁      
   \033[7m   反显      
  \033[8m   消隐    
  \033[30m   --   \033[37m   设置前景色     
  \033[40m   --   \033[47m   设置背景色      
  \033[nA   光标上移n行     
  \03[nB   光标下移n行      
  \033[nC   光标右移n行     
  \033[nD   光标左移n行
*/
enum emFontColor
{
	Font_black = 30,
	Font_red,
	Font_green,
	Font_yellow,
	Font_blue,
	Font_violet,
	Font_darkGreen,
	Font_white
};

enum emBackgroundColor
{
	background_black = 40,
	background_red,
	background_green,
	background_yellow,
	background_blue,
	background_violet,
	background_darkGreen,
	background_white
};

// archetype:为按照那种风格进行校验，如printf/scanf等
// string-index:格式化format字符串所在的位置,如void test(testA, format,...)，此时为2
// first-to-check:第一个可变参数的位置，如void test(testA, format,...)，此时为3
//__attribute__((format(archetype, string-index, first-to-check)))

void exprintf(int fc, int bc, const char* fmt, ...) __attribute__((format(printf, 3, 4)));
void exprintf(int fc, const char* fmt, ...) __attribute__((format(printf, 2, 3)));

class CLog
{
public:
	enum emLogLevel
	{
		logLevel_0, //关闭所有打印
		logLevel_1,
		logLevel_2,
		logLevel_3,
		logLevel_4,
		logLevel_5
	};
	enum emLogType
	{
		type_onlyLog = 0,
		type_fileMsg,
		type_modMsg,
		type_modVer
	};
public:
	CLog(std::string name, std::string ver, int type = type_onlyLog);
	~CLog();
	int setLevel(int lv);
	int setType(int type);
	void setColor(bool isOn);
	int getLevel() const;
	int getType() const;
	bool isColorOn() const;
	std::string getName();
	std::string getVer();

	//lv5
	void _info(const char* file, int line, const char* func, const char* fmt, ...) __attribute__((format(printf, 5, 6)));
	//lv4
	void _debug(const char* file, int line, const char* func, const char* fmt, ...) __attribute__((format(printf, 5, 6)));
	//lv3
	void _trace(const char* file, int line, const char* func, const char* fmt, ...) __attribute__((format(printf, 5, 6)));
	//lv2
	void _warning(const char* file, int line, const char* func, const char* fmt, ...) __attribute__((format(printf, 5, 6)));
	//lv1
	void _error(const char* file, int line, const char* func, const char* fmt, ...) __attribute__((format(printf, 5, 6)));
	
	#define info(fmt, ...) _info(__FILE__, __LINE__, __FUNCTION__, (fmt), ## __VA_ARGS__)
	#define debug(fmt, ...) _debug(__FILE__, __LINE__, __FUNCTION__, (fmt), ## __VA_ARGS__)
	#define trace(fmt, ...) _trace(__FILE__, __LINE__, __FUNCTION__, (fmt), ## __VA_ARGS__)
	#define warning(fmt, ...) _warning(__FILE__, __LINE__, __FUNCTION__, (fmt), ## __VA_ARGS__)
	#define error(fmt, ...) _error(__FILE__, __LINE__, __FUNCTION__, (fmt), ## __VA_ARGS__)
private:
	int m_type;
	int m_level;
	std::string m_name;
	std::string m_ver;
	bool m_isColorOn;
};

class CLogManager
{
private:
	CLogManager();
	~CLogManager();
public:
	static CLogManager* instance();
	
	CLog* getLog(std::string name);
	CLog* findLog(std::string name);
	void setLevel(int lv);
	void setType(int type);
	void setColor(bool isOn);

private:
	Infra::CRwlock m_rwlock;
	std::map<std::string, CLog*> m_mapLog;
	int m_type;
	int m_level;
	bool m_isColorOn;
};

#define Info(name, fmt, ...) CLogManager::instance()->getLog(name)->_info(__FILE__, __LINE__, __FUNCTION__, (fmt), ## __VA_ARGS__)
#define Debug(name, fmt, ...) CLogManager::instance()->getLog(name)->_debug(__FILE__, __LINE__, __FUNCTION__, (fmt), ## __VA_ARGS__)
#define Trace(name, fmt, ...) CLogManager::instance()->getLog(name)->_trace(__FILE__, __LINE__, __FUNCTION__, (fmt), ## __VA_ARGS__)
#define Warning(name, fmt, ...) CLogManager::instance()->getLog(name)->_warning(__FILE__, __LINE__, __FUNCTION__, (fmt), ## __VA_ARGS__)
#define Error(name, fmt, ...) CLogManager::instance()->getLog(name)->_error(__FILE__, __LINE__, __FUNCTION__, (fmt), ## __VA_ARGS__)

}//Infra

#endif //__LOG_H__
