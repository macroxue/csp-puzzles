#ifndef DOMAIN_H
#define DOMAIN_H

#include <assert.h>

#include <vector>
using namespace std;

//
// A domain has a set of values.
//
//  T: the type of values.
//
template <class T>
class Domain {
 public:
  // Domain defined by bounds.
  Domain(T low, T high);

  // Domain defined by discrete values.
  Domain(const T value[]);  // Ends with 0
  Domain(const T value[], size_t count);

  // Domain defined by a generator function of discrete values.
  // The generator function is called repeated to produce one value at a
  // time until it returns false. The state of the generator is saved in
  // memory that cookie points to.
  Domain(bool (*generator_fn)(void *, T &), void *cookie);

  // Is the domain empty?
  bool IsEmpty() const;

  // Is there a single value in the domain?
  bool IsSingle() const;

  // Does the domain contain the given value?
  bool Contains(T value) const;

  // Find in the domain the position of the given value.
  // A return value of GetSize() means value not found.
  size_t Find(T value) const;

  // Get the size of the domain.
  size_t GetSize() const;

  // Get the bounds of the domain.
  // Result is undefined if domain is empty.
  void GetBounds(T &low, T &high) const;

  // Limit the domain to be within the given bounds.
  // The new bounds may be tighter than the given bounds.
  // The domain may become empty.
  void LimitBounds(T low, T high);

  // Get the i-th value.
  T GetValue(size_t i) const;
  T operator[](size_t i) const;

  // Erase the i-th value.
  // The previous (i+1)-th value, if exists, becomes the new i-th value.
  void EraseValueAt(size_t i);

  // Keep values that are in this domain or another domain.
  void Union(const Domain &domain);

  // Keep values that are in this domain and another domain.
  void Intersect(const Domain &domain);

  // Keep values that are in this domain but not in another domain.
  void Differ(const Domain &domain);

  // Get the count to be saved in a checkpoint.
  size_t GetCount() const;

  // Restore saved count from a checkpoint.
  void SetCount(size_t saved_count);

 private:
  vector<T> values;  // values in the domain
  size_t count;      // number of values
};

//
// Generic implementation with an array of values
//
template <class T>
Domain<T>::Domain(T low, T high) {
  for (T v = low;; v++) {
    values.push_back(v);
    if (v == high) break;
  }
  count = values.size();
}

template <class T>
Domain<T>::Domain(const T values_in[]) {
  for (size_t i = 0; values_in[i]; i++) values.push_back(values_in[i]);
  count = values.size();
}

template <class T>
Domain<T>::Domain(const T values_in[], size_t count_in) {
  for (size_t i = 0; i < count_in; i++) values.push_back(values_in[i]);
  count = values.size();
}

template <class T>
Domain<T>::Domain(bool (*generator_fn)(void *, T &), void *cookie) {
  T value;
  while (generator_fn(cookie, value)) values.push_back(value);
  count = values.size();
}

template <class T>
bool Domain<T>::IsEmpty() const {
  return GetSize() == 0;
}

template <class T>
bool Domain<T>::IsSingle() const {
  return GetSize() == 1;
}

template <class T>
bool Domain<T>::Contains(T value) const {
  return Find(value) != GetSize();
}

template <class T>
size_t Domain<T>::Find(T value) const {
  size_t i;
  for (i = 0; i < GetSize(); i++)
    if (values[i] == value) break;
  return i;
}

template <class T>
size_t Domain<T>::GetSize() const {
  return count;
}

template <class T>
void Domain<T>::GetBounds(T &low, T &high) const {
  low = high = values[0];
  for (size_t i = 1; i < count; i++) {
    if (low > values[i]) low = values[i];
    if (high < values[i]) high = values[i];
  }
}

template <class T>
void Domain<T>::LimitBounds(T low, T high) {
  for (size_t i = 0; i < count; i++) {
    if (values[i] < low || values[i] > high) {
      EraseValueAt(i);
      i--;
    }
  }
}

template <class T>
T Domain<T>::GetValue(size_t i) const {
  return values[i];
}

template <class T>
T Domain<T>::operator[](size_t i) const {
  return GetValue(i);
}

template <class T>
void Domain<T>::EraseValueAt(size_t i) {
  count--;

  // swap the i-th and the last values
  T temp = values[i];
  values[i] = values[count];
  values[count] = temp;
}

template <class T>
void Domain<T>::Union(const Domain &domain) {
  for (size_t i = 0; i < domain.GetSize(); i++) {
    T value = domain.GetValue(i);
    if (!Contains(value)) values[count++] = value;
  }
}

template <class T>
void Domain<T>::Intersect(const Domain &domain) {
  for (size_t i = 0; i < GetSize(); i++) {
    if (!domain.Contains(values[i])) {
      EraseValueAt(i);
      i--;
    }
  }
}

template <class T>
void Domain<T>::Differ(const Domain &domain) {
  for (size_t i = 0; i < domain.GetSize(); i++) {
    size_t j = Find(domain[i]);
    if (j != GetSize()) EraseValueAt(j);
  }
}

template <class T>
size_t Domain<T>::GetCount() const {
  return count;
}

template <class T>
void Domain<T>::SetCount(size_t saved_count) {
  count = saved_count;
}

#endif
