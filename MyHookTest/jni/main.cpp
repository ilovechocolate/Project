/*===============================================================
*   Copyright (C) 2016 All rights reserved.
*
*   文件名称：main.cpp
*   创 建 者：genglei.cuan@godinsec.com
*   创建日期：2016年05月16日
*   描    述：测试使用
*
*   更新日志：
*
================================================================*/
#include <elf.h>
#include "utils.h"
#include "godin_elf_hook.h"
#include "hook_module.h"



using namespace GodinHook;


typedef int(*mywrite)(int,const void *,size_t);



static mywrite backFunction;
mywrite mmyfun1 = write;//================================此全局变量类型的函数指针可以被hook


/// hook C方法的话，最好按照这样的方式
extern "C"{

static int myprintf(int fd,const void *buf,size_t size){

  char buff[128] = {0};
  char * tmp = " hooked !!!\n";
  memcpy(buff,buf,size-1);
  memcpy(buff+size-1,tmp,strlen(tmp));
  if(NULL != backFunction)
    return backFunction(fd,buff,strlen(buff));
  else
    return -1;
}

}

int main(int argc, char *argv[])
{
  //gcc编译未经优化时，即添加-O0参数时，hook不能百分百生效，一旦为-O1以上，侧可以被hook
  mywrite mmyfun2 = write;//=============gcc编译未经优化时===============在调用hook框架之前的局部变量类型的函数指针不能hook
  char *buf = "hello\n";

 GodinELfHook::registeredElfModule("/data/local/tmp/test");

 printf("------original---------\n");
 write(1,buf,strlen(buf));
 mmyfun1(1,buf,strlen(buf));
 mmyfun2(1,buf,strlen(buf));

 if(1 == GodinELfHook::registeredElfModule("/data/local/tmp/test"))
   printf("this module is allready registered!!!\n");

 //GodinELfHook::hook("/data/local/tmp/test","write",(void*)myprintf,(void**)&backFunction);
 GodinELfHook::hookAllRegisteredModule("write",(void*)myprintf,(void**)&backFunction);
 mywrite mmyfun3 = write;//============gcc编译未经优化时===================在调用hook框架之后的局部变量类型的函数指针可以被hook，但是unhook会失效
 printf("------hook---------\n");
 write(1,buf,strlen(buf));    //================ OK
 mmyfun1(1,buf,strlen(buf));  //================ OK
 mmyfun2(1,buf,strlen(buf));  //================ FALIED
 mmyfun3(1,buf,strlen(buf));  //================ OK
 printf("------call original ---------\n");
 backFunction(1,buf,strlen(buf));

 GodinELfHook::unHook("/data/local/tmp/test","write",(void*)backFunction);
 GodinELfHook::unHookAllRegisteredModule("write",(void*)backFunction);
 mywrite mmyfun4 = write;//=============在调用unhook框架之后的局部变量类型的函数指针可以被unhook
 printf("------unhook and call original---------\n");
 write(1,buf,strlen(buf));      //================ OK
 mmyfun1(1,buf,strlen(buf));    //================ OK
 mmyfun2(1,buf,strlen(buf));    //================ OK
 mmyfun3(1,buf,strlen(buf));    //===gcc编译未经优化时============= FALIED
 mmyfun4(1,buf,strlen(buf));    //================ OK

 return 0;
}
