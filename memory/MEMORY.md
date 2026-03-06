# FujiNet Stocks - Project Memory

## Build
- Run `bash build.sh 2>&1` from the project root to build all platforms.
- build.sh runs `make atari apple2 coco msdos` then copies output to /tnfs.
- Always capture full output and analyze all warnings and errors.

## Project Structure
- Multi-platform FujiNet stock ticker (Atari, Apple II, CoCo, MS-DOS).
- Platform-specific code in src/{atari,apple2,coco,msdos}/.
- Shared code: src/main.c, src/screen.c, src/stocks.c, src/network.c, src/appkey.c, src/get_line.c (cc65 only), src/sync_frame.c (non-CoCo).
- Platform headers in include/{atari,apple2,coco,msdos}/.

## Git
- Do NOT include "Co-Authored-By" lines in commit messages.

## Key Conventions
- BREAK_KEY_NAME defined in include/screen.h as "BREAK" (CoCo) or "ESC" (others) — no angle brackets; callers wrap with < >.
- All functions have doxygen-style block comments.
- get_line() returns empty string on BREAK on all platforms.
