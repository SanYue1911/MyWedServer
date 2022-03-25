#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>      //fcntl()
#include <sys/socket.h>
#include <netinet/in.h> //socketaddr_in 结构体
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>     
#include <pthread.h>
#include <stdio.h>       
#include <stdlib.h>     //atol()
#include <sys/mman.h>
#include <stdarg.h>     //va_list结构
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>    //iovec结构体
#include <map>

#include "../lock/locker.h"
#include "../CGImysql/sql_conn_pool.h"
#include "../timer/list_timer.h"
#include "../log/log.h"

class http_conn
{
public:
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;
    enum METHOD  //报文请求方法
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };
    enum CHECK_STATE  //主状态机
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    enum HTTP_CODE  //报文解析状态机
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
    enum LINE_STATUS  //行取读状态机
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

public:
    http_conn() {}
    ~http_conn() {}

public:
    void init(int sockfd, const sockaddr_in &addr, char *, int, int, string user, string passwd, string sqlname);
    void close_conn(bool real_close = true);
    void process();     //解析报文并制作回应报文
    bool read_once();   //读取浏览器发来的数据
    bool write();       //写入响应报文
    sockaddr_in *get_address()
    {
        return &m_address;
    }
    //初始化数据库读取表
    void initmysql_result(connection_pool *connPool);
    int timer_flag;
    int improv;


private:
    void init();
    HTTP_CODE process_read();           //从m_read_buf读取并处理
    bool process_write(HTTP_CODE ret);  //向m_write_buf写入响应报文

    //主状态机解析数据
    HTTP_CODE parse_request_line(char *text);
    HTTP_CODE parse_headers(char *text);
    HTTP_CODE parse_content(char *text);

    HTTP_CODE do_request(); //制作响应报文
    char *get_line() { return m_read_buf + m_start_line; };
    LINE_STATUS parse_line();//读取一行并返回报文读取状态
    void unmap();
    //制作响应报文函数
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_type();
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    static int m_epollfd;
    static int m_user_count;
    MYSQL *mysql;
    int m_state;  //读为0, 写为1

private:
    int m_sockfd;
    sockaddr_in m_address;
    char m_read_buf[READ_BUFFER_SIZE];  //储存读取的报文
    int m_read_idx;                     //读取buffer区最后一个字节的下一个位置
    int m_checked_idx;                  //已经读取到的位置
    int m_start_line;                   //已经解析的位置

    char m_write_buf[WRITE_BUFFER_SIZE];//储存发出的报文
    int m_write_idx;                    //发出报文的长度

    CHECK_STATE m_check_state;          //check_state状态
    METHOD m_method;                    //http请求状态

    //报文中的变量
    char m_real_file[FILENAME_LEN];     
    char *m_url;                        
    char *m_version;                    
    char *m_host;                       
    int m_content_length;               
    bool m_linger;

    char *m_file_address;               //读取文件在服务器中的地址
    struct stat m_file_stat;            //文件的state

    struct iovec m_iv[2];               //iovec结构体储存地址和长度
    int m_iv_count;

    int cgi;                            //是否启用的POST
    char *m_string;                     //存储请求头数据

    int bytes_to_send;                  //即将发送的数据
    int bytes_have_send;                //已经发送的数据
    char *doc_root;

    map<string, string> m_users;
    int m_TRIGMode;
    int m_close_log;

    char sql_user[100];
    char sql_passwd[100];
    char sql_name[100];
};

#endif
