#pragma once
#include "Session.h"
#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>

class SessionManager {
public:
    void add(std::shared_ptr<Session> s);
    void broadcast(const std::string& payload);
    [[nodiscard]] std::size_t clientCount() const;

private:
    mutable std::mutex                    mu_;
    std::vector<std::weak_ptr<Session>>   sessions_;

    void prune();
};
