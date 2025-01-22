#pragma once
#include "xcom.hpp"

static ssize_t xpwrite(int fd, const void *buf, size_t count, off_t offset) {
    const char *cbuf = (const char *)buf;
    ssize_t nwritten = 0, totwritten = 0;

    while(count) {
        nwritten = pwrite(fd, cbuf, count,offset);

        if (nwritten < 0) {
            if (errno == EINTR) 
                continue;
            return totwritten ;
        }

        count -= nwritten;
        cbuf += nwritten;
        totwritten += nwritten;
        offset += nwritten;
    }

    return totwritten;
}

static ssize_t xpread(int fd, void *buf, size_t count, off_t offset) {
    char *cbuf = (char *)buf;
    ssize_t nread = 0, totread = 0;
    while(count) {
        nread = pread(fd, cbuf, count, offset );

        if (nread < 0) {
            if (errno == EINTR) 
                continue;
            return totread;
        }

        count -= nread;
        cbuf += nread;
        totread += nread;
        offset += nread;
    }
    return totread;
}

static ssize_t xread(int fd, void *buf, size_t count) {
    char *cbuf = (char *)buf;
    ssize_t nread = 0, totread = 0;
    while(count) {
        nread = read(fd, cbuf, count);

        if (nread < 0) {
            if (errno == EINTR) 
                continue;
            return totread;
        }

        count -= nread;
        cbuf += nread;
        totread += nread;
    }
    return totread;
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
    ~io_idx()
    {
        release();
    }

    int load_exist(const io_meta& meta,const string &path)
    {
        fd_ = open(path.c_str(),O_RDWR);
        CHECK_RETV(fd_,-1);

        ssize_t read_len = read(fd_,&meta_,sizeof(meta_));
        if(read_len< 0)
        {
            X_P_INFO;
            return -1;
        }
        if(0 != memcmp(&meta_,&meta,sizeof(meta)))
        {
            X_P_INFO;
            return -2;
        }


        uint32_t blk_no = cnt_ % meta_.blk_cnt_max_;
        uint32_t file_no = cnt_ / meta_.blk_cnt_max_;

        auto file_size = xfile_get_size(path);
        uint64_t idx_cnt = (file_size - sizeof(meta_)) / sizeof(uint32_t);
        cnt_ = idx_cnt;
        while (idx_cnt)
        {
            uint32_t *offset_array = new uint32_t[meta_.blk_cnt_max_];
            memset(offset_array,0,sizeof(uint32_t) * meta_.blk_cnt_max_);
            index_.push_back(offset_array);

            uint64_t read_cnt = std::min(meta_.blk_cnt_max_,idx_cnt);

            ssize_t read_len = xread(fd_,offset_array,read_cnt * sizeof(uint32_t));

            if(read_cnt*sizeof(uint32_t) != read_len)
            {
                printf("read cnt not equal %ju %ju\n", read_cnt,read_len);
                return -1;
            }
            idx_cnt -= read_cnt;

        }

        return 0;
    }

    int create_new(const io_meta& meta,const string &path)
    {
        meta_ = meta;
        fd_ = open(path.c_str(),O_RDWR|O_CREAT|O_TRUNC,0666);
        CHECK_RETV(fd_,-1);

        write(fd_,&meta_,sizeof(meta_));
        return 0;
    }


    int add(int32_t blk_len)
    {
        uint32_t blk_no = cnt_ % meta_.blk_cnt_max_;
        uint32_t file_no = cnt_ / meta_.blk_cnt_max_;

        uint32_t offset = 0;
        if(0 == blk_no)
        {
            uint32_t *offset_array = new uint32_t[meta_.blk_cnt_max_];
            memset(offset_array,0,sizeof(uint32_t) * meta_.blk_cnt_max_);
            index_.push_back(offset_array);

            offset = blk_len;
            offset_array[0]  = offset;

        }else{
            uint32_t *offset_array = index_[file_no];
            
            offset = offset_array[blk_no - 1] + blk_len;
            offset_array[blk_no]  = offset;
        }

        write(fd_,&offset,sizeof(uint32_t));

        cnt_++;

        return 0;
    }

    uint32_t get_offset(uint64_t idx)
    {
        if(idx >= cnt_)
        {
            return -1;
        }
        uint64_t idx_in_blk = idx % meta_.blk_cnt_max_;
        uint64_t blk_no_ = idx / meta_.blk_cnt_max_;
        uint32_t* offset_array = index_[blk_no_];

        return offset_array[idx_in_blk];
    }

    uint32_t get_len(uint64_t idx)
    {
        if(idx >= cnt_)
        {
            return -1;
        }
        uint64_t idx_in_blk = idx % meta_.blk_cnt_max_;
        uint64_t blk_no = idx / meta_.blk_cnt_max_;
        uint32_t* offset_array = index_[blk_no];


        if(0 == idx_in_blk)
        {
            return offset_array[idx_in_blk];
        }else{
            return offset_array[idx_in_blk] - offset_array[idx_in_blk - 1];
        }
    }

    int adds(iovec *iov,uint64_t cnt)
    {
        for (size_t i = 0; i < cnt; i++)
        {
            add(iov[i].iov_len);
        }

        return 0;
    }
    void release()
    {
        for (size_t i = 0; i < index_.size(); i++)
        {
            delete[] index_[i];
        }
        index_.clear();
        close(fd_);
    }
private:
    
public:
    uint64_t cnt_ = 0;                      // 已写索引数量

    std::vector<uint32_t*> index_;
    int fd_ = -1;
    io_meta meta_;
};



