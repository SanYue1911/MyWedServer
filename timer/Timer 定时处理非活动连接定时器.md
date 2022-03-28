## Timer 定时处理非活动连接定时器

* 利用alarm定时发送信号
* 处理SIGALRM和SIGTERM信号
* 信号处理函数使用unix域管道传递信号
* 使用epoll监听管道并遍历链表处理超时连接





