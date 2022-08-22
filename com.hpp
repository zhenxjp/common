#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <fstream>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/wait.h>

using namespace std;
namespace fs = std::filesystem;

typedef const char* c_t;

#define CHECK_RETV(value, ret)                                                                                         \
	if (0 == (value))                                                                                                  \
	{                                                                                                                  \
		cout << "Check Fail,Value [" << #value << "],line [" << __LINE__ << "],func [" << __FUNCTION__ << "]" << endl; \
		return (ret);                                                                                                  \
	};
