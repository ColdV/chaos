# chaos
A C++ game server frame in embryo

这是预想发展为一个游戏服务器框架的个人代码案例.目前核心代码包含在src/common中(网络,线程,定时器等基础组件的实现)

已实现:

    · 简单的数据库封装src/common/db
    · 简单的日志库src/common/log
    · 线程和线程池的封装src/common/thread
    · 基于reactor的事件库 src/common/net
        - 支持多线程的事件处理(one loop per thread线程模型)
        - 网络事件:windows iocp的实现，linux epoll的实现.
        - 定时器事件:最小堆实现的定时器.

#BUILDING
##CMAKE(UNIX)
```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

##CMAKE(WINDOWS)
```
$ mkdir build
$ cd build
$ cmake ..
$ 运行生成在目录下的.sln 工程文件
```



#一个简单的服务器(src/servers/gamesvr/main.cpp)
#一个简单的客户端(src/client/main.cpp)
        -可用命令行执行./clien [ip][port][连接数]