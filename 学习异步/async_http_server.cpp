#include <iostream>
#include <string>
#include <memory>
#include <boost/asio.hpp>
#include <thread>

using namespace std;

class HttpConnection: public std::enable_shared_from_this<HttpConnection>
{
public:
  HttpConnection(boost::asio::io_context& io): socket_(io) {}

  void Start()
  {
    auto p = shared_from_this();
    boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(request_), "\r\n\r\n",
        [p, this](const boost::system::error_code& err, size_t len) {
      if(err)
      {
        cout<<"recv err:"<<err.message()<<"\n";
        return;
      }
      string first_line = request_.substr(0, request_.find("\r\n")); // should be like: GET / HTTP/1.0
      cout<<first_line<<"\n";
      // process with request
      // ...
      cout << "HttpConnection ID: " << this_thread::get_id() << endl;
      char str[] = "HTTP/1.0 200 OK\r\n\r\n"
          "<html>hello from http server</html>";
      boost::asio::async_write(socket_, boost::asio::buffer(str), [p, this](const boost::system::error_code& err, size_t len) {
        socket_.close();
      });
    });
  }

  boost::asio::ip::tcp::socket& Socket() { return socket_; }
private:
  boost::asio::ip::tcp::socket socket_;
  string request_;
};

class HttpServer
{
public:
  HttpServer(boost::asio::io_context& io, boost::asio::ip::tcp::endpoint ep): io_(io), acceptor_(io, ep) {}

  void Start()
  {
    auto p = std::make_shared<HttpConnection>(io_); // 事件循环io 赋值给socket_
    acceptor_.async_accept(p->Socket(), [p, this](const boost::system::error_code& err) {
      if(err)
      {
        cout<<"accept err:"<<err.message()<<"\n";
        return;
      }
      cout << "HttpServer ID: " << this_thread::get_id() << endl;
      cout<<"connection start 1" <<endl;
      p->Start();
      cout<<"connection start 2" <<endl;
      cout<<"server start 1" <<endl;
      Start();
      cout<<"server start 2" <<endl;
    });
  }
private:
  boost::asio::io_context& io_; // 事件循环
  boost::asio::ip::tcp::acceptor acceptor_;  // 接受器
};

int main(int argc, const char* argv[])
{
  if(argc != 3)
  {
    cout<<"usage: httpsvr ip port\n";
    return 0;
  }

  boost::asio::io_context io;
  boost::asio::ip::tcp::endpoint ep(boost::asio::ip::make_address(argv[1]), std::stoi(argv[2]));
  HttpServer hs(io, ep);
  hs.Start();
  io.run();
  return 0;
}