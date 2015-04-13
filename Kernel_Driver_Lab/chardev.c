/*
 * chardev.c : Create an input/output character device.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/sched.h>

#include "chardev.h"
#define SUCCESS 0
#define DEVICE_NAME "chardev"
#define BUF_LEN 80
#define DEBUG

/*
 * Is the device open right now? Used to prevent
 * concurrent access into the same device.
 */
static int Device_Open = 0;

/*
 * The message the device will give when asked.
 */

static char Message[BUF_LEN];

/*
 * How far did the process reading the message get?
 * Useful if the message is larger than the size of the buffer we get to fill in device_read.
 */

static char* Message_Ptr;
static char* call_tree_Ptr;

/*
 * This is called whenever a process attempts to open the device file.
 */

static int device_open(struct inode* inode, struct file* file)
{
#ifdef DEBUF
	printk(KERN_INFO "device_open(%p)\n", file);
#endif
	
	/*
	 * We don't want to talk to two processes at the same time.
	 */
	if (Device_Open)
	{
		return -EBUSY;
	}
	Device_Open++;
	/*
	 * Initialize the message
	 */
	Message_Ptr = Message;
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

static int device_release (struct inode* inode, struct file* file)
{
#ifdef DEBUG
	printk(KERN_INFO "device_release(%p, %p)\n", inode, file);
#endif

	/*
	 * We're now ready for our next caller.
	 */
	Device_Open--;

	module_put(THIS_MODULE);
	return SUCCESS;
}


char* string_adder(char* str1, char* str2)
{
    int len1 = 0;
    int len2 = 0;
    while(str1[len1] != '\0')
    {
	len1++;
    }
    while(str2[len2] != '\0')
    {
	len2++;
    }
   /* printf("length of str1 : %d, length of str2 : %d\n", len1, len2);*/
    int total_len = len1+len2;
    int cnt;
    char* res = kmalloc(sizeof(char)*(total_len+1), GFP_ATOMIC);
    for(cnt = 0; cnt < len1; cnt++)
    {
	res[cnt] = str1[cnt];
    }
    for(cnt = 0; cnt < len2; cnt++)
    {
	res[cnt + len1] = str2[cnt];
    }
    res[total_len] = '\0';
    return res;
}
char* int_to_str(int num)
{
    char* res = "";
    while(num > 9)
    {
	int temp = num%10;
	if (temp == 0) { res = string_adder(res, "0");}
	else if (temp == 1) { res = string_adder(res, "1"); }
	else if (temp == 2) { res = string_adder(res, "2"); }
	else if (temp == 3) { res = string_adder(res, "3"); }
	else if (temp == 4) { res = string_adder(res, "4"); }
	else if (temp == 5) { res = string_adder(res, "5"); }
	else if (temp == 6) { res = string_adder(res, "6"); }
	else if (temp == 7) { res = string_adder(res, "7"); }
	else if (temp == 8) { res = string_adder(res, "8"); }
	else { res = string_adder(res, "9"); }
	num = num/10;
    }	
    int temp = num;
    	if (temp == 0) { res = string_adder(res, "0");}
    	else if (temp == 1) { res = string_adder(res, "1"); }
	else if (temp == 2) { res = string_adder(res, "2"); }
	else if (temp == 3) { res = string_adder(res, "3"); }
	else if (temp == 4) { res = string_adder(res, "4"); }
	else if (temp == 5) { res = string_adder(res, "5"); }
	else if (temp == 6) { res = string_adder(res, "6"); }
	else if (temp == 7) { res = string_adder(res, "7"); }
	else if (temp == 8) { res = string_adder(res, "8"); }
	else { res = string_adder(res, "9"); }

   	return res;
}
	


/*
 * This function is called whenever a process which has already opened the
 * device file attempts to read from it.
 */
static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset)
{
	/*
	 * Number of bytes actually written to the buffer.
	 */
	int bytes_read = 0;
#ifdef DEBUG
	printk(KERN_INFO "device_read(%p, %p, %d)\n", file, buffer, length);
#endif

	/*
	 * If we're at the end of the message, return 0
	 * (which signifies end of file)
	 */
	if (*Message_Ptr == 0)
	{
		return 0;
	}

	/*
	 * Actually put the data into the buffer
	 */
	char* call_tree = "";
#ifdef DEBUG
	printk(KERN_INFO "Read %d bytes, %d left\n", bytes_read, length);
	printk(KERN_INFO "Current PID : %d\n", current->pid);
	printk(KERN_INFO "Parent PID : %d\n", current->real_parent->pid);
#endif
	struct task_struct* task = current;
	int stopper = 0;
	char* pathname;
	char* p;
	struct mm_struct* mm;
	int prev_pid = current->pid;
	do
	{
	    //printk(KERN_INFO "PID : %d\n", task->pid);
	    //printk(KERN_INFO "real_parent : %p\n", task->real_parent);
	    mm = task->mm;
	    if(mm)
	    {
		down_read(&mm->mmap_sem);
		if (mm->exe_file)
		{
		    pathname = kmalloc(PATH_MAX, GFP_ATOMIC);
		    if (pathname)
		    {
			p = d_path(&mm->exe_file->f_path, pathname, PATH_MAX);
		    }
		    if (task == current)
		    {
			p = string_adder("(caller)", p);
		    }
		}
		up_read(&mm->mmap_sem);
	    }
	    //printk(KERN_INFO "Process name : %s\n", p);
	    call_tree = string_adder("\n", call_tree);
	    call_tree = string_adder(")", call_tree);
	    call_tree = string_adder(int_to_str(task->pid), call_tree);
	    call_tree = string_adder("(", call_tree);
	    call_tree = string_adder(" ", call_tree);
	    call_tree = string_adder(p, call_tree);
	    call_tree = string_adder("- ", call_tree);
	    prev_pid = task->pid;
	    task = task->real_parent;
	    stopper++;
	}while((prev_pid != 0) && (stopper < 20));

	//printk(KERN_INFO "%s\n", call_tree);
	call_tree_Ptr = call_tree;
	while(length && *call_tree_Ptr)
	{
		/*
		 * Because the buffer is in the user data segment, not the kernel data segment,
		 * assignment wouldn't work.Instead, we have to use put_user which copies data from the kernel data segment to the user data segment.
		 */
		put_user(*(call_tree_Ptr++), buffer++);
		length--;
		bytes_read++;
	}
	/*
	 * Read functions are supposed to return the number
	 * of bytes actually inserted into the buffer.
	 */
	return bytes_read;
}

