#pragma once
#include "xio.hpp"
#include "xrb.hpp"
#include "xtime.hpp"
#include "xevt.hpp"
#include "xargs.hpp"
#include "json.hpp"
#include "xio.hpp"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"


class rt_conf {
public:
typedef rapidxml::xml_node<char> xml_node_t;
    int read_conf(const string &conf_path)
    {
        read_xml_conf(conf_path);
        read_json_conf();



        return err_ok;
    }
    

    string get_xml_val(const string &xpath)
    {
        vector<string> xpath_vec = xsplit(xpath,"/");
        if(0 == xpath_vec.size())
        {
            return "";
        }
        xml_node_t * cur_node = xml_doc_;

        for (auto i = 0; i < xpath_vec.size(); i++)
        {
            cur_node = cur_node->first_node(xpath_vec[i].c_str());
            if(nullptr == cur_node)
            {
                return "";
            }
        }

        return cur_node->value();
    }


private:    
    int read_json_conf()
    {
        string json_path = get_xml_val("/root/json_path/");
        CHECK0_RETV(json_path.empty(),err_data_err);

        std::ifstream ifs(json_path);
        

        try{
            json_ = nlohmann::json::parse(ifs);
        }catch(...)
        {
            LOG_ERROR("json parse err");
            xexit(1);
        }
        return err_ok;
    }
    int read_xml_conf(const string &conf_path)
    {
        xml_file_ = new rapidxml::file<char>(conf_path.c_str());
        xml_doc_ = new rapidxml::xml_document<char>();
        try{
            xml_doc_->parse<0>(xml_file_->data());
        }catch(...)
        {
            LOG_ERROR("xml parse err\n");
            xexit(1);
        }
        
        return err_ok;

    }


public:
    rapidxml::file<char> *xml_file_;
    rapidxml::xml_document<char> *xml_doc_;
    
    nlohmann::json json_;
};

class rt_mgr {
public:
    int start(int argc, char** argv)
    {
        string conf_path = xx_get_arg(argc,argv,"-c");
        conf_.read_conf(conf_path);

        int init_ret = 0;
        init_ret = create_engine();
        XASSERT(0 == init_ret);

        init_ret = create_rb();
        XASSERT(0 == init_ret);

        init_ret = create_plugs();
        XASSERT(0 == init_ret);

        return 0;
    }

    int run_all_engine()
    {
        for(auto &one : engines_)
        {
            one.second.run();
        }
        return 0;
    }
private:
    

    int create_engine()
    {
        auto engine_json = conf_.json_["engines"];
        for(auto &item : engine_json)
        {
            string engine_name = item["name"];
            engines_[engine_name].init();
        }
        return 0;
    }
    int create_rb()
    {
        auto rb_json = conf_.json_["itc_channels"];
        for(auto &item : rb_json)
        {
            rb_iov_ntf *rb = new rb_iov_ntf;
            rb->init(item["blk_cnt"],item["blk_size"]);
            rbs_[item["id"]] = rb;
        }
        return 0;
    }
    int create_plugs()
    {
        create_io_plugs();
        
        return 0;
    }

    void read_io_conf(io_context &ctx,nlohmann::json item,rb_iov_ntf* rb_ptr)
    {
        ctx.meta_.blk_cnt_max_ = item["blk_cnt_max"];
        ctx.meta_.blk_size_ = rb_ptr->blk_size_;

        string rw = item["rw"];
        if("r" == rw )
        {
            ctx.rw_type_ = io_rw_type::rw_read;
        }else if("w" == rw)
        {
            ctx.rw_type_ = io_rw_type::rw_write;
            ctx.init_type_ = item["init_type"];
        }else{
            XASSERT(false);
        }
    }
    int create_io_plugs()
    {
        auto io_json = conf_.json_["plugs"]["io_plugs"];
        for(auto item : io_json)
        {
            auto* rb_ptr = get_itc(item["itc_id"]);
            CHECK_RETV(nullptr != rb_ptr,err_data_err);
            
            auto ee_ptr = get_ee(item["engine_name"]);
            CHECK_RETV(nullptr != ee_ptr,err_data_err);

            io_context ctx;
            read_io_conf(ctx,item,rb_ptr);
            
            xio_evt *io_ptr = new xio_evt();
            int io_ret = io_ptr->init(ctx,rb_ptr,ee_ptr);
            CHECK0_RETV(io_ret,err_data_err);
            io_[item["id"]] = io_ptr;
        }

        return 0;

    }
public: 
    rb_iov_ntf* get_itc(uint32_t itc_id)
    {
        auto it = rbs_.find(itc_id);
        CHECK_RETV(it != rbs_.end(),nullptr);
        return it->second;
    }

    xepoll* get_ee(const string& ee_name)
    {
        auto it = engines_.find(ee_name);
        CHECK_RETV(it != engines_.end(),nullptr);
        return it->second.ep_; 
    }
public:
    std::map<string,epoll_thread>   engines_;
    std::map<uint32_t,rb_iov_ntf*>      rbs_;
    
    std::map<uint32_t,xio_evt*>          io_;

    rt_conf conf_;
};