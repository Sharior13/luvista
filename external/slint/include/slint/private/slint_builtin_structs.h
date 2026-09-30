#pragma once
// This file is auto-generated from api\cpp\cbindgen.rs
#include "private/slint_enums.h"
#include "private/slint_point.h"
#include "private/slint_data_transfer.h"
namespace slint::language {
/// The `KeyboardModifiers` struct provides booleans to indicate possible modifier keys on a keyboard, such as Shift, Control, etc.
/// It is provided as part of `KeyEvent`'s `modifiers` field.
///
/// Keyboard shortcuts on Apple platforms typically use the Command key (⌘), such as Command+C for "Copy". On other platforms
/// the same shortcut is typically represented using Control+C. To make it easier to develop cross-platform applications, on macOS,
/// Slint maps the Command key to the control modifier, and the Control key to the meta modifier.
///
/// On Windows, the Windows key is mapped to the meta modifier.
struct KeyboardModifiers {
    /// Indicates the Alt key on a keyboard.
    bool alt{  };
    /// Indicates the Control key on a keyboard, except on macOS, where it is the Command key (⌘).
    bool control{  };
    /// Indicates the Shift key on a keyboard.
    bool shift{  };
    /// Indicates the Control key on macos, and the Windows key on Windows.
    bool meta{  };
    /// \private
    friend bool operator==(const KeyboardModifiers&, const KeyboardModifiers&) = default;
    /// \private
    friend bool operator!=(const KeyboardModifiers&, const KeyboardModifiers&) = default;
};
/// Represents a Pointer event sent by the windowing system.
/// This structure is passed to the `pointer-event` callback of the `TouchArea` element.
struct PointerEvent {
    /// The button that was pressed or released
    PointerEventButton button{  };
    /// The kind of the event
    PointerEventKind kind{  };
    /// The keyboard modifiers pressed during the event
    KeyboardModifiers modifiers{  };
    /// The unique ID of the touch point, indicating the finger ID. 0 means it's not a touch event (e.g., mouse).
    int32_t touch_finger_id{  };
    /// \private
    friend bool operator==(const PointerEvent&, const PointerEvent&) = default;
    /// \private
    friend bool operator!=(const PointerEvent&, const PointerEvent&) = default;
};
/// Represents a Pointer scroll (or wheel) event sent by the windowing system.
/// This structure is passed to the `scroll-event` callback of the `TouchArea` element.
struct PointerScrollEvent {
    /// The amount of pixel in the horizontal direction
    float delta_x{  };
    /// The amount of pixel in the vertical direction
    float delta_y{  };
    /// The keyboard modifiers pressed during the event
    KeyboardModifiers modifiers{  };
    /// \private
    friend bool operator==(const PointerScrollEvent&, const PointerScrollEvent&) = default;
    /// \private
    friend bool operator!=(const PointerScrollEvent&, const PointerScrollEvent&) = default;
};
/// This structure is generated and passed to the key press and release callbacks of the `FocusScope` element.
struct KeyEvent {
    /// The unicode representation of the key pressed.
    SharedString text{  };
    /// The keyboard modifiers active at the time of the key press event.
    KeyboardModifiers modifiers{  };
    /// This field is set to true for key press events that are repeated,
    /// i.e. the key is held down. It's always false for key release events.
    bool repeat{  };
    /// \private
    friend bool operator==(const KeyEvent&, const KeyEvent&) = default;
    /// \private
    friend bool operator!=(const KeyEvent&, const KeyEvent&) = default;
};
/// This structure is passed to the callbacks of the `DropArea` element
struct DropEvent {
    /// The payload set on the source `DragArea`.
    DataTransfer data{  };
    /// The cursor position in the `DropArea`'s local coordinates.
    LogicalPosition position{  };
    /// The action negotiated from current modifier state, clamped to the allowed set;
    /// when no modifier is pressed, the first allowed of move, copy, link.
    /// Updated on every `DragMove`. The target's `can-drop` callback can return this
    /// to honor the user's modifier choice, or override with any other allowed action.
    DragAction proposed_action{  };
    /// \private
    friend bool operator==(const DropEvent&, const DropEvent&) = default;
    /// \private
    friend bool operator!=(const DropEvent&, const DropEvent&) = default;
};
/// Represents an item in a StandardListView and a StandardTableView.
struct StandardListViewItem {
    /// The text content of the item
    SharedString text{  };
    /// \private
    friend bool operator==(const StandardListViewItem&, const StandardListViewItem&) = default;
    /// \private
    friend bool operator!=(const StandardListViewItem&, const StandardListViewItem&) = default;
};
/// This is used to define the column and the column header of a TableView
struct TableColumn {
    /// The title of the column header
    SharedString title{  };
    /// The minimum column width (logical length)
    float min_width{  };
    /// The horizontal column stretch
    float horizontal_stretch{  };
    /// Sorts the column
    SortOrder sort_order{  };
    /// the actual width of the column (logical length)
    float width{  };
    /// \private
    friend bool operator==(const TableColumn&, const TableColumn&) = default;
    /// \private
    friend bool operator!=(const TableColumn&, const TableColumn&) = default;
};
/// This structure holds the hints that a `TextInput` gives to the platform's input method
/// (e.g. a soft keyboard) about the expected input.
/// The input method may take these hints into account, but might also ignore them.
struct InputMethodHints {
    /// The auto-capitalization behavior that the input method should apply.
    /// Defaults to `CapitalizationMode::Sentences`.
    CapitalizationMode capitalization{ (CapitalizationMode::Sentences) };
    /// Hint that the input method may automatically correct spelling mistakes as the user types.
    /// Defaults to `true`.
    bool auto_correct{ true };
    /// Hint that the input method may offer auto-completion suggestions for the entered text.
    /// Defaults to `true`.
    bool auto_complete{ true };
    /// \private
    friend bool operator==(const InputMethodHints&, const InputMethodHints&) = default;
    /// \private
    friend bool operator!=(const InputMethodHints&, const InputMethodHints&) = default;
};
}
namespace slint { using slint::language::StandardListViewItem; }
