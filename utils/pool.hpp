#pragma once
#include "handle.hpp"
#include <assert.h>
#include <concepts>
#include <stack>
#include <vector>

namespace aeo {
namespace utils {

template <typename tag_t, typename object_t, typename index_t>
  requires std::integral<index_t>
class pool {
  using handle_t = handle<tag_t, index_t>;
  using data_t = std::pair<object_t, index_t>;

  void populate_free_list(index_t start, index_t end) {
    for (index_t i = start; i < end; i++) {
      free_list.push({i, 0});
    }
  }

public:
  pool(index_t reserve_amount) {
    object_pool.reserve(reserve_amount);
    populate_free_list(0, reserve_amount);
  }

  handle_t create(object_t &&obj) {
    index_t index = 0;
    if (free_list.empty()) {
      index = object_pool.size();

      object_pool.push_back({obj, 1});

      index_t end = object_pool.size();
      populate_free_list(index + 1, end);
    } else {
      index = free_list.top();
      free_list.pop();
    }

    return handle_t(index, 1);
  }

  void destroy(handle_t handle) {
    if (true == handle.empty()) {
      return;
    }

    index_t index = handle.index();
    if (handle != object_pool[index]) {
      return;
    }

    free_list.push(index);
    object_pool[index].second = 0;
  }

  const object_t *get(handle_t handle) {
    if (true == handle.empty()) {
      return nullptr;
    }

    const index_t index = handle.index();
    assert(index < object_pool.size());

    if (object_pool[index].second != handle.generation()) {
      return nullptr;
    }

    return &object_pool[index].first;
  }

  handle_t find_handle(object_t *ptr) {
    for (index_t i = 0; i < object_pool.size(); i++) {
      if (&object_pool[i].first == ptr) {
        return {i, object_pool[i].second};
      }
    }

    return {object_pool.size, 0};
  }

  handle_t update_generation(handle_t handle) {
    index_t index = handle.index();
    object_pool[index].second++;

    return handle(index, object_pool.second);
  }

  handle_t update_generation(object_t *obj) {
    handle_t handle = find_handle(obj);
    return update_generation(handle);
  }

private:
  std::stack<index_t> free_list;
  std::vector<data_t> object_pool;
};

} // namespace utils
} // namespace aeo
