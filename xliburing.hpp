#pragma once

#include "liburing.h"
#include "xcom.hpp"

class xliburing
{
public:
    void print_ring_info(const char *p = nullptr)
    {
        if (nullptr == p)
            p = "";
        printf("[%s]sq khead=%u ktail=%u, sqe_head=%u sqe_tail=%u,cq khead=%u ktail=%u\n",
               p,
               *ring_.sq.khead, *ring_.sq.ktail,
               ring_.sq.sqe_head, ring_.sq.sqe_tail,
               *ring_.cq.khead, *ring_.cq.ktail);
    }
    bool init_params(int queue_depth, int flags)
    {
        int err = io_uring_queue_init(queue_depth, &ring_, flags);
        CHECK_RETV(0 == err, false);
        print_ring_info("after init");
        return true;
    }
    bool init()
    {
        return init_params(128, 0);
    }

    bool uring_sendmsg(int sockfd, msghdr msg, void *user_data)
    {
        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
        CHECK_RETV(nullptr != sqe, false);

        io_uring_sqe_set_data(sqe, user_data);
        io_uring_prep_sendmsg(sqe, sockfd, &msg, 0);

        int ret = io_uring_submit(&ring_);
        CHECK_RETV(1 == ret, false);

        return true;
    }

    bool uring_recvmsg(int sockfd, msghdr &msg, void *user_data)
    {
        print_ring_info("before get sqe");
        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
        CHECK_RETV(nullptr != sqe, false);
        print_ring_info("after get sqe");

        io_uring_sqe_set_data(sqe, user_data);

        print_ring_info("before prep");
        io_uring_prep_recvmsg(sqe, sockfd, &msg, 0);
        print_ring_info("after prep,before submit");

        int ret = io_uring_submit(&ring_);
        CHECK_RETV(1 == ret, false);
        print_ring_info("after submit");

        return true;
    }

    bool uring_send(int sockfd,
                    void *buf, size_t len,
                    void *user_data,
                    sockaddr_in saddr)
    {
        struct iovec iov = {
            .iov_base = buf,
            .iov_len = len,
        };

        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_name = &saddr;
        msg.msg_namelen = sizeof(struct sockaddr_in);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;

        return uring_sendmsg(sockfd, msg, user_data);
    }

    io_uring_cqe *get_cqe()
    {
        io_uring_cqe *cqe = nullptr;
        int ret = io_uring_peek_cqe(&ring_, &cqe);
        if(0 == ret){
            CHECK_RETV(nullptr != cqe,nullptr);
            return cqe;
        }
        if (-EAGAIN == ret){
            return nullptr;
        }else{
            CHECK0_RETV(ret, nullptr);
            return nullptr;
        }
    }

public:
    struct io_uring ring_;
};