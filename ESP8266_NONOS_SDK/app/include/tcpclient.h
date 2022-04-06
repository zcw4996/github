/*
 * tcpclient.h
 *
 *  Created on: 2017年2月8日
 *      Author: dell
 */

#ifndef APP_INCLUDE_TCPCLIENT_H_
#define APP_INCLUDE_TCPCLIENT_H_

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"
#define HTML_CH  "<!DOCTYPE html>"\
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
"<p class=\"class1\">%s</p>"\
"<div id=\"b\">"\
"<form method=\"post\">"\
    "<table  align=\"center\">"\
       "<tr>"\
            "<td ><strong>%s</strong></td>"\
       "</tr>"\
        "<tr >"\
           "<td> %s: </td>"\
           "<td>"\
			"<select name=\"AP_Name\" style=\"width:150px;\">"\
			 "<option selected= \"selected\">%s</option>"\
			"%s"\
            "</select></td>"\
       "</tr>"\
     "<tr >"\
     "<td> %s:</td>"\
         "<td><input type=\"password\" value=\"%s\"  onkeyup=\"value=value.replace(/[\\u4e00-\\u9fa5]/ig,\'\')\" name=\"AP_Password\" id=\"pwd4\" style=\"width:120px;\" maxlength=\"30\"> <input type=\"checkbox\" class=\"tui-checkbox\" onchange=\"changePwd4Visible()\" ></td>"\
        "</tr>"\
        "<tr>"\
            "<td>%s IP:<input type=\"radio\"  value=\"1\"  name=\"DHCP_State\" %s ><br></td>"\
            "<td>%s IP:<input type=\"radio\"  value=\"2\" name=\"DHCP_State\" %s><br></td>"\
        "</tr>"\
        "<tr >"\
            "<td>IP %s:</td>"\
            "<td><input type=\"text\"  value=\"%s\" oninput=\"value=value.replace(/[^\\d.]/g,\'\')\" name=\"IP_addr\"  style=\"width:154px;\" maxlength=\"15\"></td>"\
        "</tr>"\
        "<tr>"\
            "<td>%s:</td>"\
            "<td><input type=\"text\"  value=\"%s\" oninput=\"value=value.replace(/[^\\d.]/g,\'\')\" name=\"Subnet_mask\" style=\"width:154px;\" maxlength=\"15\"></td>"\
        "</tr>"\
        "<tr>"\
            "<td>%s:</td>"\
            "<td><input type=\"text\"  value=\"%s\" oninput=\"value=value.replace(/[^\\d.]/g,\'\')\" name=\"Gateway\" style=\"width:154px;\" maxlength=\"15\"></td>"\
        "</tr>"\
		  "<tr>"\
           "<td>%s:<br><br></td>"\
          "<td><input type=\"text\" value=\"%s\" name=\"TCP_Server_Port\" style=\"width:154px;\" maxlength=\"5\" oninput = \"value=value.replace(/[^\\d]/g,\'\')\"><br><br></td>"\
        "</tr>"\
        "<tr>"\
           "<td>%s:</td>"\
           "<td><input type=\"text\" id=\"name01\" value=\"%s\" name=\"TCP_Server_IP\" style=\"width:120px;\" maxlength=\"20\" oninput=\"value=value.replace(/[\\u4e00-\\u9fa5]/ig,\'\')\" readonly>"\
               "&nbsp;<input type=\"checkbox\" id=\"checkid\" class=\"tui-checkbox \" onclick=\"csan()\"  name=\"kjLevel\" %s/></td>"\
        "</tr>"\
        "<tr>"\
            "<td>%s:<br><br></td>"\
            "<td><input type=\"text\"  value=\"%s\" id=\"name02\" name=\"TCP_Server_Port1\" style=\"width:54px;\" maxlength=\"5\" readonly oninput = \"value=value.replace(/[^\\d]/g,\'\')\">"\
                "&nbsp;&nbsp;<select style=\"width:60px;\" name=\"Tcp_Type\">"\
                "<option %s>Short Connection</option>"\
                "<option %s>Long Connection</option>"\
            "</select><br><br></td>"\
       "</tr>"\
        "<tr >"\
            "<td><strong>%s</strong></td>"\
        "</tr>"\
        "<tr>"\
            "<td>%s: </td>"\
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
            "<td>%s:  </td>"\
            "<td><input type=\"text\" value=\"%s\" name=\"Time_Output_Interva\" style=\"width:154px;\" maxlength=\"4\" oninput = \"value=value.replace(/[^\\d]/g,\'\')\"></td>"\
        "</tr>"\
        "<tr>"\
           "<td>%s: <br><br></td>"\
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
            "<td><strong>%s</strong></td>"\
        "</tr>"\
        "<tr>"\
            "<td>%s</td>"\
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
            "<td>%s </td>"\
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
            "<td>%s</td>"\
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
            "<td>%s<br><br></td>"\
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
            "<td><strong>%s</strong></td>"\
       "</tr>"\
           "<td>%s:</td>"\
           "<td><input type=\"text\"  oninput=\"this.value=this.value.replace(/[^\\w_]/g,\'\');\" value=\"%s\" name=\"SSID\" style=\"width:154px;\" maxlength=\"30\"></td>"\
        "</tr>"\
		   "<tr>"\
            "<td>%s:</td>"\
          "<td><input type=\"password\" value=\"%s\"  oninput=\"this.value=this.value.replace(/[^\\w_]/g,\'\');\" name=\"Password\" id=\"pwd5\" style=\"width:120px;\" maxlength=\"30\"> <input type=\"checkbox\" class=\"tui-checkbox\" onchange=\"changePwd5Visible()\" ></td>"\
        "</tr>"\
		 "<tr>"\
        "<td>%s</td>"\
        "<td>%s:<input type=\"radio\" value=\"1\" name=\"Auto_Web\" %s>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;%s:<input type=\"radio\" value=\"2\" name=\"Auto_Web\" %s></td>"\
        "</tr>"\
		 "<td>%s</td>"\
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

extern struct espconn tcpcilent_esp_conn;
void ICACHE_FLASH_ATTR AP_tcpclient_init(char *TCP_ServerIP,uint32 TCP_ServerPort);
LOCAL void ICACHE_FLASH_ATTR tcp_cilent_connect_cb(void *arg);
LOCAL void ICACHE_FLASH_ATTR tcp_client_recon_cb(void *arg, sint8 err);
LOCAL void ICACHE_FLASH_ATTR tcp_cilent_discon_cb(void *arg);
LOCAL void ICACHE_FLASH_ATTR tcp_client_recv_cb(void *arg, char *pusrdata, unsigned short length);
void ICACHE_FLASH_ATTR connect_servre(void);
void ICACHE_FLASH_ATTR client_send(void *arg);
void ICACHE_FLASH_ATTR SendTcpClientData(uint8 *TxBuffer,uint32 BufLen);
#endif /* APP_INCLUDE_TCPCLIENT_H_ */
