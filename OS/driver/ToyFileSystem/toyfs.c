#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/time.h>
#include <linux/timekeeping32.h>
#include <linux/version.h>

#include "toyfs.h"

#define CURRENT_TIME        (current_kernel_time())

MODULE_LICENSE("Dual BSD/GPL");

struct file_blk block[MAX_FILES + 1];
int curr_count = 0;

static int get_block(void)
{
	int i;
	
	for (i = 2; i < MAX_FILES; ++i)
	{
		if (!block[i].busy)
		{
			block[i].busy = 1;
			return i;
		}
	}

	return -1;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0))
static int toyfs_readdir(struct file *filp, void *dirent, filldir_t filldir)
{
	loff_t pos;
	struct file_blk *blk;
	struct dir_entry *entry;
	int i;
	
	pos = filp->f_pos;
	if (pos)
		return 0;

	blk = (struct file_blk *)filp->f_path.dentry->d_inode->i_private;

	if (!S_ISDIR(blk->mode))
	{
		return -ENOTDIR;
	}

	entry = (struct dir_entry *)&blk->data[0];
	
	for (i = 0; i < blk->dir_children; ++i)
	{
		filldir(dirent, entry[i].filename, MAXLEN, pos, entry[i].idx, DT_UNKNOWN);
		filp->f_pos += sizeof(struct dir_entry);
		pos += sizeof(struct dir_entry);
	}

	return 0;
}
#else
static int toyfs_readdir(struct file *file, struct dir_context *ctx)
{
	struct file_blk *blk;
	struct dir_entry *entry;
	int i;

	if (ctx->pos != 0) 
	{
		return 0;
	}

    if (!dir_emit_dots(file, ctx))
	{
        return 0;
	}

	if (ctx->pos != 2) 
	{
		return 0;
	}
	
	blk = (struct file_blk *)file->f_path.dentry->d_inode->i_private;

	if (!S_ISDIR(blk->mode))
	{
		return -ENOTDIR;
	}

	entry = (struct dir_entry *)&blk->data[0];
	
	for (i = 0; i < blk->dir_children; ++i)
	{
		dir_emit(ctx, entry[i].filename, MAXLEN, entry[i].idx, DT_UNKNOWN);
		ctx->pos++;
	}

	return 0;

}

#endif

ssize_t toyfs_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos)
{
	struct file_blk *blk;
	char *buffer;
	
	blk = (struct file_blk *)filp->f_path.dentry->d_inode->i_private;

	if (*ppos >= blk->file_size)
		return 0;

	buffer = (char *)&blk->data[0];
	len = min((size_t) blk->file_size, len);

	if (copy_to_user(buf, buffer, len))
	{
		return -EFAULT;
	}

	*ppos += len;

	return len;
}

ssize_t toyfs_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos)
{
	struct file_blk *blk;
	char *buffer;
	
	blk = (struct file_blk *)filp->f_path.dentry->d_inode->i_private;

	buffer = (char *)&blk->data[0];
	buffer += *ppos;

	if (copy_from_user(buffer, buf, len))
	{
		return -EFAULT;
	}

	*ppos += len;
	blk->file_size = *ppos;

	return len;
}

const struct file_operations toyfs_file_operations =
{
	.read = toyfs_read,
	.write = toyfs_write,
};

const struct file_operations toyfs_dir_operations =
{
	.owner = THIS_MODULE,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0))
	.readdir = toyfs_readdir,
#else
	.iterate = toyfs_readdir,
#endif
};

static struct inode_operations toyfs_inode_ops; 

static int toyfs_do_create(struct inode *dir, struct dentry *dentry, umode_t mode)
{
	struct inode *inode;
	struct super_block *sb;
	struct dir_entry *entry;
	struct file_blk *blk, *pblk;
	int idx;
	
	sb = dir->i_sb;
	
	if (curr_count >= MAX_FILES)
	{
		return -ENOSPC;
	}

	if (!S_ISDIR(mode) && !S_ISREG(mode))
	{
		return -EINVAL;
	}

	inode = new_inode(sb);

	inode->i_op = &toyfs_inode_ops;
	inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
	
	idx = get_block();
	
	blk = &block[idx];

	inode->i_ino = idx;
	blk->mode = mode;
	curr_count++;

	if (S_ISDIR(mode))
	{
		blk->dir_children = 0;
		inode->i_fop = &toyfs_dir_operations;
	}
	else if (S_ISREG(mode))
	{
		blk->file_size = 0;
		inode->i_fop = &toyfs_file_operations;
	}

	inode->i_private = blk;

	pblk = (struct file_blk *) dir->i_private;

	entry = (struct dir_entry *)&pblk->data[0];

	entry += pblk->dir_children;
	pblk->dir_children++;
	
	entry->idx = idx;
	strcpy(entry->filename, dentry->d_name.name);

	inode_init_owner(inode, dir, mode);
	d_add(dentry, inode);

	return 0;
}

