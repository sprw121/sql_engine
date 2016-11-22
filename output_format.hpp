#ifndef _OUTPUT_FORMAT_H
#define _OUTPUT_FORMAT_H

// Exists only to communicate the format type
// to select.

// Quick and dirty

enum format_t
{
    FORMATTED,
    CSV
};

extern format_t out_format;

#endif
