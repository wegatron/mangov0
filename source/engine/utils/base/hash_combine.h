#pragma once

#include <cstddef> // Add missing include directive for size_t

namespace mango {
inline void
hash_combine(size_t &seed,
             size_t hash) { // Fix function declaration by adding closing
                            // parenthesis and semicolon
  hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
  seed ^= hash;
}
} // namespace mango