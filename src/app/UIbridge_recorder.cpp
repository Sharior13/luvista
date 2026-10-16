#include "UIbridge.h"
#include "uiConvert.h"
#include "../core/appPaths.h"
#include <filesystem>

using UiConvert::toSlint;
using UiConvert::toStd;

// the Start Recording window
void UiBridge::connectRecorder()
{
    state().on_begin_recording([this](slint::SharedString name) { beginRecording(toStd(name)); });
    state().on_stop_recording([this] { stopRecording(); });

    state().on_cancel_recording([this] {
        recorder.discard();
        state().set_record_seconds(0);
        state().set_record_level(0);
        state().set_record_state(RecordState::Idle);
        });
}


// starts the microphone. name is what the user typed, "" = rec_1, rec_2 ...
void UiBridge::beginRecording(const std::string& name)
{
    if (recorder.isRecording())
        return;

    std::string why;
    std::string path = Recorder::pathForName(AppPaths::recordingsFolder(), name, why);

    if (path.empty()) {
        state().set_record_error(toSlint(why));
        return;
    }

    if (!recorder.start(path)) {
        state().set_record_error(toSlint(recorder.lastError()));
        return;
    }

    state().set_record_error("");
    state().set_record_seconds(0);
    state().set_record_state(RecordState::Recording);
}


// stops, makes the mp3 and shows it on the Recordings page
void UiBridge::stopRecording()
{
    std::string saved = recorder.stop();
    state().set_record_level(0);

    if (saved.empty()) {
        state().set_record_error(toSlint(recorder.lastError()));
        state().set_record_state(RecordState::Idle);
        return;
    }

    recordings.refresh();
    refreshRecordings();
    refreshStorage();

    state().set_record_saved_name(toSlint(std::filesystem::path(saved).filename().string()));
    state().set_record_state(RecordState::Saved);
}


// the clock and the level meter of the recorder window
void UiBridge::tickRecorder()
{
    if (!recorder.isRecording())
        return;

    state().set_record_seconds((float)recorder.seconds());
    state().set_record_level(recorder.level());
}


// the Recordings page. Recordings are files in the profile's recordings folder, not library songs,
// so shown[RECORDINGS] holds recording rows (1 = newest) instead of song numbers
void UiBridge::refreshRecordings()
{
    std::vector<int> rows = recordings.rows(recordingSearch, recordingSort);

    shown[RECORDINGS] = rows;
    keepSelection(RECORDINGS, (int)rows.size());

    auto numbers = std::make_shared<std::vector<int>>(rows);

    auto model = std::make_shared<LazyModel<SongInfo>>((int)numbers->size(), [this, numbers](int index) {
        int row = (*numbers)[index];
        SongInfo info;

        info.title = toSlint(recordings.title(row));
        info.artist = "Voice memo";
        info.date_added = toSlint(UiConvert::ago(recordings.added(row)));
        info.length = toSlint(UiConvert::lengthText(recordings.seconds(row)));
        info.selected = selectionPage == (int)RECORDINGS && selection.contains(index);
        return info;
        });

    models[RECORDINGS] = model;
    state().set_recordings(model);
    state().set_recordings_count(recordings.count());
}
