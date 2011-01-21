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

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#ifdef WORDS_BIGENDIAN
// Swap the bytes of an int for big-endian machines
static int swap_int(int n) {
  return ((n >> 24) & 0xff) | ((n >> 8) & 0xff00) | ((n << 8) & 0xff0000) | ((n << 24) & 0xff000000);
}
#endif

// Struct to describe the header of a PBP file
typedef struct {
   char   signature[4];
   int      version;
   int      offset[8];
} HEADER;

// Correct PBP signature
char correct_sig[4] = {
   0x00,
   0x50,   // P
   0x42,   // B
   0x50   // P
};

// Names of files included in a PBP
char *filename[8] = {
   "param.sfo",
   "icon0.png",
   "icon1.pmf",
   "pic0.png",
   "pic1.png",
   "snd0.at3",
   "data_unsigned.psp",
   "data.psar"
};

int main_unpack_pbp(FILE *infile) {
  FILE *outfile;
  HEADER header;
  int loop0;
  int total_size;
   
  // Get the size of the PBP
  fseek(infile, 0, SEEK_END);
  total_size = ftell(infile);
  fseek(infile, 0, SEEK_SET);
  if (total_size < 0) {
    printf("ERROR: Could not get the input file size.\n");
    return -1;
  }
   
  // Read in the header
  if (fread(&header, sizeof(HEADER), 1, infile) < 0) {
    printf("ERROR: Could not read the input file header.\n");
    return -1;
  }
   
  // Check the signature
  for (loop0 = 0; loop0 < sizeof(correct_sig); loop0++) {
    if (header.signature[loop0] != correct_sig[loop0]) {
      printf("ERROR: Input file is not a PBP file.\n");
      return -1;
    }
  }
   
#ifdef WORDS_BIGENDIAN 
  // Swap the bytes of the offsets for big-endian machines
  for (loop0 = 0; loop0 < 8; loop0++) {
    header.offset[loop0] = swap_int(header.offset[loop0]);
  }
#endif
   
  // For each file in the PBP
  for (loop0 = 0; loop0 < 8; loop0++) {
    void *buffer;
    int size;
    
    // Get the size of this file
    if (loop0 == 7) {
      size = total_size - header.offset[loop0];
    } else {
      size = header.offset[loop0 + 1] - header.offset[loop0];
    }
      
    // Skip the file if empty
    if (!size) continue;
      
    // Open the output file
    outfile = fopen(filename[loop0], "wb");
    if (outfile == NULL) {
      printf("\nERROR: Could not open the output file. (%s)\n", filename[loop0]);
      return -1;
    }
      
    do {
      int readsize;
         
      // Make sure we don't exceed the maximum buffer size
      if (size > maxbuffer) {
	readsize = maxbuffer;
      } else {
	readsize = size;
      }
      size -= readsize;
         
      // Create the read buffer
      buffer = malloc(readsize);
      if (buffer == NULL) {
	printf("\nERROR: Could not allocate the section data buffer. (%d)\n", readsize);
	return -1;
      }
         
      // Read in the data from the PBP
      if (fread(buffer, readsize, 1, infile) < 0) {
	printf("\nERROR: Could not read in the section data.\n");
	return -1;
      }
         
      // Write the contents of the buffer to the output file
      if (fwrite(buffer, readsize, 1, outfile) < 0) {
	printf("\nERROR: Could not write out the section data.\n");
	return -1;
      }
         
      // Clean up the buffer
      free(buffer);
         
      // Repeat if we haven't finished writing the file
    } while (size);
      
    // Close the output file
    if (fclose(outfile) < 0) {
      printf("\nERROR: Could not close the output file.\n");
      return -1;
    }
  }
   
  // Close the PBP
  if (fclose(infile) < 0) {
    printf("ERROR: Could not close the input file.\n");
    return -1;
  }
   
  // Exit successful
  return 0;
} 
