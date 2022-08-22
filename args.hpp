#pragma once
#include "com.hpp"



//////////////////////////////
// 从命令行参数找指定key
// ret：key的idx，不存在返回-1
static int32_t xx_get_argidx(int argc, char** argv, const char* key)
{
	for (int i = 1; i < argc; ++i) {
		if (0 == strcmp(argv[i], key)) {
			return i;
		}
	}
	return -1;
}

// 从命令行参数找指定key对应val
// ret：不存在返回nullptr
// eg：-key1 val1，传入-key1返回val1
static const char* xx_get_arg(int argc, char** argv, const char* key) {
	int32_t idx = xx_get_argidx(argc, argv, key);
	// 没找到 || 后面没了
	if (-1 == idx|| idx == argc - 1) {
		return "";
	}
	return argv[idx + 1];
}

static int64_t xx_get_arg_i64(int argc, char** argv, const char* key){
    const char* val = xx_get_arg(argc, argv, key);
    return atoll(val);
}

#define xx_arg_have(key)  (-1 != xx_get_argidx(argc,argv,key))
#define xx_arg_str(key)   (xx_get_arg(argc,argv,key))
#define xx_arg_int(key)   (xx_get_arg_i64(argc,argv,key))