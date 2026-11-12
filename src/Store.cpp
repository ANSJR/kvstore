#include "kv/Store.hpp"
#include <mutex>

namespace kv {

void Store::set(std::string key, std::string value, std::optional<TimePoint> expiresAt) {
    std::unique_lock lock(storeMutex);
    // add heap update here later
    entryIndex.insert_or_assign(std::move(key), Entry{std::move(value), expiresAt});
}

bool Store::del(const std::string& key, TimePoint now) {
    std::unique_lock lock(storeMutex);

    auto it = entryIndex.find(key);
    if (it == entryIndex.end()) return false;
    bool live = !isExpired(it->second, now);
    // add heap removal here later
    entryIndex.erase(it);
    return live;

}

std::size_t Store::sweep(TimePoint now) {
    return 0;
}

void Store::clear() {
    std::unique_lock lock(storeMutex);
    entryIndex.clear();
    // add expiryIndex.clear() here during heap update
}

std::optional<std::string> Store::get(const std::string& key, TimePoint now) const {
    std::shared_lock lock(storeMutex);

    auto it = entryIndex.find(key);
    if (it == entryIndex.end() || isExpired(it->second, now)) return std::nullopt;
    return it->second.value;
}

std::optional<TimePoint> Store::nextExpiry() const {
    return std::nullopt;
}

std::size_t Store::size() const {
    return entryIndex.size();
}

bool Store::isExpired(const Entry& e, TimePoint now) {
    return e.expiresAt && *e.expiresAt <= now;
}

} // namespace kv