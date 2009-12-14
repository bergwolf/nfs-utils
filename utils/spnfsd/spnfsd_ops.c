/******************************************************************************

(c) 2007 Network Appliance, Inc.  All Rights Reserved.

Network Appliance provides this source code under the GPL v2 License.
The GPL v2 license is available at
http://opensource.org/licenses/gpl-license.php.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************/

#include "nfsd4_spnfs.h"
#include "spnfsd.h"
#include "nfs/nfs.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


extern int stripesize;
extern int densestriping;
extern int num_ds;
extern int num_dev;
extern char dsmountdir[];
extern struct dserver dataservers[SPNFS_MAX_DATA_SERVERS];
size_t  strlcat(char *, const char *, size_t);
size_t  strlcpy(char *, const char *, size_t);

int
spnfsd_layoutget(struct spnfs_msg *im)
{
	int ds;
	int rc;
	char fullpath[1024]; /* MSXXX */

	im->im_status = SPNFS_STATUS_SUCCESS;
	im->im_res.layoutget_res.status = 0;
	im->im_res.layoutget_res.devid = 1; /* XXX */
	im->im_res.layoutget_res.stripe_size = stripesize;
	if (densestriping)
		im->im_res.layoutget_res.stripe_type = 1; /* DMXXX enum */
	else
		im->im_res.layoutget_res.stripe_type = 0; /* DMXXX ^^^^ */
	im->im_res.layoutget_res.stripe_count = num_ds;

	for (ds = 0 ; ds < num_ds ; ds++) {
		memset(im->im_res.layoutget_res.flist[ds].fh_val, 0, 128); /*DMXXX*/
		sprintf(fullpath, "%s/%s/%ld",
			dsmountdir, dataservers[ds].ds_ip,
		        im->im_args.layoutget_args.inode);
		rc = spnfsd_getfh(fullpath,
				  im->im_res.layoutget_res.flist[ds].fh_val,
				  &im->im_res.layoutget_res.flist[ds].fh_len);
		if (rc < 0) {
			/* MSXXX needs error msg/handling */
			im->im_res.layoutget_res.status = ENOENT;
			return -1;
		}

		/*
		 * MSXXX another hack...fix the hardcoding.
		 * The fh's fsid_type is incremented by 8 to get
		 * around stateid checking.
		 */
		im->im_res.layoutget_res.flist[ds].fh_val[2] += 8;
	}

	return 0;
}

int
spnfsd_layoutcommit(struct spnfs_msg *im)
{
	char basename[1024]; /* DMXXX */
	char fullpath[1024]; /* DMXXX */
	int ds;

	im->im_status = SPNFS_STATUS_SUCCESS;
	im->im_res.layoutcommit_res.status = 0;
	sprintf(basename, "%ld", im->im_args.layoutcommit_args.inode);

	for (ds = 0 ; ds < num_ds ; ds++) {
		sprintf(fullpath, "%s/%s/%s", dsmountdir,
			dataservers[ds].ds_ip, basename);
		truncate(fullpath, im->im_args.layoutcommit_args.file_size);
	}

	return 0;
}

int
spnfsd_layoutreturn(struct spnfs_msg *im)
{
	return 0;
}

int
spnfsd_getdeviceiter(struct spnfs_msg *im)
{
	im->im_status = SPNFS_STATUS_SUCCESS;
	im->im_res.getdeviceiter_res.status = 0;

	/* verifier ignored for now */
	if (im->im_args.getdeviceiter_args.cookie >= num_dev)
		im->im_res.getdeviceiter_res.eof = 1;
	else {
		/* XXX just hardcoded for now...fix this */
		im->im_res.getdeviceiter_res.devid = 1;
		im->im_res.getdeviceiter_res.cookie = im->im_args.getdeviceiter_args.cookie + 1;
		im->im_res.getdeviceiter_res.verf = 0;
		im->im_res.getdeviceiter_res.eof = 0;
	}

	return 0;
}

