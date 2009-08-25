/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_util_AnsiEscapeCodes_H
#define db_util_AnsiEscapeCodes_H

/**
 * ANSI Escape Codes
 * http://en.wikipedia.org/wiki/ANSI_escape_code
 */

// Text attributes
// Reset/Normal - All attributes off
#define DB_ANSI_RESET             "0"
// Intensity: Bold
#define DB_ANSI_BOLD              "1"
// Intensity: Faint - not widely supported
#define DB_ANSI_FAINT             "2"
// Italic - not widely supported. Sometimes treated as inverse.
#define DB_ANSI_ITALIC            "3"
// Underline: Single - not widely supported
#define DB_ANSI_UNDERLINE_SINGLE  "4"
// Blink: Slow - less than 150 per minute
#define DB_ANSI_BLINK_SLOW        "5"
// Blink: Rapid - MS-DOS ANSI.SYS; 150+ per minute
#define DB_ANSI_BLINK_RAPID       "6"
// Image: Negative - inverse or reverse; swap foreground and background
#define DB_ANSI_NEGATIVE          "7"
// Conceal - not widely supported
#define DB_ANSI_CONCEAL           "8"
// Underline: Double
#define DB_ANSI_UNDERLINE_DOUBLE "21"
// Intensity: Normal - not bold and not faint
#define DB_ANSI_NORMAL           "22"
// Underline: None
#define DB_ANSI_UNDERLINE_NONE   "24"
// Blink: off
#define DB_ANSI_BLINK_OFF        "25"
// Image: Positive
#define DB_ANSI_POSITIVE         "27"
// Reveal - conceal off
#define DB_ANSI_REVEAL           "28"

// Foreground colors (normal intensity)
#define DB_ANSI_FG_BLACK         "30"
#define DB_ANSI_FG_RED           "31"
#define DB_ANSI_FG_GREEN         "32"
#define DB_ANSI_FG_YELLOW        "33"
#define DB_ANSI_FG_BLUE          "34"
#define DB_ANSI_FG_MAGENTA       "35"
#define DB_ANSI_FG_CYAN          "36"
#define DB_ANSI_FG_WHITE         "37"

// Background colors (normal intensity)
#define DB_ANSI_BG_BLACK         "40"
#define DB_ANSI_BG_RED           "41"
#define DB_ANSI_BG_GREEN         "42"
#define DB_ANSI_BG_YELLOW        "43"
#define DB_ANSI_BG_BLUE          "44"
#define DB_ANSI_BG_MAGENTA       "45"
#define DB_ANSI_BG_CYAN          "46"
#define DB_ANSI_BG_WHITE         "47"

// Foreground colors (high intensity)
#define DB_ANSI_FG_HI_BLACK      "90"
#define DB_ANSI_FG_HI_RED        "91"
#define DB_ANSI_FG_HI_GREEN      "92"
#define DB_ANSI_FG_HI_YELLOW     "93"
#define DB_ANSI_FG_HI_BLUE       "94"
#define DB_ANSI_FG_HI_MAGENTA    "95"
#define DB_ANSI_FG_HI_CYAN       "96"
#define DB_ANSI_FG_HI_WHITE      "97"

// Background colors (high intensity)
#define DB_ANSI_BG_HI_BLACK     "100"
#define DB_ANSI_BG_HI_RED       "101"
#define DB_ANSI_BG_HI_GREEN     "102"
#define DB_ANSI_BG_HI_YELLOW    "103"
#define DB_ANSI_BG_HI_BLUE      "104"
#define DB_ANSI_BG_HI_MAGENTA   "105"
#define DB_ANSI_BG_HI_CYAN      "106"
#define DB_ANSI_BG_HI_WHITE     "107"

// Control Sequence Introducer
#define DB_ANSI_CSI           "\x1b["
// Parameter Separator
#define DB_ANSI_SEP               ";"
// Select Graphic Rendition
#define DB_ANSI_SGR               "m"
// reset everything
#define DB_ANSI_OFF DB_ANSI_CSI DB_ANSI_SGR

#endif
