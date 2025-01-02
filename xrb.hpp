#pragma once
#include "xcom.hpp"

int is_power_of_two(uint64_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

uint64_t next_power_of_two(uint64_t n) {
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    return n + 1;
}

class rb_base
{
public:
    
    uint64_t get_unread_cnt()
    {
        uint64_t unread_cnt = w_idx_ - r_idx_;
        std::atomic_thread_fence(std::memory_order_acquire);
        return unread_cnt;
    }

    uint64_t get_unread_cnt_tail()
    {
        uint64_t unread_cnt = w_idx_ - r_idx_;
        uint64_t r_pos = w_idx_ & blk_mask_;
        uint64_t tail_cnt = blk_cnt_ - r_pos;
        std::atomic_thread_fence(std::memory_order_acquire);

        return std::min(tail_cnt, unread_cnt);
    }

    uint64_t get_free_cnt()
    {
        uint64_t unread_cnt = w_idx_ - r_idx_;
        uint64_t free_cnt = blk_cnt_ - unread_cnt;
        std::atomic_thread_fence(std::memory_order_acquire);
        return free_cnt;
    }

    // 计算尾部以前的空闲空间
    uint64_t get_free_cnt_tail()
    {
        uint64_t unread_cnt = w_idx_ - r_idx_;
        uint64_t free_cnt = blk_cnt_ - unread_cnt;

        uint64_t w_pos = w_idx_ & blk_mask_;
        uint64_t tail_cnt = blk_cnt_ - w_pos;
        std::atomic_thread_fence(std::memory_order_acquire);
        return std::min(tail_cnt, free_cnt);
    }

    void writer_done(uint64_t cnt)
    {
        std::atomic_thread_fence(std::memory_order_release);
        w_idx_ = w_idx_ + cnt;
        print();
    }

    void reader_done(uint64_t cnt)
    {
        std::atomic_thread_fence(std::memory_order_release);
        r_idx_ = r_idx_ + cnt;
        print();
    }

    void print()
    {
        if(!print_change_)
            return ;

        print_info();
    }

    void print_info(const char *p = nullptr)
    {
        if(p == nullptr)
            p = "";

        printf("[%s][0x%p]w_idx=%ju,r_idx=%ju,get_unread_cnt=%ju,get_free_cnt=%ju\n",
               p,this,w_idx_, r_idx_, get_unread_cnt(), get_free_cnt());
    }

    void init(uint64_t blk_cnt)
    { 
        blk_cnt = next_power_of_two(blk_cnt);
        blk_cnt_ = blk_cnt;
        blk_mask_ = blk_cnt - 1;
    }
    

protected:
    bool print_change_ = false;

    volatile uint64_t w_idx_ = 0;
    volatile uint64_t r_idx_ = 0;

    volatile uint64_t blk_cnt_ = 0;
    volatile uint64_t blk_mask_ = 0;
};


class rb:public rb_base
{
public:
    bool init(uint64_t blk_cnt,uint64_t blk_size)
    {
        blk_cnt_ = blk_cnt;
        blk_size_ = blk_size;
        buf_ = (char*)malloc(blk_cnt*blk_size);

        return true;
    }

    char* writer_get_buffer()
    {
        if (get_free_cnt() == 0)
        {
            return nullptr;
        }
        
        return get_buffer(w_idx_);
    }

    char* reader_get_buffer()
    {
        if (get_unread_cnt() == 0)
        {
            return nullptr;
        }
        return get_buffer(r_idx_);
    }

private:
    char* get_buffer(uint64_t idx)
    {
        idx = idx % blk_cnt_;// TODO
        return buf_ + idx * blk_size_;
    }

    char *buf_ = nullptr;
    volatile uint64_t blk_size_ = 0;
};


class rb_iov:public rb_base
{
public:
    bool init(uint64_t blk_cnt,uint64_t blk_size)
    {
        rb_base::init(blk_cnt);
        blk_size_ = next_power_of_two(blk_size);

        iov_ = new iovec[blk_cnt];
        mem_ = (char*)malloc(blk_cnt*blk_size);

        for (size_t i = 0; i < blk_cnt; i++)
        {
            iov_[i].iov_base = mem_ + i*blk_size_;
            iov_[i].iov_len = blk_size_;
        }
        
        return true;
    }

    iovec* writer_get_blk(uint64_t &cnt)
    {
        uint64_t wpos = w_idx_ & blk_mask_;
        iovec *ret = &iov_[wpos];
        cnt = get_free_cnt_tail();
        
        return ret;
    } 

    iovec* reader_get_blk(uint64_t &cnt)
    {
        uint64_t rpos = r_idx_ & blk_mask_;
        iovec *ret = &iov_[rpos];
        cnt = get_unread_cnt_tail();
        
        return ret;
    }


public:
    iovec *iov_ = nullptr;
    char *mem_ = nullptr;

    uint64_t blk_size_ = 0;
};