int
spnfsd_getdeviceinfo(struct spnfs_msg *im)
{
	struct spnfs_device *devp;
	struct spnfs_data_server *dsp;
	u_int32_t devid;
	int ds;

	im->im_status = SPNFS_STATUS_SUCCESS;
	im->im_res.getdeviceinfo_res.status = 0;

	devid = im->im_args.getdeviceinfo_args.devid;

	/* XXX fix this if/when we support multiple devices */
	if (devid != 1) {
		im->im_res.getdeviceinfo_res.status = -ENOENT;
		return -1;
	}

	devp = &im->im_res.getdeviceinfo_res.devinfo;
	devp->dscount = num_ds;

	for (ds = 0 ; ds < num_ds ; ds++) {
		dsp = &devp->dslist[ds];
		dsp->dsid = dataservers[ds].ds_id;
		memset(dsp->netid, 0, 5);
		strlcpy(dsp->netid, "tcp", 4);
		sprintf(dsp->addr, "%s.%d.%d",
			dataservers[ds].ds_ip,
			dataservers[ds].ds_port >> 8,
			dataservers[ds].ds_port & 0xff);
	}

	return 0;
}

int
spnfsd_setattr(struct spnfs_msg *im)
{
	char basename[1024]; /* DMXXX */
	char fullpath[1024]; /* DMXXX */
	int ds;

	im->im_status = SPNFS_STATUS_SUCCESS;
	im->im_res.setattr_res.status = 0;
	sprintf(basename, "%ld", im->im_args.setattr_args.inode);

	for (ds = 0 ; ds < num_ds ; ds++) {
		sprintf(fullpath, "%s/%s/%s", dsmountdir,
			dataservers[ds].ds_ip, basename);
		truncate(fullpath, im->im_args.setattr_args.file_size);
	}

	return 0;
}

int
spnfsd_open(struct spnfs_msg *im)
{
	char basename[1024]; /* DMXXX */
	char fullpath[1024]; /* DMXXX */
	char dirpath[1024]; /* DMXXX */
	int ds;
	int fd, status;
	struct stat buf;

	im->im_status = SPNFS_STATUS_SUCCESS;
	im->im_res.open_res.status = 0;
	sprintf(basename, "%ld", im->im_args.open_args.inode);

	for (ds = 0 ; ds < num_ds ; ds++) {
		sprintf(dirpath, "%s/%s", dsmountdir,
			dataservers[ds].ds_ip);
		sprintf(fullpath, "%s/%s", dirpath, basename);
		status = stat(dirpath, &buf);
		im->im_res.open_res.status = errno;
		if (status != 0) {
			perror(dirpath);
			break;
		}
		if ((fd = open(fullpath, O_WRONLY|O_CREAT, 0777)) < 0) {
			perror(fullpath);
			im->im_res.open_res.status = errno;
			break;
		} else {
			if (fchmod(fd, 0777) != 0) {
				/* DM: we'll consider this non-fatal for now */
				perror("chmod stripe");
			}
			im->im_res.open_res.status = 0;
			close(fd);
		}
	}

	return im->im_res.open_res.status;
}

int
spnfsd_close(struct spnfs_msg *im)
{
	im->im_status = SPNFS_STATUS_SUCCESS;
	printf("spnfsd_close received: %d\n", im->im_args.close_args.x);
	im->im_res.close_res.y = 7331;
	return 0;
}

int
spnfsd_create(struct spnfs_msg *im)
{
	return 0;
}

int
spnfsd_remove(struct spnfs_msg *im)
{
	char basename[1024]; /* DMXXX */
	char fullpath[1024]; /* DMXXX */
	int ds;

	im->im_status = SPNFS_STATUS_SUCCESS;
	im->im_res.remove_res.status = 0;
	sprintf(basename, "%ld", im->im_args.remove_args.inode);

	for (ds = 0 ; ds < num_ds ; ds++) {
		sprintf(fullpath, "%s/%s/%s", dsmountdir,
			dataservers[ds].ds_ip, basename);
		unlink(fullpath);
	}

	return 0;
}

int
spnfsd_commit(struct spnfs_msg *im)
{
	return 0;
}

int
spnfsd_getfh(char *path, unsigned char *fh_val, unsigned int *fh_len)
{
	int fd, err;
	struct nfsctl_arg arg;
	unsigned char res[130]; /* XXX align this to proper structure */
	extern int nfsservctl();

	if ((fd = open(path, O_RDONLY)) < 0) {
		perror(path);
		return -1;
	}

	arg.ca_fd2fh.fd = fd;
	if ((err = nfsservctl(NFSCTL_FD2FH, &arg, res)) < 0) {
		close(fd);
		return -1;
	}

	/* XXX use proper structure */
	*fh_len = (short)res[0];
	memcpy(fh_val, &res[2], *fh_len);

	close(fd);
	return 0;
}
