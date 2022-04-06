/*
 * tcpserver.h
 *
 *  Created on: 2017年2月8日
 *      Author: dell
 */

#ifndef APP_INCLUDE_TCPSERVER_H_
#define APP_INCLUDE_TCPSERVER_H_

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"

#define SERVER_PORT   80
#define APP_PORT      50000
#define FLASH_READ_SIZE  12000
#define FLASH_READ_SIZE1 924
#define FLASH_READ_SIZE2 950
#define FLASH_READ_SIZE3 1087
#define FLASH_READ_SIZE4 1086

#define LASTDST_WEEK   8
#define IPTYPE      0
#define SUBTYPE     1
#define GATEWAYTYPE 2

#define HTML  "<!DOCTYPE html>"\
"<html lang=\"en\">"\
"<head>"\
    "<meta charset=\"UTF-8\">"\
    "<style type=\"text/css\">"\
	        ".tui-checkbox:checked {"\
           " background:black"\
        "}"\
        ".tui-checkbox {"\
           "width:25px;"\
            "height:25px;"\
            "background-color:#ffffff;"\
            "border:solid 1px #dddddd;"\
            "-webkit-border-radius:50%;"\
           " border-radius:50%;"\
            "font-size:0.8rem;"\
            "margin:0;"\
            "padding:0;"\
            "position:relative;"\
            "display:inline-block;"\
            "vertical-align:top;"\
            "cursor:default;"\
            "-webkit-appearance:none;"\
            "-webkit-user-select:none;"\
            "user-select:none;"\
            "-webkit-transition:background-color ease 0.1s;"\
            "transition:background-color ease 0.1s;"\
        "}"\
        ".class1{"\
            "font-size: 25px;"\
            "text-align: center;"\
        "}"\
        ".class2{"\
            "font-weight: 600;"\
            "font-size: 15;"\
        "}"\
		 ".class3{"\
            "font-size: 10px;"\
            "text-align: right;"\
        "}"\
        "#b{"\
            "margin:0px auto"\
        "}"\
        ".btns {"\
            "width:120px; height:35px; color: black }"\
    "</style>"\
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, minimum-scale=0.5, maximum-scale=2.0, user-scalable=yes\" />"\
"</head>"\
"<body style=\"background-color: grey\">"\
"<p class=\"class1\">GX-NTP Clock Setup</p>"\
"<div id=\"b\">"\
"<form method=\"post\">"\
    "<table  align=\"center\">"\
       "<tr>"\
            "<td class=\"class2\">Router Connection Setting</td>"\
       "</tr>"\
        "<tr >"\
           "<td> AP_Name: </td>"\
           "<td>"\
			"<select name=\"AP_Name\" style=\"width:150px;\">"\
			 "<option selected= \"selected\">%s</option>"\
			"%s"\
            "</select></td>"\
       "</tr>"\
     "<tr >"\
     "<td> AP_Password:</td>"\
         "<td><input type=\"password\" value=\"%s\"  onkeyup=\"value=value.replace(/[\\u4e00-\\u9fa5]/ig,\'\')\" name=\"AP_Password\" id=\"pwd4\" style=\"width:120px;\" maxlength=\"30\"> <input type=\"checkbox\" class=\"tui-checkbox\" onchange=\"changePwd4Visible()\" ></td>"\
        "</tr>"\
        "<tr>"\
            "<td>Dynamic IP:<input type=\"radio\"  value=\"1\"  name=\"DHCP_State\" %s ><br></td>"\
            "<td>Static IP:<input type=\"radio\"  value=\"2\" name=\"DHCP_State\" %s><br></td>"\
        "</tr>"\
        "<tr >"\
            "<td>IP addr:</td>"\
            "<td><input type=\"text\"  value=\"%s\" oninput=\"value=value.replace(/[^\\d.]/g,\'\')\" name=\"IP_addr\"  style=\"width:154px;\" maxlength=\"15\"></td>"\
        "</tr>"\
        "<tr>"\
            "<td>Subnet mask:</td>"\
            "<td><input type=\"text\"  value=\"%s\" oninput=\"value=value.replace(/[^\\d.]/g,\'\')\" name=\"Subnet_mask\" style=\"width:154px;\" maxlength=\"15\"></td>"\
        "</tr>"\
        "<tr>"\
            "<td>Gateway:</td>"\
            "<td><input type=\"text\"  value=\"%s\" oninput=\"value=value.replace(/[^\\d.]/g,\'\')\" name=\"Gateway\" style=\"width:154px;\" maxlength=\"15\"></td>"\
        "</tr>"\
		  "<tr>"\
           "<td>Local TCP Server Port:<br><br></td>"\
          "<td><input type=\"text\" value=\"%s\" name=\"TCP_Server_Port\" style=\"width:154px;\" maxlength=\"5\" oninput = \"value=value.replace(/[^\\d]/g,\'\')\"><br><br></td>"\
        "</tr>"\
        "<tr>"\
           "<td>TCP Server IP:</td>"\
           "<td><input type=\"text\" id=\"name01\" value=\"%s\" name=\"TCP_Server_IP\" style=\"width:120px;\" maxlength=\"20\" oninput=\"value=value.replace(/[\\u4e00-\\u9fa5]/ig,\'\')\" readonly>"\
               "&nbsp;<input type=\"checkbox\" id=\"checkid\" class=\"tui-checkbox \" onclick=\"csan()\"  name=\"kjLevel\" %s/></td>"\
        "</tr>"\
        "<tr>"\
            "<td>TCP Server Port:<br><br></td>"\
            "<td><input type=\"text\"  value=\"%s\" id=\"name02\" name=\"TCP_Server_Port1\" style=\"width:54px;\" maxlength=\"5\" readonly oninput = \"value=value.replace(/[^\\d]/g,\'\')\">"\
                "&nbsp;&nbsp;<select style=\"width:60px;\" name=\"Tcp_Type\">"\
                "<option %s>Short Connection</option>"\
                "<option %s>Long Connection</option>"\
            "</select><br><br></td>"\
       "</tr>"\
        "<tr >"\
            "<td class=\"class2\">NTP Network Setting</td>"\
        "</tr>"\
        "<tr>"\
            "<td>Ntp Server: </td>"\
            "<td><input type=\"text\" value=\"%s\" onkeyup=\"value=value.replace(/[\\u4e00-\\u9fa5]/ig,\'\')\" name=\"Ntp_Serve\" style=\"width:154px;\" maxlength=\"20\" list=\"saleslist\">"\
              "<datalist id=\"saleslist\">"\
                "<option>Auto</option>"\
                "<option>118.24.4.66</option>"\
                "<option>118.24.195.65</option>"\
                "<option>58.220.133.132</option>"\
                "<option>103.18.128.60</option>"\
                "<option>216.218.254.202</option>"\
                "<option>202.73.57.107</option>"\
				"<option>128.199.134.40</option>"\
                "<option>211.233.40.78</option>"\
				"<option>106.247.248.106</option>"\
                "<option>129.250.35.251</option>"\
                "<option>131.188.3.220</option>"\
                "<option>131.188.3.223</option>"\
                "<option>203.114.74.17</option>"\
            "</datalist></td>"\
        "</tr>"\
        "<tr>"\
            "<td>Time Output Interva:  </td>"\
            "<td><input type=\"text\" value=\"%s\" name=\"Time_Output_Interva\" style=\"width:154px;\" maxlength=\"4\" oninput = \"value=value.replace(/[^\\d]/g,\'\')\"></td>"\
        "</tr>"\
        "<tr>"\
           "<td>Timezone: <br><br></td>"\
            "<td>"\
            "<select name=\"Timezone\">"\
			    "<option selected= \"selected\">UTC%s</option>"\
                "<option>UTC+0:00</option >"\
                "<option>UTC+1:00</option>"\
                "<option>UTC+2:00</option>"\
                "<option>UTC+3:00</option>"\
                "<option>UTC+3:30</option>"\
                "<option>UTC+4:00</option>"\
                "<option>UTC+4:30</option>"\
                "<option>UTC+5:00</option>"\
                "<option>UTC+5:30</option>"\
                "<option>UTC+6:00</option>"\
                "<option>UTC+6:30</option>"\
                "<option>UTC+7:00</option>"\
                "<option>UTC+8:00</option>"\
                "<option>UTC+9:00</option>"\
                "<option>UTC+9:30</option>"\
                "<option>UTC+10:00</option>"\
                "<option>UTC+10:30</option>"\
                "<option>UTC+11:00</option>"\
                "<option>UTC+11:30</option>"\
                "<option>UTC+12:00</option>"\
                "<option>UTC+12:45</option>"\
                "<option>UTC+13:00</option>"\
                "<option>UTC+14:00</option>"\
                "<option>UTC-1:00</option>"\
                "<option>UTC-2:00</option>"\
               "<option>UTC-3:00</option>"\
                "<option>UTC-3:30</option>"\
               "<option>UTC-4:00</option>"\
               "<option>UTC-5:00</option>"\
               "<option>UTC-6:00</option>"\
               "<option>UTC-7:00</option>"\
               "<option>UTC-8:00</option>"\
               "<option>UTC-9:00</option>"\
               "<option>UTC-9:30</option>"\
                "<option>UTC-10:00</option>"\
                "<option>UTC-11:00</option>"\
                "<option>UTC-12:00</option>"\
            "</select><br><br></td>"\
        "</tr>"\
	        "<tr>"\
            "<td class=\"class2\">DST Setting</td>"\
        "</tr>"\
        "<tr>"\
            "<td>DST Starts:hour</td>"\
            "<td><select name=\"hourStart\">"\
			    "<option selected= \"selected\">%s</option>"\
                "<option>0</option>"\
                "<option>1</option>"\
                "<option>2</option>"\
                "<option>3</option>"\
               " <option>4</option>"\
               " <option>5</option>"\
                "<option>6</option>"\
                "<option>7</option>"\
                "<option>8</option>"\
                "<option>9</option>"\
                "<option>10</option>"\
                "<option>11</option>"\
                "<option>12</option>"\
                "<option>13</option>"\
                "<option>14</option>"\
                "<option>15</option>"\
                "<option>16</option>"\
                "<option>17</option>"\
                "<option>18</option>"\
                "<option>19</option>"\
                "<option>20</option>"\
                "<option>21</option>"\
                "<option>22</option>"\
                "<option>23</option>"\
            "</select></td>"\
        "</tr>"\
        "<tr>"\
            "<td>DST Starts:Data </td>"\
            "<td><select name=\"WeekendSelectStart\">"\
			    "<option selected= \"selected\">%s</option>"\
                "<option>1st</option>"\
                "<option>2nd</option>"\
                "<option>3rd</option>"\
                "<option>4th</option>"\
                "<option>5th</option>"\
                "<option>last</option>"\
            "</select>"\
            "<select name=\"DST_Week_Start\">"\
			"<option selected= \"selected\">%s</option>"\
            "<option>Sun</option>"\
            "<option>Mon</option>"\
            "<option>Tue</option>"\
            "<option>Wed</option>"\
            "<option>Thu</option>"\
            "<option>Fri</option>"\
            "<option>Sat</option>"\
            "</select>"\
            "<select name=\"DST_Mon_Start\">"\
			    "<option selected= \"selected\">%s</option>"\
                "<option>Jan</option>"\
                "<option>Feb</option>"\
                "<option>Mar</option>"\
                "<option>Apr</option>"\
                "<option>May</option>"\
                "<option>Jun</option>"\
                "<option>Jul</option>"\
                "<option>Aug</option>"\
                "<option>Sep</option>"\
                "<option>Oct</option>"\
                "<option>Nov</option>"\
                "<option>Dec</option>"\
            "</select></td>"\
    "</tr>"\
      "<tr>"\
            "<td>DST Ends:hour</td>"\
            "<td><select name=\"hourEnd\">"\
			    "<option selected= \"selected\">%s</option>"\
                "<option>0</option>"\
                "<option>1</option>"\
                "<option>2</option>"\
                "<option>3</option>"\
                "<option>4</option>"\
                "<option>5</option>"\
                "<option>6</option>"\
                "<option>7</option>"\
                "<option>8</option>"\
                "<option>9</option>"\
                "<option>10</option>"\
                "<option>11</option>"\
                "<option>12</option>"\
                "<option>13</option>"\
                "<option>14</option>"\
                "<option>15</option>"\
                "<option>16</option>"\
                "<option>17</option>"\
                "<option>18</option>"\
                "<option>19</option>"\
                "<option>20</option>"\
                "<option>21</option>"\
               "<option>22</option>"\
               "<option>23</option>"\
           "</select></td>"\
        "</tr>"\
        "<tr>"\
            "<td>DST Ends:Data<br><br></td>"\
            "<td><select name=\"WeekendSelectEnd\">"\
			    "<option selected= \"selected\">%s</option>"\
                "<option>1st</option>"\
                "<option>2nd</option>"\
                "<option>3rd</option>"\
                "<option>4th</option>"\
                "<option>5th</option>"\
                "<option>last</option>"\
            "</select>"\
             "<select name=\"DST_Week_End\">"\
                    "<option selected= \"selected\">%s</option>"\
					"<option>Sun</option>"\
                    "<option>Mon</option>"\
                    "<option>Tue</option>"\
                    "<option>Wed</option>"\
                    "<option>Thu</option>"\
                    "<option>Fri</option>"\
                    "<option>Sat</option>"\
             "</select>"\
              "<select name=\"DST_Mon_End\">"\
                    "<option selected= \"selected\">%s</option>"\
					"<option>Jan</option>"\
                    "<option>Feb</option>"\
                   "<option>Mar</option>"\
                    "<option>Apr</option>"\
                   "<option>May</option>"\
                   "<option>Jun</option>"\
                   "<option>Jul</option>"\
                    "<option>Aug</option>"\
                    "<option>Sep</option>"\
                    "<option>Oct</option>"\
                   " <option>Nov</option>"\
                   " <option>Dec</option>"\
                "</select><br><br></td>"\
		"<tr>"\
		"<tr>"\
            "<td class=\"class2\">Timer Wifi Setting</td>"\
       "</tr>"\
           "<td>SSID:</td>"\
           "<td><input type=\"text\"  oninput=\"this.value=this.value.replace(/[^\\w_]/g,\'\');\" value=\"%s\" name=\"SSID\" style=\"width:154px;\" maxlength=\"30\"></td>"\
        "</tr>"\
		   "<tr>"\
            "<td>Password:</td>"\
          "<td><input type=\"password\" value=\"%s\"  oninput=\"this.value=this.value.replace(/[^\\w_]/g,\'\');\" name=\"Password\" id=\"pwd5\" style=\"width:120px;\" maxlength=\"30\"> <input type=\"checkbox\" class=\"tui-checkbox\" onchange=\"changePwd5Visible()\" ></td>"\
        "</tr>"\
		 "<tr>"\
        "<td>Auto Access</td>"\
        "<td>Yes:<input type=\"radio\" value=\"1\" name=\"Auto_Web\" %s>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;No:<input type=\"radio\" value=\"2\" name=\"Auto_Web\" %s></td>"\
        "</tr>"\
		"<td>Language Choice</td>"\
            "<td><select style=\"width:154px;\" name=\"language\">"\
                "<option %s>English</option>"\
                "<option %s>%s</option>"\
            "</select></td>"\
        "</tr>"\
        "<tr >"\
            "<td><br></td>"\
            "<td><br></td>"\
        "</tr>"\
        "<tr >"\
            "<td> <input  type=\"submit\" class=\"btns\" name=\"Save\" value=\"Save\"></td>"\
            "<td>&nbsp;&nbsp;&nbsp;<input type=\"submit\" class=\"btns\" name=\"Refresh\" value=\"Refresh\"></td>"\
        "</tr>"\
   " </table>"\
