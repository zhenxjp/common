#pragma once

#include "xcom.hpp"

#define SAMECHAR(a, b) (((a) | 0x20) == ((b) | 0x20))

static const char *xstrstr(const char *pSrc, const char *pDst, bool bCaseSensitive = true)
{
    if (nullptr == pSrc)
    {
        return nullptr;
    }

    const char *pToFind = NULL;
    const char *pBase = NULL;
    while (0 != *pSrc)
    {
        pBase = pSrc;
        pToFind = pDst;
        if (bCaseSensitive)
        {
            while (0 != *pBase && 0 != *pToFind && *pBase == *pToFind)
            {
                ++pBase;
                ++pToFind;
            }
        }
        else
        {
            while (0 != *pBase && 0 != *pToFind && SAMECHAR(*pBase, *pToFind))
            {
                ++pBase;
                ++pToFind;
            }
        }
        if (0 == *pToFind)
        {
            return pSrc;
        }
        else
        {
            ++pSrc;
        }
    }
    return NULL;
}

static string find_str_ex(const char *&buf, const char *key1, const char *key2)
{
    const char *find1 = xstrstr(buf, key1);
    const char *find2 = xstrstr(find1, key2);

    if (!find1 || !find2)
    {
        return "";
    }
    const char *begin = find1 + strlen(key1);
    int diff = find2 - find1;
    int len = diff - strlen(key1);

    buf = find2 + strlen(key2);
    return string(begin, len);
}

static string find_str(const char *buf, const char *key1, const char *key2)
{
    const char *temp = buf;
    return find_str_ex(temp,key1,key2);
}

static vector<string> find_strs(const char *buf, const char *key1, const char *key2)
{
    vector<string> ret;
    while(true)
    {
        string one = find_str_ex(buf,key1,key2);
        if(one.empty())
            break;
        ret.push_back(one);
    }
    return ret;
}

static vector<string> xsplit(const string &str, const string &pattern)
{
    vector<string> resVec;
    if ("" == str)
    {
        return resVec;
    }

    //方便截取最后一段数据
    string strs = str + pattern;

    size_t pos_last = 0;
    while(true)
    {
        size_t pos_find = strs.find(pattern,pos_last);
        if(string::npos == pos_find)
        {
            break;
        }
        string find = strs.substr(pos_last,pos_find-pos_last);
        if(!find.empty())
        {
            resVec.push_back(find);
        }
        
        pos_last = pos_find + pattern.size();   
    }
    if(pos_last < str.size())
    {
        string last = str.substr(pos_last,str.size()-pos_last);
        if(!last.empty())
        {
            resVec.push_back(last);
        }
        
    }
    return resVec;
}

static string xmerge(const vector<string> &strs, const string &pattern)
{
    string ret;
    for (const auto &i : strs)
    {
        ret += i;
        ret += pattern;
    }

    return ret;
}



static string delete_tag(string ori_buf,char left,char right)
{
    string txt_buf;
    bool in = false;
    for (int i = 0; i < ori_buf.length(); ++i)
    {
        char c = ori_buf[i];

        if (in)
        {
            if (right == c)
            {
                in = false;
            }
        }
        else
        {
            if (left == c)
            {
                in = true;
            }
            else
            {
                txt_buf += c;
            }
        }
    }

    return txt_buf;
}

static string delete_html_tag(string ori_buf)
{
    return delete_tag(ori_buf,'<','>');
}

static int replace_all(std::string &str, const std::string &old_value, const std::string &new_value)
{
    int find_cnt = 0;
    while (true)
    {
        std::string::size_type pos(0);
        if ((pos = str.find(old_value)) != std::string::npos)
        {
            str.replace(pos, old_value.length(), new_value);
            ++find_cnt;
        }
        else
        {
            break;
        }
    }
    return find_cnt;
}
static int replace_all2(std::string &str, const std::string &old_value, const std::string &new_value)
{
    int find_cnt = 0;
    std::string::size_type pos(0);
    while (true)
    {
        
        if ((pos = str.find(old_value,pos)) != std::string::npos)
        {
            str.replace(pos, old_value.length(), new_value);
            ++find_cnt;
            pos += old_value.size();
        }
        else
        {
            break;
        }
    }
    return find_cnt;
}

