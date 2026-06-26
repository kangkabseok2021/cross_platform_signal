#include "Serializer.h"
#include "SessionManager.h"
#include "TelemetryServer.h"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <gtest/gtest.h>
#include <chrono>
#include <thread>

TEST(Integration, ServerBroadcastsJsonToConnectedClient) {
    SessionManager  mgr;
    TelemetryServer server(mgr, 19001);
    std::thread     srv([&] { server.run(1); });

    // Give the acceptor time to bind
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // Connect a synchronous Beast client
    net::io_context ioc;
    tcp::resolver   resolver(ioc);
    auto            ep = resolver.resolve("127.0.0.1", "19001");
    websocket::stream<beast::tcp_stream> ws(ioc);
    beast::get_lowest_layer(ws).connect(ep);
    ws.handshake("127.0.0.1", "/");

    // Allow server-side session to register
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Broadcast one frame
    FrameResult r;
    r.frame_id     = 7;
    r.timestamp_ms = 12345;
    r.contours.push_back({0, 0, 10, 10});
    mgr.broadcast(Serializer::toJson(r).dump());

    // Receive and validate
    beast::flat_buffer buf;
    ws.read(buf);
    auto j = nlohmann::json::parse(beast::buffers_to_string(buf.data()));

    EXPECT_EQ(j["frame_id"].get<uint64_t>(), 7u);
    EXPECT_EQ(j["timestamp_ms"].get<uint64_t>(), 12345u);
    ASSERT_FALSE(j["contours"].empty());
    EXPECT_EQ(j["contours"][0]["area"].get<int>(), 100);

    ws.close(websocket::close_code::normal);
    server.stop();
    srv.join();
}
