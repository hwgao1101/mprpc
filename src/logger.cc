#include "logger.h"
#include <time.h>
#include <iostream>

/*
由于RPC服务器端采用了epoll + 多线程，并发处理来自客户端的请求，所以有可能造成多线程同时写日志信息
将日志信息写入一个queue中，然后新建日志线程。但是需要注意的是，由于rpcprovider类中是epoll + 多线程，所以会创建多个日志线程，这样必须要保证线程安全。
*/

// 获取日志的单例
Logger& Logger::GetInstance()//这是获取Logger类实例的静态成员函数，实现了单例模式。 单例模式确保了整个程序中只有一个Logger对象存在。。
{
    static Logger logger;//定义一个局部的静态变量， 在第一次调用GetInstance()时被初始化，并且其生命周期与程序的周期相同，因为局部静态变量的构造函数只在程序首次加载该函数的地址时执行了一次，这就确保了logger对象只被创建一次
    return logger;
}

Logger::Logger()
{
    // 启动专门的写日志线程，这样做的好处是可以将日志写入操作与主线程的执行解耦，提高系统的性能和响应速度。
    std::thread writeLogTask([&](){//([&]())是一个lambda表达式，定义了线程的入口点，即线程要执行的任务
        for (;;)
        {
            // 获取当前的日期，然后取日志信息，写入相应的日志文件当中 a+
            time_t now = time(nullptr);
            tm *nowtm = localtime(&now);

            char file_name[128];
            sprintf(file_name, "%d-%d-%d-log.txt", nowtm->tm_year+1900, nowtm->tm_mon+1, nowtm->tm_mday);

            FILE *pf = fopen(file_name, "a+");
            if (pf == nullptr)
            {
                std::cout << "logger file : " << file_name << " open error!" << std::endl;
                exit(EXIT_FAILURE);
            }
            
            //从日志队列中获取日志信息
            std::string msg = m_lckQue.Pop();//m_lckQue是一个线程安全的队列，用来存储待写入日志文件的日志信息

            char time_buf[128] = {0};
            sprintf(time_buf, "%d:%d:%d =>[%s] ", 
                    nowtm->tm_hour, 
                    nowtm->tm_min, 
                    nowtm->tm_sec,
                    (m_loglevel == INFO ? "info" : "error"));
            msg.insert(0, time_buf);
            msg.append("\n");

            fputs(msg.c_str(), pf);
            fclose(pf);
        }
    });
    // 设置分离线程，守护线程
    writeLogTask.detach();//将日志线程设置为分离线程，使得他能够在后台独立运行，不会影响主线程的执行
}

// 设置日志级别 
void Logger::SetLogLevel(LogLevel level)
{
    m_loglevel = level;
}

// 写日志， 把日志信息写入lockqueue缓冲区当中
void Logger::Log(std::string msg)
{
    m_lckQue.Push(msg);
}