/*
 * This function is called when somebody tries to write into our device file.
 */
static ssize_t device_write(struct file* file, char __user* buffer, size_t length, loff_t* offset)
{
	ssize_t i;
#ifdef DEBUG
	printk(KERN_INFO "device_write(%p, %s, %d)\n", file, buffer, length);
#endif
	for(i = 0; i < length && i < BUF_LEN; i++)
	{
		get_user(Message[i], buffer + i);
	}
	Message_Ptr = Message;

	/*
	 * Again, return the number of input characters used.
	 */
	return i;
}

/*
 * This function is called whenever a process tries to do an ioctl on our
 * device file. We get two extra parameters (additional to the inode and file structures,
 * which all device functions get) : the number of the ioctl called and the parameter given to the ioctl function.
 *
 * If the ioctl is write or eead/write (meaning output is returned to the calling process), the ioctl call returns the output of this function.
 *
 */

/*int*/long device_ioctl(/*struct inode* inode, */struct file* file, unsigned int ioctl_num, unsigned long ioctl_param)
{
    	/*
	 * For Linux kernel version 2.6.39 and after ones, ioctl calls has changed into
	 * unlocked_ioctl calls, because of the inefficiency of 'Big Kernel Lock' that is due to
	 * traditional ioctl calls cause.
	 * Plus, we don't have to pass the inode* parameter to ioctl functions, the return type
	 * has changed into long.
	 */
	int i;
	char* temp;
	char ch;

	/*
	 * Switch according to the ioctl called.
	 */
	switch (ioctl_num)
	{
		case IOCTL_SET_MSG:
			/*
			 * Receive a pointer to a message (in user space) and set that
			 * to be the device's message. Get the parameter given to ioctl by the process.
			 */
			temp = (char*)ioctl_param;
			/*
			 * Find the length of the message.
			 */
			get_user(ch, temp);
			for(i = 0; ch && i < BUF_LEN; i++, temp++)
			{
				get_user(ch, temp);
			}
			device_write(file, (char*)ioctl_param, i, 0);
			break;
		case IOCTL_GET_MSG:
			/*
			 * Give the current message to the calling process - 
			 * the parameter we got is a pointer, fill it.
			 */
			i = device_read(file, (char*)ioctl_param, 1023, 0);

			/*
			 * Put a zero at the end of the buffer, so it will be properly terminated
			 */
			put_user('\0', (char*)ioctl_param + i);
			break;
		case IOCTL_GET_NTH_BYTE:
			/*
			 * This ioctl is both input (ioctl_param) and output (the return value of this function)
			 */
			return Message[ioctl_param];
			break;
	}

	return SUCCESS;
}

/* Module Declarations */

/*
 * This structure will hold the functions to be called when a process does something to the device we created.
 * Since a pointer to this structure is kept in the devices table, it can't be local to init_module. NULL is for unimplemented functions.
 */

struct file_operations Fops = 
{
	read : device_read,
	write : device_write,
	unlocked_ioctl : device_ioctl,
	open : device_open,
	release : device_release
};

/*
 * Initialize the module : Register the character device.
 */
int init_module()
{
	int ret_val;
	/*
	 * Register the character device (atleast try)
	 */
	ret_val = register_chrdev(MAJOR_NUM, DEVICE_NAME, &Fops);

	/*
	 * Negative values signify an error.
	 */
	if (ret_val < 0)
	{
		printk(KERN_ALERT "%s failed with %d\n", "Sorry, registering the character device ", ret_val);
		return ret_val;
	}

	printk(KERN_INFO "%s The major device number is %d.\n", "Registeration is a success", MAJOR_NUM);
	printk(KERN_INFO "If you want to talk to the device driver,\n");
	printk(KERN_INFO "you'll have to create a device file. \n");
	printk(KERN_INFO "We suggest you use:\n");
	printk(KERN_INFO "mknod %s c %d 0\n", DEVICE_FILE_NAME, MAJOR_NUM);
	printk(KERN_INFO "The device file name is important, because\n");
	printk(KERN_INFO "the ioctl program assumes that's the\n");
	printk(KERN_INFO "file you'll use.\n");

	return 0;
}

/*
 * Cleanup : unregister the appropriate file from /proc
 */
void cleanup_module()
{
	//int ret;

	/*
	 * Unregister the device
	 */
	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);

	/*
	 * If there's an error, report it
	 */
}
