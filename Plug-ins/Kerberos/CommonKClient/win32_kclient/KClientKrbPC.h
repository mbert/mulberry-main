int xxx_get_ticket_for_service(
	char *serviceName,
	char *buf,
	unsigned KRB_INT32 *buflen,
	int checksum,
	des_cblock sessionKey,
	Key_schedule schedule,
	char *version,
	int includeVersion);

// krb.h doesnt define this for windows, does for dos and os2... go figure
// ??? gross hack here
long krb_mk_priv( u_char *, u_char *, u_long, Key_schedule, C_Block,
                  struct sockaddr_in *, struct sockaddr_in *);