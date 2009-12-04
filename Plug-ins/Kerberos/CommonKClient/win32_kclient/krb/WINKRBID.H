#define DES_PASSWD                  100
#define DES_PASSWD_QRY              WM_USER + 1
#define DES_PASSWD_IDC              WM_USER + 2

#define DEFTEXTSIZE 144

extern int FAR PASCAL PasswordCB( HWND, unsigned, WORD, LONG );
extern HANDLE FAR PASCAL hQKrbInst( void );
#define DES_PASSWD_IDC              102

/* This is for the Windows version.
 * These IDs are used to locate members of a string table which can be
 * used to determine if the DLL is for:
 * LWP or WINSOCK
 * DES functions exported or not
 * DEBUG information contained or not
 *
 */

#define IDS_DES   666
#define IDS_DEBUG 667
#define IDS_TCP   668
#define IDS_TZ    669
#define IDS_KRB_CONF  670
#define IDS_KRB_RLM_TRANS 671
