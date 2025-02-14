#include <boost/thread/shared_mutex.hpp>
#include <unordered_map>

template <typename K, typename V>
class unordered_map_ts {
public:
  V &operator[](const K &key) {
    boost::unique_lock<boost::shared_mutex> lock(mtx);
    return data[key];
  }

  template <typename E>
  V &emplace(const K &key, const V &val) {
    boost::unique_lock<boost::shared_mutex> lock(mtx);

    if (!data.contains(key)) {
      auto [it, inserted] = data.emplace(key, val);
      return it->second;
    }

    throw E(key);
  }

  template <typename E>
  V &find(const K &key) {
    boost::shared_lock<boost::shared_mutex> lock(mtx);
    auto it = data.find(key);
    if (it != data.end()) {
      return it->second;
    }

    throw E(key);
  }

private:
  boost::shared_mutex mtx;

  std::unordered_map<K, V> data;
};
