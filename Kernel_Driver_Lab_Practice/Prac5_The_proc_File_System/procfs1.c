/*
 * procfs1.c : create a "file" in /proc
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>

#define procfs_name "helloworld"

/*
 * This structure hold information about the /proc file
 */

struct proc_dir_entry;

struct proc_dir_entry* Our_Proc_File;

/*
 * Put data into the proc fs file.
 *
 * Arguments
 * ============================
 * 1. The buffer where the data is to be inserted, if we decide to use it.
 * 2. A pointer to a pointer to characters. This is useful if we don't want to use the buffer allocated by the kernel.
 * 3. The current position in the file
 * 4. The size o the buffer in the first argument.
 * 5. Write a "1" here to indicate EOF.
 * 6. A pointer to data (useful in case one common read for multiple /proc/... entries).
 *
 * Usage and Return Value
 * ============================
 * A return value of zero means we have no further information at this time (end of file).
 * A negative return value is an error condition.
 *
 * For More Information
 * ============================
 * The way I discovered what to do with this function wasn't by reading documentation, but by reading the code which used it.
 * I just looked to see what uses the get_info field of proc_dir_entry struct (I used a combination of find and grep, if you're interested), and I saw that it is used in
 * <kernel source directory>/fs/proc/array.c
 *
 * If something is unknown about the kernel, this is usually the way to go.
 * In Linux we have the great adbantage of having the kernel source code for free - use it!
 */

static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);


static struct file_operations fops = 
{
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};


int procfile_read(char* buffer, char** buffer_location, off_t offset, int buffer_length, int *eof, void* data)
{
	int ret;

	printk(KERN_INFO "procfile_read (/proc/%s) called\n", procfs_name);

	/*
	 * We give all of our information in one go, so if the user asks us
	 * if have more information the answer should always be no.
	 * This is important because the standard read function from the library
	 * would continue to issus the read system call until the kernel replies that
	 * it has no more information, or until its buffer is filled.
	 */

	if (offset > 0)
	{
		/* We have finished to read, return 0 */
		ret = 0;
	}
	else
	{
		/* fill the buffer, return the buffer size. */
		ret = sprintf(buffer, "HelloWorld!\n");
	}

	return ret;
}

int init_module()
{
	Our_Proc_File = proc_create(procfs_name, 0644, NULL, &fops);
	/*
	 * Note on proc_create
	 * =====================
	 * This function does not exist on Linux Kernel version 3.13.x but does exists on version 2.6.x
	 * So to compile this module in 3.13.x or higher version of linux kernel,
	 * document's "create_proc_entry" should be alternated with "proc_create",
	 * which requires one more parameter file_operations pointer.
	 * That's why I declared that structure ahead of this part and used it here.
	 */

	if (Our_Proc_File == NULL)
	{
		remove_proc_entry(procfs_name, NULL);
		/*
		 * Note on remove_proc_entry
		 * ==========================
		 * For current Linux Kernel version 3.13.x or higher, 
		 * we should pass NULL for root of proc file system.(This is the 2nd parameter.)
		 */
		printk(KERN_ALERT "Error : Could not initialize /proc/%s\n", procfs_name);
		return -ENOMEM;
	}

	Our_Proc_File->read_proc = procfile_read;
	Our_Proc_File->owner = THIS_MODULE;
	Our_Proc_File->mode = S_IFREG | S_IRUGO;
	Our_Proc_File->uid = 0;
	Our_Proc_File->gid = 0;
	Our_Proc_File->size = 37;

	printk(KERN_INFO "/proc/%s created\n", procfs_name);
	return 0;	/* Everything is OK. */
}

void cleanup_module()
{
	remove_proc_entry(procfs_name, NULL);
	printk(KERN_INFO "/proc/%s removed\n", procfs_name);
}