"</form>"\
	"<br><br>"\
	"<p class=\"class3\">GX WIFI Tranmission NTP Time System 1.4</p>"\
"</div>"\
	"<script>"\
	"window.onload =csan();"\
    "var pwd4 = document.getElementById(\"pwd4\");"\
    "var pwd5 = document.getElementById(\"pwd5\");"\
    "function changePwd4Visible() {"\
        "pwd4.type = event.target.checked ? \"text\" : \"password\";"\
   "}"\
       "function changePwd5Visible() {"\
        "pwd5.type = event.target.checked ? \"text\" : \"password\";"\
    "}"\
    "function csan(){"\
        "var id2=document.getElementById(\"checkid\");"\
        "if(id2.checked){"\
            "document.getElementById(\"name01\").readOnly=false;"\
            "document.getElementById(\"name02\").readOnly=false;"\
        "}else{"\
            "document.getElementById(\"name01\").value=\"\";"\
            "document.getElementById(\"name01\").readOnly=true;"\
            "document.getElementById(\"name02\").value=\"\";"\
            "document.getElementById(\"name02\").readOnly=true;"\
        "}"\
    "}"\
"</script>"\
"<br><br><br>"\
"</body>"\
"</html>"

enum{
	Captive_OK = 1,
	Visit_IP_OK,
	TCP_Link_Config
}Link_Way;
typedef struct{
	uint32 Dst_Hour;
	uint32 Dst_WeekSel;
	uint32 Dst_Week;
	uint32 Dst_Mon;
}DstPacket;

