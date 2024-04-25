#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

/*
这个代码是作为rpc远程调用的服务提供方，因此也叫做provider
这里的代码的作用是，接收到RPC传过来的调用的参数，把远程调用请求的参数解析出来，在这里把操作执行完，然后把执行过后的结果放在response中返回回去，一同返回回去的
还有状态码等一下东西，具体见代码当中

UserService原来是一个本地服务，提供了两个进程内的本地方法，Login和GetFriendLists
*/
class UserService : public fixbug::UserServiceRpc // 使用在rpc服务发布端（rpc服务提供者）
{
public:
    bool Login(std::string name, std::string pwd)//这里是被调用方自己的业务，这里的业务就是执行一下登录，完全是自己的需求，完全不属于RPC通信框架
    {
        std::cout << "doing local service: Login" << std::endl;
        std::cout << "name:" << name << " pwd:" << pwd << std::endl;
        return false;
    }

    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing local service: Register" << std::endl;
        std::cout << "id:" << id << "name:" << name << " pwd:" << pwd << std::endl;
        return true;
    }

    /*
    重写基类UserServiceRpc的虚函数 下面这些方法都是框架直接调用的, 因为RPC通信框架只是提供一个远程的调用功能，所以远程端必须实现proto文件编译生成的class UserServiceRpc : public ::google::protobuf::Service里面的定义的虚函数方法
    1. caller   ===>   Login(LoginRequest)  => muduo =>   callee 
    2. callee   ===>    Login(LoginRequest)  => 交到下面重写的这个Login方法上了
    */
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done)
    {
        // 框架给业务上报了请求参数LoginRequest，应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 做本地业务
        bool login_result = Login(name, pwd); //因为Login是这里实现的，需要RPC把结果返回回去

        // 把响应写入  包括错误码、错误消息、返回值
        fixbug::ResultCode *code = response->mutable_result();//创建一个code指针变量指向mutable_result，方便后面对mutable_result里面存储信息
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_sucess(login_result);

        // 执行回调操作   执行响应对象数据的序列化和网络发送（都是由框架来完成的）
        done->Run();
    }

    void Register(::google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool ret = Register(id, name, pwd);//在本地执行业务

        response->mutable_result()->set_errcode(0);//把执行业务的结果放到response里面去
        response->mutable_result()->set_errmsg("");
        response->set_sucess(ret);

        done->Run();//执行回调操作
    }
};

int main(int argc, char **argv)
{
    // 调用框架的初始化操作
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象。把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());

    // 启动一个rpc服务发布节点   Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

    return 0;
}