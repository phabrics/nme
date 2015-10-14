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

/* the new TAP function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_openvpn,tap) {
  int rc;
  struct gc_arena gc = gc_new ();
  const char *dev, *dev_type, *dev_node, *dev_addr;
  const char *inet, *netmask, *inet6, *remote6, *netbits;
  unsigned char *hwaddr;
  unsigned int hwaddr_len;
  struct tuntap *tt;
  int arg_i;
  int usage;
  
  /* check our arguments: */
  usage = 0;

  dev = dev_type = dev_node = dev_addr =
    inet = netmask = inet6 = remote6 = netbits = NULL;
  
  arg_i = 1;

  for (;;) {
    /* the interface we're supposed to use: */
    if (TME_ARG_IS(args[arg_i + 0], "dev")
	&& args[arg_i + 1] != NULL) {
      dev = args[arg_i + 1];
    }

    else if (TME_ARG_IS(args[arg_i + 0], "dev-type")
	&& args[arg_i + 1] != NULL) {
      dev_type = args[arg_i + 1];
    }

    else if (TME_ARG_IS(args[arg_i + 0], "dev-node")
	&& args[arg_i + 1] != NULL) {
      dev_node = args[arg_i + 1];
    }

    else if (TME_ARG_IS(args[arg_i + 0], "dev-addr")
	&& args[arg_i + 1] != NULL) {
      dev_addr = args[arg_i + 1];
    }

    else if(TME_ARG_IS(args[arg_i + 0], "inet") 
	 && args[arg_i + 1] != NULL) {
      inet = args[arg_i + 1];
    }

    else if (TME_ARG_IS(args[arg_i + 0], "netmask")
	&& args[arg_i + 1] != NULL) {
      netmask =  args[arg_i + 1];
    }

    else if(TME_ARG_IS(args[arg_i + 0], "inet6") 
	 && args[arg_i + 1] != NULL) {
      inet6 = args[arg_i + 1];
    }

    else if(TME_ARG_IS(args[arg_i + 0], "remote6") 
	 && args[arg_i + 1] != NULL) {
      remote6 = args[arg_i + 1];
    }

    else if (TME_ARG_IS(args[arg_i + 0], "netbits")
	&& args[arg_i + 1] != NULL) {
      netbits =  args[arg_i + 1];
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
			    "%s %s [ dev %s ] [ dev-type %s ] [ dev-node %s ] [ dev-addr %s ] [ inet %s ] [ netmask %s ] [ inet6 %s] [remote6 %s]",
			    _("usage:"),
			    args[0],
			    _("NAME"),
			    _("TYPE"),
			    _("NODE"),
			    _("LLADDR"),
			    _("IPADDRESS"),
			    _("IPADDRESS"),
			    _("IP6ADDRESS"),
			    _("IP6ADDRESS"));
    return (EINVAL);
  }
  
  if(!dev) dev = "tap";
  
  tme_log(&element->tme_element_log_handle, 0, TME_OK, 
	  (&element->tme_element_log_handle, 
	   "trying tap device (%s, type %s, node %s) with addresses: link %s, ip %s/%s, ip6 %s/%s prefixlen %s",
	   dev,
	   (dev_type) ? (dev_type) : "",
	   (dev_node) ? (dev_node) : "",
	   (dev_addr) ? (dev_addr) : "",
	   (inet) ? (inet) : "",
	   (netmask) ? (netmask) : "",
	   (inet6) ? (inet6) : "",
	   (remote6) ? (remote6) : "",
	   (netbits) ? (netbits) : ""));
	  
  error_reset();
  tt = init_tun(dev,
		dev_type,
		TOP_SUBNET,
		inet,
		netmask,		
		inet6,
		(netbits) ? (atoi(netbits)) : (0),
		remote6,
		0,
		0,
		0,
		NULL);

  if(ifconfig_order() == IFCONFIG_BEFORE_TUN_OPEN) {
    /* guess actual tun/tap unit number that will be returned
       by open_tun */
    const char *guess = guess_tuntap_dev(dev,
					 dev_type,
					 dev_node,
					 &gc);
    do_ifconfig(tt, guess, TME_ETHERNET_FRAME_MAX, NULL);
  }
  
  /* open the tun device */
  open_tun(dev,
	   dev_type,
	   dev_node,
	   tt);

  /* set the hardware address */
  if(dev_addr)
    set_lladdr(tt->actual_name, dev_addr, NULL);
  
  /* do ifconfig */
  if(ifconfig_order() == IFCONFIG_AFTER_TUN_OPEN) {
    do_ifconfig(tt, tt->actual_name, TME_ETHERNET_FRAME_MAX, NULL);
  }
  
  /* find the interface we will use: */
  if(inet || inet6)
    rc = tme_eth_ifaddrs_find((inet) ? (inet) : (inet6),
			      (inet) ? (AF_INET) : (AF_INET6),
			      NULL, &hwaddr, &hwaddr_len);
    
  return tme_eth_init(element, tt->fd, 4096, NULL, hwaddr, NULL);
  
}
