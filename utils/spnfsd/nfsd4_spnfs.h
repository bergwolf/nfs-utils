/*
 * include/linux/nfsd4_spnfs.h
 *
 * spNFS - simple pNFS implementation with userspace daemon
 *
 */


#ifndef NFS_SPNFS_H
#define NFS_SPNFS_H


#ifdef __KERNEL__
#include "sunrpc/svc.h"
#include "nfsd/nfsfh.h"
#else
#include <sys/types.h>
#endif /* __KERNEL__ */

#define SPNFS_STATUS_INVALIDMSG		0x01
#define SPNFS_STATUS_AGAIN		0x02
#define SPNFS_STATUS_FAIL		0x04
#define SPNFS_STATUS_SUCCESS		0x08

#define SPNFS_TYPE_LAYOUTGET		0x01
#define SPNFS_TYPE_LAYOUTCOMMIT		0x02
#define SPNFS_TYPE_LAYOUTRETURN		0x03
#define SPNFS_TYPE_GETDEVICELIST	0x04
#define SPNFS_TYPE_GETDEVICEINFO	0x05
#define SPNFS_TYPE_SETATTR		0x06
#define SPNFS_TYPE_OPEN			0x07
#define	SPNFS_TYPE_CLOSE		0x08
#define SPNFS_TYPE_CREATE		0x09
#define SPNFS_TYPE_REMOVE		0x0a
#define	SPNFS_TYPE_COMMIT		0x0b

#define SPNFS_MAX_DATA_SERVERS		2
#define SPNFS_MAX_LAYOUT		8

/* layout */
struct spnfs_msg_layoutget_args {
	unsigned long inode;
};

struct spnfs_filelayout_list {
	u_int32_t	dev_id;
	u_int32_t	dev_index;
	u_int32_t	fh_len;
	unsigned char	fh_val[128]; /* DMXXX fix this const */
};

struct spnfs_msg_layoutget_res {
	int status;
	u_int64_t stripe_size;
	u_int32_t stripe_type;
	u_int32_t layout_count;
	struct spnfs_filelayout_list flist[SPNFS_MAX_LAYOUT];
};

/* layoutcommit */
struct spnfs_msg_layoutcommit_args {
	unsigned long inode;
	u_int64_t file_size;
};

struct spnfs_msg_layoutcommit_res {
	int status;
};

/* layoutreturn */
/* No op for the daemon */
/*
struct spnfs_msg_layoutreturn_args {
};

struct spnfs_msg_layoutreturn_res {
};
*/

/* getdevicelist */
struct spnfs_msg_getdevicelist_args {
	unsigned long inode;
};

struct spnfs_getdevicelist_dev {
	u_int32_t devid;
	char netid[5];
	char addr[29];
};

struct spnfs_msg_getdevicelist_res {
	int status;
	int count;
	struct spnfs_getdevicelist_dev dlist[SPNFS_MAX_DATA_SERVERS];
};

/* getdeviceinfo */
struct spnfs_msg_getdeviceinfo_args {
	u_int32_t devid;
};

struct spnfs_msg_getdeviceinfo_res {
	int status;
	struct spnfs_getdevicelist_dev dinfo;
};

/* setattr */
struct spnfs_msg_setattr_args {
	unsigned long inode;
	int file_size;
};

struct spnfs_msg_setattr_res {
	int status;
};

/* open */
struct spnfs_msg_open_args {
	unsigned long inode;
	int create;
	int createmode;
	int truncate;
};

struct spnfs_msg_open_res {
	int status;
};

/* close */
/* No op for daemon */
struct spnfs_msg_close_args {
	int x;
};

struct spnfs_msg_close_res {
	int y;
};

/* create */
/*
struct spnfs_msg_create_args {
	int x;
};

struct spnfs_msg_create_res {
	int y;
};
*/

/* remove */
struct spnfs_msg_remove_args {
	unsigned long inode;
};

struct spnfs_msg_remove_res {
	int status;
};

/* commit */
/*
struct spnfs_msg_commit_args {
	int x;
};

struct spnfs_msg_commit_res {
	int y;
};
*/

/* bundle args and responses */
union spnfs_msg_args {
	struct spnfs_msg_layoutget_args		layoutget_args;
	struct spnfs_msg_layoutcommit_args	layoutcommit_args;
//	struct spnfs_msg_layoutreturn_args	layoutreturn_args;
	struct spnfs_msg_getdevicelist_args	getdevicelist_args;
	struct spnfs_msg_getdeviceinfo_args	getdeviceinfo_args;
	struct spnfs_msg_setattr_args		setattr_args;
	struct spnfs_msg_open_args		open_args;
	struct spnfs_msg_close_args		close_args;
//	struct spnfs_msg_create_args		create_args;
	struct spnfs_msg_remove_args		remove_args;
//	struct spnfs_msg_commit_args		commit_args;
};

union spnfs_msg_res {
	struct spnfs_msg_layoutget_res		layoutget_res;
	struct spnfs_msg_layoutcommit_res	layoutcommit_res;
//	struct spnfs_msg_layoutreturn_res	layoutreturn_res;
	struct spnfs_msg_getdevicelist_res	getdevicelist_res;
	struct spnfs_msg_getdeviceinfo_res	getdeviceinfo_res;
	struct spnfs_msg_setattr_res		setattr_res;
	struct spnfs_msg_open_res		open_res;
	struct spnfs_msg_close_res		close_res;
//	struct spnfs_msg_create_res		create_res;
	struct spnfs_msg_remove_res		remove_res;
//	struct spnfs_msg_commit_res		commit_res;
};

/* a spnfs message, args and response */
struct spnfs_msg {
	unsigned char		im_type;
	unsigned char		im_status;
	union spnfs_msg_args	im_args;
	union spnfs_msg_res	im_res;
};

#ifdef __KERNEL__

/* pipe mgmt structure.  messages flow through here */
struct spnfs {
	char			spnfs_path[48];   /* path to pipe */
	struct dentry		*spnfs_dentry;    /* dentry for pipe */
	wait_queue_head_t	spnfs_wq;
	struct spnfs_msg	spnfs_im;         /* spnfs message */
	struct mutex		spnfs_lock;       /* Serializes upcalls */
	struct mutex		spnfs_plock;
};

int spnfs_layout_type(void);
int spnfs_layoutget(struct inode *, void *);
int spnfs_layoutcomit(void);
int spnfs_layoutreturn(void);
int spnfs_getdevicelist(struct super_block *, void *);
int spnfs_getdeviceinfo(void);
int spnfs_setattr(void);
int spnfs_open(struct inode *, void *);

int nfsd_spnfs_new(void);
void nfsd_spnfs_delete(void);
int spnfs_upcall(struct spnfs *, struct spnfs_msg *, union spnfs_msg_res *);

#endif /* __KERNEL__ */

#endif /* NFS_SPNFS_H */
