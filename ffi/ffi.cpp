#include <string>
#include <map>
#include <dlfcn.h>
#include <functional>
#include <iostream>

#include "ffi.h"

FFI::FFI()
{
	
}

FFI::~FFI()
{
	for(auto const& [key, val] : libs)
	{
		dlclose(val);
	}
	libs.clear();
	funs.clear();
}

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
	{
		std::cout<<dlerror()<<"\n";
		panic("failed to load library " + name + path);
	}
	libs[name] = handle;
	std::cout<<"loaded " << name << "\n";
	return 1;
}

int FFI::loadFun(std::string name, std::string lib)
{
	void *handle = libs[lib];
	if(!handle)
		panic("lib "+lib+" not loaded\n");
	void *tfun;
	tfun = (void*)dlsym(handle, name.c_str());
	if(!tfun)
	{
		std::cout<<name<<" "<<lib<<"\n";
		std::cout<<dlerror()<<"\n";
		panic("failed to load function " + name + lib);
	}
	funs[name] = tfun;
	return 1;
}

int FFI::callFun(std::string name)
{
	void (*f1)();
	f1 = (void(*)())funs[name];
	f1();
	return 1;
}

void *FFI::getFun(std::string name)
{
	std::cout<<name<<"\n";
	if(!name.c_str())
		return NULL;
	return (void*)funs[name];
}

