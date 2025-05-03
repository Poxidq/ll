#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/atomic.h>
#include <linux/usb.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("V");

#define INT_STACK_SET_MAX_SIZE _IOW('s', 1, int)

static int usb_vid = 0x2d95;
static int usb_pid = 0x6002;
module_param(usb_vid, int, 0);
module_param(usb_pid, int, 0);

struct int_stack
{
    int *data;
    int top;
    int max_size;
    struct mutex lock;
};

static struct int_stack stack;
static atomic_t usb_key_count = ATOMIC_INIT(0);
static atomic_t usb_key_present = ATOMIC_INIT(0);

// Helper function to check USB devices during initialization
static int check_usb_device(struct usb_device *udev, void *found_ptr)
{
    int *found = (int *)found_ptr;
    if (udev->descriptor.idVendor == usb_vid &&
        udev->descriptor.idProduct == usb_pid)
    {
        *found = 1;
        return 1;
    }
    return 0;
}

static int int_stack_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int int_stack_release(struct inode *inode, struct file *file)
{
    return 0;
}

static long int_stack_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int max_size;
    int ret = 0;

    if (cmd != INT_STACK_SET_MAX_SIZE)
        return -ENOTTY;

    if (copy_from_user(&max_size, (int __user *)arg, sizeof(max_size)))
        return -EFAULT;

    mutex_lock(&stack.lock);

    if (max_size <= 0)
    {
        ret = -EINVAL;
        goto out;
    }

    if (max_size == stack.max_size)
        goto out;

    if (!stack.max_size)
    {
        stack.data = kmalloc_array(max_size, sizeof(int), GFP_KERNEL);
        if (!stack.data)
        {
            ret = -ENOMEM;
            goto out;
        }
        stack.max_size = max_size;
        stack.top = -1;
    }
    else if (max_size > stack.max_size)
    {
        int *temp = kmalloc_array(max_size, sizeof(int), GFP_KERNEL);
        if (!temp)
        {
            ret = -ENOMEM;
            goto out;
        }
        memcpy(temp, stack.data, (stack.top + 1) * sizeof(int));
        kfree(stack.data);
        stack.data = temp;
        stack.max_size = max_size;
    }
    else
    {
        int *temp = kmalloc_array(max_size, sizeof(int), GFP_KERNEL);
        if (!temp)
        {
            ret = -ENOMEM;
            goto out;
        }
        int elements = min(stack.top + 1, max_size);
        if (elements > 0)
            memcpy(temp, stack.data, elements * sizeof(int));
        stack.top = elements - 1;
        kfree(stack.data);
        stack.data = temp;
        stack.max_size = max_size;
    }

out:
    mutex_unlock(&stack.lock);
    return ret;
}

static ssize_t int_stack_read(struct file *file, char __user *buffer, size_t count, loff_t *pos)
{
    int value;
    int ret = 0;

    if (count < sizeof(int))
        return -EINVAL;

    mutex_lock(&stack.lock);

    if (stack.top < 0)
    {
        ret = 0;
        goto out;
    }

    value = stack.data[stack.top--];
    if (copy_to_user(buffer, &value, sizeof(int)))
        ret = -EFAULT;
    else
        ret = sizeof(int);

out:
    mutex_unlock(&stack.lock);
    return ret;
}

static ssize_t int_stack_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    int value;
    int ret = 0;

    if (count != sizeof(int))
        return -EINVAL;

    if (copy_from_user(&value, buffer, sizeof(int)))
        return -EFAULT;

    mutex_lock(&stack.lock);

    if (stack.top >= stack.max_size - 1)
    {
        ret = -ERANGE;
        goto out;
    }

    stack.data[++stack.top] = value;
    ret = sizeof(int);

out:
    mutex_unlock(&stack.lock);
    return ret;
}

static struct file_operations int_stack_fops = {
    .owner = THIS_MODULE,
    .open = int_stack_open,
    .release = int_stack_release,
    .read = int_stack_read,
    .write = int_stack_write,
    .unlocked_ioctl = int_stack_ioctl,
};

static struct miscdevice int_stack_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "int_stack",
    .fops = &int_stack_fops,
};

static int usb_key_probe(struct usb_interface *interface,
                         const struct usb_device_id *id)
{
    struct usb_device *udev = interface_to_usbdev(interface);

    if (udev->descriptor.idVendor == usb_vid &&
        udev->descriptor.idProduct == usb_pid)
    {
        if (atomic_cmpxchg(&usb_key_present, 0, 1) == 0)
        {
            if (misc_register(&int_stack_miscdev))
            {
                atomic_set(&usb_key_present, 0);
                return -ENODEV;
            }
            pr_info("USB key detected, chardev created\n");
        }
        return 0;
    }
    return -ENODEV;
}

static void usb_key_disconnect(struct usb_interface *interface)
{
    struct usb_device *udev = interface_to_usbdev(interface);

    if (udev->descriptor.idVendor == usb_vid &&
        udev->descriptor.idProduct == usb_pid)
    {
        if (atomic_cmpxchg(&usb_key_present, 1, 0) == 1)
        {
            misc_deregister(&int_stack_miscdev);
            pr_info("USB key removed, chardev destroyed\n");
        }
    }
}

static struct usb_device_id usb_key_id_table[] = {
    {USB_DEVICE(0x0000, 0x0000)}, {}};

MODULE_DEVICE_TABLE(usb, usb_key_id_table);

static struct usb_driver usb_key_driver = {
    .name = "int_stack_usb_key",
    .probe = usb_key_probe,
    .disconnect = usb_key_disconnect,
    .id_table = usb_key_id_table,
};

static int __init int_stack_init(void)
{
    /* Initialize empty stack */
    stack.data = NULL;
    stack.top = -1;
    stack.max_size = 0;
    mutex_init(&stack.lock);

    /* Dynamic VID/PID injection */
    usb_key_id_table[0].idVendor = usb_vid;
    usb_key_id_table[0].idProduct = usb_pid;
    usb_key_id_table[0].match_flags = USB_DEVICE_ID_MATCH_DEVICE;

    int ret = usb_register(&usb_key_driver);
    if (ret)
    {
        pr_err("Failed to register USB driver\n");
        return ret;
    }
    pr_info("Module loaded, waiting for USB key...\n");
    return 0;
}

static void __exit int_stack_exit(void)
{
    usb_deregister(&usb_key_driver);
    if (stack.data)
        kfree(stack.data);
    mutex_destroy(&stack.lock);
    pr_info("Module unloaded\n");
}

module_init(int_stack_init);
module_exit(int_stack_exit);