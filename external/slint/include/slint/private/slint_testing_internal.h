#pragma once

/* Generated with cbindgen:0.29.4 */

#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

namespace slint {
namespace cbindgen_private {

/// Describes the kind of layout an element represents.
enum class LayoutKind : uint8_t {
    /// A `HorizontalLayout`.
    HorizontalLayout,
    /// A `VerticalLayout`.
    VerticalLayout,
    /// A `GridLayout`.
    GridLayout,
    /// A flex box layout.
    FlexboxLayout,
};

/// `ElementHandle` wraps an existing element in a Slint UI. An ElementHandle does not keep
/// the corresponding element in the UI alive. Use [`Self::is_valid()`] to verify that
/// it is still alive.
///
/// Obtain instances of `ElementHandle` by querying your application through
/// [`Self::find_by_accessible_label()`].
struct ElementHandle {
    ItemWeak item;
    uintptr_t element_index;
};

extern "C" {

void slint_testing_init_backend();

void slint_testing_configure_test_fonts();

void slint_mock_elapsed_time(uint64_t time_in_ms);

uint64_t slint_get_mocked_time();

void slint_send_mouse_click(float x, float y, const WindowAdapterRc *window_adapter);

void slint_send_keyboard_key_text(const SharedString *text,
                                  bool pressed,
                                  const WindowAdapterRc *window_adapter);

void slint_send_keyboard_char(const SharedString *string,
                              bool pressed,
                              const WindowAdapterRc *window_adapter);

void slint_send_keyboard_string_sequence(const SharedString *sequence,
                                         const WindowAdapterRc *window_adapter);

void slint_testing_use_native_popup(const WindowAdapterRc *window_adapter, bool native);

uintptr_t slint_testing_active_popup_count(const WindowAdapterRc *window_adapter);

bool slint_testing_element_visit_elements(const ItemTreeRc *root,
                                          void *user_data,
                                          bool (*visitor)(void*, const ElementHandle*));

void slint_testing_element_find_by_accessible_label(const ItemTreeRc *root,
                                                    const Slice<uint8_t> *label,
                                                    SharedVector<ElementHandle> *out);

void slint_testing_element_find_by_element_id(const ItemTreeRc *root,
                                              const Slice<uint8_t> *element_id,
                                              SharedVector<ElementHandle> *out);

void slint_testing_element_find_by_element_type_name(const ItemTreeRc *root,
                                                     const Slice<uint8_t> *type_name,
                                                     SharedVector<ElementHandle> *out);

bool slint_testing_element_id(const ElementHandle *element, SharedString *out);

bool slint_testing_element_type_name(const ElementHandle *element, SharedString *out);

bool slint_testing_element_bases(const ElementHandle *element, SharedVector<SharedString> *out);

bool slint_testing_element_layout_kind(const ElementHandle *element, LayoutKind *out);

void slint_testing_set_system_accent_color(uint32_t argb_encoded);

}  // extern "C"

}  // namespace cbindgen_private
}  // namespace slint
