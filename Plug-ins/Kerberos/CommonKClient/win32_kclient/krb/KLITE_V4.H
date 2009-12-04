int	CALLBACK krb_get_pw_in_tkt (LPSTR, LPSTR, LPSTR, LPSTR, LPSTR, int, LPSTR) ;
int	CALLBACK krb_get_cred (LPSTR, LPSTR, LPSTR, LPCREDENTIALS) ;
int	CALLBACK krb_get_lrealm (LPSTR, int) ;
BOOL	CALLBACK k_isname (LPSTR) ;
BOOL	CALLBACK k_isrealm (LPSTR) ;
LPSTR CALLBACK get_krb_err_txt_entry (int) ;
LONG	CALLBACK lsh_klist (HANDLE, TICKETINFO FAR *) ;
int	CALLBACK krb_mk_req (KTEXT, LPSTR, LPSTR, LPSTR, long) ;
void	CALLBACK set_krb_debug (BOOL)  ;
void	CALLBACK set_krb_ap_req_debug (BOOL)  ;
int	CALLBACK kname_parse (LPSTR, LPSTR, LPSTR, LPSTR) ;
int	CALLBACK dest_tkt (void) ;


// NOTE: the following function IS NOT IMPLEMENTED in KLite.  It always
// returns an error code.  However, it is needed so that KClient can call it.
LONG CALLBACK lsh_changepwd (LPSTR, LPSTR, LPSTR, LPSTR) ;
