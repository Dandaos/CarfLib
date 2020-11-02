#include "HttpServer.h"
#include "HttpContext.h"
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
    callback_(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(buf.beginRead(),buf.readableBytes());
    if (response.closeConnection())
    {
      conn->shutdownWrite();
    }
}