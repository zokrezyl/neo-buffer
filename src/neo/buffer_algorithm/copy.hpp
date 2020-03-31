#pragma once

#include <neo/buffer_concepts.hpp>

#include "./size.hpp"

#include <algorithm>
#include <cstddef>

namespace neo {

/**
 * Copy data from the source buffer into the destination buffer, with a maximum of `max_copy`. The
 * actual number of bytes that are copied is the minimum of the buffer sizes and `max_copy`. The
 * number of bytes copied is returned.
 */
constexpr std::size_t
buffer_copy(mutable_buffer dest, const_buffer src, std::size_t max_copy) noexcept {
    // Calculate how much we should copy in this operation. It will be the minimum of the buffer
    // sizes and the maximum bytes we want to copy
    const auto n_to_copy = (std::min)(src.size(), (std::min)(dest.size(), max_copy));
    // Do the copy!
    auto in  = src.data();
    auto out = dest.data();
    for (auto r = n_to_copy; r; --r, ++in, ++out) {
        *out = *in;
    }
    return n_to_copy;
}

// Catch copying from mutable->mutable and call the overload of const->mutable
constexpr std::size_t
buffer_copy(mutable_buffer dest, mutable_buffer src, std::size_t max_copy) noexcept {
    return buffer_copy(dest, const_buffer(src), max_copy);
}

/**
 * Copy data from the `src` buffer into the `dest` buffer, up to `max_copy`
 * bytes. The operation is bounds-checked, and the number of bytes copied is
 * returned.
 */
template <mutable_buffer_sequence MutableSeq, const_buffer_sequence ConstSeq>
constexpr std::size_t
buffer_copy(const MutableSeq& dest, const ConstSeq& src, std::size_t max_copy) noexcept {
    // Keep count of how many bytes remain
    auto remaining_to_copy = max_copy;
    // And how many bytes we have copied so far (to return later)
    std::size_t total_copied = 0;
    // Iterators into the destination
    auto       dest_iter = buffer_sequence_begin(dest);
    const auto dest_stop = buffer_sequence_end(dest);
    // Iterators from the source
    auto       src_iter = buffer_sequence_begin(src);
    const auto src_stop = buffer_sequence_end(src);

    // Because buffers may be unaligned, we need to keep track of if we are
    // part of the way into either buffer.
    std::size_t src_offset  = 0;
    std::size_t dest_offset = 0;

    // We copied buffers in pairs and advance the iterators as we consume them.
    while (dest_iter != dest_stop && src_iter != src_stop && remaining_to_copy) {
        // The source buffer, with our offset prefix removed:
        const_buffer src_buf = *src_iter + src_offset;
        // The dest buffer, with the offset prefix removed:
        mutable_buffer dest_buf = *dest_iter + dest_offset;

        // Do the copY
        const std::size_t n_copied = buffer_copy(dest_buf, src_buf, remaining_to_copy);
        // Accumulate
        total_copied += n_copied;
        // Decrement our remaining
        remaining_to_copy -= n_copied;
        // Advance the offsets
        src_offset += n_copied;
        dest_offset += n_copied;
        // If we've exhausted either buffer, advance the corresponding iterator and set the its
        // buffer offset back to zero.
        if (src_buf.size() == n_copied) {
            ++src_iter;
            src_offset = 0;
        }
        if (dest_buf.size() == n_copied) {
            ++dest_iter;
            dest_offset = 0;
        }
    }

    return total_copied;
}

/**
 * Copy the data from the `src` buffer sequence into the `dest` buffer sequence.
 * The operation is bounds-checked, and the number of bytes successfully copied
 * is returned.
 *
 * This overload is guaranteed to exhaust at least one of the source or
 * destination buffers.
 */
template <mutable_buffer_sequence MutableSeq, const_buffer_sequence ConstSeq>
constexpr std::size_t buffer_copy(const MutableSeq& dest, const ConstSeq& src) noexcept {
    auto src_size  = buffer_size(src);
    auto dest_size = buffer_size(dest);
    auto min_size  = (src_size > dest_size) ? dest_size : src_size;
    return buffer_copy(dest, src, min_size);
}

}  // namespace neo
