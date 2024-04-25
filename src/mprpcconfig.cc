#include "mprpcconfig.h"

#include <iostream>
#include <string>

// 负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char *config_file) // 函数参数是一个const char* 的指针变量类型的参数
{
    FILE *pf = fopen(config_file, "r");//fopen函数读取这个指针指向的文件，是以一个”r“只读的操作去获取
    if (nullptr == pf)
    {
        std::cout << config_file << " is note exist!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 1.注释   2.正确的配置项 =    3.去掉开头的多余的空格 
    while(!feof(pf)) //使用feof函数来检查是否达到末尾，如果没有到达末尾就一直执行
    {
        char buf[512] = {0};//缓冲区初始化
        fgets(buf, 512, pf);//从pf文件中读取最多511字节加一个终止符'\0'的数据到缓冲区buf

        // 去掉字符串前面多余的空格
        std::string read_buf(buf);//拷贝构造函数
        Trim(read_buf);

        // 判断#的注释
        if (read_buf[0] == '#' || read_buf.empty())
        {
            continue;
        }

        // 解析配置项
        int idx = read_buf.find('=');//找 =
        if (idx == -1)//没找到=说明配置文件没写对
        {
            // 配置项不合法
            continue;
        }

        std::string key;
        std::string value;
        key = read_buf.substr(0, idx);
        Trim(key);
        // rpcserverip=127.0.0.1\n
        int endidx = read_buf.find('\n', idx);
        value = read_buf.substr(idx+1, endidx-idx-1);
        Trim(value);
        m_configMap.insert({key, value});
    }

    fclose(pf);
}

// 查询配置项信息
std::string MprpcConfig::Load(const std::string &key)
{
    auto it = m_configMap.find(key);
    if (it == m_configMap.end())
    {
        return "";
    }
    return it->second;
}

// 去掉字符串前后的空格，这里只是一个简单的处理示例，在实际中，可能要对string进行更多的控制，来让他满足我们的要求
void MprpcConfig::Trim(std::string &src_buf)
{
    int idx = src_buf.find_first_not_of(' ');//寻找第一个不是' '的字符，是C++函数库中提供的函数，直接调用
    if (idx != -1)
    {
        // 说明字符串前面有空格
        src_buf = src_buf.substr(idx, src_buf.size()-idx);//获取从idx开始，一直到最后的字符串
    }
    // 去掉字符串后面多余的空格（字符串后面可能也有空格）
    idx = src_buf.find_last_not_of(' ');
    if (idx != -1)
    {
        // 说明字符串后面有空格
        src_buf = src_buf.substr(0, idx+1);
    }
}