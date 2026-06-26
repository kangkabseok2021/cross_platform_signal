#include "SessionManager.h"
#include <algorithm>

void SessionManager::add(std::shared_ptr<Session> s) {
    std::lock_guard lock(mu_);
    sessions_.push_back(std::move(s));
}

void SessionManager::broadcast(const std::string& payload) {
    std::lock_guard lock(mu_);
    for (auto& wp : sessions_) {
        if (auto sp = wp.lock()) sp->sendJson(payload);
    }
    prune();
}

std::size_t SessionManager::clientCount() const {
    std::lock_guard lock(mu_);
    std::size_t n = 0;
    for (const auto& wp : sessions_) {
        if (!wp.expired()) ++n;
    }
    return n;
}

void SessionManager::prune() {
    sessions_.erase(
        std::remove_if(sessions_.begin(), sessions_.end(),
            [](const std::weak_ptr<Session>& wp) { return wp.expired(); }),
        sessions_.end());
}
