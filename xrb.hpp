#pragma once
#include "xcom.hpp"

class rb_base
{
public:
    
    uint64_t get_unread_cnt()
    {
        atomic_thread_fence(std::memory_order_acquire);
        return w_idx_ - r_idx_;
    }

    uint64_t get_free_cnt()
    {
        return blk_cnt_ - get_unread_cnt();
    }

    // 获取可写位置
    uint64_t writer_get_idx()
    {
        if (0 == get_free_cnt())
        {
            return -1;
        }
        return w_idx_;
    }
    // 获取可读位置
    uint64_t reader_get_idx()
    {
        if (0 == get_unread_cnt())
        {
            return -1;
        }
        return r_idx_;
    }

    void writer_done(uint64_t cnt)
    {
        auto temp = w_idx_ + cnt;
        std::atomic_thread_fence(std::memory_order_release);
        w_idx_ = temp;
        print();
    }

    void reader_done(uint64_t cnt)
    {
        auto temp = r_idx_ + cnt;
        std::atomic_thread_fence(std::memory_order_release);
        r_idx_ = temp;
        print();
    }

    void print()
    {
        if(!print_)
            return ;

        printf("[0x%p]w_idx=%ju,r_idx=%ju,get_unread_cnt=%ju,get_free_cnt=%ju\n",
               this,w_idx_, r_idx_, get_unread_cnt(), get_free_cnt());
    }

    void init(uint64_t blk_cnt)
    {
        blk_cnt_ = blk_cnt;
    }
    

protected:
    bool print_ = false;

    volatile uint64_t w_idx_ = 0;
    volatile uint64_t r_idx_ = 0;

    volatile uint64_t blk_cnt_ = 0;
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
        return get_buffer(writer_get_idx());
    }

    char* reader_get_buffer()
    {
        return get_buffer(reader_get_idx());
    }

private:
    char* get_buffer(uint64_t idx)
    {
        if(-1 == idx){
            return nullptr;
        }
        idx = idx % blk_cnt_;// TODO
        return buf_ + idx * blk_size_;
    }

    char *buf_ = nullptr;
    volatile uint64_t blk_size_ = 0;
};