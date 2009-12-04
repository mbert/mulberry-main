
/* 
Use this instead of krblayer.h.  The lsh_changepwd() funtion
needs LPSTR instead of char * in the prototype.  
*/

LPSTR krb_err_func(int offset, long code);

#define NO_TICKETS 0
#define EXPD_TICKETS 2
#define GOOD_TICKETS 1
#define KRBERR(code) (code + krb_err_base)

// typedef struct {
//	  char	  principal[MAX_K_NAME_SZ];	  /* Principal name/instance/realm */
//	  int	  btickets;					  /* Do we have tickets? */
//    long    lifetime; 				  /* Lifetime -- needs to have
//									     room for 255 5-minute
//									     periods * 5 * 60 */
//    long    issue_date;				  /* The issue time */
//
// } TICKETINFO;

extern TICKETINFO ticketinfo;
extern int oldtickets;

long FAR PASCAL lsh_checkpwd(char *principal, char *password);

// sse: long FAR PASCAL lsh_changepwd(char *principal, char *password, char *newpassword, LPSTR kadm_info);
long FAR PASCAL lsh_changepwd(LPSTR principal, LPSTR password, LPSTR newpassword, LPSTR kadm_info);

long FAR PASCAL lsh_kinit(char *principal, char *password, int lifetime);
long FAR PASCAL lsh_klist(HWND hlist, TICKETINFO FAR *ticketinfo);
long FAR PASCAL lsh_kdestroy(void);
long lsh_time(void);
long lsh_getprincipal(char *principal);
