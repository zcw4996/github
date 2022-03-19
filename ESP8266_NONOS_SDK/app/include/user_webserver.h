#ifndef __USER_WEBSERVER_H__
#define __USER_WEBSERVER_H__

#define SERVER_PORT 80
#define SERVER_SSL_PORT 443

#define URLSize 10

typedef enum ProtocolType {
    GET = 0,
    POST,
} ProtocolType;

typedef enum _ParmType {
    REBOOT=0,
    RESTORE,
	SOFTAP
} ParmType;

typedef struct URL_Frame {
    enum ProtocolType Type;
    char pSelect[URLSize];
    char pCommand[URLSize];
    char pFilename[URLSize];
} URL_Frame;

typedef struct _rst_parm {
    ParmType parmtype;
    struct espconn *pespconn;
} rst_parm;

//#define GLOBAL_DEBUG
 
#if defined(GLOBAL_DEBUG)
#define WEBSERVER_DEBUG(format, ...) os_printf("[%s:%d]" format "\r\n", __FILE__,__LINE__,##__VA_ARGS__)
#else
#define WEBSERVER_DEBUG(format, ...) 
#endif

void user_webserver_init(uint32 port);
void user_webserver_scan_start(void);

#endif
