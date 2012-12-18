#ifndef SET_H
#define SET_H

#include <stdint.h>

template <size_t BITS>
class Set {
    public:
        Set();
        void      Clear();
        void      Add(size_t bit);
        void      Remove(size_t bit);
        bool      Has(size_t bit) const;
        uint64_t  Hash() const;

        bool      operator ==(Set &set) const;

    private:
        uint64_t  Mask(size_t bit) const;

        uint64_t  bits[(BITS+63)/64];
};

template <size_t BITS>
Set<BITS>::Set()
{
    Clear();
}

template <size_t BITS>
void Set<BITS>::Clear()
{
    for (size_t i = 0; i < sizeof(bits)/8; i++)
        bits[i] = 0;
}

template <size_t BITS>
void Set<BITS>::Add(size_t bit)
{
    bits[bit >> 6] |= Mask(bit & 63);
}

template <size_t BITS>
void Set<BITS>::Remove(size_t bit)
{
    bits[bit >> 6] &= ~Mask(bit & 63);
}

template <size_t BITS>
bool Set<BITS>::Has(size_t bit) const
{
    return (bits[bit >> 6] & Mask(bit & 63)) != 0;
}

template <size_t BITS>
uint64_t Set<BITS>::Hash() const
{
    uint64_t sum = 0;
    for (size_t i = 0; i < sizeof(bits)/8; i++)
        sum += bits[i];
    return sum;
}

template <size_t BITS>
bool Set<BITS>::operator ==(Set &set) const
{
    for (size_t i = 0; i < sizeof(bits)/8; i++)
        if (bits[i] != set.bits[i])
            return false;
    return true;
}

template <size_t BITS>
uint64_t Set<BITS>::Mask(size_t bit) const
{
    return (uint64_t)1 << bit;
}

#endif
