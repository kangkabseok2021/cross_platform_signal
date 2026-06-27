#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <functional>
#include <memory>
#include <string>

namespace beast     = boost::beast;
namespace net       = boost::asio;
using     tcp       = net::ip::tcp;
namespace websocket = beast::websocket;

class Session : public std::enable_shared_from_this<Session> {
public:
    explicit Session(tcp::socket socket);
    void start(std::function<void()> on_ready = {});
    void sendJson(const std::string& payload);

private:
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer                   buf_;

    void onAccept(beast::error_code ec, std::function<void()> on_ready);
    void doRead();
    void onRead(beast::error_code ec, std::size_t bytes);
};
