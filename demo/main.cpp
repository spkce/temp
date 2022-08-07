#include "stdio.h"
#include "message.h"



void func(std::string str)
{
	printf("%s", str.c_str());
}


int main(int argc, char const *argv[])
{

	CMessage::instance()->bind("all", CMessage::EventProc_t(func));
	while (1);

	
	return 0;
}
