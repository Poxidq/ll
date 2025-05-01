# Kernel Stack Module & CLI

### Overview
- **int_stack.ko**: Kernel module implementing a thread-safe integer stack.
- **kernel_stack**: CLI tool to interact with the stack.

---

### Features
- Push/pop integers.
- Dynamic stack resizing (`set-size`).
- Error handling (full/empty stack, invalid sizes).

---

### Build & Load
```bash
# Compile module
make

# Load module
sudo insmod int_stack.ko

# Verify
lsmod | grep int_stack
```

### CLI Usage
```bash
# Set stack size
sudo ./kernel_stack set-size 5

# Push values
sudo ./kernel_stack push 10

# Pop
sudo ./kernel_stack pop       # → 10

# Unwind (pop all)
sudo ./kernel_stack unwind

# Errors
sudo ./kernel_stack push 99   # → ERROR: stack is full
sudo ./kernel_stack pop       # → NULL (empty)
```


### Reference list
- https://www.kernel.org/doc/Documentation/kbuild/makefiles.txt
- https://sysprog21.github.io/lkmpg/