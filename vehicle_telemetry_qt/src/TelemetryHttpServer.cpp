#include "TelemetryHttpServer.h"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>

using json = nlohmann::json;

static const char* INDEX_HTML = R"html(<!DOCTYPE html>
<html lang="en">
<head><meta charset="UTF-8"><title>Vehicle Telemetry</title>
<style>
  body { font-family: monospace; background:#0d0d1a; color:#e2e8f0; padding:20px; }
  h1 { color:#60a5fa; }
  table { border-collapse:collapse; width:100%; }
  th,td { border:1px solid #2a2a3e; padding:8px 12px; text-align:left; }
  th { background:#1e1e2e; color:#a78bfa; }
  .ok { color:#22c55e; } .fault { color:#ef4444; }
</style></head>
<body>
<h1>Vehicle Telemetry</h1>
<p id="ts"></p>
<table id="tbl"><tr><th>Channel</th><th>Raw</th><th>Filtered</th><th>Unit</th><th>Alpha</th><th>Status</th></tr></table>
<script>
async function refresh() {
  const r = await fetch('/api/v1/telemetry');
  const d = await r.json();
  document.getElementById('ts').textContent = 'Updated: ' + d.ts;
  const rows = d.channels.map(c =>
    `<tr><td>${c.name}</td><td>${c.raw.toFixed(2)}</td><td>${c.filtered.toFixed(2)}</td>
     <td>${c.unit}</td><td>${c.alpha.toFixed(3)}</td>
     <td class="${c.fault?'fault':'ok'}">${c.fault?'FAULT':'OK'}</td></tr>`
  ).join('');
  document.getElementById('tbl').innerHTML =
    '<tr><th>Channel</th><th>Raw</th><th>Filtered</th><th>Unit</th><th>Alpha</th><th>Status</th></tr>' + rows;
}
setInterval(refresh, 500);
refresh();
</script></body></html>)html";

static std::string iso_now() {
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    std::tm tm_buf{};
#ifdef _WIN32
    gmtime_s(&tm_buf, &t);
#else
    gmtime_r(&t, &tm_buf);
#endif
    oss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

struct TelemetryHttpServer::Impl {
    httplib::Server svr;
    std::thread     thread;
    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
    GetSnapshot get_snap;
    SetAlpha    set_alpha;

    Impl(GetSnapshot gs, SetAlpha sa) : get_snap(std::move(gs)), set_alpha(std::move(sa)) {
        svr.set_pre_routing_handler([](const httplib::Request&, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            return httplib::Server::HandlerResponse::Unhandled;
        });

        svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
            res.set_content(INDEX_HTML, "text/html");
        });

        svr.Get("/api/v1/health", [this](const httplib::Request&, httplib::Response& res) {
            auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - start_time).count();
            json body = {{"status", "ok"}, {"uptime_s", uptime}};
            res.set_content(body.dump(), "application/json");
        });

        svr.Get("/api/v1/telemetry", [this](const httplib::Request&, httplib::Response& res) {
            auto snap = get_snap();
            json channels = json::array();
            for (std::size_t i = 0; i < 4; ++i) {
                const auto& ch = snap.channels[i];
                channels.push_back({
                    {"id",       static_cast<int>(i)},
                    {"name",     ch.name},
                    {"unit",     ch.unit},
                    {"raw",      ch.raw},
                    {"filtered", ch.filtered},
                    {"alpha",    snap.alphas[i]},
                    {"fault",    ch.fault}
                });
            }
            json body = {{"ts", iso_now()}, {"channels", channels}};
            res.set_content(body.dump(), "application/json");
        });

        svr.Get("/api/v1/channels/(\\d+)", [this](const httplib::Request& req, httplib::Response& res) {
            int id = std::stoi(req.matches[1]);
            if (id < 0 || id >= 4) { res.status = 404; return; }
            auto snap = get_snap();
            const auto& ch = snap.channels[static_cast<std::size_t>(id)];
            json body = {
                {"id",       id},
                {"name",     ch.name},
                {"unit",     ch.unit},
                {"raw",      ch.raw},
                {"filtered", ch.filtered},
                {"alpha",    snap.alphas[static_cast<std::size_t>(id)]},
                {"fault",    ch.fault}
            };
            res.set_content(body.dump(), "application/json");
        });

        svr.Post("/api/v1/channels/(\\d+)/alpha", [this](const httplib::Request& req, httplib::Response& res) {
            int id = std::stoi(req.matches[1]);
            if (id < 0 || id >= 4) { res.status = 400; return; }
            try {
                auto body = json::parse(req.body);
                double alpha = body.at("alpha").get<double>();
                set_alpha(id, alpha);
                json resp = {{"id", id}, {"alpha", alpha}, {"status", "ok"}};
                res.set_content(resp.dump(), "application/json");
            } catch (...) {
                res.status = 400;
            }
        });
    }
};

TelemetryHttpServer::TelemetryHttpServer(GetSnapshot get_snap, SetAlpha set_alpha)
    : impl_(std::make_unique<Impl>(std::move(get_snap), std::move(set_alpha))) {}

TelemetryHttpServer::~TelemetryHttpServer() { stop(); }

void TelemetryHttpServer::start(uint16_t port) {
    impl_->thread = std::thread([this, port] {
        impl_->svr.listen("0.0.0.0", static_cast<int>(port));
    });
}

void TelemetryHttpServer::stop() {
    impl_->svr.stop();
    if (impl_->thread.joinable())
        impl_->thread.join();
}
