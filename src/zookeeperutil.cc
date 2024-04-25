#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <semaphore.h>
#include <iostream>

/*
1.使用Zookeeper的意义：为了支持高并发，每个客户端都保存了一份服务提供者的列表，但是如果列表有更新，想要得到最新的URL列表（RPC服务的IP和端口号Port），必须要手动更新配置文件，很不方便
因此：需要动态更新URL列表，由此引入Zookeeper服务配置中心
zookeeper是为分布式应用提供一致性协调服务的中间件
还提供全局分布式锁，起到协调管理各个分布式节点的功能
*/

// 全局的watcher观察器   zkserver给zkclient的通知
void global_watcher(zhandle_t *zh, int type,
                   int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT)  // 回调的消息类型是和会话相关的消息类型，检测到一些类型了包括：连接成功、连接丢失、会话过期、重新连接
	{
		if (state == ZOO_CONNECTED_STATE)  // zkclient和zkserver连接成功
		{
			sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);//V操作
		}
	}
}

ZkClient::ZkClient() : m_zhandle(nullptr)
{
}

ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle); // 关闭句柄，释放资源  MySQL_Conn
    }
}

// 连接zkserver
void ZkClient::Start()
{
	//从配置文件中读取zookeeper客户端ip和端口port
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;
    
	/*
	zookeeper_mt：多线程版本
	zookeeper的API客户端程序提供了三个线程
	API调用线程 
	网络I/O线程  pthread_create  poll （底层为poll,且会在1/3的timeout时间内发送ping 心跳保持与zkserver的通信）
	watcher回调线程 pthread_create 当zkclient接收zkserver的响应后，zkserver给zkclient通知zookeeper是异步连接过程，需要绑定一个全局的回调函数global_watcher(新线程连接)
	*/
	//下面的m_zhandle只是创建了一个句柄，并不代表zkserver创建成功，要看全局回调函数global_watcher是否连接成功
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);//zookeeper_init函数会直接建立三个线程：API调用线程、网络I/O收发线程、watcher回调线程
    if (nullptr == m_zhandle) 
    {
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }
	//下面代码使用了POSIX线程库中的信号量semaphore机制，与zookeeper客户端相结合，用于同步控制线程和控制对共享资源的访问。
    sem_t sem;//声明信号量变量
    sem_init(&sem, 0, 0);//初始化这个信号量，第二个0表示信号量的类型是匿名信号量，非共享的，第三个参数是信号量的初始值（这里设置为0,表示没有线程可以立刻获取到这个信号量）
    zoo_set_context(m_zhandle, &sem);//在ZooKeeper操作完成时唤醒等待的线程，这里zoo_set_context设置m_zhandle状态与sem信号量相关，这里初始化为0,表示初始时都不能访问ZooKeeper，直到ZooKeeper把信号量加1

    sem_wait(&sem);//使用信号量的机制可以避免在zookeeper客户端未完全初始化的情况下执行依赖于zookeeper的操作
    std::cout << "zookeeper_init success!" << std::endl;
}


//Create用于在zookeeper中创建znode节点
void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
	// 先判断path表示的znode节点是否存在，如果存在，就不再重复创建了
	flag = zoo_exists(m_zhandle, path, 0, nullptr);
	if (ZNONODE == flag) // 表示path的znode节点不存在
	{
		// 创建指定path的znode节点了
		flag = zoo_create(m_zhandle, path, data, datalen,
			&ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
		if (flag == ZOK)
		{
			std::cout << "znode create success... path:" << path << std::endl;
		}
		else
		{
			std::cout << "flag:" << flag << std::endl;
			std::cout << "znode create error... path:" << path << std::endl;
			exit(EXIT_FAILURE);
		}
	}
}

// 根据指定的path，获取znode节点的值
std::string ZkClient::GetData(const char *path)
{
    char buffer[64];
	int bufferlen = sizeof(buffer);
	int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
	if (flag != ZOK)
	{
		std::cout << "get znode error... path:" << path << std::endl;
		return "";
	}
	else
	{
		return buffer;
	}
}