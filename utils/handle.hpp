#pragma once
#include "types.hpp"
#include <type_traits>

namespace aeo {
namespace utils {

template <typename tag, typename index_t = u32> class handle final {
  static_assert(sizeof(index_t) >= sizeof(u16) &&
                    sizeof(index_t) <= sizeof(u32),
                "unsupported index type");

  template <typename pool_tag, typename obj_type, typename index_type>
  friend class pool;
  handle(index_t idx, index_t gen) : index_val(idx), generation(gen) {};

public:
  handle() = default;
  handle(void *ptr)
      : index_val(static_cast<index_t>(reinterpret_cast<uintptr_t>(ptr))),
        generation(static_cast<index_t>(reinterpret_cast<uintptr_t>(ptr) >>
                                        sizeof(index_t) * 8)) {
    static_assert(sizeof(void *) <= sizeof(index_t) * 2,
                  "can't pack pointer into a handle");
  }

  inline bool empty() const { return generation == 0; }
  inline bool valid() const { return generation != 0; }
  inline index_t index() const { return index_val; }
  inline index_t gen() const { return generation; }
  inline void *index_as_void() const { return static_cast<void *>(index_val); };
  inline void *handle_as_void() {
    using cast_t = std::conditional_t<std::is_same_v<index_t, u32>, u64, u32>;
    constexpr u64 shift_bits = sizeof(index_t) * 8;

    return reinterpret_cast<void *>(
        static_cast<uintptr_t>(static_cast<cast_t>(generation) << shift_bits) |
        static_cast<cast_t>(index_val));
  }
  inline bool operator==(const handle<tag, index_t> &other) const {
    return (other.index_val == index_val) && (other.generation == generation);
  }
  inline bool operator!=(const handle<tag, index_t> &other) const {
    return (other.index_val != index_val) || (other.generation != generation);
  }
  inline explicit operator bool() const { return generation != 0; }

private:
  index_t index_val;
  index_t generation;
};

} // namespace utils
} // namespace aeo
