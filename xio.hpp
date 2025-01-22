#pragma once
#include "xcom.hpp"
#include "xio_idx.hpp"
#include "xfile.hpp"

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



// ./io_save/io_save_20170512.index
// ./io_save/io_save_20170512.0
// ./io_save/io_save_20170512.1
class io
{
public:
    int init(const io_context &ctx)
    {
        ctx_ = ctx;
        xfile_fix_foder(ctx_.path_);
        idx_path_ = ctx_.path_ + ctx_.prefix_ + ".index";
        data_pre_ = ctx_.path_ + ctx_.prefix_ + ".";

        if (io_rw_type::rw_write == ctx_.rw_type_)
        {
            if(io_init_type::init_new ==ctx_.init_type_)
            {
                return write_new();
            }else
            {
                return write_exist();
            }
        }else{

        }
        
        return 0;
    }

    int write_data(iovec *iov,uint64_t cnt,uint32_t &written)
    {
        // todo check blk
        


        
    }
private:
    int write_exist()
    {
        idx_.load_exist(ctx_.meta_,idx_path_);
        return 0;
    }


    int write_new()
    {
        // 创建目录
        bool c_ret = xfile_create_foder(ctx_.path_);
        CHECK_RETV(c_ret,-1);

        // del old
        string cmd = "rm -rf " + data_pre_ + "*";
        printf("del cmd:%s\n",cmd.c_str());
        system(cmd.c_str());
        
        idx_.create_new(ctx_.meta_,idx_path_);
        return 0;
    }
public:
    uint32_t w_f_no_ = 0;
    uint32_t w_f_idx_ = 0;
    vector<int> fds_;

    io_context ctx_;
    
    io_idx idx_;
    string idx_path_ = "";

    string data_pre_ = "";
};