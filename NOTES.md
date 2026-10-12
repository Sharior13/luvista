# Luvista - merged project

`luvista.zip` and `luvista-merged.zip` combined. Layers (each only talks to the one below):

    ui/        Slint screens. Show state, send clicks. No decisions.
    src/app/   UIbridge*.cpp: turns clicks into calls and answers into models. No rules.
    src/core/  The rules: Library (pages/search/sort), PlayerController (queue, repeat, autoplay),
               Playlists, PlaylistPicker, Profiles + UserSession + Credentials, Settings, Recordings.
    src/audio/ Sound in and out: miniaudio backend, Recorder, CoverArt, AudioInfo.
    src/main.cpp  puts the pieces together.

Behaviour decisions made in the merge
- Queue: the page you click becomes the queue (from luvista.zip); "Add to queue" songs play first (from luvista-merged).
- Repeat: off (stops at the end) / all / one. Autoplay (Settings): when the queue ends, go on with All Songs.
- Recordings are per profile (<profile>/recordings) and live only on the Recordings page, never in the song library.
- Profiles have email + password (salted PBKDF2-SHA256). "Remember me" off = no card on the login screen.
  Profiles made before this merge have no password: pick their card. Guest mode is unchanged.
- Language/Theme are saved per profile but each offers one choice for now.
- First start scans <home>/Music (was a hard-coded C:/Users/lenovo/Music).
- "Remove from library" on a song only forgets it: the file stays on disk and later scans do not bring it back (removed-songs.dat). "Add folder" on that folder brings removed songs back. Deleting a RECORDING deletes its file.
- "Remove from library" on a song only forgets it: the file stays on disk and later scans do not bring it back (removed-songs.dat). "Add folder" on that folder brings removed songs back. Deleting a RECORDING deletes its file.
- Settings > Music folders: the default Music folder is always listed first and cannot be removed; "Add folder" adds more (kept in music-folders.dat, all are scanned at start). Removing a folder takes its songs out of the library (files stay on disk, favorites/playlist entries of those songs go). Songs from folders added by an older version are not listed as folders but stay in the library.
