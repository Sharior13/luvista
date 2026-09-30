#pragma once

/* Generated with cbindgen:0.29.4 */

#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>
#include "private/slint_enums_internal.h"
#include "private/slint_builtin_structs.h"
namespace slint::cbindgen_private::types {
using KeyboardModifiers = ::slint::language::KeyboardModifiers;
}

namespace slint {
namespace cbindgen_private {
namespace types {

/// Internal representation of the `Keys` type.
/// This is semver exempt and is only used to set up the native menu in the backends.
struct KeysInner {
    /// The `key` used to trigger the shortcut
    ///
    /// Note: This is currently converted to lowercase when the shortcut is created!
    SharedString key;
    /// `KeyboardModifier`s that need to be pressed for the shortcut to fire
    KeyboardModifiers modifiers;
    /// Whether to ignore shift state when matching the shortcut
    bool ignore_shift;
    /// Whether to ignore alt state when matching the shortcut
    bool ignore_alt;

    bool operator==(const KeysInner& other) const {
        return key == other.key &&
               modifiers == other.modifiers &&
               ignore_shift == other.ignore_shift &&
               ignore_alt == other.ignore_alt;
    }
    bool operator!=(const KeysInner& other) const {
        return key != other.key ||
               modifiers != other.modifiers ||
               ignore_shift != other.ignore_shift ||
               ignore_alt != other.ignore_alt;
    }
};

/// The `Keys` type is the Rust representation of Slint's `keys` primitive type.
///
/// It can be created with the `@keys` macro in Slint and defines which key event(s) activate a KeyBinding.
///
/// See also the Slint documentation on [Key Bindings](slint:KeyBindingOverview).
///
/// In `.slint` files, `Keys` values are typically created via the `@keys(...)` macro.
/// From backend code, they can be created from a list of string parts with the similar
/// syntax as the macro:
///
/// ```rust
/// use slint::Keys;
///
/// let save = Keys::from_parts(["Control", "S"])?;
/// let undo = Keys::from_parts(["Control", "Shift?", "Z"])?;
/// let f5 = Keys::from_parts(["F5"])?;
/// let zoom_in = Keys::from_parts(["Control", "Plus"])?;
/// let euro = Keys::from_parts(["Control", "€"])?;
/// let empty = Keys::from_parts([])?;  // same as Keys::default()
/// # Ok::<(), i_slint_core::input::KeysParseError>(())
/// ```
/// ## Parts format
///
/// Each element is either a modifier or a key (case-sensitive, matching the `@keys` macro):
/// - **Modifiers** (optional): `Control`, `Alt`, `Shift`, `Meta`
/// - **Optional modifiers**: `Shift?`, `Alt?` (match regardless of that modifier's state)
/// - **Named key** (required, exactly one): A named key (`Return`, `Tab`, `F1`, `Plus`, `Space`, `A`–`Z`, etc.)
/// - **String literal fallback**: If no named key matches, the part is treated as a string
///   literal — it must be a single lowercase grapheme cluster (e.g., `"€"`, `"é"`)
///
/// Keys with layout-dependent shifted variants (digits `Digit0`–`Digit9`, symbols like
/// `Plus`, `Comma`, etc.) automatically get `Shift?` behavior, just like the `@keys` macro.
struct Keys {
    KeysInner inner;

    bool operator==(const Keys& other) const {
        return inner == other.inner;
    }
    bool operator!=(const Keys& other) const {
        return inner != other.inner;
    }
};

extern "C" {

void slint_keys(const SharedString *key,
                bool alt,
                bool control,
                bool shift,
                bool meta,
                bool ignore_shift,
                bool ignore_alt,
                Keys *out);

void slint_keys_to_string(const Keys *shortcut, SharedString *out);

bool slint_keys_from_parts(Slice<SharedString> parts, Keys *out);

void slint_keys_to_parts(const Keys *keys, SharedVector<SharedString> *out);

/// The title a window shows when the application doesn't set one
void slint_default_window_title(SharedString *out);

}  // extern "C"

}  // namespace types
}  // namespace cbindgen_private
}  // namespace slint
