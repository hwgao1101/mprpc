# 分布式RPC通信框架

>en_US [English](../README.md) 

本项目是基于强大的开源网络库Muduo和数据序列化库protobuf实现的分布式RPC通信框架， 实现分布式系统中远程服务的通信和调用。

![RPC通信框架图](../assets/rpc_01.png)


## 环境配置
1. linux系统环境：ubuntu20.04
2. 帮助构建高性能、可靠的服务器应用程序：[boost库 + muduo库](https://blog.csdn.net/m0_46392035/article/details/124502811)
3. 用于C++中数据序列化和反序列化：[protobuf](https://github.com/protocolbuffers/protobuf)
4. 分布式协调服务：[zookeeper](https://blog.csdn.net/weixin_43559950/article/details/118890904)

## 功能要点
1. 使用muduo网络库实现高并发的RPC同步调用请求处理
2. 使用Protobuf作为RPC方法调用和参数的序列化和反序列化
3. 基于线程安全的缓冲队列实现异步日志输出
4. 使用zookeeper作为服务治理中间件，提供服务注册和服务发现功能
