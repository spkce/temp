## 1. 如何使用虚拟键盘使用
虚拟键盘作为独立进程，需要首先启动。
需要接受虚拟键盘按键消息的进程使用类CKeyboard接受按键消息。

## 2. 如何使用CKeyboard接收按键消息
CKeyboard使用前首先需要初始化
```c++
CKeyboard::instance()->init();
```
然后注册事件回调函数
```c++
CKeyboard::instance()->attach("all", func1);
CKeyboard::instance()->attach("DEL", func2);
CKeyboard::instance()->attach("GO", func3);
```
DEL为退格键的按键事件，GO为回车键事件， all为其他按键键事件

## 3. 主动发送消息给虚拟键盘
使用send接口发送事件
```c++
str = "SHOW";
CKeyboard::instance()->send(str.c_str(), str.length());
```
注意：在完成初始化后需要发送一个事件，虚拟键盘中进行会话注册
demo中的keyborad事件的用处也就时如此
```c++
std::string str = "keyborad";
CKeyboard::instance()->send(str.c_str(), str.length());
```