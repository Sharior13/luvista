Shine: a fixed-point MP3 encoder (https://github.com/toots/shine), commit ab5e352.
Licensed under the GNU Library General Public License v2 (see COPYING).
These are the files from src/lib of that repository, with ONE small change so Visual Studio can
compile them: __attribute__((unused)) was replaced by a SHINE_UNUSED macro (defined in types.h)
in l3mdct.c and l3subband.c.