static int find_str_cnt(const std::string &str, const std::string &key)
{
    int find_cnt = 0;
    std::string::size_type pos(0);
    while (true)
    {
        
        if ((pos = str.find(key,pos)) != std::string::npos)
        {
            ++find_cnt;
            pos += key.size();
        }
        else
        {
            break;
        }
    }
    return find_cnt;
}

#include <locale>
#include <codecvt>

static string utf8_encode(const wstring &source)
{
    return wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(source);
}

static wstring utf8_decode(const string &source)
{
    return wstring_convert<codecvt_utf8<wchar_t>>().from_bytes(source);
}

static string xx_to_str(const wstring &source)
{
    return utf8_encode(source);
}

static wstring xx_to_wstr(const string &source)
{
    return utf8_decode(source);
}

static int get_utf8_cnt(const string &str)
{
    wstring wstr = utf8_decode(str);
    return wstr.length();
}

static unsigned char ToHex(unsigned char x) 
{ 
    return  x > 9 ? x + 55 : x + 48; 
}
static string get_url_encode(string str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (isalnum((unsigned char)str[i]) || 
            (str[i] == '-') ||
            (str[i] == '_') || 
            (str[i] == '.') || 
            (str[i] == '~'))
            strTemp += str[i];
        else if (str[i] == ' ')
            strTemp += "+";
        else
        {
            strTemp += '%';
            strTemp += ToHex((unsigned char)str[i] >> 4);
            strTemp += ToHex((unsigned char)str[i] % 16);
        }
    }
    return strTemp;
}

static bool str_ends_with(const string &str,const string & x)
{
    if(str.size() < x.size())
        return false;

    string temp = str.substr(str.size() - x.size(),x.size());
    return temp == x;
}

static bool str_start_with(const string &str,const string & x)
{
    if(str.size() < x.size())
        return false;

    string temp = str.substr(0,x.size());
    return temp == x;
}


/////////////////////////////////////////////////////////////////////////


static void test_str_with()
{
    cout<<str_start_with("aaabbbccc","aaa")<<endl;;
    cout<<str_start_with("aaabbbccc","aa")<<endl;;
    cout<<str_start_with("aaabbbccc","aab")<<endl;;

    cout<<str_ends_with("aaabbbccc","ccc")<<endl;;
    cout<<str_ends_with("aaabbbccc","cc")<<endl;;
    cout<<str_ends_with("aaabbbccc","cbc")<<endl;;
    cout<<"===================="<<endl;

}
static void test_str_xsplit_one(string all,string part)
{
    auto ret = xsplit(all,part);
    cout<<"=========="<<endl;
    cout<<"all="<<all<<endl;
    cout<<"part="<<part<<endl;
    cout<<ret.size()<<endl;
    
    for(auto it:ret)
    {
        cout<<it<<endl;
    }
}


static void test_str_xsplit()
{
    test_str_xsplit_one("##aaa##","##");
    test_str_xsplit_one("aaa##","##");
    test_str_xsplit_one("##aaa","##");
    test_str_xsplit_one("aaa","##");

    test_str_xsplit_one("aaa##bbb","##");
    test_str_xsplit_one("##aaa##bbb","##");
    test_str_xsplit_one("aaa##bbb##","##");
    test_str_xsplit_one("##aaa##bbb##","##");

    test_str_xsplit_one("aaa##bbb##ccc","##");
    test_str_xsplit_one("##aaa##bbb##ccc","##");
    test_str_xsplit_one("aaa##bbb##ccc##","##");
    test_str_xsplit_one("##haaa##bbb##ccc##","##");


}

static void test_str()
{
    cout<< get_url_encode("啦啦啦")<<endl;

    wstring dd = L"你好";
    string ddbytes = utf8_encode(dd); // 按照UTF8格式进行编码，转为常规string，即得到UTF8格式的字节流

    wstring ret = utf8_decode(ddbytes); // 按照UTF8格式对字节流进行解码，转为wstring

    const char *buf = "xxx1111yyy";
    string s = find_str(buf, "xxx", "yyy");
    cout << s << endl;

    
    exit(0);
}