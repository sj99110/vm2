#include <string>
#include <map>
#include <dlfcn.h>
#include <functional>
#include <iostream>

#include "ffi.h"

void panic(std::string str)
{
	std::cout<<str<<"\n";
	exit(1);
}

int FFI::openLib(std::string path, std::string name)
{
	void *handle;
	handle = dlopen(path.c_str(), RTLD_LAZY);
	if(!handle)
		panic("failed to load library " + name + path);
	libs[name] = handle;
	return 1;
}

int FFI::loadFun(std::string name, std::string lib)
{
	void *handle = libs[name];
	void (*tfun)();
	tfun = (void (*)())dlsym(handle, name.c_str());
	funs[name] = tfun;
	return 1;
}

int FFI::callFun(std::string name)
{
	funs[name]();
	return 1;
}

void *FFI::getFun(std::string name)
{
	return funs[name].target<void(*)()>();
}

