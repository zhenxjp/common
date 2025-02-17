#pragma once
#include "xee.hpp"
#include "xio.hpp"
#include "xrb.hpp"

#include <sys/eventfd.h>
typedef std::function<void(uint64_t)> ntf_cb_t;

class xntf_evt : public xevent {
public:
    // 定义回调函数类型

    int init(xepoll *ee)
    {
        efd_ = eventfd(0,0);
        ee->insert(EPOLLIN,this);
        return 0;
    }

    virtual int32_t get_fd()override
    {
        return efd_;
    }

    virtual void handle_evt(uint32_t evt_type)
    {
        if(nullptr == cb_)
            return;

        uint64_t val = 0;
        int ret = read(efd_,&val,sizeof(val));
        XASSERT(ret == sizeof(val));
        cb_(val);
    }

    void ntf()
    {
        eventfd_write(efd_,1);
    }

    void set_cb(ntf_cb_t cb)
    {
        cb_ = cb;
    }

private:
    int efd_ = -1;
    ntf_cb_t cb_ = nullptr;
};


class rb_iov_ntf : public rb_iov {
public:
    int init_ee(xepoll *ee)
    {
        ntf_.init(ee);
        return 0;
    }

    void set_cb(ntf_cb_t cb)
    {
        ntf_.set_cb(cb);
    }

    void writer_done_ntf(uint64_t cnt)
    {
        writer_done(cnt);
        ntf_.ntf();
    }

private:
    xntf_evt ntf_;
};


// 读盘
class xior_evt : public xevent {
public:
    int init(const io_context &ctx,rb_iov_ntf *rb,xepoll *ee)
    {
        ntf_.init(ee);
        io_.init(ctx);
        rb_ = rb;
        
        ntf_.set_cb([this](uint64_t val)
        {
            uint64_t cnt = 1024;
            iovec *iov= rb_->writer_get_blk(cnt);

            uint32_t readed = 0;
            io_.read_data(iov,cnt,rb_->w_idx(),readed);

            rb_->writer_done_ntf(readed);
            ntf_.ntf();
        });
        ntf_.ntf();

        return 0;
    }

private:
    io io_;
    rb_iov_ntf *rb_;

    xntf_evt ntf_;
};



// 写盘
class xiow_evt : public xevent {
public:
    int init(const io_context &ctx,rb_iov_ntf *rb,xepoll *ee)
    {
        io_.init(ctx);
        rb_ = rb;
        rb_->set_cb([this](uint64_t val)
        {
            uint64_t cnt = 1024;
            iovec *iov= rb_->writer_get_blk(cnt);

            uint32_t written = 0;
            io_.write_data(iov,cnt,written);

            rb_->writer_done_ntf(written);
        });

        return 0;
    }
private:
    io io_;
    rb_iov_ntf *rb_;
};