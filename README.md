# fujinet-stocks

A real-time stock ticker application for FujiNet-enabled 8-bit computers.
Tracks up to 10 symbols, displays a scrolling ticker tape, and lets you look
up, add, edit, and delete stocks — all from a 40/42-column text screen.

## Supported Platforms

| Platform | Toolchain |
|----------|-----------|
| CoCo (Color Computer) | cmoc |
| Apple II | cc65 |
| Atari | cc65 |
| Commodore 64 | cc65 |
| Adam | z88dk |
| Adam (CP/M) | z88dk |
| MS-DOS | wcc |
| MSX (ROM) | cc65 |

## Features

- Scrolling ticker tape (row 0) showing symbol, price, and change %
- 10 stock slots displayed in two columns of five
- Auto-refresh every 5 minutes via the [Finnhub](https://finnhub.io) API
- Company info screen: name, exchange, industry, market cap, IPO date, website
- Symbol lookup by name or partial ticker
- Stocks persisted across sessions using FujiNet AppKey storage
- Fixed-point arithmetic throughout — no floating point required on the 8-bit target

## Screen Layout

```
Row  0      : scrolling ticker tape  (AAPL $192.50 +1.23%   GOOG $...)
Rows 1-21   : stock list / info screen / lookup results
Rows 22-23  : command menu
```

The stock list is centred in the content area as two columns of five:

```
 1: AAPL        6: MSFT
 2: GOOG        7: AMZN
 3: TSLA        8: NVDA
 4: META        9: AMD
 5: NFLX       10: INTC
```

## Keyboard Controls

| Key | Action |
|-----|--------|
| Arrow keys | Move selection |
| `S` | Toggle stock list display |
| `E` | Edit the selected slot (type a new symbol) |
| `D` | Delete the selected stock |
| `I` | Show company info for the selected stock |
| `L` | Look up a symbol by name or partial ticker |
| `BREAK` | Quit (saves stock list before exiting) |

## Requirements

- A FujiNet device connected to your 8-bit computer
- A free [Finnhub](https://finnhub.io) API key
- Platform toolchain: **cmoc** (CoCo), **cc65** (Apple II, Atari, C64), Open Watcom C (msdos)
- FujiNet library **4.10.0** (fetched automatically by the build)

## Configuration

Set your Finnhub API key in [include/network.h](include/network.h):

```c
#define FINNHUB_API_KEY  "your_key_here"
```

## Building

Build for a specific platform:

```sh
make coco
make apple2
make atari
make c64
make adam
make msdos
make msxrom
make adam_cpm
```

Build all configured platforms:

```sh
make
```

Platform-specific targets (e.g. disk image):

```sh
make apple2/disk
```

## Project Structure

```
src/
  main.c          – entry point (init, load, fetch, loop, save)
  stocks.c        – stock list management, scroll string, persistence
  screen.c        – main loop, rendering, keyboard handling
  network.c       – Finnhub API calls (quote, profile, lookup)
  appkey.c        – FujiNet AppKey read/write
  get_line.c      – line-input helper
  sync_frame.c    – platform frame-sync stub
  <platform>/     – platform-specific init and time implementations
include/
  stocks.h        – shared types and prototypes
  screen.h        – screen layout constants
  network.h       – API URLs, key, and prototypes
  appkey.h        – AppKey IDs and prototypes
  <platform>/     – platform-specific character definitions
```

## Data Storage

The stock symbol list is saved to two FujiNet AppKeys (creator `0x0901`,
app `0x01`) as pipe-separated strings:

- Key 1 → slots 1–5  (e.g. `AAPL|GOOG|TSLA`)
- Key 2 → slots 6–10 (e.g. `MSFT|AMZN`)
