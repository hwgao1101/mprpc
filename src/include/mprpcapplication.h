#pragma once

#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"

// mprpc框架的基础类，负责框架的一些初始化操作
class MprpcApplication
{
public:
    static void Init(int argc, char **argv); //static 关键字常用于成员函数或成员变量，表示属于类本身，不属于某个类的实例化对象
    static MprpcApplication& GetInstance(); //&返回一个当前对象的引用
    static MprpcConfig& GetConfig();
private:
    static MprpcConfig m_config;

    MprpcApplication(){}
    MprpcApplication(const MprpcApplication&) = delete;//初始化时利用了C++11中的新特性“=delete”是C++11的新特性，用于显式的删除函数，这意味这个类不能通过拷贝构造来实现实例化
    MprpcApplication(MprpcApplication&&) = delete;
};