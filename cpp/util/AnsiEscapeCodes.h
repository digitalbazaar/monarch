/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_util_AnsiEscapeCodes_H
#define monarch_util_AnsiEscapeCodes_H

/**
 * ANSI Escape Codes
 * http://en.wikipedia.org/wiki/ANSI_escape_code
 */

// Text attributes
// Reset/Normal - All attributes off
#define MO_ANSI_RESET             "0"
// Intensity: Bold
#define MO_ANSI_BOLD              "1"
// Intensity: Faint - not widely supported
#define MO_ANSI_FAINT             "2"
// Italic - not widely supported. Sometimes treated as inverse.
#define MO_ANSI_ITALIC            "3"
// Underline: Single - not widely supported
#define MO_ANSI_UNDERLINE_SINGLE  "4"
// Blink: Slow - less than 150 per minute
#define MO_ANSI_BLINK_SLOW        "5"
// Blink: Rapid - MS-DOS ANSI.SYS; 150+ per minute
#define MO_ANSI_BLINK_RAPID       "6"
// Image: Negative - inverse or reverse; swap foreground and background
#define MO_ANSI_NEGATIVE          "7"
// Conceal - not widely supported
#define MO_ANSI_CONCEAL           "8"
// Underline: Double
#define MO_ANSI_UNDERLINE_DOUBLE "21"
// Intensity: Normal - not bold and not faint
#define MO_ANSI_NORMAL           "22"
// Underline: None
#define MO_ANSI_UNDERLINE_NONE   "24"
// Blink: off
#define MO_ANSI_BLINK_OFF        "25"
// Image: Positive
#define MO_ANSI_POSITIVE         "27"
// Reveal - conceal off
#define MO_ANSI_REVEAL           "28"

// Foreground colors (normal intensity)
#define MO_ANSI_FG_BLACK         "30"
#define MO_ANSI_FG_RED           "31"
#define MO_ANSI_FG_GREEN         "32"
#define MO_ANSI_FG_YELLOW        "33"
#define MO_ANSI_FG_BLUE          "34"
#define MO_ANSI_FG_MAGENTA       "35"
#define MO_ANSI_FG_CYAN          "36"
#define MO_ANSI_FG_WHITE         "37"

// Background colors (normal intensity)
#define MO_ANSI_BG_BLACK         "40"
#define MO_ANSI_BG_RED           "41"
#define MO_ANSI_BG_GREEN         "42"
#define MO_ANSI_BG_YELLOW        "43"
#define MO_ANSI_BG_BLUE          "44"
#define MO_ANSI_BG_MAGENTA       "45"
#define MO_ANSI_BG_CYAN          "46"
#define MO_ANSI_BG_WHITE         "47"

// Foreground colors (high intensity)
#define MO_ANSI_FG_HI_BLACK      "90"
#define MO_ANSI_FG_HI_RED        "91"
#define MO_ANSI_FG_HI_GREEN      "92"
#define MO_ANSI_FG_HI_YELLOW     "93"
#define MO_ANSI_FG_HI_BLUE       "94"
#define MO_ANSI_FG_HI_MAGENTA    "95"
#define MO_ANSI_FG_HI_CYAN       "96"
#define MO_ANSI_FG_HI_WHITE      "97"

// Background colors (high intensity)
#define MO_ANSI_BG_HI_BLACK     "100"
#define MO_ANSI_BG_HI_RED       "101"
#define MO_ANSI_BG_HI_GREEN     "102"
#define MO_ANSI_BG_HI_YELLOW    "103"
#define MO_ANSI_BG_HI_BLUE      "104"
#define MO_ANSI_BG_HI_MAGENTA   "105"
#define MO_ANSI_BG_HI_CYAN      "106"
#define MO_ANSI_BG_HI_WHITE     "107"

// Control Sequence Introducer
#define MO_ANSI_CSI           "\x1b["
// Parameter Separator
#define MO_ANSI_SEP               ";"
// Select Graphic Rendition
#define MO_ANSI_SGR               "m"
// reset everything
#define MO_ANSI_OFF MO_ANSI_CSI MO_ANSI_SGR

#endif
