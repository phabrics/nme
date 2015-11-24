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
#include "options.h"

/* the new TAP function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_openvpn,tap) {
  int rc;
  struct gc_arena gc = gc_new ();
  unsigned char *hwaddr;
  unsigned int hwaddr_len;
  struct tuntap *tt;
  int arg_i;
  struct frame frame;
  struct env_set *es;
  struct options options;       /**< Options loaded from command line or
                                 *   configuration file. */
  
  arg_i = 0;

  while(args[++arg_i] != NULL);

  error_reset();

  /* initialize environmental variable store */
  es = env_set_create(NULL);
#ifdef WIN32
  init_win32();
  set_win_sys_path_via_env(es);
#endif

  /* initialize options to default state */
  init_options(&options, true);

  /* parse command line options, and read configuration file */
  parse_argv(&options, arg_i, args, M_USAGE, OPT_P_DEFAULT, NULL, es);

  /* set dev options */
  init_options_dev(&options);

  /* tun/tap persist command? */
  if(do_persist_tuntap(&options))
    return (TME_OK);

  /* sanity check on options */
  options_postprocess(&options);

  /* show all option settings */
  show_settings(&options);

  /* print version number */
  //msg(M_INFO, "%s", title_string);
  show_library_versions(M_INFO);

  /* misc stuff */
  pre_setup(&options);

  /* set certain options as environmental variables */
  setenv_settings(es, &options);

  /* TUN/TAP specific stuff */
  tt = init_tun(options.dev,
		options.dev_type,
		options.topology,
		options.ifconfig_local,
		options.ifconfig_remote_netmask,
		options.ifconfig_ipv6_local,
		options.ifconfig_ipv6_netbits,
		options.ifconfig_ipv6_remote,
		0, //addr_host (&c->c1.link_socket_addr.local),
		0, //addr_host (&c->c1.link_socket_addr.remote),
		!options.ifconfig_nowarn,
		es);

  if(!tt) return (EINVAL);

  memset(&frame, 0, sizeof(frame));  
  /*
   * Adjust frame size based on the --tun-mtu-extra parameter.
   */
  if(options.ce.tun_mtu_extra_defined)
    tun_adjust_frame_parameters(&frame, options.ce.tun_mtu_extra);

  /* See frame_finalize_options (struct context *c, const struct options *o) */
  frame_align_to_extra_frame(&frame);
  frame_or_align_flags(&frame,
		       FRAME_HEADROOM_MARKER_FRAGMENT
		       |FRAME_HEADROOM_MARKER_READ_LINK
		       |FRAME_HEADROOM_MARKER_READ_STREAM);
  
  frame_finalize(&frame,
		 options.ce.link_mtu_defined,
		 options.ce.link_mtu,
		 options.ce.tun_mtu_defined,
		 options.ce.tun_mtu);

  /* flag tunnel for IPv6 config if --tun-ipv6 is set */
  tt->ipv6 = options.tun_ipv6;

  init_tun_post(tt, &frame, &options.tuntap_options);
  
  if(ifconfig_order() == IFCONFIG_BEFORE_TUN_OPEN) {
    /* guess actual tun/tap unit number that will be returned
       by open_tun */
    const char *guess = guess_tuntap_dev(options.dev,
					 options.dev_type,
					 options.dev_node,
					 &gc);
    do_ifconfig(tt, guess, TUN_MTU_SIZE(&frame), es);
  }

#ifndef OPENVPN_ETH
  /* temporarily turn off ipv6 to disable protocol info being prepended to packets on Linux */
  tt->ipv6 = FALSE;
#endif
  /* open the tun device */
  open_tun(options.dev,
	   options.dev_type,
	   options.dev_node,
	   tt);

  tt->ipv6 = options.tun_ipv6;

  /* set the hardware address */
  if(options.lladdr)
    set_lladdr(tt->actual_name, options.lladdr, es);
  
  /* do ifconfig */
  if(ifconfig_order() == IFCONFIG_AFTER_TUN_OPEN) {
    do_ifconfig(tt, tt->actual_name, TUN_MTU_SIZE(&frame), es);
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
