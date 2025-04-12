# Summary
```
jnz/jmp/nop
__get_cpuid
```

# Usage

```bash
$ gcc -Wall genkey.c -o genkey -lcrypto -lssl
$ ./genkey
# example output
HWID: 120FA600FFFB8B17
Your license key: cc0a840edb4679a2b962008ae7fee5c0
$ ./hack_app # providing license key
# checking that works:
$ getfattr -n user.license hack_app
# file: hack_app
user.license="cc0a840edb4679a2b962008ae7fee5c0"
```

```bash
$ gcc patch.c -o patch
$ ./patch hack_app
Patched at offset 0x000015ab
Patching completed successfully!
$ chmod +x hack_app.patched
$ ./hack_app.patched
Welcome to Lab2 super secure program!
Your app is licensed to this PC!
Press Enter to continue...
```