static int toyfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
	return toyfs_do_create(dir, dentry, S_IFDIR | mode);
}

static int toyfs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
	(void) excl;
	return toyfs_do_create(dir, dentry, S_IFDIR | mode);
}

static struct inode *toyfs_iget(struct super_block *sb, int idx)
{
    struct inode *inode;
    struct file_blk *blk;

    inode = new_inode(sb);
    inode->i_ino = idx;
    inode->i_sb = sb;
    inode->i_op = &toyfs_inode_ops;

    blk = &block[idx];

    if (S_ISDIR(blk->mode))
        inode->i_fop = &toyfs_dir_operations;
    else if (S_ISREG(blk->mode))
        inode->i_fop = &toyfs_file_operations;

    inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
    inode->i_private = blk;

    return inode;
}


struct dentry *toyfs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags)
{
    struct super_block *sb = parent_inode->i_sb;
    struct file_blk *blk;
    struct dir_entry *entry;
    int i;

    blk = (struct file_blk *)parent_inode->i_private;
    entry = (struct dir_entry *)&blk->data[0];
    for (i = 0; i < blk->dir_children; i++) 
	{
        if (!strcmp(entry[i].filename, child_dentry->d_name.name)) 
		{
            struct inode *inode = toyfs_iget(sb, entry[i].idx);
            struct file_blk *inner = (struct file_blk *)inode->i_private;
            inode_init_owner(inode, parent_inode, inner->mode);
            d_add(child_dentry, inode);
            return NULL;
        }
    }

    return NULL;
}

int toyfs_rmdir(struct inode *dir, struct dentry *dentry)
{
    struct inode *inode = dentry->d_inode;
    struct file_blk *blk = (struct file_blk *)inode->i_private;

    blk->busy = 0;
    return simple_rmdir(dir, dentry);
}

int toyfs_unlink(struct inode *dir, struct dentry *dentry)
{

    int i;
    struct inode *inode = dentry->d_inode;
    struct file_blk *blk = (struct file_blk *)inode->i_private;
    struct file_blk *pblk = (struct file_blk *)dir->i_private;
    struct dir_entry *entry;

    entry = (struct dir_entry *)&pblk->data[0];

    for (i = 0; i < pblk->dir_children; i++) {
        if (!strcmp(entry[i].filename, dentry->d_name.name)) 
		{
            int j;
            for (j = i; j < pblk->dir_children - 1; j++) 
			{
                memcpy(&entry[j], &entry[j+1], sizeof(struct dir_entry));
            }
            pblk->dir_children --;
            break;
        }
    }

    blk->busy = 0;
    return simple_unlink(dir, dentry);
}

static struct inode_operations toyfs_inode_ops = 
{
	.create = toyfs_create,
	.lookup = toyfs_lookup,
	.mkdir = toyfs_mkdir,
	.rmdir = toyfs_rmdir,
	.unlink = toyfs_unlink,
};

int toyfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct inode *root_inode;
	int mode = S_IFDIR;
	
	root_inode = new_inode(sb);
	root_inode->i_ino = 1;
	inode_init_owner(root_inode, NULL, mode);
	root_inode->i_sb = sb;
	root_inode->i_op = &toyfs_inode_ops;
	root_inode->i_fop = &toyfs_dir_operations;
	root_inode->i_atime = root_inode->i_mtime = root_inode->i_ctime = CURRENT_TIME;
	
	block[1].mode = mode;
	block[1].dir_children = 0;
	block[1].idx = 1;
	block[1].busy = 1 ;
	root_inode->i_private = &block[1];
	
	sb->s_root = d_make_root(root_inode);
	curr_count++;
	
	return 0;
}

static struct dentry *toyfs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data)
{
	return mount_nodev(fs_type, flags, data, toyfs_fill_super);
}

static void toyfs_kill_superblock( struct super_block *sb)
{
	kill_anon_super(sb);
}

struct file_system_type toyfs_fs_type =
{
	.owner = THIS_MODULE,
	.name = "toyfs",
	.mount = toyfs_mount,
	.kill_sb = toyfs_kill_superblock,
};

static int toyfs_init(void)
{
	int ret;
	
	memset(block, 0, sizeof(block));
	ret = register_filesystem(&toyfs_fs_type);
	if (ret)
	{
		printk("register toyfs failed\n");
	}

    return ret;
}

static void toyfs_exit(void)
{
	unregister_filesystem(&toyfs_fs_type); 
}

module_init(toyfs_init);
module_exit(toyfs_exit);

MODULE_LICENSE("GPL");
