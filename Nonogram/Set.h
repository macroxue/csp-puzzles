#ifndef SET_H
#define SET_H

#include <stdint.h>

template <size_t BITS>
class Set {
 public:
  Set();
  void Clear();
  void Add(size_t bit);
  void Remove(size_t bit);
  bool Has(size_t bit) const;

  bool operator==(const Set &set) const;
  uint64_t Hash(__uint128_t a[], size_t b) const;

 private:
  uint64_t Mask(size_t bit) const;

  uint64_t bits[(BITS + 63) / 64];
};

template <size_t BITS>
Set<BITS>::Set() {
  Clear();
}

template <size_t BITS>
void Set<BITS>::Clear() {
  for (size_t i = 0; i < sizeof(bits) / 8; i++) bits[i] = 0;
}

template <size_t BITS>
void Set<BITS>::Add(size_t bit) {
  bits[bit >> 6] |= Mask(bit & 63);
}

template <size_t BITS>
void Set<BITS>::Remove(size_t bit) {
  bits[bit >> 6] &= ~Mask(bit & 63);
}

template <size_t BITS>
bool Set<BITS>::Has(size_t bit) const {
  return (bits[bit >> 6] & Mask(bit & 63)) != 0;
}

template <size_t BITS>
bool Set<BITS>::operator==(const Set &set) const {
  for (size_t i = 0; i < sizeof(bits) / 8; i++)
    if (bits[i] != set.bits[i]) return false;
  return true;
}

template <size_t BITS>
uint64_t Set<BITS>::Mask(size_t bit) const {
  return (uint64_t)1 << bit;
}

template <size_t BITS>
uint64_t Set<BITS>::Hash(__uint128_t a[], size_t b) const {
  __uint128_t sum = 0;
  for (size_t i = 0; i < sizeof(bits) / 8; i++) {
    sum += a[i] * bits[i];
  }
  return sum >> (128 - b);
}

#endif
