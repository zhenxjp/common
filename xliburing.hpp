#pragma once

#include "liburing.h"
#include "xcom.hpp"
#include <sys/eventfd.h>
#include <poll.h>


static void msec_to_ts(struct __kernel_timespec *ts, unsigned int msec)
{
	ts->tv_sec = msec / 1000;
	ts->tv_nsec = (msec % 1000) * 1000000;
}
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

    bool uring_sendmsg(int sockfd, msghdr *msg,unsigned int flags,
                         void *user_data = nullptr)
    {
        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
        CHECK_RETV(nullptr != sqe, false);
        io_uring_sqe_set_data(sqe, user_data);

        io_uring_prep_sendmsg(sqe, sockfd, msg, 0);

        int ret = io_uring_submit(&ring_);
        CHECK_RETV(1 == ret, false);
        return true;
    }

    bool uring_recvmsg(int sockfd, msghdr *msg, unsigned int flags,
                        void *user_data = nullptr,
                        bool multishot = false)
    {
        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
        CHECK_RETV(nullptr != sqe, false);
        io_uring_sqe_set_data(sqe, user_data);

        if(multishot)
        {
            io_uring_prep_recvmsg_multishot(sqe, sockfd, msg, 0);

        }else{
            io_uring_prep_recvmsg(sqe, sockfd, msg, 0);
        }


        int ret = io_uring_submit(&ring_);
        CHECK_RETV(1 == ret, false);
        return true;
    }

    bool uring_event_fd(int fd, unsigned int poll_mask = POLLIN,
                        void *user_data = nullptr,
                        bool multishot = false)
    {
        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
        CHECK_RETV(nullptr != sqe, false);
        io_uring_sqe_set_data(sqe, user_data);

        if(multishot)
        {
            io_uring_prep_poll_multishot(sqe, fd, POLLIN);

        }else{
            io_uring_prep_poll_add(sqe, fd, POLLIN);
        }


        int ret = io_uring_submit(&ring_);
        CHECK_RETV(1 == ret, false);
        return true;
    }

    // flags |= IORING_TIMEOUT_MULTISHOT:重复触发
    bool uring_timeout(int timeout_ms,unsigned count, unsigned flags,
                        void *user_data = nullptr)
    {
        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
        CHECK_RETV(nullptr != sqe, false);
        io_uring_sqe_set_data(sqe, user_data);

        struct __kernel_timespec ts;
        msec_to_ts(&ts,timeout_ms);

        io_uring_prep_timeout(sqe, &ts, count, flags);

        int ret = io_uring_submit(&ring_);
        CHECK_RETV(1 == ret, false);

        return true;
    }

    // 循环方式
    /**
    if (0 == ret && nullptr != cqe)
    {
        io_uring_for_each_cqe(&r.ring_, head, cqe)
        {
            ++num_completed;
        }
        r.cqe_seen(num_completed);
    }
    **/
    int get_cqes(io_uring_cqe **cqes,int cnt,int timeout_ms = -1)
    {
        __kernel_timespec *ts_use = nullptr;
        struct __kernel_timespec ts;

        if(-1 != timeout_ms)
        {
            msec_to_ts(&ts,timeout_ms);
            ts_use = &ts;
        }

	    int ret = io_uring_wait_cqes(&ring_, cqes, cnt, ts_use, nullptr);
        return ret;
    }



    io_uring_cqe* get_cqe(int timeout_ms = -1)
    {
        io_uring_cqe* cqe = nullptr;
        int ret = get_cqes(&cqe, 1, timeout_ms);
        if(0 == ret)
            return cqe;
        else
            return nullptr;
    }

    io_uring_cqe *peek_cqe()
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

    void cqe_seen(int cnt = 1)
    {
        io_uring_cq_advance(&ring_, cnt);
    }

public:
    struct io_uring ring_;
};