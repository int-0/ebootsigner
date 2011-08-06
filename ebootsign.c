#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#ifndef __APPLE__
#include <malloc.h>
#else
#include <malloc/malloc.h>
#endif
#include <sys/unistd.h>

#if defined(_MSC_VER) || defined (__MINGW32__)
#define mkdir(p,m) _mkdir(p)
#endif

#define MAX_BUFFER_SIZE 1024*1024*5+1024*512
#define MAX_ORIGIN_FILE_SIZE 1024*1024*4

#include "main_crypter.h"
#include "unpack-pbp.h"
#include "pack-pbp.h"
#include "fix-realocations.h"

char *filename_list[10] = {
  "name",
  "EBOOT_signed.PBP",
  "param.sfo",
  "icon0.png",
  "icon1.pmf",
  "pic0.png",
  "pic1.png",
  "snd0.at3",
  "data.psp",
  "data.psar"
};

void clean_tmp(void) {
  remove("param.sfo");
  remove("icon0.png");
  remove("icon1.pmf");
  remove("pic0.png");
  remove("pic1.png");
  remove("snd0.at3");
  remove("data.psp");
  remove("data_unsigned.psp");
  remove("data.psar");
  chdir("../");
  rmdir("tmp_sgn");
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("USAGE: %s <unsigned.pbp> <outfile.pbp>\n", argv[0]);
    return 1;
  }

  const char *filename = argv[1];
  FILE *infile;
  // Try to open given filename
  infile = fopen(filename, "r");
  if (infile == NULL) {
    printf("ERROR: Could not open %s\n", filename);
    return -1;
  }

  // Try to make given filename
  char *dest_file = argv[2];
  FILE *outfile;
  remove(dest_file);
  outfile = fopen(dest_file, "w");
  if (outfile == NULL) {
    printf("ERROR: Could not create %s\n", dest_file);
    return -1;
  }

  int err = 0;

  // Make temp directory
  mkdir("./tmp_sgn/", 0777);
  chdir("./tmp_sgn/");
  remove("param.sfo");
  remove("icon0.png");
  remove("icon1.pmf");
  remove("pic0.png");
  remove("pic1.png");
  remove("snd0.at3");
  remove("data.psp");
  remove("data_unsigned.psp");
  remove("data.psar");
  
  // Unpack PBP file
  err = unpack_pbp(infile);
  if(err != 0) {
    printf("Error while unpacking: %d\n",err);
    clean_tmp();
    return 1;
  }
  
  // Fix PRX realocations
  err = fix_realocations();
  if(err != 0) {
    printf("Error while fixing realocations: %d\n",err);
    clean_tmp();
    return 1;
  }
  
  // Crypt PRX file
  err = main_crypter();
  if(err != 0) {
    printf("Error while crypting: %d\n",err);
    clean_tmp();
    return 1;
  }

  // Repacking
  err = pack_pbp(outfile, filename_list);
  if(err != 0) {
    printf("Error while packing: %d\n",err);
    clean_tmp();
    return 1;
  }

  // Done
  clean_tmp();
  return 0;
}
