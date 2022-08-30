#include "stdio.h"
#include "ctime.h"
#include "message.h"
#include <string>

std:: string input;

void func1(std::string str)
{
	printf("%s\n", str.c_str());
	input += str;
}

void func2(std::string str)
{
	input.pop_back();
}

void func3(std::string str)
{
	printf("%s\n", input.c_str());
}

int main(int argc, char const *argv[])
{
	CKeyboard::instance()->init();
	CKeyboard::instance()->attach("all", func1);
	CKeyboard::instance()->attach("DEL", func2);
	CKeyboard::instance()->attach("GO", func3);
	Infra::CTime::delay_ms(1000);
	std::string str = "keyborad";
	CKeyboard::instance()->send(str.c_str(), str.length());
	str = "SHOW";
	CKeyboard::instance()->send(str.c_str(), str.length());
	while (1);


	
	return 0;
}
