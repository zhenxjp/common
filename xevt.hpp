#pragma once
#include "xee.hpp"
#include "xio.hpp"
#include "xrb.hpp"

#include <sys/eventfd.h>
typedef std::function<void(uint64_t)> ntf_cb_t;

class xntf_evt : public xevent {
public:
    // 定义回调函数类型
    xntf_evt()
    {
        efd_ = eventfd(0,0);
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

    void set_cb(xepoll *ee,ntf_cb_t cb)
    {
        ee->insert(EPOLLIN,this);
        cb_ = cb;
    }

private:
    int efd_ = -1;
    ntf_cb_t cb_ = nullptr;
};


class rb_iov_ntf : public rb_iov {
public:
    void set_cb(xepoll *ee,ntf_cb_t cb)
    {
        ntf_.set_cb(ee,cb);
    }

    void writer_done_ntf(uint64_t cnt)
    {
        writer_done(cnt);
        ntf_.ntf();
    }

public:
    xntf_evt ntf_;
};



class xio_evt  {
public:
    int init(const io_context &ctx,rb_iov_ntf *rb,xepoll *ee)
    {
        if(io_rw_type::rw_read == ctx.rw_type_)
        {
            return init_read(ctx,rb,ee);
        }else if (io_rw_type::rw_write == ctx.rw_type_)
        {
            return init_write(ctx,rb,ee);
        }
        return err_data_err;
    }
private:
    int init_read(const io_context &ctx,rb_iov_ntf *rb,xepoll *ee)
    {
        io_.init(ctx);
        rb_ = rb;
        
        ntf_.set_cb(ee,[this](uint64_t val)
        {
            ntf_.ntf();

            uint64_t cnt = 1024;
            iovec *iov= rb_->writer_get_blk(cnt);
            if (0 == cnt)
                return;

            uint32_t readed = 0;
            io_.read_data(iov,cnt,rb_->w_idx(),readed);

            if(0 == readed)
                return;

            rb_->writer_done_ntf(readed);
            
            //LOG_INFO("read all:{},readed:{}",rb_->w_idx(),readed);
        });
        ntf_.ntf();

        return 0;
    }
    int init_write(const io_context &ctx,rb_iov_ntf *rb,xepoll *ee)
    {
        int iret = io_.init(ctx);
        XASSERT(iret == 0);
        rb_ = rb;
        rb_->set_cb(ee,[this](uint64_t val)
        {
            uint64_t cnt = 1024;
            iovec *iov= rb_->reader_get_blk(cnt);
            if (0 == cnt)
                return;

            uint32_t written = 0;
            io_.write_data(iov,cnt,written);

            rb_->reader_done(written);
            //LOG_INFO("write all:{},written:{}",rb_->r_idx(),written);

            if(written <= cnt)
            {
                rb_->ntf_.ntf();
            }
            
        });

        return 0;
    }
private:
    io io_;
    rb_iov_ntf *rb_;

    xntf_evt ntf_;
};