typedef struct{
   char *DstHourS_Point;
   char *DstHourE_Point;
   char *DstWeekSeleS_Point;
   char *DstWeekSeleE_Point;
   char *DstWeekS_Point;
   char *DstWeekE_Point;
   char *DstMonS_Point;
   char *DstMonE_Point;
}Dst_point_packet;

typedef struct{
	char *Ssidpoint;
	uint32 SsidLen;
	char *Pskpoint;
	uint32 PskLen;
	char *NtpServerPoint;
	uint32 NtpServerLen;
	char *TimeZonePoint;
	uint32 TimeZoneLen;
	char *DstStartPoint;
	char *DstEndPoint;
	Dst_point_packet Dst_point;
	char *Ip_Point;
	uint32 Ip_Len;
	char *Mask_Point;
	uint32 Mask_Len;
	char *Gateway_Point;
	uint32 Gateway_Len;
	char *DHCP_State;
}AppSetPacket;

#define  TCPPCBSIZE  10
typedef struct {
	uint8_t TcpClientCount;  /* 此变量用来存储当前有几个客户端连接 */
	//uint8_t TcpClientWritePoint; /* 写指针 */
	//struct espconn *EspConnBuffer[10];
}TcpServerPacket;
extern struct espconn Dns_Client_conn;     //建立一个espconn结构体
extern ip_addr_t esp_server_ip;
extern char tempSaveData[];
extern char DnsBuffer[];
extern char TcpClientToServer[];
extern DstPacket DstStart,DstEnd;
void ICACHE_FLASH_ATTR AP_tcpserver_init(u16 port);
 void ICACHE_FLASH_ATTR tcp_server_listen(void *arg);
 void ICACHE_FLASH_ATTR tcp_server_recon_cb(void *arg, sint8 err);
 void ICACHE_FLASH_ATTR tcp_server_discon_cb(void *arg);
 void ICACHE_FLASH_ATTR tcp_server_recv_cb(void *arg, char *pusrdata, unsigned short length);
 void tcp_server_multi_send(void);
