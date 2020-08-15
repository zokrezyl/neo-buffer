#pragma once

#include <neo/as_buffer.hpp>

#include <neo/assert.hpp>
#include <neo/iterator_concepts.hpp>
#include <neo/iterator_facade.hpp>
#include <neo/ref.hpp>

namespace neo {

template <typename Range>
requires(as_buffer_convertible<iter_value_t<decltype(std::begin(ref_v<Range>))>>)  //
    class buffer_range_adaptor {
public:
    using range_type     = std::remove_cvref_t<Range>;
    using inner_iterator = decltype(std::begin(ref_v<Range>));
    using inner_sentinel = decltype(std::end(ref_v<Range>));
    using value_type     = as_buffer_t<iter_value_t<inner_iterator>>;

private:
    [[no_unique_address]] wrap_if_reference_t<Range> _range;

    enum { uses_sentinel = !same_as<inner_iterator, inner_sentinel> };

    struct nothing {};
    struct sentinel_base {
        struct sentinel_type {};
    };

public:
    constexpr explicit buffer_range_adaptor() = default;
    constexpr explicit buffer_range_adaptor(Range&& rng)
        : _range(NEO_FWD(rng)) {}

    auto& range() noexcept { return unref(_range); }
    auto& range() const noexcept { return unref(_range); }

    class iterator : public iterator_facade<iterator>,
                     public std::conditional_t<uses_sentinel, sentinel_base, nothing> {
        [[no_unique_address]] inner_iterator _it;

    public:
        constexpr iterator() = default;
        constexpr explicit iterator(inner_iterator it)
            : _it(it) {}

        constexpr decltype(auto) dereference() const noexcept {
            if constexpr (uses_sentinel) {
                neo_assert(expects,
                           *this != sentinel_base::sentinel_type(),
                           "Dereferenced a past-the-end buffer_range_adaptor::iterator");
            }
            return neo::as_buffer(*_it);
        }

        constexpr void advance(std::ptrdiff_t off) noexcept
            requires(random_access_iterator<inner_iterator>) {
            _it += off;
        }

        constexpr void increment() noexcept {
            if constexpr (uses_sentinel) {
                neo_assert(expects,
                           !at_end(),
                           "Advanced a past-the-end buffer_range_adaptor::iterator");
            }
            ++_it;
        }

        constexpr void decrement() noexcept requires(bidirectional_iterator<inner_iterator>) {
            --_it;
        }

        constexpr bool equal_to(iterator other) const noexcept { return _it == other._it; }

        constexpr bool at_end() const noexcept requires(bool(uses_sentinel)) {
            return _it == inner_sentinel();
        }
    };

public:
    constexpr iterator begin() noexcept { return iterator(std::begin(range())); }
    constexpr auto     end() noexcept {
        if constexpr (uses_sentinel) {
            return typename iterator::sentinel_type();
        } else {
            return iterator(std::end(range()));
        }
    }
};

template <typename Range>
requires as_buffer_convertible<iter_value_t<decltype(std::begin(ref_v<Range>))>>
buffer_range_adaptor(Range&& rng) -> buffer_range_adaptor<Range>;

}  // namespace neo