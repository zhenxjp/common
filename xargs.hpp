#pragma once
#include "xcom.hpp"



//////////////////////////////
// 从命令行参数找指定key
// ret：key的idx，不存在返回-1
static int32_t xx_get_argidx(int argc, char** argv, c_t key)
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
static c_t xx_get_arg(int argc, char** argv, c_t key,c_t default_ret = "") {
	int32_t idx = xx_get_argidx(argc, argv, key);
	// 没找到 || 后面没了
	if (-1 == idx|| idx == argc - 1) {
		return default_ret;
	}
	return argv[idx + 1];
}

static int64_t xx_get_arg_i64(int argc, char** argv, c_t key,int64_t default_ret = -1){
	int32_t idx = xx_get_argidx(argc, argv, key);
	// 没找到 || 后面没了
	if (-1 == idx|| idx == argc - 1) {
		return default_ret;
	}

    return atoll(argv[idx + 1]);
}





#define xx_arg_have(key)  (-1 != xx_get_argidx(argc,argv,key))

void xx_check_arg(int argc, char** argv, c_t key)
{
	if(!xx_arg_have(key))
	{
		printf("need arg %s\n",key);
		exit(0);
	}
}
#define xx_arg_str(key)   (xx_check_arg(argc,argv,key),xx_get_arg(argc,argv,key))
#define xx_arg_int(key)   (xx_check_arg(argc,argv,key),xx_get_arg_i64(argc,argv,key))

#define xx_arg_str_default(key,default_ret)   (xx_get_arg(argc,argv,key,default_ret))
#define xx_arg_int_default(key,default_ret)   (xx_get_arg_i64(argc,argv,key,default_ret))
//////////////////////////////
struct arg_info_copy
{
	int argc = 0;
	char** argv = nullptr;
	bool init = false;
};
static arg_info_copy __arg_info_global;

static void xxx_init_arg(int argc, char** argv)
{
	__arg_info_global.argc = argc;
	__arg_info_global.argv = argv;
	__arg_info_global.init = true;
}

#define xxx_arg_have(key)  (-1 != xx_get_argidx(__arg_info_global.argc,__arg_info_global.argv,key))
#define xxx_arg_str(key)   (xx_get_arg(__arg_info_global.argc,__arg_info_global.argv,key))
#define xxx_arg_int(key)   (xx_get_arg_i64(__arg_info_global.argc,__arg_info_global.argv,key))
