/*
 * spnfsd.h
 *
 * spNFS - simple pNFS implementation with userspace daemon
 *
 */

#ifndef _PATH_SPNFSDCONF
#define _PATH_SPNFSDCONF "/etc/spnfsd.conf"
#endif

#ifndef DEFAULT_STRIPE_SIZE
#define DEFAULT_STRIPE_SIZE 4096
#endif

#ifndef DEFAULT_DS_PORT
#define DEFAULT_DS_PORT 2049
#endif

struct dserver {
        char    *ds_ip;
        int     ds_port;
        char    *ds_path;
	int	ds_devid;
};

/* DMXXX future struct for whole config */
struct spnfsd_config {
	int		verbose;
	int		stripesize;
	int		densestriping;
	int		num_ds;
	struct dserver	dataservers[SPNFS_MAX_DATA_SERVERS];
};

int spnfsd_layoutget(struct spnfs_msg *);
int spnfsd_layoutcommit(struct spnfs_msg *);
int spnfsd_layoutreturn(struct spnfs_msg *);
int spnfsd_getdevicelist(struct spnfs_msg *);
int spnfsd_getdeviceinfo(struct spnfs_msg *);
int spnfsd_setattr(struct spnfs_msg *);
int spnfsd_open(struct spnfs_msg *);
int spnfsd_close(struct spnfs_msg *);
int spnfsd_create(struct spnfs_msg *);
int spnfsd_remove(struct spnfs_msg *);
int spnfsd_commit(struct spnfs_msg *);
int spnfsd_getfh(char *, unsigned char *, unsigned int *);
