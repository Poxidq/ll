#include <cpuid.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <string.h>

void compute_md5(char *str, unsigned char digest[16]);

void compute_md5(char *str, unsigned char digest[16]) {
  MD5_CTX ctx;
  MD5_Init(&ctx);
  MD5_Update(&ctx, str, strlen(str));
  MD5_Final(digest, &ctx);
}

int main() {
  unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
  __get_cpuid(1, &eax, &ebx, &ecx, &edx);

  unsigned int part1 =
      eax << 0x18 | eax >> 0x18 | (eax & 0xff00) << 8 | eax >> 8 & 0xff00;
  unsigned int part2 =
      edx << 0x18 | edx >> 0x18 | (edx & 0xff00) << 8 | edx >> 8 & 0xff00;

  char buf[20];
  snprintf(buf, sizeof(buf), "%08X%08X", part1, part2);
  char md5decode[16];
  unsigned char digest[16];

  printf("HWID: %s\n", buf);
  compute_md5(buf, digest);
  printf("Your license key: ");
  for (int i = 0; i < 16; i++)
    printf("%02x", digest[0xf - i]);

  printf("\n");

  return 0;
}
