#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/atomic.h>

// IOCTL call 's' with command 1 that allows to copy from user (write parameters)
#define INT_STACK_SET_MAX_SIZE _IOW('s', 1, int)

struct int_stack
{
    int *data;
    int top;
    int max_size;
    struct mutex lock;
    // atomic_t refcount; // if 0 then can we can release
};

static struct int_stack stack;

// static struct int_stack stack = {
//     .refcount = ATOMIC_INIT(0),
// };

static DEFINE_MUTEX(init_mutex);

static int int_stack_open(struct inode *inode, struct file *file)
{
    // mutex_lock(&init_mutex);
    // if (atomic_inc_return(&stack.refcount) == 1)
    // {
    //     stack.data = NULL;
    //     stack.top = -1;
    //     stack.max_size = 0;
    //     mutex_init(&stack.lock);
    // }
    // mutex_unlock(&init_mutex);
    return 0;
}

static int int_stack_release(struct inode *inode, struct file *file)
{
    // if (atomic_dec_and_test(&stack.refcount))
    // {
    //     if (stack.data)
    //         kfree(stack.data);
    //     mutex_destroy(&stack.lock);
    // }
    return 0;
}

static long int_stack_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int max_size;
    int ret = 0;

    switch (cmd)
    {
    case INT_STACK_SET_MAX_SIZE:
        if (copy_from_user(&max_size, (int __user *)arg, sizeof(max_size)))
            return -EFAULT;

        mutex_lock(&stack.lock);

        if (max_size <= 0)
        {
            ret = -EINVAL;
            goto out;
        }

        if (max_size == stack.max_size)
        {
            ret = 0;
            goto out;
        }

        if (stack.max_size == 0)
        {
            // Initial configuration
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
            // Resize to a larger size
            int *temp = kmalloc_array(max_size, sizeof(int), GFP_KERNEL);
            if (!temp)
            {
                ret = -ENOMEM;
                goto out;
            }

            // Copy existing data
            if (stack.top >= 0)
            {
                size_t bytes_to_copy = (stack.top + 1) * sizeof(int);
                memcpy(temp, stack.data, bytes_to_copy);
            }

            kfree(stack.data); // Free old array
            stack.data = temp;
            stack.max_size = max_size;
        }
        else
        {
            // Handle stack size decrease
            int *temp = kmalloc_array(max_size, sizeof(int), GFP_KERNEL);
            if (!temp)
            {
                ret = -ENOMEM;
                goto out;
            }

            if (stack.top >= 0)
            {
                int elements_to_keep = min(stack.top + 1, max_size);

                size_t bytes_to_copy = elements_to_keep * sizeof(int);
                memcpy(temp, stack.data, bytes_to_copy);

                stack.top = elements_to_keep - 1;
            }
            else
            {
                stack.top = -1;
            }

            kfree(stack.data);
            stack.data = temp;
            stack.max_size = max_size;
        }

        ret = 0;

    out:
        mutex_unlock(&stack.lock);
        return ret;

    default:
        return -ENOTTY;
    }
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

    value = stack.data[stack.top];
    stack.top--;

    if (copy_to_user(buffer, &value, sizeof(int)))
    {
        ret = -EFAULT;
        goto out;
    }

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
    if (stack.max_size == 0)
    {

        ret = -ERANGE;
        goto out;
    }

    if (stack.top >= stack.max_size - 1)
    {
        ret = -ERANGE;
        goto out;
    }

    stack.top++;
    stack.data[stack.top] = value;
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
    .unlocked_ioctl = int_stack_ioctl, // we use our own locking #-# https://lwn.net/Articles/119652/
};

static struct miscdevice int_stack_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "int_stack",
    .fops = &int_stack_fops,
};

static int __init int_stack_init(void)
{
    stack.data = NULL;
    stack.top = -1;
    stack.max_size = 0;
    mutex_init(&stack.lock);
    return misc_register(&int_stack_miscdev);
}

static void __exit int_stack_exit(void)
{
    misc_deregister(&int_stack_miscdev);
    if (stack.data)
        kfree(stack.data);
    mutex_destroy(&stack.lock);
}

module_init(int_stack_init);
module_exit(int_stack_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("V");