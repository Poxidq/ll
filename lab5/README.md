# Example of running

```bash
[root@v src]# lsusb
Bus 001 Device 006: ID 2d95:6004 something
[root@v src]# sudo insmod int_stack.ko usb_vid=0x2d95 usb_pid=0x6004
[root@v src]# dmesg | tail
[  614.386409] usbcore: registered new interface driver int_stack_usb_key
[  614.386412] Module loaded, waiting for USB key...
[root@v src]# ls /dev/int_stack
ls: cannot access '/dev/int_stack': No such file or directory
[root@v src]# dmesg | tail
[  634.135043] usb 1-10: new high-speed USB device number 8 using xhci_hcd
[  634.349678] usb 1-10: New USB device found, idVendor=2d95, idProduct=6004, bcdDevice= 5.15
[  634.349689] usb 1-10: New USB device strings: Mfr=1, Product=2, SerialNumber=3
[  634.349692] usb 1-10: Product: V2247
[  634.349693] usb 1-10: Manufacturer: vivo
[  634.349695] usb 1-10: SerialNumber: 10AE3F0TA40040N
[  634.379729] USB key detected, chardev created
[root@v src]# ls /dev/int_stack
/dev/int_stack
[root@v src]# ./kernel_stack set-size 3
[root@v src]# ./kernel_stack push 3
[root@v src]# ./kernel_stack push 4
[root@v src]# ./kernel_stack push 5
[root@v src]# ./kernel_stack push 6
ERROR: stack is full
[root@v src]# ./kernel_stack pop
5
[root@v src]# dmesg | tail
[  664.223955] usb 1-10: USB disconnect, device number 8
[  664.224108] USB key removed, chardev destroyed
[root@v src]# ./kernel_stack pop
open: No such file or directory
[root@v src]# dmesg | tail
[  681.333346] usb 1-10: new high-speed USB device number 9 using xhci_hcd
[  681.550743] usb 1-10: New USB device found, idVendor=2d95, idProduct=6004, bcdDevice= 5.15
[  681.550747] usb 1-10: New USB device strings: Mfr=1, Product=2, SerialNumber=3
[  681.550749] usb 1-10: Product: V2247
[  681.550750] usb 1-10: Manufacturer: vivo
[  681.550752] usb 1-10: SerialNumber: 10AE3F0TA40040N
[  681.580800] USB key detected, chardev created
[root@v src]# ./kernel_stack pop
4
[root@v src]# ./kernel_stack pop
3
[root@v src]# ./kernel_stack pop
Stack is empty
```