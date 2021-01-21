#pragma once

#include <map>
#include <functional>
#include <string>

typedef void* FFIFun;

class FFI
{
	private:
	std::map<std::string, FFIFun> funs;
	std::map<std::string, void*> libs;
	public:
	FFI();
	~FFI();
	int openLib(std::string path, std::string name);
	int loadFun(std::string name, std::string lib);
	int callFun(std::string);
	void *getFun(std::string name);
	void* operator[](std::string name)
	{
		return (void*)funs[name];
	}
};

void panic(std::string str);
