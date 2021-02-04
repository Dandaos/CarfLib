#include "HttpServer.h"
#include "HttpContext.h"
#include<sys/mman.h> 
void HttpServer::onConnection(TcpConnectionPtr&conn)
{
    conn->setContext(HttpContext());
}
void HttpServer::onMessage(TcpConnectionPtr conn,Buffer*buf)
{
    HttpContext* context=boost::any_cast<HttpContext>(conn->getMutableContext());
    if (!context->parseRequest(buf))
    {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdownWrite();
        LOG_WARN("Bad Request!");
    }

    if (context->gotAll())
    {
        onRequest(conn, context->request());
        context->reset();
    }
}
void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
    const std::string& connection = req.getHeader("Connection");
    bool close = connection == "close" ||(req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
    HttpResponse response(close);
    defaultHttpCallback(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(buf.beginRead(),buf.readableBytes());
    if (response.closeConnection())
    {
      conn->shutdownWrite();
    }
}
void HttpServer::sql_pool_init(std::string url, std::string User, std::string PassWord, 
std::string DBName, int Port, int MaxConn, int close_log){
    sql_pool->init(url,User,PassWord,DBName,Port,MaxConn,close_log);
    //先从连接池中取一个连接
    MYSQL *mysql = NULL;
    connectionRAII mysqlcon(&mysql, sql_pool);
    //在user表中检索username，passwd数据
    if (mysql_query(mysql, "SELECT username,passwd FROM user"))
    {
        LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
    }
    //从表中检索完整的结果集
    MYSQL_RES *result = mysql_store_result(mysql);
    //返回结果集中的列数
    int num_fields = mysql_num_fields(result);
    //返回所有字段结构的数组
    MYSQL_FIELD *fields = mysql_fetch_fields(result);
    //从结果集中获取下一行，将对应的用户名和密码，存入map中
    while (MYSQL_ROW row = mysql_fetch_row(result))
    {
        std::string temp1(row[0]);
        std::string temp2(row[1]);
        users[temp1] = temp2;
    }
}
void HttpServer::defaultHttpCallback(const HttpRequest&req,HttpResponse*resp)
{
    char file_name[200];
    getcwd(file_name,200);
    int len=strlen(file_name);
    if (req.path() == "/")
    {
        strcpy(file_name+len,"/root/judge.html");
    }
    else if(req.path()=="/0"){ //注册页面
        strcpy(file_name+len,"/root/register.html");
    }
    else if(req.path()=="/1"){  //登录页面
        strcpy(file_name+len,"/root/log.html");
    }
    else if(req.path()=="/3CGISQL.cgi"||req.path()=="/2CGISQL.cgi"){
        //提取账户、密码
        char name[100], password[100];
        int i;
        const std::string &body=req.getBody();
        for (i = 5; body[i] != '&'; ++i)
            name[i - 5] = body[i];
        name[i - 5] = '\0';
        int j = 0;
        for (i = i + 10; body[i] != '\0'; ++i, ++j)
            password[j] = body[i];
        password[j] = '\0';
        //构建SQL语句
        char *sql_insert = (char *)malloc(sizeof(char) * 200);
        strcpy(sql_insert, "INSERT INTO user(username, passwd) VALUES(");
        strcat(sql_insert, "'");
        strcat(sql_insert, name);
        strcat(sql_insert, "', '");
        strcat(sql_insert, password);
        strcat(sql_insert, "')");
        //获取一个SQL连接
        MYSQL*mysql=nullptr;
        connectionRAII mysqlcon(&mysql, sql_pool);
        if(req.path()=="/3CGISQL.cgi"){    //注册
            if (users.find(name) == users.end())
            {
                int res = mysql_query(mysql, sql_insert);
                if (!res){
                    strcpy(file_name+len, "/root/log.html"); //注册成功，转到登录页面
                    users.insert(pair<string, string>(name, password));
                }
                else
                    strcpy(file_name+len, "/root/registerError.html");
            }
            else
                strcpy(file_name+len, "/root/registerError.html");
        }
        else{      //登录
            if (users.find(name) != users.end() && users[name] == password)
                strcpy(file_name+len, "/root/welcome.html");
            else
                strcpy(file_name+len, "/root/logError.html");
        }
    }
    else if(req.path()=="/5"){
        strcpy(file_name+len,"/root/picture.html");
    }
    else if(req.path()=="/6"){
        strcpy(file_name+len,"/root/video.html");
    }
    else if(req.path()=="/7"){
        strcpy(file_name+len,"/root/fans.html");
    }
    else{
        strcat(file_name,"/root");
        strcat(file_name,req.path().c_str());
        //std::cout<<file_name<<std::endl;
    }
    int file_fd=open(file_name,O_RDONLY);
    if(file_fd==-1){
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("404 NOT FOUND!");
        resp->addHeader("Server","Carflib");
    }
    else{
        std::string body;
        char buf[1024];
        int n=0;
        while((n=read(file_fd,buf,1024))!=0){
            body+=std::string(buf,n);
        }
        //std::cout<<body.size()<<std::endl;
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        //resp->setContentType("text/html");
        resp->addHeader("Server", "Carflib");
        resp->setBody(body);
        close(file_fd);
    }
}