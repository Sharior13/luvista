#pragma once

/* Generated with cbindgen:0.29.4 */

#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>
#include "private/slint_enums_internal.h"
#include "private/slint_sharedvector.h"
#ifndef SLINT_FEATURE_FREESTANDING
#    include <filesystem>
#endif
namespace slint { struct DataTransfer; struct SharedString; }
namespace slint::cbindgen_private::types {
#ifndef SLINT_FEATURE_FREESTANDING
using PathValueType = std::filesystem::path::value_type;
// The Rust side uses u16 path units on Windows and u8 elsewhere.
#    ifdef _WIN32
static_assert(sizeof(PathValueType) == 2);
#    else
static_assert(sizeof(PathValueType) == 1);
#    endif
#else
using PathValueType = uint8_t;
#endif
}

namespace slint {
namespace cbindgen_private {
namespace types {

/// Opaque placeholder used by C++ to reserve storage with the same size and
/// alignment as Rust's `DataTransfer`. The actual `DataTransfer` contains
/// `Option<Rc<...>>` fields whose layout cannot be expressed via cbindgen, so
/// C++ never inspects these fields directly: copy/destruction goes through
/// the `slint_data_transfer_*` FFI functions below, which operate on a real
/// `DataTransfer`.
///
/// The three pointer-sized fields correspond to:
/// - `_rc_inner`: thin `Rc` pointer for `Option<Rc<DataTransferInner>>` (null = `None`)
/// - `_rc_any_0`/`_rc_any_1`: data + vtable pointers of the
///   `Option<Rc<dyn Any>>` user data fat pointer (null data = `None`)
struct DataTransferOpaque {
    void *_rc_inner;
    void *_rc_any_0;
    void *_rc_any_1;

    bool operator==(const DataTransferOpaque& other) const {
        return _rc_inner == other._rc_inner &&
               _rc_any_0 == other._rc_any_0 &&
               _rc_any_1 == other._rc_any_1;
    }
    bool operator!=(const DataTransferOpaque& other) const {
        return _rc_inner != other._rc_inner ||
               _rc_any_0 != other._rc_any_0 ||
               _rc_any_1 != other._rc_any_1;
    }
};

extern "C" {

/// Default-construct a `DataTransfer` in place at `out`.
///
/// # Safety
/// `out` must be valid for writes of `DataTransfer` and must not currently
/// hold an initialized `DataTransfer` (otherwise the previous value is leaked).
void slint_data_transfer_init_default(DataTransfer *out);

/// Drop a `DataTransfer` in place.
///
/// # Safety
/// `d` must point to an initialized `DataTransfer` and must not be used after
/// this call returns.
void slint_data_transfer_drop(DataTransfer *d);

/// Clone `src` into the uninitialized memory at `out`.
///
/// # Safety
/// `out` must be valid for writes of `DataTransfer` and must not currently
/// hold an initialized `DataTransfer` (otherwise the previous value is leaked).
void slint_data_transfer_clone(DataTransfer *out, const DataTransfer *src);

/// Compare two `DataTransfer` values for equality (same semantics as
/// `<DataTransfer as PartialEq>::eq`).
bool slint_data_transfer_eq(const DataTransfer *a, const DataTransfer *b);

/// Set the plain text representation of `d` to a clone of `text`.
///
/// An empty `text` clears the previously-set plain text instead of storing it.
void slint_data_transfer_set_plain_text(DataTransfer *d, const SharedString *text);

/// Set the image representation of `d` to a clone of `image`.
///
/// A default-constructed `image` clears the previously-set image instead of storing it.
void slint_data_transfer_set_image(DataTransfer *d, const Image *image);

/// Set the file path list of `d`. Each path is a slice of its native
/// representation units (see `PathValueType`).
///
/// An empty `paths` clears the previously-set file paths instead of storing it.
void slint_data_transfer_set_file_paths(DataTransfer *d, Slice<Slice<PathValueType>> paths);

/// Returns `true` if `d` advertises a plain text representation.
bool slint_data_transfer_has_plain_text(const DataTransfer *d);

/// Returns `true` if `d` advertises an image representation.
bool slint_data_transfer_has_image(const DataTransfer *d);

/// Returns `true` if `d` advertises a list of file paths.
bool slint_data_transfer_has_file_paths(const DataTransfer *d);

/// Returns `true` if `d` carries no data: no plain text, no image, no file paths,
/// and no user data.
bool slint_data_transfer_is_empty(const DataTransfer *d);

/// If `d` has a plain text representation, write a clone of it into `out` and
/// return `true`. Otherwise leave `out` unchanged and return `false`.
///
/// `out` must point to an initialized `SharedString`.
bool slint_data_transfer_plain_text(const DataTransfer *d, SharedString *out);

/// If `d` has an image representation, write a clone of it into `out` and
/// return `true`. Otherwise leave `out` unchanged and return `false`.
///
/// `out` must point to an initialized `Image`.
bool slint_data_transfer_image(const DataTransfer *d, Image *out);

/// If `d` has a list of file paths, write them into `out` and return `true`.
/// Otherwise leave `out` unchanged and return `false`. Each path is a vector
/// of its native representation units (see `PathValueType`).
///
/// `out` must point to an initialized `SharedVector<SharedVector<PathValueType>>`.
bool slint_data_transfer_file_paths(const DataTransfer *d,
                                    SharedVector<SharedVector<PathValueType>> *out);

/// Store `handle` as the user data of `d`. `drop_fn(handle)` runs when `d` is dropped.
void slint_data_transfer_set_user_data(DataTransfer *d, void *handle, void (*drop_fn)(void*));

/// Write a borrowed pointer to `d`'s C++ user-data handle into `out_handle`.
/// Returns `false` if `d` has no C++ user data.
bool slint_data_transfer_user_data(const DataTransfer *d, const void **out_handle);

/// Clear the user data of `d`, if any.
void slint_data_transfer_clear_user_data(DataTransfer *d);

/// The title a window shows when the application doesn't set one
void slint_default_window_title(SharedString *out);

}  // extern "C"

}  // namespace types
}  // namespace cbindgen_private
}  // namespace slint
