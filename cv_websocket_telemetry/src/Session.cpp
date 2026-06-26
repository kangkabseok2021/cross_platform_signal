#include "Session.h"

Session::Session(tcp::socket socket) : ws_(std::move(socket)) {}

void Session::start() {
    ws_.async_accept([self = shared_from_this()](beast::error_code ec) {
        self->onAccept(ec);
    });
}

void Session::onAccept(beast::error_code ec) {
    if (ec) return;
    doRead();
}

void Session::doRead() {
    ws_.async_read(buf_,
        [self = shared_from_this()](beast::error_code ec, std::size_t bytes) {
            self->onRead(ec, bytes);
        });
}

void Session::onRead(beast::error_code ec, std::size_t) {
    if (ec == websocket::error::closed) return;
    if (ec) return;
    buf_.consume(buf_.size());
    doRead();
}

void Session::sendJson(const std::string& payload) {
    auto self = shared_from_this();
    auto msg  = std::make_shared<std::string>(payload);
    net::post(ws_.get_executor(), [self, msg]() {
        self->ws_.async_write(net::buffer(*msg),
            [self, msg](beast::error_code, std::size_t) {});
    });
}
