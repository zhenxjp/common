#pragma once
#include "xcom.hpp"

static ssize_t xwrite(int fd, const void *buf, size_t len) {
    const char *cbuf = (const char *)buf;
    ssize_t nwritten = 0, totwritten = 0;

    while(len) {
        nwritten = write(fd, cbuf, len);

        if (nwritten < 0) {
            if (errno == EINTR) continue;
            return totwritten ? totwritten : -1;
        }

        len -= nwritten;
        cbuf += nwritten;
        totwritten += nwritten;
    }

    return totwritten;
}


struct io_meta
{
    uint64_t io_type_ = 0;
    uint64_t blk_size_ = 1024;
    uint64_t blk_cnt_max_ = 1024;

};

class io_idx
{
public:

    int create_new(const io_meta& meta,const string &path)
    {
        fd_ = open(path.c_str(),O_RDWR|O_CREAT|O_TRUNC,0666);
        CHECK_RETV(fd_,-1);

        xwrite(fd_,&meta,sizeof(meta));
        return 0;
    }

    int add(int32_t blk_len)
    {
        check_add_new_offset_blk();
        uint32_t offset = last_offset_ + blk_len;
        index_[file_idx_][idx_] = offset;
        write(fd_,&offset,sizeof(uint32_t));

        idx_++;
        cnt_++;
        last_offset_+= blk_len;


        if(idx_ == meta_.blk_cnt_max_)
        {
            idx_ = 0;
            file_idx_++;
            last_offset_ = 0;
        }

        return 0;
    }

    uint32_t get(uint64_t idx)
    {
        if(idx >= cnt_)
        {
            return -1;
        }
        return index_[idx / meta_.blk_cnt_max_][idx % meta_.blk_cnt_max_];
    }

    int adds(iovec *iov,uint64_t cnt)
    {
        for (size_t i = 0; i < cnt; i++)
        {
            add(iov[i].iov_len);
        }

        return 0;
    }
private:
    void check_add_new_offset_blk()
    {
        if(file_idx_ >= index_.size())
        {
            uint32_t *offset = new uint32_t[meta_.blk_cnt_max_];
            index_.push_back(offset);
        }
    }
public:
    uint64_t cnt_ = 0;
    uint64_t file_idx_ = 0;
    uint64_t idx_ = 0;
    uint64_t last_offset_ = 0;


    std::vector<uint32_t*> index_;
    int fd_ = -1;
    io_meta meta_;
};



