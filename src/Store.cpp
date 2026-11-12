#include "kv/Store.hpp"

namespace kv {

void Store::set(std::string key, std::string value, std::optional<TimePoint> expiresAt) {

}

bool Store::del(const std::string& key) {
    return false;
}

std::size_t Store::sweep(TimePoint now) {
    return 0;
}

void Store::clear() {
    
}

std::optional<std::string> Store::get(const std::string& key) const {
    return std::nullopt;
}

std::optional<TimePoint> Store::nextExpiry() const {
    return std::nullopt;
}

std::size_t Store::size() const {
    return entryIndex.size();
}

bool Store::isExpired(const Entry& e, TimePoint now) {
    return false;
}

} // namespace kv