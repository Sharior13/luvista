#pragma once
// This file is auto-generated from api\cpp\cbindgen.rs
namespace slint::language {
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::CapitalizationMode &self) {
    switch (self) {
    case slint::cbindgen_private::CapitalizationMode::None: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("CapitalizationMode", "none");
    case slint::cbindgen_private::CapitalizationMode::Sentences: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("CapitalizationMode", "sentences");
    case slint::cbindgen_private::CapitalizationMode::Words: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("CapitalizationMode", "words");
    case slint::cbindgen_private::CapitalizationMode::Characters: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("CapitalizationMode", "characters");
    }
    return {};
}
inline slint::cbindgen_private::CapitalizationMode from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::CapitalizationMode *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "none") return slint::cbindgen_private::CapitalizationMode::None;
    if (value_str == "sentences") return slint::cbindgen_private::CapitalizationMode::Sentences;
    if (value_str == "words") return slint::cbindgen_private::CapitalizationMode::Words;
    if (value_str == "characters") return slint::cbindgen_private::CapitalizationMode::Characters;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::PointerEventKind &self) {
    switch (self) {
    case slint::cbindgen_private::PointerEventKind::Cancel: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("PointerEventKind", "cancel");
    case slint::cbindgen_private::PointerEventKind::Down: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("PointerEventKind", "down");
    case slint::cbindgen_private::PointerEventKind::Up: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("PointerEventKind", "up");
    case slint::cbindgen_private::PointerEventKind::Move: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("PointerEventKind", "move");
    }
    return {};
}
inline slint::cbindgen_private::PointerEventKind from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::PointerEventKind *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "cancel") return slint::cbindgen_private::PointerEventKind::Cancel;
    if (value_str == "down") return slint::cbindgen_private::PointerEventKind::Down;
    if (value_str == "up") return slint::cbindgen_private::PointerEventKind::Up;
    if (value_str == "move") return slint::cbindgen_private::PointerEventKind::Move;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::PointerEventButton &self) {
    switch (self) {
    case slint::cbindgen_private::PointerEventButton::Other: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("PointerEventButton", "other");
    case slint::cbindgen_private::PointerEventButton::Left: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("PointerEventButton", "left");
    case slint::cbindgen_private::PointerEventButton::Right: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("PointerEventButton", "right");
    case slint::cbindgen_private::PointerEventButton::Middle: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("PointerEventButton", "middle");
    case slint::cbindgen_private::PointerEventButton::Back: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("PointerEventButton", "back");
    case slint::cbindgen_private::PointerEventButton::Forward: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("PointerEventButton", "forward");
    }
    return {};
}
inline slint::cbindgen_private::PointerEventButton from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::PointerEventButton *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "other") return slint::cbindgen_private::PointerEventButton::Other;
    if (value_str == "left") return slint::cbindgen_private::PointerEventButton::Left;
    if (value_str == "right") return slint::cbindgen_private::PointerEventButton::Right;
    if (value_str == "middle") return slint::cbindgen_private::PointerEventButton::Middle;
    if (value_str == "back") return slint::cbindgen_private::PointerEventButton::Back;
    if (value_str == "forward") return slint::cbindgen_private::PointerEventButton::Forward;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::AccessibleRole &self) {
    switch (self) {
    case slint::cbindgen_private::AccessibleRole::None: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "none");
    case slint::cbindgen_private::AccessibleRole::Button: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "button");
    case slint::cbindgen_private::AccessibleRole::Checkbox: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "checkbox");
    case slint::cbindgen_private::AccessibleRole::Combobox: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "combobox");
    case slint::cbindgen_private::AccessibleRole::Groupbox: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "groupbox");
    case slint::cbindgen_private::AccessibleRole::Image: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "image");
    case slint::cbindgen_private::AccessibleRole::List: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "list");
    case slint::cbindgen_private::AccessibleRole::Slider: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "slider");
    case slint::cbindgen_private::AccessibleRole::Spinbox: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "spinbox");
    case slint::cbindgen_private::AccessibleRole::Tab: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "tab");
    case slint::cbindgen_private::AccessibleRole::TabList: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "tab-list");
    case slint::cbindgen_private::AccessibleRole::TabPanel: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "tab-panel");
    case slint::cbindgen_private::AccessibleRole::Text: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "text");
    case slint::cbindgen_private::AccessibleRole::Table: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "table");
    case slint::cbindgen_private::AccessibleRole::Tree: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "tree");
    case slint::cbindgen_private::AccessibleRole::ProgressIndicator: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "progress-indicator");
    case slint::cbindgen_private::AccessibleRole::TextInput: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "text-input");
    case slint::cbindgen_private::AccessibleRole::Switch: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "switch");
    case slint::cbindgen_private::AccessibleRole::ListItem: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "list-item");
    case slint::cbindgen_private::AccessibleRole::RadioButton: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "radio-button");
    case slint::cbindgen_private::AccessibleRole::RadioGroup: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "radio-group");
    case slint::cbindgen_private::AccessibleRole::WindowTitleBar: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "window-title-bar");
    case slint::cbindgen_private::AccessibleRole::Banner: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "banner");
    case slint::cbindgen_private::AccessibleRole::Complementary: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "complementary");
    case slint::cbindgen_private::AccessibleRole::ContentInfo: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "content-info");
    case slint::cbindgen_private::AccessibleRole::Form: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "form");
    case slint::cbindgen_private::AccessibleRole::Main: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "main");
    case slint::cbindgen_private::AccessibleRole::Navigation: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "navigation");
    case slint::cbindgen_private::AccessibleRole::Region: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "region");
    case slint::cbindgen_private::AccessibleRole::Search: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleRole", "search");
    }
    return {};
}
inline slint::cbindgen_private::AccessibleRole from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::AccessibleRole *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "none") return slint::cbindgen_private::AccessibleRole::None;
    if (value_str == "button") return slint::cbindgen_private::AccessibleRole::Button;
    if (value_str == "checkbox") return slint::cbindgen_private::AccessibleRole::Checkbox;
    if (value_str == "combobox") return slint::cbindgen_private::AccessibleRole::Combobox;
    if (value_str == "groupbox") return slint::cbindgen_private::AccessibleRole::Groupbox;
    if (value_str == "image") return slint::cbindgen_private::AccessibleRole::Image;
    if (value_str == "list") return slint::cbindgen_private::AccessibleRole::List;
    if (value_str == "slider") return slint::cbindgen_private::AccessibleRole::Slider;
    if (value_str == "spinbox") return slint::cbindgen_private::AccessibleRole::Spinbox;
    if (value_str == "tab") return slint::cbindgen_private::AccessibleRole::Tab;
    if (value_str == "tab-list") return slint::cbindgen_private::AccessibleRole::TabList;
    if (value_str == "tab-panel") return slint::cbindgen_private::AccessibleRole::TabPanel;
    if (value_str == "text") return slint::cbindgen_private::AccessibleRole::Text;
    if (value_str == "table") return slint::cbindgen_private::AccessibleRole::Table;
    if (value_str == "tree") return slint::cbindgen_private::AccessibleRole::Tree;
    if (value_str == "progress-indicator") return slint::cbindgen_private::AccessibleRole::ProgressIndicator;
    if (value_str == "text-input") return slint::cbindgen_private::AccessibleRole::TextInput;
    if (value_str == "switch") return slint::cbindgen_private::AccessibleRole::Switch;
    if (value_str == "list-item") return slint::cbindgen_private::AccessibleRole::ListItem;
    if (value_str == "radio-button") return slint::cbindgen_private::AccessibleRole::RadioButton;
    if (value_str == "radio-group") return slint::cbindgen_private::AccessibleRole::RadioGroup;
    if (value_str == "window-title-bar") return slint::cbindgen_private::AccessibleRole::WindowTitleBar;
    if (value_str == "banner") return slint::cbindgen_private::AccessibleRole::Banner;
    if (value_str == "complementary") return slint::cbindgen_private::AccessibleRole::Complementary;
    if (value_str == "content-info") return slint::cbindgen_private::AccessibleRole::ContentInfo;
    if (value_str == "form") return slint::cbindgen_private::AccessibleRole::Form;
    if (value_str == "main") return slint::cbindgen_private::AccessibleRole::Main;
    if (value_str == "navigation") return slint::cbindgen_private::AccessibleRole::Navigation;
    if (value_str == "region") return slint::cbindgen_private::AccessibleRole::Region;
    if (value_str == "search") return slint::cbindgen_private::AccessibleRole::Search;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::AccessibleLiveness &self) {
    switch (self) {
    case slint::cbindgen_private::AccessibleLiveness::Off: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleLiveness", "off");
    case slint::cbindgen_private::AccessibleLiveness::Polite: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleLiveness", "polite");
    case slint::cbindgen_private::AccessibleLiveness::Assertive: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AccessibleLiveness", "assertive");
    }
    return {};
}
inline slint::cbindgen_private::AccessibleLiveness from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::AccessibleLiveness *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "off") return slint::cbindgen_private::AccessibleLiveness::Off;
    if (value_str == "polite") return slint::cbindgen_private::AccessibleLiveness::Polite;
    if (value_str == "assertive") return slint::cbindgen_private::AccessibleLiveness::Assertive;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::SortOrder &self) {
    switch (self) {
    case slint::cbindgen_private::SortOrder::Unsorted: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("SortOrder", "unsorted");
    case slint::cbindgen_private::SortOrder::Ascending: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("SortOrder", "ascending");
    case slint::cbindgen_private::SortOrder::Descending: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("SortOrder", "descending");
    }
    return {};
}
inline slint::cbindgen_private::SortOrder from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::SortOrder *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "unsorted") return slint::cbindgen_private::SortOrder::Unsorted;
    if (value_str == "ascending") return slint::cbindgen_private::SortOrder::Ascending;
    if (value_str == "descending") return slint::cbindgen_private::SortOrder::Descending;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::Orientation &self) {
    switch (self) {
    case slint::cbindgen_private::Orientation::Horizontal: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("Orientation", "horizontal");
    case slint::cbindgen_private::Orientation::Vertical: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("Orientation", "vertical");
    }
    return {};
}
inline slint::cbindgen_private::Orientation from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::Orientation *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "horizontal") return slint::cbindgen_private::Orientation::Horizontal;
    if (value_str == "vertical") return slint::cbindgen_private::Orientation::Vertical;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::ColorScheme &self) {
    switch (self) {
    case slint::cbindgen_private::ColorScheme::Unknown: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ColorScheme", "unknown");
    case slint::cbindgen_private::ColorScheme::Dark: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ColorScheme", "dark");
    case slint::cbindgen_private::ColorScheme::Light: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ColorScheme", "light");
    }
    return {};
}
inline slint::cbindgen_private::ColorScheme from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::ColorScheme *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "unknown") return slint::cbindgen_private::ColorScheme::Unknown;
    if (value_str == "dark") return slint::cbindgen_private::ColorScheme::Dark;
    if (value_str == "light") return slint::cbindgen_private::ColorScheme::Light;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::DragAction &self) {
    switch (self) {
    case slint::cbindgen_private::DragAction::None: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("DragAction", "none");
    case slint::cbindgen_private::DragAction::Copy: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("DragAction", "copy");
    case slint::cbindgen_private::DragAction::Move: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("DragAction", "move");
    case slint::cbindgen_private::DragAction::Link: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("DragAction", "link");
    }
    return {};
}
inline slint::cbindgen_private::DragAction from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::DragAction *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "none") return slint::cbindgen_private::DragAction::None;
    if (value_str == "copy") return slint::cbindgen_private::DragAction::Copy;
    if (value_str == "move") return slint::cbindgen_private::DragAction::Move;
    if (value_str == "link") return slint::cbindgen_private::DragAction::Link;
    return {};
}
} // namespace slint::language
namespace slint::cbindgen_private {
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::TextHorizontalAlignment &self) {
    switch (self) {
    case slint::cbindgen_private::TextHorizontalAlignment::Start: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("TextHorizontalAlignment", "start");
    case slint::cbindgen_private::TextHorizontalAlignment::End: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("TextHorizontalAlignment", "end");
    case slint::cbindgen_private::TextHorizontalAlignment::Left: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("TextHorizontalAlignment", "left");
    case slint::cbindgen_private::TextHorizontalAlignment::Center: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("TextHorizontalAlignment", "center");
    case slint::cbindgen_private::TextHorizontalAlignment::Right: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("TextHorizontalAlignment", "right");
    }
    return {};
}
inline slint::cbindgen_private::TextHorizontalAlignment from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::TextHorizontalAlignment *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "start") return slint::cbindgen_private::TextHorizontalAlignment::Start;
    if (value_str == "end") return slint::cbindgen_private::TextHorizontalAlignment::End;
    if (value_str == "left") return slint::cbindgen_private::TextHorizontalAlignment::Left;
    if (value_str == "center") return slint::cbindgen_private::TextHorizontalAlignment::Center;
    if (value_str == "right") return slint::cbindgen_private::TextHorizontalAlignment::Right;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::TextVerticalAlignment &self) {
    switch (self) {
    case slint::cbindgen_private::TextVerticalAlignment::Top: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("TextVerticalAlignment", "top");
    case slint::cbindgen_private::TextVerticalAlignment::Center: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("TextVerticalAlignment", "center");
    case slint::cbindgen_private::TextVerticalAlignment::Bottom: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("TextVerticalAlignment", "bottom");
    }
    return {};
}
inline slint::cbindgen_private::TextVerticalAlignment from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::TextVerticalAlignment *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "top") return slint::cbindgen_private::TextVerticalAlignment::Top;
    if (value_str == "center") return slint::cbindgen_private::TextVerticalAlignment::Center;
    if (value_str == "bottom") return slint::cbindgen_private::TextVerticalAlignment::Bottom;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::TextWrap &self) {
    switch (self) {
    case slint::cbindgen_private::TextWrap::NoWrap: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("TextWrap", "no-wrap");
    case slint::cbindgen_private::TextWrap::WordWrap: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("TextWrap", "word-wrap");
    case slint::cbindgen_private::TextWrap::CharWrap: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("TextWrap", "char-wrap");
    }
    return {};
}
inline slint::cbindgen_private::TextWrap from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::TextWrap *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "no-wrap") return slint::cbindgen_private::TextWrap::NoWrap;
    if (value_str == "word-wrap") return slint::cbindgen_private::TextWrap::WordWrap;
    if (value_str == "char-wrap") return slint::cbindgen_private::TextWrap::CharWrap;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::TextOverflow &self) {
    switch (self) {
    case slint::cbindgen_private::TextOverflow::Clip: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("TextOverflow", "clip");
    case slint::cbindgen_private::TextOverflow::Elide: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("TextOverflow", "elide");
    }
    return {};
}
inline slint::cbindgen_private::TextOverflow from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::TextOverflow *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "clip") return slint::cbindgen_private::TextOverflow::Clip;
    if (value_str == "elide") return slint::cbindgen_private::TextOverflow::Elide;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::TextStrokeStyle &self) {
    switch (self) {
    case slint::cbindgen_private::TextStrokeStyle::Outside: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("TextStrokeStyle", "outside");
    case slint::cbindgen_private::TextStrokeStyle::Center: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("TextStrokeStyle", "center");
    }
    return {};
}
inline slint::cbindgen_private::TextStrokeStyle from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::TextStrokeStyle *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "outside") return slint::cbindgen_private::TextStrokeStyle::Outside;
    if (value_str == "center") return slint::cbindgen_private::TextStrokeStyle::Center;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::EventResult &self) {
    switch (self) {
    case slint::cbindgen_private::EventResult::Reject: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("EventResult", "reject");
    case slint::cbindgen_private::EventResult::Accept: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("EventResult", "accept");
    }
    return {};
}
inline slint::cbindgen_private::EventResult from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::EventResult *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "reject") return slint::cbindgen_private::EventResult::Reject;
    if (value_str == "accept") return slint::cbindgen_private::EventResult::Accept;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::FillRule &self) {
    switch (self) {
    case slint::cbindgen_private::FillRule::Nonzero: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("FillRule", "nonzero");
    case slint::cbindgen_private::FillRule::Evenodd: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("FillRule", "evenodd");
    }
    return {};
}
inline slint::cbindgen_private::FillRule from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::FillRule *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "nonzero") return slint::cbindgen_private::FillRule::Nonzero;
    if (value_str == "evenodd") return slint::cbindgen_private::FillRule::Evenodd;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::StandardButtonKind &self) {
    switch (self) {
    case slint::cbindgen_private::StandardButtonKind::Ok: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("StandardButtonKind", "ok");
    case slint::cbindgen_private::StandardButtonKind::Cancel: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("StandardButtonKind", "cancel");
    case slint::cbindgen_private::StandardButtonKind::Apply: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("StandardButtonKind", "apply");
    case slint::cbindgen_private::StandardButtonKind::Close: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("StandardButtonKind", "close");
    case slint::cbindgen_private::StandardButtonKind::Reset: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("StandardButtonKind", "reset");
    case slint::cbindgen_private::StandardButtonKind::Help: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("StandardButtonKind", "help");
    case slint::cbindgen_private::StandardButtonKind::Yes: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("StandardButtonKind", "yes");
    case slint::cbindgen_private::StandardButtonKind::No: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("StandardButtonKind", "no");
    case slint::cbindgen_private::StandardButtonKind::Abort: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("StandardButtonKind", "abort");
    case slint::cbindgen_private::StandardButtonKind::Retry: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("StandardButtonKind", "retry");
    case slint::cbindgen_private::StandardButtonKind::Ignore: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("StandardButtonKind", "ignore");
    }
    return {};
}
inline slint::cbindgen_private::StandardButtonKind from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::StandardButtonKind *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "ok") return slint::cbindgen_private::StandardButtonKind::Ok;
    if (value_str == "cancel") return slint::cbindgen_private::StandardButtonKind::Cancel;
    if (value_str == "apply") return slint::cbindgen_private::StandardButtonKind::Apply;
    if (value_str == "close") return slint::cbindgen_private::StandardButtonKind::Close;
    if (value_str == "reset") return slint::cbindgen_private::StandardButtonKind::Reset;
    if (value_str == "help") return slint::cbindgen_private::StandardButtonKind::Help;
    if (value_str == "yes") return slint::cbindgen_private::StandardButtonKind::Yes;
    if (value_str == "no") return slint::cbindgen_private::StandardButtonKind::No;
    if (value_str == "abort") return slint::cbindgen_private::StandardButtonKind::Abort;
    if (value_str == "retry") return slint::cbindgen_private::StandardButtonKind::Retry;
    if (value_str == "ignore") return slint::cbindgen_private::StandardButtonKind::Ignore;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::DialogButtonRole &self) {
    switch (self) {
    case slint::cbindgen_private::DialogButtonRole::None: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("DialogButtonRole", "none");
    case slint::cbindgen_private::DialogButtonRole::Accept: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("DialogButtonRole", "accept");
    case slint::cbindgen_private::DialogButtonRole::Reject: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("DialogButtonRole", "reject");
    case slint::cbindgen_private::DialogButtonRole::Apply: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("DialogButtonRole", "apply");
    case slint::cbindgen_private::DialogButtonRole::Reset: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("DialogButtonRole", "reset");
    case slint::cbindgen_private::DialogButtonRole::Help: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("DialogButtonRole", "help");
    case slint::cbindgen_private::DialogButtonRole::Action: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("DialogButtonRole", "action");
    }
    return {};
}
inline slint::cbindgen_private::DialogButtonRole from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::DialogButtonRole *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "none") return slint::cbindgen_private::DialogButtonRole::None;
    if (value_str == "accept") return slint::cbindgen_private::DialogButtonRole::Accept;
    if (value_str == "reject") return slint::cbindgen_private::DialogButtonRole::Reject;
    if (value_str == "apply") return slint::cbindgen_private::DialogButtonRole::Apply;
    if (value_str == "reset") return slint::cbindgen_private::DialogButtonRole::Reset;
    if (value_str == "help") return slint::cbindgen_private::DialogButtonRole::Help;
    if (value_str == "action") return slint::cbindgen_private::DialogButtonRole::Action;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::FocusReason &self) {
    switch (self) {
    case slint::cbindgen_private::FocusReason::Programmatic: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("FocusReason", "programmatic");
    case slint::cbindgen_private::FocusReason::TabNavigation: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("FocusReason", "tab-navigation");
    case slint::cbindgen_private::FocusReason::PointerClick: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("FocusReason", "pointer-click");
    case slint::cbindgen_private::FocusReason::PopupActivation: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("FocusReason", "popup-activation");
    case slint::cbindgen_private::FocusReason::WindowActivation: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("FocusReason", "window-activation");
    }
    return {};
}
inline slint::cbindgen_private::FocusReason from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::FocusReason *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "programmatic") return slint::cbindgen_private::FocusReason::Programmatic;
    if (value_str == "tab-navigation") return slint::cbindgen_private::FocusReason::TabNavigation;
    if (value_str == "pointer-click") return slint::cbindgen_private::FocusReason::PointerClick;
    if (value_str == "popup-activation") return slint::cbindgen_private::FocusReason::PopupActivation;
    if (value_str == "window-activation") return slint::cbindgen_private::FocusReason::WindowActivation;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::BuiltInMouseCursor &self) {
    switch (self) {
    case slint::cbindgen_private::BuiltInMouseCursor::Default: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "default");
    case slint::cbindgen_private::BuiltInMouseCursor::None: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "none");
    case slint::cbindgen_private::BuiltInMouseCursor::Help: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "help");
    case slint::cbindgen_private::BuiltInMouseCursor::Pointer: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "pointer");
    case slint::cbindgen_private::BuiltInMouseCursor::Progress: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "progress");
    case slint::cbindgen_private::BuiltInMouseCursor::Wait: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "wait");
    case slint::cbindgen_private::BuiltInMouseCursor::Crosshair: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "crosshair");
    case slint::cbindgen_private::BuiltInMouseCursor::Text: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "text");
    case slint::cbindgen_private::BuiltInMouseCursor::Alias: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "alias");
    case slint::cbindgen_private::BuiltInMouseCursor::Copy: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "copy");
    case slint::cbindgen_private::BuiltInMouseCursor::Move: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "move");
    case slint::cbindgen_private::BuiltInMouseCursor::NoDrop: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "no-drop");
    case slint::cbindgen_private::BuiltInMouseCursor::NotAllowed: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "not-allowed");
    case slint::cbindgen_private::BuiltInMouseCursor::Grab: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "grab");
    case slint::cbindgen_private::BuiltInMouseCursor::Grabbing: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "grabbing");
    case slint::cbindgen_private::BuiltInMouseCursor::ColResize: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "col-resize");
    case slint::cbindgen_private::BuiltInMouseCursor::RowResize: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "row-resize");
    case slint::cbindgen_private::BuiltInMouseCursor::NResize: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "n-resize");
    case slint::cbindgen_private::BuiltInMouseCursor::EResize: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "e-resize");
    case slint::cbindgen_private::BuiltInMouseCursor::SResize: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "s-resize");
    case slint::cbindgen_private::BuiltInMouseCursor::WResize: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "w-resize");
    case slint::cbindgen_private::BuiltInMouseCursor::NeResize: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "ne-resize");
    case slint::cbindgen_private::BuiltInMouseCursor::NwResize: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "nw-resize");
    case slint::cbindgen_private::BuiltInMouseCursor::SeResize: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "se-resize");
    case slint::cbindgen_private::BuiltInMouseCursor::SwResize: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "sw-resize");
    case slint::cbindgen_private::BuiltInMouseCursor::EwResize: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "ew-resize");
    case slint::cbindgen_private::BuiltInMouseCursor::NsResize: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "ns-resize");
    case slint::cbindgen_private::BuiltInMouseCursor::NeswResize: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "nesw-resize");
    case slint::cbindgen_private::BuiltInMouseCursor::NwseResize: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("BuiltInMouseCursor", "nwse-resize");
    }
    return {};
}
inline slint::cbindgen_private::BuiltInMouseCursor from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::BuiltInMouseCursor *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "default") return slint::cbindgen_private::BuiltInMouseCursor::Default;
    if (value_str == "none") return slint::cbindgen_private::BuiltInMouseCursor::None;
    if (value_str == "help") return slint::cbindgen_private::BuiltInMouseCursor::Help;
    if (value_str == "pointer") return slint::cbindgen_private::BuiltInMouseCursor::Pointer;
    if (value_str == "progress") return slint::cbindgen_private::BuiltInMouseCursor::Progress;
    if (value_str == "wait") return slint::cbindgen_private::BuiltInMouseCursor::Wait;
    if (value_str == "crosshair") return slint::cbindgen_private::BuiltInMouseCursor::Crosshair;
    if (value_str == "text") return slint::cbindgen_private::BuiltInMouseCursor::Text;
    if (value_str == "alias") return slint::cbindgen_private::BuiltInMouseCursor::Alias;
    if (value_str == "copy") return slint::cbindgen_private::BuiltInMouseCursor::Copy;
    if (value_str == "move") return slint::cbindgen_private::BuiltInMouseCursor::Move;
    if (value_str == "no-drop") return slint::cbindgen_private::BuiltInMouseCursor::NoDrop;
    if (value_str == "not-allowed") return slint::cbindgen_private::BuiltInMouseCursor::NotAllowed;
    if (value_str == "grab") return slint::cbindgen_private::BuiltInMouseCursor::Grab;
    if (value_str == "grabbing") return slint::cbindgen_private::BuiltInMouseCursor::Grabbing;
    if (value_str == "col-resize") return slint::cbindgen_private::BuiltInMouseCursor::ColResize;
    if (value_str == "row-resize") return slint::cbindgen_private::BuiltInMouseCursor::RowResize;
    if (value_str == "n-resize") return slint::cbindgen_private::BuiltInMouseCursor::NResize;
    if (value_str == "e-resize") return slint::cbindgen_private::BuiltInMouseCursor::EResize;
    if (value_str == "s-resize") return slint::cbindgen_private::BuiltInMouseCursor::SResize;
    if (value_str == "w-resize") return slint::cbindgen_private::BuiltInMouseCursor::WResize;
    if (value_str == "ne-resize") return slint::cbindgen_private::BuiltInMouseCursor::NeResize;
    if (value_str == "nw-resize") return slint::cbindgen_private::BuiltInMouseCursor::NwResize;
    if (value_str == "se-resize") return slint::cbindgen_private::BuiltInMouseCursor::SeResize;
    if (value_str == "sw-resize") return slint::cbindgen_private::BuiltInMouseCursor::SwResize;
    if (value_str == "ew-resize") return slint::cbindgen_private::BuiltInMouseCursor::EwResize;
    if (value_str == "ns-resize") return slint::cbindgen_private::BuiltInMouseCursor::NsResize;
    if (value_str == "nesw-resize") return slint::cbindgen_private::BuiltInMouseCursor::NeswResize;
    if (value_str == "nwse-resize") return slint::cbindgen_private::BuiltInMouseCursor::NwseResize;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::ImageFit &self) {
    switch (self) {
    case slint::cbindgen_private::ImageFit::Fill: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ImageFit", "fill");
    case slint::cbindgen_private::ImageFit::Contain: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ImageFit", "contain");
    case slint::cbindgen_private::ImageFit::Cover: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ImageFit", "cover");
    case slint::cbindgen_private::ImageFit::Preserve: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ImageFit", "preserve");
    }
    return {};
}
inline slint::cbindgen_private::ImageFit from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::ImageFit *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "fill") return slint::cbindgen_private::ImageFit::Fill;
    if (value_str == "contain") return slint::cbindgen_private::ImageFit::Contain;
    if (value_str == "cover") return slint::cbindgen_private::ImageFit::Cover;
    if (value_str == "preserve") return slint::cbindgen_private::ImageFit::Preserve;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::ImageHorizontalAlignment &self) {
    switch (self) {
    case slint::cbindgen_private::ImageHorizontalAlignment::Center: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ImageHorizontalAlignment", "center");
    case slint::cbindgen_private::ImageHorizontalAlignment::Left: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ImageHorizontalAlignment", "left");
    case slint::cbindgen_private::ImageHorizontalAlignment::Right: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ImageHorizontalAlignment", "right");
    }
    return {};
}
inline slint::cbindgen_private::ImageHorizontalAlignment from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::ImageHorizontalAlignment *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "center") return slint::cbindgen_private::ImageHorizontalAlignment::Center;
    if (value_str == "left") return slint::cbindgen_private::ImageHorizontalAlignment::Left;
    if (value_str == "right") return slint::cbindgen_private::ImageHorizontalAlignment::Right;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::ImageVerticalAlignment &self) {
    switch (self) {
    case slint::cbindgen_private::ImageVerticalAlignment::Center: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ImageVerticalAlignment", "center");
    case slint::cbindgen_private::ImageVerticalAlignment::Top: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ImageVerticalAlignment", "top");
    case slint::cbindgen_private::ImageVerticalAlignment::Bottom: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ImageVerticalAlignment", "bottom");
    }
    return {};
}
inline slint::cbindgen_private::ImageVerticalAlignment from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::ImageVerticalAlignment *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "center") return slint::cbindgen_private::ImageVerticalAlignment::Center;
    if (value_str == "top") return slint::cbindgen_private::ImageVerticalAlignment::Top;
    if (value_str == "bottom") return slint::cbindgen_private::ImageVerticalAlignment::Bottom;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::ImageRendering &self) {
    switch (self) {
    case slint::cbindgen_private::ImageRendering::Smooth: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ImageRendering", "smooth");
    case slint::cbindgen_private::ImageRendering::Pixelated: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ImageRendering", "pixelated");
    }
    return {};
}
inline slint::cbindgen_private::ImageRendering from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::ImageRendering *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "smooth") return slint::cbindgen_private::ImageRendering::Smooth;
    if (value_str == "pixelated") return slint::cbindgen_private::ImageRendering::Pixelated;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::ImageTiling &self) {
    switch (self) {
    case slint::cbindgen_private::ImageTiling::None: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ImageTiling", "none");
    case slint::cbindgen_private::ImageTiling::Repeat: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ImageTiling", "repeat");
    case slint::cbindgen_private::ImageTiling::Round: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ImageTiling", "round");
    }
    return {};
}
inline slint::cbindgen_private::ImageTiling from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::ImageTiling *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "none") return slint::cbindgen_private::ImageTiling::None;
    if (value_str == "repeat") return slint::cbindgen_private::ImageTiling::Repeat;
    if (value_str == "round") return slint::cbindgen_private::ImageTiling::Round;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::InputType &self) {
    switch (self) {
    case slint::cbindgen_private::InputType::Text: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("InputType", "text");
    case slint::cbindgen_private::InputType::Password: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("InputType", "password");
    case slint::cbindgen_private::InputType::Number: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("InputType", "number");
    case slint::cbindgen_private::InputType::Decimal: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("InputType", "decimal");
    case slint::cbindgen_private::InputType::Search: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("InputType", "search");
    }
    return {};
}
inline slint::cbindgen_private::InputType from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::InputType *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "text") return slint::cbindgen_private::InputType::Text;
    if (value_str == "password") return slint::cbindgen_private::InputType::Password;
    if (value_str == "number") return slint::cbindgen_private::InputType::Number;
    if (value_str == "decimal") return slint::cbindgen_private::InputType::Decimal;
    if (value_str == "search") return slint::cbindgen_private::InputType::Search;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::LayoutAlignment &self) {
    switch (self) {
    case slint::cbindgen_private::LayoutAlignment::Stretch: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("LayoutAlignment", "stretch");
    case slint::cbindgen_private::LayoutAlignment::Center: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("LayoutAlignment", "center");
    case slint::cbindgen_private::LayoutAlignment::Start: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("LayoutAlignment", "start");
    case slint::cbindgen_private::LayoutAlignment::End: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("LayoutAlignment", "end");
    case slint::cbindgen_private::LayoutAlignment::SpaceBetween: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("LayoutAlignment", "space-between");
    case slint::cbindgen_private::LayoutAlignment::SpaceAround: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("LayoutAlignment", "space-around");
    case slint::cbindgen_private::LayoutAlignment::SpaceEvenly: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("LayoutAlignment", "space-evenly");
    }
    return {};
}
inline slint::cbindgen_private::LayoutAlignment from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::LayoutAlignment *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "stretch") return slint::cbindgen_private::LayoutAlignment::Stretch;
    if (value_str == "center") return slint::cbindgen_private::LayoutAlignment::Center;
    if (value_str == "start") return slint::cbindgen_private::LayoutAlignment::Start;
    if (value_str == "end") return slint::cbindgen_private::LayoutAlignment::End;
    if (value_str == "space-between") return slint::cbindgen_private::LayoutAlignment::SpaceBetween;
    if (value_str == "space-around") return slint::cbindgen_private::LayoutAlignment::SpaceAround;
    if (value_str == "space-evenly") return slint::cbindgen_private::LayoutAlignment::SpaceEvenly;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::FlexboxLayoutDirection &self) {
    switch (self) {
    case slint::cbindgen_private::FlexboxLayoutDirection::Row: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("FlexboxLayoutDirection", "row");
    case slint::cbindgen_private::FlexboxLayoutDirection::RowReverse: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("FlexboxLayoutDirection", "row-reverse");
    case slint::cbindgen_private::FlexboxLayoutDirection::Column: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("FlexboxLayoutDirection", "column");
    case slint::cbindgen_private::FlexboxLayoutDirection::ColumnReverse: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("FlexboxLayoutDirection", "column-reverse");
    }
    return {};
}
inline slint::cbindgen_private::FlexboxLayoutDirection from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::FlexboxLayoutDirection *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "row") return slint::cbindgen_private::FlexboxLayoutDirection::Row;
    if (value_str == "row-reverse") return slint::cbindgen_private::FlexboxLayoutDirection::RowReverse;
    if (value_str == "column") return slint::cbindgen_private::FlexboxLayoutDirection::Column;
    if (value_str == "column-reverse") return slint::cbindgen_private::FlexboxLayoutDirection::ColumnReverse;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::CrossAxisAlignment &self) {
    switch (self) {
    case slint::cbindgen_private::CrossAxisAlignment::Auto: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("CrossAxisAlignment", "auto");
    case slint::cbindgen_private::CrossAxisAlignment::Stretch: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("CrossAxisAlignment", "stretch");
    case slint::cbindgen_private::CrossAxisAlignment::Start: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("CrossAxisAlignment", "start");
    case slint::cbindgen_private::CrossAxisAlignment::End: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("CrossAxisAlignment", "end");
    case slint::cbindgen_private::CrossAxisAlignment::Center: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("CrossAxisAlignment", "center");
    }
    return {};
}
inline slint::cbindgen_private::CrossAxisAlignment from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::CrossAxisAlignment *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "auto") return slint::cbindgen_private::CrossAxisAlignment::Auto;
    if (value_str == "stretch") return slint::cbindgen_private::CrossAxisAlignment::Stretch;
    if (value_str == "start") return slint::cbindgen_private::CrossAxisAlignment::Start;
    if (value_str == "end") return slint::cbindgen_private::CrossAxisAlignment::End;
    if (value_str == "center") return slint::cbindgen_private::CrossAxisAlignment::Center;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::FlexboxLayoutWrap &self) {
    switch (self) {
    case slint::cbindgen_private::FlexboxLayoutWrap::Wrap: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("FlexboxLayoutWrap", "wrap");
    case slint::cbindgen_private::FlexboxLayoutWrap::NoWrap: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("FlexboxLayoutWrap", "no-wrap");
    case slint::cbindgen_private::FlexboxLayoutWrap::WrapReverse: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("FlexboxLayoutWrap", "wrap-reverse");
    }
    return {};
}
inline slint::cbindgen_private::FlexboxLayoutWrap from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::FlexboxLayoutWrap *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "wrap") return slint::cbindgen_private::FlexboxLayoutWrap::Wrap;
    if (value_str == "no-wrap") return slint::cbindgen_private::FlexboxLayoutWrap::NoWrap;
    if (value_str == "wrap-reverse") return slint::cbindgen_private::FlexboxLayoutWrap::WrapReverse;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::PathEvent &self) {
    switch (self) {
    case slint::cbindgen_private::PathEvent::Begin: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("PathEvent", "begin");
    case slint::cbindgen_private::PathEvent::Line: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("PathEvent", "line");
    case slint::cbindgen_private::PathEvent::Quadratic: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("PathEvent", "quadratic");
    case slint::cbindgen_private::PathEvent::Cubic: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("PathEvent", "cubic");
    case slint::cbindgen_private::PathEvent::EndOpen: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("PathEvent", "end-open");
    case slint::cbindgen_private::PathEvent::EndClosed: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("PathEvent", "end-closed");
    }
    return {};
}
inline slint::cbindgen_private::PathEvent from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::PathEvent *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "begin") return slint::cbindgen_private::PathEvent::Begin;
    if (value_str == "line") return slint::cbindgen_private::PathEvent::Line;
    if (value_str == "quadratic") return slint::cbindgen_private::PathEvent::Quadratic;
    if (value_str == "cubic") return slint::cbindgen_private::PathEvent::Cubic;
    if (value_str == "end-open") return slint::cbindgen_private::PathEvent::EndOpen;
    if (value_str == "end-closed") return slint::cbindgen_private::PathEvent::EndClosed;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::AnimationDirection &self) {
    switch (self) {
    case slint::cbindgen_private::AnimationDirection::Normal: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AnimationDirection", "normal");
    case slint::cbindgen_private::AnimationDirection::Reverse: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AnimationDirection", "reverse");
    case slint::cbindgen_private::AnimationDirection::Alternate: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AnimationDirection", "alternate");
    case slint::cbindgen_private::AnimationDirection::AlternateReverse: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("AnimationDirection", "alternate-reverse");
    }
    return {};
}
inline slint::cbindgen_private::AnimationDirection from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::AnimationDirection *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "normal") return slint::cbindgen_private::AnimationDirection::Normal;
    if (value_str == "reverse") return slint::cbindgen_private::AnimationDirection::Reverse;
    if (value_str == "alternate") return slint::cbindgen_private::AnimationDirection::Alternate;
    if (value_str == "alternate-reverse") return slint::cbindgen_private::AnimationDirection::AlternateReverse;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::ScrollBarPolicy &self) {
    switch (self) {
    case slint::cbindgen_private::ScrollBarPolicy::AsNeeded: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ScrollBarPolicy", "as-needed");
    case slint::cbindgen_private::ScrollBarPolicy::AlwaysOff: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ScrollBarPolicy", "always-off");
    case slint::cbindgen_private::ScrollBarPolicy::AlwaysOn: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("ScrollBarPolicy", "always-on");
    }
    return {};
}
inline slint::cbindgen_private::ScrollBarPolicy from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::ScrollBarPolicy *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "as-needed") return slint::cbindgen_private::ScrollBarPolicy::AsNeeded;
    if (value_str == "always-off") return slint::cbindgen_private::ScrollBarPolicy::AlwaysOff;
    if (value_str == "always-on") return slint::cbindgen_private::ScrollBarPolicy::AlwaysOn;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::PopupClosePolicy &self) {
    switch (self) {
    case slint::cbindgen_private::PopupClosePolicy::CloseOnClick: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("PopupClosePolicy", "close-on-click");
    case slint::cbindgen_private::PopupClosePolicy::CloseOnClickOutside: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("PopupClosePolicy", "close-on-click-outside");
    case slint::cbindgen_private::PopupClosePolicy::NoAutoClose: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("PopupClosePolicy", "no-auto-close");
    }
    return {};
}
inline slint::cbindgen_private::PopupClosePolicy from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::PopupClosePolicy *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "close-on-click") return slint::cbindgen_private::PopupClosePolicy::CloseOnClick;
    if (value_str == "close-on-click-outside") return slint::cbindgen_private::PopupClosePolicy::CloseOnClickOutside;
    if (value_str == "no-auto-close") return slint::cbindgen_private::PopupClosePolicy::NoAutoClose;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::LineCap &self) {
    switch (self) {
    case slint::cbindgen_private::LineCap::Butt: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("LineCap", "butt");
    case slint::cbindgen_private::LineCap::Round: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("LineCap", "round");
    case slint::cbindgen_private::LineCap::Square: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("LineCap", "square");
    }
    return {};
}
inline slint::cbindgen_private::LineCap from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::LineCap *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "butt") return slint::cbindgen_private::LineCap::Butt;
    if (value_str == "round") return slint::cbindgen_private::LineCap::Round;
    if (value_str == "square") return slint::cbindgen_private::LineCap::Square;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::LineJoin &self) {
    switch (self) {
    case slint::cbindgen_private::LineJoin::Miter: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("LineJoin", "miter");
    case slint::cbindgen_private::LineJoin::Round: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("LineJoin", "round");
    case slint::cbindgen_private::LineJoin::Bevel: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("LineJoin", "bevel");
    }
    return {};
}
inline slint::cbindgen_private::LineJoin from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::LineJoin *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "miter") return slint::cbindgen_private::LineJoin::Miter;
    if (value_str == "round") return slint::cbindgen_private::LineJoin::Round;
    if (value_str == "bevel") return slint::cbindgen_private::LineJoin::Bevel;
    return {};
}
inline slint::interpreter::Value into_slint_value([[maybe_unused]] const slint::cbindgen_private::OperatingSystemType &self) {
    switch (self) {
    case slint::cbindgen_private::OperatingSystemType::Android: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("OperatingSystemType", "android");
    case slint::cbindgen_private::OperatingSystemType::Ios: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("OperatingSystemType", "ios");
    case slint::cbindgen_private::OperatingSystemType::Macos: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("OperatingSystemType", "macos");
    case slint::cbindgen_private::OperatingSystemType::Linux: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("OperatingSystemType", "linux");
    case slint::cbindgen_private::OperatingSystemType::Windows: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("OperatingSystemType", "windows");
    case slint::cbindgen_private::OperatingSystemType::Other: return slint::private_api::live_preview::LiveReloadingComponent::value_from_enum("OperatingSystemType", "other");
    }
    return {};
}
inline slint::cbindgen_private::OperatingSystemType from_slint_value(const slint::interpreter::Value &val, const slint::cbindgen_private::OperatingSystemType *) {
    auto value_str = slint::private_api::live_preview::LiveReloadingComponent::get_enum_value(val);
    if (value_str == "android") return slint::cbindgen_private::OperatingSystemType::Android;
    if (value_str == "ios") return slint::cbindgen_private::OperatingSystemType::Ios;
    if (value_str == "macos") return slint::cbindgen_private::OperatingSystemType::Macos;
    if (value_str == "linux") return slint::cbindgen_private::OperatingSystemType::Linux;
    if (value_str == "windows") return slint::cbindgen_private::OperatingSystemType::Windows;
    if (value_str == "other") return slint::cbindgen_private::OperatingSystemType::Other;
    return {};
}
} // namespace slint::cbindgen_private
namespace slint::language {
inline slint::interpreter::Value into_slint_value(const slint::language::KeyboardModifiers &self) {
    using slint::private_api::live_preview::into_slint_value;
    slint::interpreter::Struct s;
    s.set_field("alt", into_slint_value(self.alt));
    s.set_field("control", into_slint_value(self.control));
    s.set_field("shift", into_slint_value(self.shift));
    s.set_field("meta", into_slint_value(self.meta));
    return s;
}
inline slint::language::KeyboardModifiers from_slint_value(const slint::interpreter::Value &val, const slint::language::KeyboardModifiers *) {
    auto s = val.to_struct().value();
    slint::language::KeyboardModifiers r;
    r.alt = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.alt)>>(s.get_field("alt").value());
    r.control = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.control)>>(s.get_field("control").value());
    r.shift = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.shift)>>(s.get_field("shift").value());
    r.meta = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.meta)>>(s.get_field("meta").value());
    return r;
}
inline slint::interpreter::Value into_slint_value(const slint::language::PointerEvent &self) {
    using slint::private_api::live_preview::into_slint_value;
    slint::interpreter::Struct s;
    s.set_field("button", into_slint_value(self.button));
    s.set_field("kind", into_slint_value(self.kind));
    s.set_field("modifiers", into_slint_value(self.modifiers));
    s.set_field("touch_finger_id", into_slint_value(self.touch_finger_id));
    return s;
}
inline slint::language::PointerEvent from_slint_value(const slint::interpreter::Value &val, const slint::language::PointerEvent *) {
    auto s = val.to_struct().value();
    slint::language::PointerEvent r;
    r.button = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.button)>>(s.get_field("button").value());
    r.kind = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.kind)>>(s.get_field("kind").value());
    r.modifiers = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.modifiers)>>(s.get_field("modifiers").value());
    r.touch_finger_id = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.touch_finger_id)>>(s.get_field("touch_finger_id").value());
    return r;
}
inline slint::interpreter::Value into_slint_value(const slint::language::PointerScrollEvent &self) {
    using slint::private_api::live_preview::into_slint_value;
    slint::interpreter::Struct s;
    s.set_field("delta_x", into_slint_value(self.delta_x));
    s.set_field("delta_y", into_slint_value(self.delta_y));
    s.set_field("modifiers", into_slint_value(self.modifiers));
    return s;
}
inline slint::language::PointerScrollEvent from_slint_value(const slint::interpreter::Value &val, const slint::language::PointerScrollEvent *) {
    auto s = val.to_struct().value();
    slint::language::PointerScrollEvent r;
    r.delta_x = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.delta_x)>>(s.get_field("delta_x").value());
    r.delta_y = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.delta_y)>>(s.get_field("delta_y").value());
    r.modifiers = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.modifiers)>>(s.get_field("modifiers").value());
    return r;
}
inline slint::interpreter::Value into_slint_value(const slint::language::KeyEvent &self) {
    using slint::private_api::live_preview::into_slint_value;
    slint::interpreter::Struct s;
    s.set_field("text", into_slint_value(self.text));
    s.set_field("modifiers", into_slint_value(self.modifiers));
    s.set_field("repeat", into_slint_value(self.repeat));
    return s;
}
inline slint::language::KeyEvent from_slint_value(const slint::interpreter::Value &val, const slint::language::KeyEvent *) {
    auto s = val.to_struct().value();
    slint::language::KeyEvent r;
    r.text = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.text)>>(s.get_field("text").value());
    r.modifiers = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.modifiers)>>(s.get_field("modifiers").value());
    r.repeat = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.repeat)>>(s.get_field("repeat").value());
    return r;
}
inline slint::interpreter::Value into_slint_value(const slint::language::DropEvent &self) {
    using slint::private_api::live_preview::into_slint_value;
    slint::interpreter::Struct s;
    s.set_field("data", into_slint_value(self.data));
    s.set_field("position", into_slint_value(self.position));
    s.set_field("proposed_action", into_slint_value(self.proposed_action));
    return s;
}
inline slint::language::DropEvent from_slint_value(const slint::interpreter::Value &val, const slint::language::DropEvent *) {
    auto s = val.to_struct().value();
    slint::language::DropEvent r;
    r.data = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.data)>>(s.get_field("data").value());
    r.position = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.position)>>(s.get_field("position").value());
    r.proposed_action = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.proposed_action)>>(s.get_field("proposed_action").value());
    return r;
}
inline slint::interpreter::Value into_slint_value(const slint::language::TableColumn &self) {
    using slint::private_api::live_preview::into_slint_value;
    slint::interpreter::Struct s;
    s.set_field("title", into_slint_value(self.title));
    s.set_field("min_width", into_slint_value(self.min_width));
    s.set_field("horizontal_stretch", into_slint_value(self.horizontal_stretch));
    s.set_field("sort_order", into_slint_value(self.sort_order));
    s.set_field("width", into_slint_value(self.width));
    return s;
}
inline slint::language::TableColumn from_slint_value(const slint::interpreter::Value &val, const slint::language::TableColumn *) {
    auto s = val.to_struct().value();
    slint::language::TableColumn r;
    r.title = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.title)>>(s.get_field("title").value());
    r.min_width = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.min_width)>>(s.get_field("min_width").value());
    r.horizontal_stretch = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.horizontal_stretch)>>(s.get_field("horizontal_stretch").value());
    r.sort_order = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.sort_order)>>(s.get_field("sort_order").value());
    r.width = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.width)>>(s.get_field("width").value());
    return r;
}
inline slint::interpreter::Value into_slint_value(const slint::language::InputMethodHints &self) {
    using slint::private_api::live_preview::into_slint_value;
    slint::interpreter::Struct s;
    s.set_field("capitalization", into_slint_value(self.capitalization));
    s.set_field("auto_correct", into_slint_value(self.auto_correct));
    s.set_field("auto_complete", into_slint_value(self.auto_complete));
    return s;
}
inline slint::language::InputMethodHints from_slint_value(const slint::interpreter::Value &val, const slint::language::InputMethodHints *) {
    auto s = val.to_struct().value();
    slint::language::InputMethodHints r;
    r.capitalization = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.capitalization)>>(s.get_field("capitalization").value());
    r.auto_correct = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.auto_correct)>>(s.get_field("auto_correct").value());
    r.auto_complete = slint::private_api::live_preview::from_slint_value<std::decay_t<decltype(r.auto_complete)>>(s.get_field("auto_complete").value());
    return r;
}
} // namespace slint::language
