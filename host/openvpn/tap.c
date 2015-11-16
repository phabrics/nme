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
#define OPENVPN_HOST
#include "eth-impl.c"

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
  struct frame frame;
  
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

    else if (TME_ARG_IS(args[arg_i + 0], "prefixlen")
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
	   "trying tap device (%s, type %s, node %s) with addresses: link %s, ip %s/%s, ip6 %s/%s rem6 %s",
	   dev,
	   (dev_type) ? (dev_type) : "",
	   (dev_node) ? (dev_node) : "",
	   (dev_addr) ? (dev_addr) : "",
	   (inet) ? (inet) : "",
	   (netmask) ? (netmask) : "",
	   (inet6) ? (inet6) : "",
	   (netbits) ? (netbits) : "",
	   (remote6) ? (remote6) : ""));
	  
  error_reset();
#ifdef WIN32
  init_win32();
#endif
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

  if(!tt) return (EINVAL);

  memset(&frame, 0, sizeof(frame));  
  tun_adjust_frame_parameters(&frame, TAP_MTU_EXTRA_DEFAULT);

  frame_align_to_extra_frame(&frame);
  frame_or_align_flags(&frame,
		       FRAME_HEADROOM_MARKER_FRAGMENT
		       |FRAME_HEADROOM_MARKER_READ_LINK
		       |FRAME_HEADROOM_MARKER_READ_STREAM);
  
  frame_finalize(&frame,
		 false,
		 LINK_MTU_DEFAULT,
		 true,
		 TUN_MTU_DEFAULT);

#ifdef WIN32
  overlapped_io_init (&tt->reads, &frame, FALSE, true);
  overlapped_io_init (&tt->writes, &frame, TRUE, true);
  tt->rw_handle.read = tt->reads.overlapped.hEvent;
  tt->rw_handle.write = tt->writes.overlapped.hEvent;
  tt->adapter_index = TUN_ADAPTER_INDEX_INVALID;
#endif
  
  if(inet6) tt->ipv6 = TRUE;
  
  if(ifconfig_order() == IFCONFIG_BEFORE_TUN_OPEN) {
    /* guess actual tun/tap unit number that will be returned
       by open_tun */
    const char *guess = guess_tuntap_dev(dev,
					 dev_type,
					 dev_node,
					 &gc);
    do_ifconfig(tt, guess, TUN_MTU_SIZE(&frame), NULL);
  }

#ifndef OPENVPN_ETH
  /* temporarily turn off ipv6 to disable protocol info being prepended to packets on Linux */
  tt->ipv6 = FALSE;
#endif
  /* open the tun device */
  open_tun(dev,
	   dev_type,
	   dev_node,
	   tt);

  if(inet6) tt->ipv6 = TRUE;

  /* set the hardware address */
  if(dev_addr)
    set_lladdr(tt->actual_name, dev_addr, NULL);
  
  /* do ifconfig */
  if(ifconfig_order() == IFCONFIG_AFTER_TUN_OPEN) {
    do_ifconfig(tt, tt->actual_name, TUN_MTU_SIZE(&frame), NULL);
  }
  
  /* find the interface we will use: */
#ifdef HAVE_IFADDRS_H
  struct ifaddrs *ifa;
  
  rc = tme_eth_ifaddrs_find(tt->actual_name, AF_UNSPEC, &ifa, &hwaddr, &hwaddr_len);
    
  if(hwaddr_len == TME_ETHERNET_ADDR_SIZE) {
    tme_log(&element->tme_element_log_handle, 0, TME_OK, 
	    (&element->tme_element_log_handle, 
	     "hardware address on tap interface %s set to %02x:%02x:%02x:%02x:%02x:%02x",
	     ifa->ifa_name, 
	     hwaddr[0],
	     hwaddr[1],
	     hwaddr[2],
	     hwaddr[3],
	     hwaddr[4],
	     hwaddr[5]));
  }
#endif
  return tme_eth_init(element,
#ifdef OPENVPN_ETH
		      tt,
#else
		      tt->fd,
#endif
		      MAX_RW_SIZE_TUN(&frame), NULL, hwaddr, NULL);
  
}
