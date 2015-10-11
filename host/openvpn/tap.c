/* host/openvpn/tap.c - TUN TAP Ethernet support: */

/*
 * Copyright (c) 2001, 2003 Matt Fredette
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Matt Fredette.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <tme/common.h>

/* includes: */
#include "eth-if.h"
#include "syshead.h"
#include "tun.h"

/* macros: */
/* device names: */
#define TME_DEV (0)
#define TME_DEV_TYPE (1)
#define TME_DEV_NODE (2)
#define TME_DEV_ADDR (3)
#define TME_DEV_TOTAL (4)

/* interface addresses: */
#define TME_IP_ADDRS_INET (0)
#define TME_IP_ADDRS_NETMASK (1)
#define TME_IP_ADDRS_TOTAL (2)

/* retrieve ethernet arguments */
int _tme_openvpn_tap_args(const char * const args[], 
			  char *devs[],
			  struct in_addr *ip_addrs,
			  char **_output)
{
  int arg_i;
  int usage;
  
  /* check our arguments: */
  usage = 0;

  memset(ip_addrs, 0, TME_IP_ADDRS_TOTAL * sizeof(struct in_addr));

  arg_i = 1;

  for (;;) {
    /* the interface we're supposed to use: */
    if (TME_ARG_IS(args[arg_i + 0], "dev")
	&& args[arg_i + 1] != NULL) {
      devs[TME_DEV] = args[arg_i + 1];
    }

    else if (TME_ARG_IS(args[arg_i + 0], "dev-type")
	&& args[arg_i + 1] != NULL) {
      devs[TME_DEV_TYPE] = args[arg_i + 1];
    }

    else if (TME_ARG_IS(args[arg_i + 0], "dev-node")
	&& args[arg_i + 1] != NULL) {
      devs[TME_DEV_NODE] = args[arg_i + 1];
    }

    else if (TME_ARG_IS(args[arg_i + 0], "dev-addr")
	&& args[arg_i + 1] != NULL) {
      devs[TME_DEV_ADDR] = args[arg_i + 1];
    }

    else if(TME_ARG_IS(args[arg_i + 0], "inet") 
	 && args[arg_i + 1] != NULL) {
      inet_pton(AF_INET, args[arg_i + 1], ip_addrs + TME_IP_ADDRS_INET);      
    }
    /*
    else if(TME_ARG_IS(args[arg_i + 0], "inet6") 
	 && args[arg_i + 1] != NULL) {
      inet_pton(AF_INET6, args[arg_i + 1], ip_addrs + TME_IP_ADDRS_INET6);      
    }
    */
    else if (TME_ARG_IS(args[arg_i + 0], "netmask")
	&& args[arg_i + 1] != NULL) {
      inet_pton(AF_INET, args[arg_i + 1], ip_addrs + TME_IP_ADDRS_NETMASK);
    }

    /* if we ran out of arguments: */
    else if (args[arg_i + 0] == NULL) {
      break;
    }

    /* otherwise this is a bad argument: */
    else {
      tme_output_append_error(_output,
			      "%s %s", 
			      args[arg_i],
			      _("unexpected"));
      usage = TRUE;
      break;
    }
    arg_i += 2;
  }

  if (usage) {
    tme_output_append_error(_output,
			    "%s %s [ dev %s ] [ dev-type %s ] [ dev-node %s ] [ dev-addr %s ] [ inet %s ] [ netmask %s ]",
			    _("usage:"),
			    args[0],
			    _("NAME"),
			    _("TYPE"),
			    _("NODE"),
			    _("LLADDR"),
			    _("IPADDRESS"),
			    _("IPADDRESS"));
    return (EINVAL);
  }
  return (TME_OK);
}

/* the new TAP function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_openvpn,tap) {
  int i;
  struct gc_arena gc = gc_new ();
  char *devs[TME_DEV_TOTAL];
  char tap_hosts[TME_IP_ADDRS_TOTAL][NI_MAXHOST];
  struct in_addr tap_addrs[TME_IP_ADDRS_TOTAL];
  unsigned char *hwaddr;
  unsigned int hwaddr_len;
  struct tuntap *tt;
  
  for(i=0;i<TME_DEV_TOTAL;i++) {
    devs[i]=NULL;
  }
  /* get the arguments: */
  _tme_openvpn_tap_args(args, devs, tap_addrs, _output);

  if(!devs[TME_DEV]) devs[TME_DEV] = "tap";
  
  for(i=0;i<TME_IP_ADDRS_TOTAL;i++) {
    inet_ntop(AF_INET, &tap_addrs[i], tap_hosts[i], NI_MAXHOST);
  }
  tme_log(&element->tme_element_log_handle, 0, TME_OK, 
	  (&element->tme_element_log_handle, 
	   "trying tap device (%s, %s, %s) with link-layer address %s, ip address %s, netmask %s",
	   ((devs[TME_DEV]) ? (devs[TME_DEV]) : ""),
	   ((devs[TME_DEV_TYPE]) ? (devs[TME_DEV_TYPE]) : ""),
	   ((devs[TME_DEV_NODE]) ? (devs[TME_DEV_TYPE]) : ""),
	   ((devs[TME_DEV_ADDR]) ? (devs[TME_DEV_ADDR]) : ""),
	   tap_hosts[TME_IP_ADDRS_INET],
	   tap_hosts[TME_IP_ADDRS_NETMASK]));

  tt = init_tun(devs[TME_DEV],
		devs[TME_DEV_TYPE],
		TOP_SUBNET,
		tap_hosts[TME_IP_ADDRS_INET],
		tap_hosts[TME_IP_ADDRS_NETMASK],		
		NULL, 0, NULL, 0, 0, 0, NULL);

  if(ifconfig_order() == IFCONFIG_BEFORE_TUN_OPEN) {
    /* guess actual tun/tap unit number that will be returned
       by open_tun */
    const char *guess = guess_tuntap_dev(devs[TME_DEV],
					 devs[TME_DEV_TYPE],
					 devs[TME_DEV_NODE],
					 &gc);
    do_ifconfig(tt, guess, TME_ETHERNET_FRAME_MAX, NULL);
  }
  
  /* open the tun device */
  open_tun(devs[TME_DEV],
	   devs[TME_DEV_TYPE],
	   devs[TME_DEV_NODE],
	   tt);

  /* set the hardware address */
  if(devs[TME_DEV_ADDR])
    set_lladdr(tt->actual_name, devs[TME_DEV_ADDR], NULL);

  /* do ifconfig */
  if(ifconfig_order() == IFCONFIG_AFTER_TUN_OPEN) {
    do_ifconfig(tt, tt->actual_name, TME_ETHERNET_FRAME_MAX, NULL);
  }

  return tme_eth_init(element, tt->fd, 4096, NULL, hwaddr, NULL);

}
