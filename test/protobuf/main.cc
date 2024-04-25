#include "test.pb.h"
#include <iostream>
#include <string>
using namespace fixbug;

int main()
{
    // LoginResponse rsp;
    // ResultCode *rc = rsp.mutable_result();
    // rc->set_errcode(1);
    // rc->set_errmsg("登录处理失败了");
    
    GetFriendListsResponse rsp;
    ResultCode *rc = rsp.mutable_result();
    rc->set_errcode(0);

    User *user1 = rsp.add_friend_list();
    user1->set_name("zhang san");
    user1->set_age(20);
    user1->set_sex(User::MAN);

    User *user2 = rsp.add_friend_list();
    user2->set_name("li si");
    user2->set_age(22);
    user2->set_sex(User::MAN);

    std::cout << rsp.friend_list_size() << std::endl;

    return 0;
}

int main1()
{
    //下面这些只是涉及的业务示例，“进行登录，设置数据信息”，并不是分布式框架的代码

    // 封装了login请求对象的数据
    LoginRequest req;
    req.set_name("zhang san");
    req.set_pwd("123456");

    // 对象数据序列化 =》 char*， 下面相当于是调用方把Login所用到的东西序列化打包到send_str当中
    std::string send_str;
    if (req.SerializeToString(&send_str)) //SerializeToString是继承于message类的， 获取字符串指针类型的指针变量
    {
        std::cout << send_str.c_str() << std::endl;
    }

    //中间经历传输过程，但是这个过程由RPC通信框架完成，我们不需要进行过多考虑

    // 从send_str反序列化一个login请求对象
    LoginRequest reqB;
    if (reqB.ParseFromString(send_str)) //这里相当于被调用方把原来打包好的再反序列化出来
    {
        std::cout << reqB.name() << std::endl;
        std::cout << reqB.pwd() << std::endl;
    }

    return 0;
}