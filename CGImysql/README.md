## MYSQL连接池

* 使用单例模式；
* 使用链表实现连接池；
* 使用信号量及互斥锁保证线程安全；
* 调用使用RAII机制（资源获取即初始化）；