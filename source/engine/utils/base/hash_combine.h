#pragma once

namespace mango {
inline void hash_combine(size_t &seed, size_t hash) {
  hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
  seed ^= hash;
}
} // namespace mango