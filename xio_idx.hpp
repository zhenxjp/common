#pragma once
#include "xcom.hpp"

static ssize_t xwrite(int fd, const void *buf, size_t count) {
    const char *cbuf = (const char *)buf;
    ssize_t nwritten = 0, totwritten = 0;

    while(count) {
        nwritten = write(fd, cbuf, count);

        if (nwritten < 0) {
            if (errno == EINTR) 
                continue;
            return totwritten ;
        }

        count -= nwritten;
        cbuf += nwritten;
        totwritten += nwritten;
    }

    return totwritten;
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

typedef uint32_t idx_t ;
const uint32_t IDX_LEN = sizeof(idx_t);
const uint32_t META_LEN = sizeof(io_meta);
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
        ssize_t read_len = xread(fd_,&meta_,META_LEN);

        if(0 != memcmp(&meta_,&meta,META_LEN))
        {
            X_P_INFO;
            return -2;
        }

        auto file_size = xfile_get_size(path);
        uint64_t idx_cnt = (file_size - META_LEN) / IDX_LEN;
        
        while (idx_cnt)
        {
            idx_t *offset_array = create_idx_to_vec();
            uint64_t read_cnt = std::min(meta_.blk_cnt_max_,idx_cnt);
            
            ssize_t read_len = xread(fd_,offset_array,read_cnt * IDX_LEN);

            if(read_cnt * IDX_LEN != read_len)
            {
                printf("read cnt not equal %ju %ju\n", read_cnt,read_len);
                return -1;
            }
            idx_cnt -= read_cnt;
        }
        cnt_ = idx_cnt;
        
        return 0;
    }


    int create_new(const io_meta& meta,const string &path)
    {
        meta_ = meta;
        fd_ = open(path.c_str(),O_RDWR|O_CREAT|O_TRUNC,0666);
        CHECK_RETV(fd_,-1);

        xwrite(fd_,&meta_,META_LEN);
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

    uint32_t get_blk_end(uint32_t file_no,uint32_t blk_no)
    {
        assert(file_no < index_.size());
        assert(blk_no  < meta_.blk_cnt_max_);
        return index_[file_no][blk_no];
    
    }
    uint32_t get_blk_start(uint32_t file_no,uint32_t blk_no)
    {
        assert(file_no < index_.size());
        assert(blk_no  < meta_.blk_cnt_max_);
        if(0 == blk_no)
            return 0;
        return index_[file_no][blk_no-1];
    }

    

    int add_idx(iovec *iov,uint32_t cnt,uint32_t file_no,uint32_t blk_no)
    {
        assert(file_no < index_.size());
        assert(blk_no + cnt <= meta_.blk_cnt_max_);
        assert(-1 != fd_);
        for (size_t i = 0; i < cnt; i++)
        {
            index_[file_no][blk_no+i] = iov[i].iov_len;
        }
        xwrite(fd_,index_[file_no]+blk_no,cnt*IDX_LEN);
        cnt_+=cnt;
        return 0;
    }

    int fill_iov_len(iovec *iov,uint32_t cnt,uint32_t file_no,uint32_t blk_no)
    {
        assert(file_no < index_.size());
        assert(blk_no + cnt <= meta_.blk_cnt_max_);

        for(size_t i = 0; i < cnt; i++)
        {
            iov[i].iov_len = get_blk_end(file_no,blk_no+i) - get_blk_start(file_no,blk_no+i);
        }
        
        return 0;
    }

    idx_t* create_idx_to_vec()
    {
        idx_t *offset_array = new idx_t[meta_.blk_cnt_max_];
        index_.push_back(offset_array);
        return offset_array;
    }
public:
    uint64_t cnt_ = 0;                      // 已写索引数量

    std::vector<idx_t*> index_;
    int fd_ = -1;
    io_meta meta_;
};



