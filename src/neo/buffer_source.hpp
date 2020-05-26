#pragma once

#include <neo/buffer_range.hpp>

#include <neo/concepts.hpp>

namespace neo {

/**
 * A "buffer_source" is an object that allows the user to read buffers from an
 * abstract source, and notify that source when it has consumed some input.
 */
// clang-format off
template <typename T>
concept buffer_source =
    requires (T source, std::size_t size) {
        { source.data(size) } -> buffer_range;
        source.consume(size);
    };
// clang-format on

class proto_buffer_source {
    proto_buffer_source() = delete;

    proto_buffer_range data(std::size_t);
    void               consume(std::size_t);
};

template <buffer_source T>
[[nodiscard]] constexpr bool buffer_source_known_empty(T&& t) noexcept {
    constexpr bool has_empty = requires {
        { t.empty() }
        ->simple_boolean;
    };
    if constexpr (has_empty) {
        return t.empty();
    } else {
        return false;
    }
}

}  // namespace neo
