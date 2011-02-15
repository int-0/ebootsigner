/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |     ___|    ____| |    \    PSPDEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2005 Dan Peori <peori@oopo.net>
# Licenced under Academic Free License version 2.0
# Review pspsdk README & LICENSE files for further details.
#
# 2006-12-26 - Andrew Whatson <whatson@gmail.com>
#   - rewrote for easier reading
#   - gave "correct" names to UNKNOWN.* files
#   - improved memory efficiency with large PBPs
#   - no longer outputs empty files
#
# $Id: unpack-pbp.c 2228 2007-05-01 05:22:03Z oopo $
*/

// Struct to describe the header of a PBP file
typedef struct {
   char   signature[4];
   int      version;
   int      offset[8];
} HEADER;

int unpack_pbp(FILE *infile);
