
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pwd.h>
#include <grp.h>
#include <keyutils.h>
#include <nfsidmap.h>

#include <unistd.h>
#include "xlog.h"

int verbose = 0;
char *usage="Usage: %s [-v] [-t timeout] key desc";

#define MAX_ID_LEN   11
#define IDMAP_NAMESZ 128
#define USER  1
#define GROUP 0

/*
 * Find either a user or group id based on the name@domain string
 */
int id_lookup(char *name_at_domain, key_serial_t key, int type)
{
	char id[MAX_ID_LEN];
	uid_t uid = 0;
	gid_t gid = 0;
	int rc;

	if (type == USER) {
		rc = nfs4_owner_to_uid(name_at_domain, &uid);
		sprintf(id, "%u", uid);
	} else {
		rc = nfs4_group_owner_to_gid(name_at_domain, &gid);
		sprintf(id, "%u", gid);
	}
	if (rc < 0)
		xlog_err("id_lookup: %s: failed: %m",
			(type == USER ? "nfs4_owner_to_uid" : "nfs4_group_owner_to_gid"));

	if (rc == 0) {
		rc = keyctl_instantiate(key, id, strlen(id) + 1, 0);
		if (rc < 0)
			xlog_err("id_lookup: keyctl_instantiate failed: %m");
	}

	return rc;
}

/*
 * Find the name@domain string from either a user or group id
 */
int name_lookup(char *id, key_serial_t key, int type)
{
	char name[IDMAP_NAMESZ];
	char domain[NFS4_MAX_DOMAIN_LEN];
	uid_t uid;
	gid_t gid;
	int rc;

	rc = nfs4_get_default_domain(NULL, domain, NFS4_MAX_DOMAIN_LEN);
	if (rc != 0) {
		rc = -1;
		xlog_err("name_lookup: nfs4_get_default_domain failed: %m");
		goto out;
	}

	if (type == USER) {
		uid = atoi(id);
		rc = nfs4_uid_to_name(uid, domain, name, IDMAP_NAMESZ);
	} else {
		gid = atoi(id);
		rc = nfs4_gid_to_name(gid, domain, name, IDMAP_NAMESZ);
	}
	if (rc < 0)
		xlog_err("name_lookup: %s: failed: %m",
			(type == USER ? "nfs4_uid_to_name" : "nfs4_gid_to_name"));

	if (rc == 0) {
		rc = keyctl_instantiate(key, &name, strlen(name), 0);
		if (rc < 0)
			xlog_err("name_lookup: keyctl_instantiate failed: %m");
	}
out:
	return rc;
}

int main(int argc, char **argv)
{
	char *arg;
	char *value;
	char *type;
	int rc = 1, opt;
	int timeout = 600;
	key_serial_t key;
	char *progname;

	/* Set the basename */
	if ((progname = strrchr(argv[0], '/')) != NULL)
		progname++;
	else
		progname = argv[0];

	xlog_open(progname);
	xlog_syslog(1);
	xlog_stderr(0);

	while ((opt = getopt(argc, argv, "t:v")) != -1) {
		switch (opt) {
		case 'v':
			verbose++;
			break;
		case 't':
			timeout = atoi(optarg);
			break;
		default:
			xlog_warn(usage, progname);
			break;
		}
	}

	if ((argc - optind) != 2) {
		xlog_err("Bad arg count. Check /etc/request-key.conf");
		xlog_warn(usage, progname);
		return 1;
	}

	if (verbose)
		nfs4_set_debug(verbose, NULL);

	key = strtol(argv[optind++], NULL, 10);

	arg = strdup(argv[optind]);
	if (arg == NULL) {
		xlog_err("strdup failed: %m");
		return 1;
	}
	type = strtok(arg, ":");
	value = strtok(NULL, ":");

	if (verbose) {
		xlog_warn("key: %ld type: %s value: %s timeout %ld",
			key, type, value, timeout);
	}

	if (strcmp(type, "uid") == 0)
		rc = id_lookup(value, key, USER);
	else if (strcmp(type, "gid") == 0)
		rc = id_lookup(value, key, GROUP);
	else if (strcmp(type, "user") == 0)
		rc = name_lookup(value, key, USER);
	else if (strcmp(type, "group") == 0)
		rc = name_lookup(value, key, GROUP);

	/* Set timeout to 10 (600 seconds) minutes */
	if (rc == 0)
		keyctl_set_timeout(key, timeout);

	free(arg);
	return rc;
}
