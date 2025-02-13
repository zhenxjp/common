#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <set>

#include <iostream>
#include <fstream>
#include <cstdlib>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <signal.h>
#include <atomic> 
#include <dirent.h>
#include <sys/stat.h>
//////////////////////////////
using namespace std;

//////////////////////////////
#define X_ARRAY_CNT(v)	(sizeof(v)/sizeof((v)[0]))
//////////////////////////////
typedef const char* c_t;
typedef unsigned int ip_t;
typedef unsigned short port_t;
//////////////////////////////
#define invalid_sock -1
//////////////////////////////

static bool _xxx_printf = true;
#define X_P_INFO  \
    if(_xxx_printf){\
        printf("err!file:%s,line:%u,func:%s,last err=%u %s\n",\
            __FILE__,__LINE__,__FUNCTION__,errno,strerror(errno));\
    }
#define X_P_VAL(val)\
    if(_xxx_printf){\
        printf(#val"=%d \n",val);\
    }
	
#define CHECK_RETV(value, ret) if (0 == (value)){X_P_INFO;return (ret);};
#define CHECK_RET(value) if (0 == (value)){X_P_INFO;return ;};

#define CHECK0_RETV(value, ret) if (0 != (value)){X_P_INFO;X_P_VAL(value);return (ret);};



#define XASSERT(value)  if (0 == (value)){X_P_INFO;xexit(0);};

#define XSAFE_DELETE(p) if(p){delete p;p=nullptr;}
#define XSAFE_DELETE_ARRAY(p) if(p){delete [] p;p=nullptr;}
#define XSAFE_CLOSE(fd) if(fd>0){close(fd);fd=-1;}
//////////////////////////////
static bool cpu_bind(pthread_t th,int32_t cpu_idx)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_idx,&mask);
    return 0 ==  pthread_setaffinity_np(th,sizeof(mask),&mask);
}

static uint32_t get_cur_tid()
{
    return ::syscall(SYS_gettid);
}

static void xexit(int code)
{
    exit(code);
}