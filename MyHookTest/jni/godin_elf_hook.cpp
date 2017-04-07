/*===============================================================
*   Copyright (C) 2016 All rights reserved.
*
*   文件名称：godin_elf_hook.cpp
*   创 建 者：genglei.cuan@godinsec.com
*   创建日期：2016年05月16日
*   描    述：hook框架对外API
*
*   更新日志：
*
================================================================*/
#include "godin_elf_hook.h"
#include <string.h>
#include "utils.h"


GodinHook::GodinELfHook::ModuleMap GodinHook::GodinELfHook::module_map_;


bool GodinHook::GodinELfHook::isRegisteredModule(string name)
{
  ModuleMap::iterator it = module_map_.find(name);
  if(it == module_map_.end())
    return false;
  else
    return true;
}

GodinHook::GodinELfHook::RETINFO GodinHook::GodinELfHook::registeredElfModule(string name,pid_t pid=0)
{
  /// 判断是否已经注册
  if(isRegisteredModule(name))
    return EFL_MODULE_ALLREADY_REGISTERED;
  /// 计算moudle在进程中加载的基地址
  size_t baseAddr = Utils::getModuleBaseAddress(name.c_str(),pid);
  if(0 == baseAddr)
    return NOT_FIND_ELFMODULE;
  /// 判断baseAddr处是否是一个合法的ELF文件
  if(!isElfFile(baseAddr))
    return NOT_ELF_FILE;
  /// 创建HookModule
  HookModule * module = new HookModule(name,baseAddr);
  if(NULL == module)
    return MEM_ERR;
  /// 初始化Module
  if(!module->initModule())
    return ELF_MODULE_INIT_ERR;
  /// 注册Module
  module_map_.insert(ModuleMap::value_type(name,module));
  /// 修改module 状态
  module->setIsRegistered(true);

  return GODIN_OK;

}

bool GodinHook::GodinELfHook::isHookedInModule(string moduleName, string hookSymbol)
{
  if(!isRegisteredModule(moduleName))
      return false;
  HookModule * module = getHookModule(moduleName);
  return module->isSymbolAllreadyHooked(hookSymbol);

}

GodinHook::GodinELfHook::RETINFO GodinHook::GodinELfHook::hook(string moduleName, string hookSymbol,void *newFunc, void **backFunc)
{
  /// 模块是否已经被注册
  if(!isRegisteredModule(moduleName))
    return EFL_MODULE_NOT_REGISTERRD;

  /// symbol是否已经被hook
  if(isHookedInModule(moduleName,hookSymbol))
    return ALLREADY_HOOKED;

  /// 开始hook
  HookModule * module = getHookModule(moduleName);
  if(module->hook(hookSymbol.c_str(),newFunc,backFunc)){
    module->addHookedSymbol(hookSymbol);
    return HOOKED;
  }else
    return HOOKED_ERR;
}

GodinHook::GodinELfHook::RETINFO GodinHook::GodinELfHook::hook(GodinHook::HookModule *module, string hookSymbol, void *newFunc, void **backFunc)
{
  /// symbol是否已经被hook
  if(module->isSymbolAllreadyHooked(hookSymbol))
    return ALLREADY_HOOKED;
  /// 开始hook
  if(module->hook(hookSymbol.c_str(),newFunc,backFunc)){
    module->addHookedSymbol(hookSymbol);
    return HOOKED;
  }else
    return HOOKED_ERR;
}

GodinHook::GodinELfHook::RETINFO GodinHook::GodinELfHook::unhook(GodinHook::HookModule *module, string hookSymbol, void *backFunc)
{
  /// symbol是否已经被hook
  if(!module->isSymbolAllreadyHooked(hookSymbol))
    return NOT_HOOKED;
  /// 开始unhook
  if(module->unhook(hookSymbol.c_str(),backFunc)){
    return GODIN_OK;
  }else
    return UNHOOKED_ERR;
}

GodinHook::GodinELfHook::RETINFO GodinHook::GodinELfHook::hookAllRegisteredModule(string hookSymbol, void *newFunc, void **backFunc)
{
 // LOGV("---hook all----");
  ModuleMap::iterator it;
  for(it = module_map_.begin();it != module_map_.end();it++){
    HookModule * module = it->second;
    RETINFO ret = hook(module,hookSymbol,newFunc,backFunc);
    if(HOOKED_ERR == ret)
      return ret;
  }
  return GODIN_OK;
}

GodinHook::GodinELfHook::RETINFO GodinHook::GodinELfHook::unHook(string moduleName, string hookSymbol,void *backFunc)
{

  /// 模块是否已经被注册
  if(!isRegisteredModule(moduleName))
    return EFL_MODULE_NOT_REGISTERRD;
  /// symbol是否已经被hook
  if(!isHookedInModule(moduleName,hookSymbol))
    return NOT_HOOKED;
  /// 进行unhook
  /// 兼顾资源回收
  HookModule * module = getHookModule(moduleName);
  if(!module->unhook(hookSymbol.c_str(),backFunc))
    return UNHOOKED_ERR;
  module->removeHookedSymbol(hookSymbol);
  delete  module;
  module = NULL;
  module_map_.erase(moduleName);
  return GODIN_OK;
}

GodinHook::GodinELfHook::RETINFO GodinHook::GodinELfHook::unHookAllRegisteredModule(string hookSymbol, void *backFunc)
{
  ModuleMap::iterator it = module_map_.begin();
  while(it != module_map_.end()){
    HookModule * module = it->second;
    RETINFO ret = unhook(module,hookSymbol,backFunc);
    if(UNHOOKED_ERR == ret)
      return ret;
    else if(GODIN_OK == ret){
      module->removeHookedSymbol(hookSymbol);
      delete  module;
      module = NULL;
      module_map_.erase(it++);
    }else if(NOT_HOOKED == ret)
      it++;
  }
  return GODIN_OK;
}

/// 参考 AOSP/bionic/linker/linker_phdr.cpp::VerifyElfHeader()
bool GodinHook::GodinELfHook::isElfFile(size_t baseAddress)
{
  void * base_addr = baseAddress;
  ELFW(Ehdr) *ehdr = reinterpret_cast<ELFW(Ehdr) *>(base_addr);

  /// 前四字节内容：“\177ELF”
  if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) { return false; }

  /// 32 还是 64
  int elf_class = ehdr->e_ident[EI_CLASS];
#if defined(__LP64__)
  if (elf_class != ELFCLASS64) { return false; }
#else
  if (elf_class != ELFCLASS32) { return false; }
#endif
  if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) { return false; }
  if (ehdr->e_version != EV_CURRENT) { return false; }
  if (ehdr->e_machine != GetTargetElfMachine()) { return false; }
  return true;
}

/// 参考 AOSP/bionic/linker/linker_phdr.cpp::GetTargetElfMachine()
int GodinHook::GodinELfHook::GetTargetElfMachine()
{
#if defined(__arm__)
  return EM_ARM;
#elif defined(__aarch64__)
  return EM_AARCH64;
#elif defined(__i386__)
  return EM_386;
#elif defined(__mips__)
  return EM_MIPS;
#elif defined(__x86_64__)
  return EM_X86_64;
#endif
}

GodinHook::HookModule *GodinHook::GodinELfHook::getHookModule(string name)
{
  ModuleMap::iterator it = module_map_.find(name);

  return it->second;
}