void ICACHE_FLASH_ATTR AP_tcpserverAPP_init(u16 port);
void ICACHE_FLASH_ATTR tcp_server_recvApp_cb(void *arg, char *pusrdata, unsigned short length);
void ICACHE_FLASH_ATTR tcp_serverApp_listen(void *arg);
void ICACHE_FLASH_ATTR GetIPData2(uint32 IP_addr[],char * IP_Point);
void ICACHE_FLASH_ATTR user_dns_server_init();
void ICACHE_FLASH_ATTR webserver_sent_cb(void *arg);
void ICACHE_FLASH_ATTR SetSSID(char *SSID,uint32 SSIDlen,char *PSWD,uint32 PSWDlen);
void ICACHE_FLASH_ATTR user_scan(void);
void ICACHE_FLASH_ATTR scan_done(void *arg, STATUS status);
void ICACHE_FLASH_ATTR Get_StaticIP_Set3(char *StaticIP,char *Sub_Mask,char* Gate_way);
void ICACHE_FLASH_ATTR tcp_Appserver_recon_cb(void *arg, sint8 err);
void ICACHE_FLASH_ATTR tcp_Appserver_discon_cb(void *arg);
void ICACHE_FLASH_ATTR ChangeTcpPort(char* TcpPortPoint,uint32 Len);
void ICACHE_FLASH_ATTR PrintfBuffer(char *Point,uint32 Len);
void ICACHE_FLASH_ATTR user_esp_platform_dns_found(const char *name, ip_addr_t *ipaddr, void *arg);
#endif /* APP_INCLUDE_TCPSERVER_H_ */
