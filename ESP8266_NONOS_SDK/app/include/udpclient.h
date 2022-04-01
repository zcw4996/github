/*
 * udpclient.h
 *
 *  Created on: 2017年2月10日
 *      Author: dell
 */

#ifndef APP_INCLUDE_UDPCLIENT_H_
#define APP_INCLUDE_UDPCLIENT_H_

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"

#define HTML_RETURN_OK "<!DOCTYPE html>"\
"<html lang=\"en\">"\
"<head>"\
    "<meta charset=\"UTF-8\">"\
    "<style type=\"text/css\">"\
        ".class1{"\
           " font-size: 25px;"\
            "text-align: center;"\
        "}"\
        ".class2{"\
            "font-size: 18px;"\
            "text-align: center;"\
        "}"\
    "</style>"\
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, minimum-scale=0.5, maximum-scale=2.0, user-scalable=yes\" />"\
"</head>"\
"<body style=\"background-color: grey\">"\
"<p class=\"class1\">%s</p>"\
"<br><br><br>"\
"<h2 align=\"center\">%s</h2>"\
"<form method=\"post\">"\
    "<table align=\"center\">"\
        "<tr>"\
            "<td></td>"\
        "</tr>"\
        "<br><br>"\
        "<tr>"\
           "<td> <input  type=\"submit\" name=\"Successful_setup\" value=\"Return\"></td>"\
        "</tr>"\
    "</table>"\
"</form>"\
"<br><br><br>"\
"</body>"\
"</html>"


void ICACHE_FLASH_ATTR AP_udpclient_init(void);
LOCAL void ICACHE_FLASH_ATTR udpcilent_recv_cb(void *arg, char *pusrdata, unsigned short length);
void ICACHE_FLASH_ATTR client_send(void *arg);
uint32_t get_current_stamp(void);
void set_sntp_server_ip(ip_addr_t *ipaddr);
#endif /* APP_INCLUDE_UDPCLIENT_H_ */
