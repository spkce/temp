#include "stdio.h"
#include "ctime.h"
#include "message.h"
#include <string>



void func(std::string str)
{
	printf("%s", str.c_str());
}


int main(int argc, char const *argv[])
{
	CKeyboard::instance()->init();
	while (1)
	{
	}

	
	return 0;
}
