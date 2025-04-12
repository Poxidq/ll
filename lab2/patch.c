#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void patchProgram(unsigned char *buffer, int buflen,
                  const unsigned char *needle, int needlelen,
                  const unsigned char *replacement);

int main(int argc, char *argv[]) {
  FILE *fileptr;
  unsigned char *buffer;
  long filelen;

  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return 1;
  }

  char *filename = argv[1];

  fileptr = fopen(filename, "rb");
  if (!fileptr) {
    perror("Failed to open file");
    return 1;
  }

  fseek(fileptr, 0, SEEK_END);
  filelen = ftell(fileptr);
  rewind(fileptr);

  buffer = (unsigned char *)malloc(filelen);
  if (!buffer) {
    perror("Memory allocation failed");
    fclose(fileptr);
    return 1;
  }

  if (fread(buffer, 1, filelen, fileptr) != (size_t)filelen) {
    perror("Failed to read file");
    fclose(fileptr);
    free(buffer);
    return 1;
  }
  fclose(fileptr);

  const unsigned char needle[] = {0x85, 0xc0, 0x75, 0x07, 0xc7, 0x45};
  const unsigned char replacement[] = {0x85, 0xc0, 0x48, 0x90, 0xc7, 0x45};
  const int needlelen = sizeof(needle);

  patchProgram(buffer, filelen, needle, needlelen, replacement);

  char *fixed_filename = strcat(filename, ".patched");
  fileptr = fopen(fixed_filename, "wb");
  if (!fileptr) {
    perror("Failed to open file for writing");
    free(buffer);
    return 1;
  }
  fwrite(buffer, 1, filelen, fileptr);
  fclose(fileptr);

  free(buffer);
  printf("Patching completed successfully!\n");
  return 0;
}

void patchProgram(unsigned char *buffer, int buflen,
                  const unsigned char *needle, int needlelen,
                  const unsigned char *replacement) {
  if (needlelen < 1 || buflen < needlelen)
    return;

  for (int i = 0; i <= buflen - needlelen; i++) {
    if (memcmp(buffer + i, needle, needlelen) == 0) {
      memcpy(buffer + i, replacement, needlelen);
      printf("Patched at offset 0x%08x\n", 0x10000 + i);
      return; // patch only first occurrence
    }
  }
}
