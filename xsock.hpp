#pragma once
#include "xcom.hpp"

static sockaddr_in fill_addr(const char *ip, port_t port)
{
	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	return addr;
}
// in_addr_t uint32_t
static in_addr_t get_addr_ip(sockaddr_in *addr){
    return addr->sin_addr.s_addr;
}
// in_port_t uint16_t
static in_port_t get_addr_port(sockaddr_in *addr){
    return htons(addr->sin_port);
}

static string get_addr_str(sockaddr_in* addr){
    char sz[100]={0};
    inet_ntop(AF_INET,(void*)&addr->sin_addr,sz,100);
    string ret = string(sz)+":"+to_string(get_addr_port(addr));
    return  ret;
}
//////////////////////////////
class xsock
{
public:
    // SHUT_RD
    // SHUT_WR
    // SHUT_RDWR
    void xshutdown(int how = SHUT_RDWR ){
        shutdown(sock_,how);
    }
    void close(){
        if(invalid_sock == sock_){
            return ;
        }
        ::close(sock_);
        sock_ = invalid_sock;
    }

    void close_rst()
    {
        set_linger(1,0);
        close();
    }

    bool tcp_create(){
        return socket_create(SOCK_STREAM);
    }

    bool udp_create(){
        return socket_create(SOCK_DGRAM);
    }

    bool xbind(const char *ip, port_t port)
	{
		sockaddr_in addr = fill_addr(ip,port);

		int ret = bind(sock_, (struct sockaddr*)&addr, sizeof(addr));
        CHECK_RETV(0 == ret,false);
		return true;
	}

	bool xlisten(int iBacklog=128)
	{
		int ret = ::listen(sock_, iBacklog);
        CHECK_RETV(0 == ret,false);
		return true;
	}


    int xaccept()
	{
		int ret;

		struct sockaddr_in addr;
		socklen_t	addrlen = sizeof(addr);

		ret = ::accept(sock_, (struct sockaddr*)&addr, &addrlen);
		return ret;
	}



    bool connect(const char *ip, port_t port){
        sockaddr_in addr = fill_addr(ip,port);
        auto ret = ::connect(sock_,(struct sockaddr*)&addr,sizeof(addr));
        if(-1 == ret && errno != EINPROGRESS)
        {
            CHECK_RETV(false,false);
        }
        

        return true;
    }

    int recv_from(void *buf, 
                size_t n, 
                int flags = 0,
                struct sockaddr *src_addr = nullptr, 
                socklen_t *addrlen = nullptr)
    {
        auto ret = ::recvfrom(sock_,buf,n,flags,src_addr,addrlen);
        if(0 == ret){
            // When a stream socket peer has performed an orderly shutdown, the return value will be 0
            return 0;
        }else if(-1 == ret){
            // -1 if an error occurred.  In the event of an error,  errno is set to indicate the error.
            if(EINTR != errno && 
                EAGAIN != errno && 
                EWOULDBLOCK != errno){
                CHECK_RETV(false,-1);
            }
            return -1;
        }else{
            // return the number of bytes received
            return ret;
        }
    }

    int sendto(const void *buf, size_t len, int flags = 0,
                const struct sockaddr *dest_addr = nullptr, socklen_t addrlen = 0)
    {
        auto ret = ::sendto(sock_,buf,len,flags,dest_addr,addrlen);
        if(-1 == ret){
            // -1 if an error occurred.  In the event of an error,  errno is set to indicate the error.
            if(EINTR != errno && 
                EAGAIN != errno && 
                EWOULDBLOCK != errno){
                close();
            }
            CHECK_RETV(false,-1);
        }else{
            // return the number of bytes received
            return ret;
        }
    }
    uint32_t get_recv_buf_size(){
        uint32_t real = 0;
        socklen_t len = sizeof(uint32_t);

        auto ret = getsockopt(sock_,SOL_SOCKET,SO_RCVBUF,(char*)&real,&len);
        CHECK_RETV(-1 != ret,-1);
        return real;
    }
    bool set_recv_buf_size(uint32_t buf_size){
        auto ret = setsockopt(sock_,SOL_SOCKET,SO_RCVBUF,(char*)&buf_size,sizeof(buf_size));
        CHECK_RETV(-1 != ret,false);

        uint32_t real = get_recv_buf_size();
        if(real != buf_size){
            printf("set_recv_buf_size err,set=%u,real=%u \n",buf_size,real);
        }
        return true;
    }

    sockaddr_in get_local_addr(){
        sockaddr_in addr;
        
        socklen_t len = sizeof(addr);
        auto ret = getsockname(sock_,(sockaddr*)&addr,&len);
        if(0 != ret){
            memset(&addr,0,sizeof(addr));
        }
        return addr;
    }
    
    sockaddr_in get_peer_addr(){
        sockaddr_in addr;
        
        socklen_t len = sizeof(addr);
        auto ret = getpeername(sock_,(sockaddr*)&addr,&len);
        if(0 != ret){
            memset(&addr,0,sizeof(addr));
        }
        return addr;
    }

    bool reuse_addr(int32_t flag){
        
        auto ret = setsockopt(sock_,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));
        CHECK_RETV(0 == ret,false);

        return true;
    }

    bool reuse_port(int32_t flag){
        
        auto ret = setsockopt(sock_,SOL_SOCKET,SO_REUSEPORT,&flag,sizeof(flag));
        CHECK_RETV(0 == ret,false);

        return true;
    }

    bool set_nonblock(int32_t value)
	{
		int oldflags = ::fcntl(sock_, F_GETFL, 0);
        CHECK_RETV(-1 != oldflags,false);

		if (value != 0)
			oldflags |= O_NONBLOCK;
		else
			oldflags &= ~O_NONBLOCK;
		/* Store modified flag word in the descriptor. */
		auto ret = ::fcntl(sock_, F_SETFL, oldflags);
        CHECK_RETV(0 == ret,false);
		return true;
	}

    bool set_linger(int l_onoff,int l_linger)
    {
        linger l;
        l.l_onoff = l_onoff;
        l.l_linger = l_linger;

        int ret = setsockopt(sock_,SOL_SOCKET,SO_LINGER,&l,sizeof(l));
        CHECK_RETV(0 == ret,false);
        return true;
    }

    bool is_error()
    {
        int result = 0;
        socklen_t result_len = sizeof(result);
        int ret = getsockopt(sock_,SOL_SOCKET,SO_ERROR,(char*)&result,&result_len);
        CHECK_RETV(0 == ret,true);
        CHECK_RETV(0 == result,true);
        return false;
    }

private:

    bool socket_create(int __type){
        close();
        sock_ = ::socket(AF_INET, __type, 0);
		CHECK_RETV(invalid_sock != sock_,false);
        return true;
    }
public:
    int32_t sock_{-1};
};