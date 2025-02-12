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

    std::string path_ = "./io_save/";
    std::string prefix_ = "io_pre";
};

static io_idx* idx_op(const std::string &key,const std::string &op)
{
    static std::mutex lock;
    static std::map<std::string,io_idx*>    idx_map;

    std::lock_guard<std::mutex> l(lock);

    if("del" == op)
    {
        idx_map.erase(key);
        return nullptr;
    }else if("get_new" == op)
    {
        auto it = idx_map.find(key);
        XASSERT(it == idx_map.end());

        io_idx *idx = new (std::nothrow) io_idx();
        XASSERT(idx);
        idx_map.insert(std::make_pair(key,idx));
        return idx;
    }else if("get_exist" == op)
    {
        auto it = idx_map.find(key);
        XASSERT(it != idx_map.end());
        return it->second;
    }else{
        LOG_ERROR("io_op_type err: {}", op);
        XASSERT(0);
        return nullptr;
    }

}

static inline void calc_iov_cnt(iovec *iov,uint32_t iov_cnt,uint32_t tot_len,
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
        

        init_fd();
        if (io_rw_type::rw_write == ctx_.rw_type_)
        {
            if(io_init_type::init_new ==ctx_.init_type_)
            {
                return init_write_new();
            }else
            {
                return init_write_exist();
            }
        }else{
            return init_read();
        }
    }
    // 不许跨文件读
    int read_data(iovec *iov,uint32_t cnt,uint32_t start_idx,uint32_t &readed)
    {
        readed = 0;

        auto cur_cnt = idx_->cnt();
        if(cur_cnt <= start_idx || 0 == cnt)
            return err_ok;


        uint32_t blk_max = ctx_.meta_.blk_cnt_max_;

        uint32_t blk_no = start_idx % blk_max;
        uint32_t file_no = start_idx / blk_max;

        cnt = std::min(blk_max - blk_no,cnt);// 不跨文件
        cnt = std::min(cur_cnt - start_idx,(uint64_t)cnt);// 不能超过最多大数量
        
        if(0 == cnt)
            return err_ok;

        if(-1 == fds_[file_no])
        {
            int open_ret = open_file(file_no,O_RDWR);
            CHECK0_RETV(open_ret,open_ret);
        }
        
        uint32_t offset = idx_->get_blk_start(file_no,blk_no);
        idx_->fill_iov_len(iov,cnt,file_no,blk_no);
        ssize_t rret = preadv(fds_[file_no],iov,cnt,offset);
        if(-1 != rret)
        {
            uint32_t remain = 0;
            calc_iov_cnt(iov,cnt,rret,readed,remain);
            if (0 != remain || cnt != readed)
                LOG_WARN("xxx read war rret: {}, readed: {}, remain: {}", rret, readed, remain);
        }else{
            LOG_ERROR("preadv errno={} {}", errno, strerror(errno));
            return err_file_read_err;
        }

        return err_ok;
    }

    int open_file(uint32_t file_no,int open_flags)
    {
        std::string path = data_pre() + std::to_string(file_no);
        fds_[file_no] = open(path.c_str(),open_flags,0666);
        if(-1 == fds_[file_no])
        {
            LOG_ERROR("open file errno={} {} ,path={}", errno, strerror(errno), path);
            return err_file_open_err;
        }
        return err_ok;
    }

    // 不许跨文件写
    int write_data(iovec *iov,uint32_t cnt,uint32_t &written)
    {
        written = 0;

        uint32_t blk_max = ctx_.meta_.blk_cnt_max_;
        uint64_t w_cnt = idx_->w_get_cnt();

        uint32_t blk_no = w_cnt % blk_max;
        uint32_t file_no = w_cnt / blk_max;
        
        cnt = std::min(blk_max - blk_no,cnt);

        // 需要新文件
        if(-1 == fds_[file_no])
        {
            // XASSERT(0 == blk_no);
            int open_ret = open_file(file_no,O_RDWR|O_CREAT);
            CHECK0_RETV(open_ret,open_ret);

            idx_->create_idx_to_vec();
        }

        uint32_t offset = idx_->get_blk_start(file_no,blk_no);

        // 写一次，能写多少是多少
        ssize_t wret = pwritev(fds_[file_no],iov,cnt,offset);
        if (-1 != wret)
        {
            uint32_t remain = 0;
            calc_iov_cnt(iov, cnt, wret, written, remain);
            if (0 != remain || cnt != written)
                LOG_WARN("xxx write war wret: {}, written: {}, remain: {}", wret, written, remain);

            idx_->add_idx(iov, written, file_no, blk_no);
        }

        return err_ok;
    }

    void release()
    {
        for (size_t i = 0; i < fds_.size(); i++)
        {
            XSAFE_CLOSE(fds_[i]);
        }
        fds_.clear();
        // XSAFE_DELETE(idx_);
    }

private:
    int init_read()
    {
        idx_ = idx_op(data_pre(),"get_exist");
        XASSERT(nullptr != idx_);
        int equal_ret = ctx_.meta_.equal(idx_->meta_);
        CHECK0_RETV(equal_ret,err_data_err);

        return err_ok;
    }

    void init_fd()
    {
        int64_t max_file_cnt = 65535;
        int64_t sys_max_open_cnt = sysconf(_SC_OPEN_MAX);
        if(-1 != sys_max_open_cnt && sys_max_open_cnt < max_file_cnt)
            max_file_cnt = sys_max_open_cnt;
        max_file_cnt += 1000;
        fds_.resize(max_file_cnt,-1);
    }

    int init_write_exist()
    {
        idx_ = idx_op(data_pre(),"get_new");
        XASSERT(nullptr != idx_);
        int idx_ret = idx_->load_exist(ctx_.meta_,idx_path());
        CHECK_RETV(0 == idx_ret,idx_ret);

        int equal_ret = ctx_.meta_.equal(idx_->meta_);
        CHECK0_RETV(equal_ret,err_data_err);

        return err_ok;
    }

    int init_write_new()
    {
        idx_ = idx_op(data_pre(),"get_new");
        XASSERT(nullptr != idx_);
        // 创建目录
        bool c_ret = xfile_create_foder(ctx_.path_);
        CHECK_RETV(c_ret,err_file_op);

        // del old
        std::string cmd = "rm -rf " + data_pre() + "*";
        int sys_ret = system(cmd.c_str());
        // LOG_INFO("del cmd: {} ret={}", cmd, sys_ret);
        
        int idx_init = idx_->create_new(ctx_.meta_,idx_path());
        CHECK_RETV(0 == idx_init,idx_init);
        return err_ok;
    }

    const std::string data_pre()
    {
        return ctx_.path_ + ctx_.prefix_ + ".";
    }

    const std::string idx_path()
    {
        return ctx_.path_ + ctx_.prefix_ + ".index";
    }
public:
    std::vector<int>    fds_ ;

    io_context ctx_;
    
    io_idx *idx_;
};