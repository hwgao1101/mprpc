#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "mprpcchannel.h"

/*
这里的代码是caller要调用远程的服务的示例，这只是完成的调用方的代码，在这里，有login和register两个业务
为了能够实现远程调用，使用RPC通信框架来帮助通信，实现业务的时候不需要关注通信的底层实现
- 初始化mprpc框架（Init,把ip给设置进去）
- 创建一个UserServiceRpc_Stub的对象，这个对象是框架的一部分，用这个对象来进行序列化、反序列化和通信
- 创建需要传输的参数，fixbug::LoginRequest request， 设置请求中的参数
- 创建一个对象，存储远程调用返回后的响应 fixbug::LoginResponse response
- 用stub和刚才创建的传输参数的对象来调用远程函数获得response
*/


int main(int argc, char **argv)
{
    // 整个程序启动以后，想使用mprpc框架来享受rpc服务调用，一定需要先调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法Login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());//创建一个stub对象，stub对象对参数进行序列化和反序列化以及调用框架
    // rpc方法的请求参数
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    // rpc方法的响应
    fixbug::LoginResponse response;//创建一个响应，用来保存被调用方程序运行的结果
    // 发起rpc方法的调用  同步的rpc调用过程  MprpcChannel::callmethod
    //用创建的stub对象调用方法
    stub.Login(nullptr, &request, &response, nullptr); // RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送

    // 一次rpc调用完成，读调用的结果，这里是调用方成功调用以后，读取response的结果
    if (0 == response.result().errcode())
    {
        std::cout << "rpc login response success:" << response.sucess() << std::endl;
    }
    else
    {
        std::cout << "rpc login response error : " << response.result().errmsg() << std::endl;
    }

    // 演示调用远程发布的rpc方法Register
    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("mprpc");
    req.set_pwd("666666");
    fixbug::RegisterResponse rsp;

    // 以同步的方式发起rpc调用请求，等待返回结果
    stub.Register(nullptr, &req, &rsp, nullptr); 

    // 一次rpc调用完成，读调用的结果
    if (0 == rsp.result().errcode())
    {
        std::cout << "rpc register response success:" << rsp.sucess() << std::endl;
    }
    else
    {
        std::cout << "rpc register response error : " << rsp.result().errmsg() << std::endl;
    }
    
    return 0;
}