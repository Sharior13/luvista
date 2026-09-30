#pragma once
// This file is auto-generated from api\cpp\cbindgen.rs
#include "private/slint_builtin_structs.h"
#include "private/slint_enums_internal.h"
#include "private/slint_point.h"
#include "private/slint_image.h"
#include "private/slint_data_transfer.h"
#include "private/slint_keys.h"
namespace slint::cbindgen_private {
enum class KeyEventType : uint8_t;
using slint::language::KeyboardModifiers;
using slint::language::PointerEvent;
using slint::language::PointerScrollEvent;
using slint::language::KeyEvent;
using slint::language::DropEvent;
using slint::language::StandardListViewItem;
using slint::language::TableColumn;
/// A structure to hold metrics of a font for a specified pixel size.
struct FontMetrics {
    /// The distance between the baseline and the top of the tallest glyph in the font.
    float ascent{  };
    /// The distance between the baseline and the bottom of the tallest glyph in the font.
    /// This is usually negative.
    float descent{  };
    /// The distance between the baseline and the horizontal midpoint of the tallest glyph in the font,
    /// or zero if not specified by the font.
    float x_height{  };
    /// The distance between the baseline and the top of a regular upper-case glyph in the font,
    /// or zero if not specified by the font.
    float cap_height{  };
    /// \private
    friend bool operator==(const FontMetrics&, const FontMetrics&) = default;
    /// \private
    friend bool operator!=(const FontMetrics&, const FontMetrics&) = default;
};
using slint::language::InputMethodHints;
/// An item in the menu of a menu bar or context menu
struct MenuEntry {
    /// The text of the menu entry
    SharedString title{  };
    /// the icon associated with the menu entry
    Image icon{  };
    /// an opaque id that can be used to identify the menu entry
    SharedString id{  };
    /// whether the menu entry is enabled
    bool enabled{  };
    /// whether the menu entry is checkable
    bool checkable{  };
    /// whether the menu entry is checked
    bool checked{  };
    /// Sub menu
    bool has_sub_menu{  };
    /// The menu entry is a separator
    bool is_separator{  };
    /// The shortcut keys
    Keys shortcut{  };
    /// \private
    friend bool operator==(const MenuEntry&, const MenuEntry&) = default;
    /// \private
    friend bool operator!=(const MenuEntry&, const MenuEntry&) = default;
};
/// A structure representing the four edges of an axis-aligned rectangle
struct Edges {
    /// The left edge value
    float left{  };
    /// The top edge value
    float top{  };
    /// The right edge value
    float right{  };
    /// The bottom edge value
    float bottom{  };
    /// \private
    friend bool operator==(const Edges&, const Edges&) = default;
    /// \private
    friend bool operator!=(const Edges&, const Edges&) = default;
};
}
