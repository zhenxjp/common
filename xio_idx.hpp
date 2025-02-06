#pragma once
#include "xcom.hpp"
#include "xerrcode.hpp"


static ssize_t xwrite(int fd, const void *buf, size_t count) {
    const char *cbuf = (const char *)buf;
    ssize_t nwritten = 0, totwritten = 0;

    while(count) {
        nwritten = write(fd, cbuf, count);

        if (nwritten < 0) {
            if (errno == EINTR) 
                continue;
            printf("write errno=%u %s\n",errno,strerror(errno));
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
            printf("read errno=%u %s\n",errno,strerror(errno));
            return totread;
        }
        if (nread == 0)
            return totread;

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
    string dump() const
    {
        string str;
        str += "io_type_:" + to_string(io_type_) + ",";
        str += "blk_size_:" + to_string(blk_size_) + ",";
        str += "blk_cnt_max_:" + to_string(blk_cnt_max_) + "\n";
        return str;
    }
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

    // 加载已存在索引
    int load_exist(const io_meta& meta,const string &path)
    {
        int ret = read_idx_head(meta,path);
        if(err_ok != ret)
        {
            return create_new(meta,path);
        }
       

        auto file_size = xfile_get_size(path);
        XASSERT(file_size >= META_LEN);

        uint64_t idx_cnt = (file_size - META_LEN) / IDX_LEN;// todo check
        
        while (idx_cnt)
        {
            idx_t *offset_array = create_idx_to_vec();
            uint64_t read_cnt = std::min(meta_.blk_cnt_max_,idx_cnt);
            
            ssize_t read_len = xread(fd_,offset_array,read_cnt * IDX_LEN);

            // 索引没读全，可能丢数据
            if(read_cnt * IDX_LEN != read_len)
            {
                printf("read cnt not equal %ju %ju\n", read_cnt,read_len);
                return err_data_err;
            }
            idx_cnt -= read_cnt;
            cnt_ += read_cnt;
        }
        
        return err_ok;
    }


    int create_new(const io_meta& meta,const string &path)
    {
        release();
        meta_ = meta;
        fd_ = open(path.c_str(),O_RDWR|O_CREAT|O_TRUNC,0666);
        CHECK_RETV(fd_,err_file_open_err);

        auto wret = xwrite(fd_,&meta_,META_LEN);
        if(META_LEN != wret)
        {
            printf("write meta error %u %ju\n",META_LEN,wret);
            return err_file_write_err;
        }
        return err_ok;
    }

    void release()
    {
        cnt_ = 0;
        for (size_t i = 0; i < index_.size(); i++)
        {
            XSAFE_DELETE_ARRAY(index_[i]);
        }
        index_.clear();
        XSAFE_CLOSE(fd_);
    }

    idx_t get_blk_end(uint32_t file_no,uint32_t blk_no)
    {
        XASSERT(file_no < index_.size());
        XASSERT(blk_no  < meta_.blk_cnt_max_);
        return index_[file_no][blk_no];
    }
    idx_t get_blk_start(uint32_t file_no,uint32_t blk_no)
    {
        XASSERT(file_no < index_.size());
        XASSERT(blk_no  < meta_.blk_cnt_max_);
        if(0 == blk_no)
            return 0;
        return index_[file_no][blk_no-1];
    }

    

    int add_idx(iovec *iov,uint32_t cnt,uint32_t file_no,uint32_t blk_no)
    {
        XASSERT(file_no < index_.size());
        XASSERT(blk_no + cnt <= meta_.blk_cnt_max_);
        XASSERT(-1 != fd_);

        idx_t last_off = 0;
        if(blk_no > 0)
            last_off = get_blk_end(file_no,blk_no-1);

        for (size_t i = 0; i < cnt; i++)
        {
            index_[file_no][blk_no + i] = last_off + iov[i].iov_len;
            last_off += iov[i].iov_len;
        }
        xwrite(fd_,index_[file_no]+blk_no,cnt*IDX_LEN);
        std::atomic_thread_fence(std::memory_order_release);
        cnt_+=cnt;
        return err_ok;
    }

    int fill_iov_len(iovec *iov,uint32_t cnt,uint32_t file_no,uint32_t blk_no)
    {
        XASSERT(file_no < index_.size());
        XASSERT(blk_no + cnt <= meta_.blk_cnt_max_);

        for(size_t i = 0; i < cnt; i++)
        {
            iov[i].iov_len = get_blk_end(file_no,blk_no+i) - get_blk_start(file_no,blk_no+i);
        }
        
        return err_ok;
    }

    idx_t* create_idx_to_vec()
    {
        idx_t *offset_array = new (std::nothrow) idx_t[meta_.blk_cnt_max_];
        XASSERT(nullptr != offset_array);
        index_.push_back(offset_array);
        return offset_array;
    }

    uint64_t cnt()
    {
        uint64_t temp = cnt_;
        std::atomic_thread_fence(std::memory_order_acquire);
        return temp;
    }
private:
    int read_idx_head(const io_meta& meta,const string &path)
    {
        if(!xfile_exists(path))
        {
            printf("read_idx_head idx file not exist:%s\n",path.c_str());
            return err_file_not_found;
        }

        fd_ = open(path.c_str(),O_RDWR);
        if(-1 == fd_)
        {
            printf("read_idx_head open file %s error,errno=%u\n",path.c_str(),errno);
            return err_file_open_err;
        }
        
        ssize_t read_len = xread(fd_,&meta_,META_LEN);
        if(read_len != META_LEN )
        {
            printf("load exist failed read len=%ju,ok=%u\n",read_len,META_LEN);
            return err_data_err;
        }

        if(0 != memcmp(&meta_,&meta,META_LEN))
        {
            printf("load exist failed");
            printf("real meta dump:\n%s\n",meta_.dump().c_str());
            printf("init meta dump:\n%s\n",meta.dump().c_str());

            return err_data_err;
        }
        return err_ok;
    }
private:
    uint64_t cnt_ = 0;                      // 已写索引数量

    std::vector<idx_t*> index_;
    int fd_ = -1;
    io_meta meta_;
};



