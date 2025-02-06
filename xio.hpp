#pragma once
#include "xcom.hpp"
#include "xio_idx.hpp"
#include "xfile.hpp"
#include <map>
#include <mutex>

enum class io_rw_type : char {
    rw_read,
    rw_write,
    rw_end
};

enum class io_init_type : char {
    init_exist,
    init_new,
    init_end
};

struct io_context
{
    io_meta meta_;
    io_rw_type rw_type_ = io_rw_type::rw_write;
    io_init_type init_type_ = io_init_type::init_new;// for write

    string path_ = "./io_save/";
    string prefix_ = "io_save";
};



static io_idx* get_idx(const string &key)
{
    static std::mutex lock;
    static std::map<string,io_idx*>    idx_map;

    std::lock_guard<std::mutex> l(lock);

    auto it = idx_map.find(key);
    if(it != idx_map.end())
    {
        return it->second;
    }else{
        io_idx *idx = new (std::nothrow) io_idx();
        idx_map.insert(std::make_pair(key,idx));
        return idx;
    }
}

static void calc_iov_cnt(iovec *iov,uint32_t iov_cnt,uint32_t tot_len,
                            uint32_t &iov_full_cnt,uint32_t &remain)
{
    iov_full_cnt = 0;
    remain = tot_len;
    for(int i = 0; i < iov_cnt; i++)
    {
        if(remain >= iov[i].iov_len)
        {
            iov_full_cnt++;
            remain -= iov[i].iov_len;
        }else{
            break;
        }
    }
}

// ./io_save/io_save_20170512.index
// ./io_save/io_save_20170512.0
// ./io_save/io_save_20170512.1
class io
{
public:
    ~io()
    {
        release();
    }
    int init(const io_context &ctx)
    {
        ctx_ = ctx;
        xfile_fix_foder(ctx_.path_);
        idx_path_ = ctx_.path_ + ctx_.prefix_ + ".index";
        data_pre_ = ctx_.path_ + ctx_.prefix_ + ".";
        idx_ = get_idx(data_pre_);

        init_fd();
        if (io_rw_type::rw_write == ctx_.rw_type_)
        {
            if(io_init_type::init_new ==ctx_.init_type_)
            {
                init_write_new();
            }else
            {
                init_write_exist();
            }
        }else{
            init_read();
        }
        
        
        return 0;
    }
    // 不许跨文件读
    int read_data(iovec *iov,uint32_t cnt,uint32_t start_idx,uint32_t &read)
    {
        read = 0;
        
        uint32_t blk_max = idx_->meta_.blk_cnt_max_;

        uint32_t blk_no = start_idx % blk_max;
        uint32_t file_no = start_idx / blk_max;
        uint32_t offset = idx_->get_blk_start(file_no,blk_no);

        cnt = std::min(blk_max - blk_no,cnt);// 不跨文件
        cnt = std::min(idx_->cnt_ - start_idx,(uint64_t)cnt);// 不能超过最多大数量
        if(0 == cnt)
            return 0;

        if(-1 == fds_[file_no])
        {
            string path = data_pre_ + std::to_string(file_no);
            fds_[file_no] = open(path.c_str(),O_RDWR);
        }
        

        idx_->fill_iov_len(iov,cnt,file_no,blk_no);
        ssize_t rret = preadv(fds_[file_no],iov,cnt,offset);
        uint32_t remain = 0;
        calc_iov_cnt(iov,cnt,rret,read,remain);
        if (0 != remain || cnt != read)
            printf("xxx read war rret:%ld,read:%u,remain:%u\n",rret,read,remain);
        return 0;
    }



    // 不许跨文件写
    int write_data(iovec *iov,uint32_t cnt,uint32_t &written)
    {
        written = 0;

        uint32_t blk_max = idx_->meta_.blk_cnt_max_;

        uint32_t blk_no = idx_->cnt_ % blk_max;
        uint32_t file_no = idx_->cnt_ / blk_max;
        
        cnt = std::min(blk_max - blk_no,cnt);

        // 需要新文件
        if(-1 == fds_[file_no])
        {
            XASSERT(0 == blk_no);
            create_file(file_no);
        }

        uint32_t offset = idx_->get_blk_start(file_no,blk_no);

        // 写一次，能写多少是多少
        ssize_t wret = pwritev(fds_[file_no],iov,cnt,offset);

        uint32_t remain = 0;
        calc_iov_cnt(iov,cnt,wret,written,remain);
        if (0 != remain || cnt != written)
            printf("xxx write war wret:%ld,written:%u,remain:%u\n",wret,written,remain);

        idx_->add_idx(iov,written,file_no,blk_no);


        return 0;
    }

    void release()
    {
        for (size_t i = 0; i < max_file_cnt_; i++)
        {
            if (-1 != fds_[i])
            {
                close(fds_[i]);
                fds_[i] = -1;
            }
        }
        delete [] fds_;
    }

private:
    void init_read()
    {

    }

    void init_fd()
    {
        int64_t max_file_cnt_ = 65535;
        int64_t sys_max_open_cnt = sysconf(_SC_OPEN_MAX);
        if(-1 != sys_max_open_cnt && sys_max_open_cnt < max_file_cnt_)
            max_file_cnt_ = sys_max_open_cnt;
        fds_ = new (std::nothrow) int[max_file_cnt_];
        std::fill_n(fds_,max_file_cnt_,-1);
    }

    int create_file(uint32_t file_no)
    {
        string path = data_pre_ + std::to_string(file_no);
        int fd = open(path.c_str(),O_RDWR|O_CREAT|O_TRUNC,0666);
        fds_[file_no] = fd;

        idx_->create_idx_to_vec();
        return 0;
    }

    int init_write_exist()
    {
        idx_->load_exist(ctx_.meta_,idx_path_);
        return 0;
    }

    int init_write_new()
    {
        // 创建目录
        bool c_ret = xfile_create_foder(ctx_.path_);
        CHECK_RETV(c_ret,-1);

        // del old
        string cmd = "rm -rf " + data_pre_ + "*";
        printf("del cmd:%s\n",cmd.c_str());
        system(cmd.c_str());
        
        idx_->create_new(ctx_.meta_,idx_path_);
        return 0;
    }
public:

    int *fds_ = nullptr;
    uint32_t max_file_cnt_ = 0;

    io_context ctx_;
    
    io_idx *idx_;
    string idx_path_ = "";

    string data_pre_ = "";
};