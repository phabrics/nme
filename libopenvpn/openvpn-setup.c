/* host/openvpn/link.c - OpenVPN Ethernet setup: */

/*
 * Copyright (c) 2016 Ruben Agin
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
#include "openvpn-setup.h"
#include "options.h"

const struct link_socket *accept_from; /* possibly do accept() on a parent link_socket */

struct signal_info *sig;      /**< Internal error signaling object. */

static inline
struct tuntap *setup_tuntap(struct frame *frame, struct link_socket_addr *lsa, struct options *options, struct env_set *es) {
  /* TUN/TAP specific stuff */
  struct gc_arena gc;
  struct tuntap *tt = init_tun(options->dev,
			       options->dev_type,
			       options->topology,
			       options->ifconfig_local,
			       options->ifconfig_remote_netmask,
			       options->ifconfig_ipv6_local,
			       options->ifconfig_ipv6_netbits,
			       options->ifconfig_ipv6_remote,
			       ((lsa!=NULL) ? (addr_host(&lsa->local)) : (0)),
			       ((lsa!=NULL) ? (addr_host(&lsa->remote)) : (0)),
			       !options->ifconfig_nowarn,
			       es);

  if(!tt) return tt;

  gc = gc_new ();  

  /* flag tunnel for IPv6 config if --tun-ipv6 is set */
  tt->ipv6 = options->tun_ipv6;

  init_tun_post(tt, frame, &options->tuntap_options);
  
  if(ifconfig_order() == IFCONFIG_BEFORE_TUN_OPEN) {
    /* guess actual tun/tap unit number that will be returned
       by open_tun */
    const char *guess = guess_tuntap_dev(options->dev,
					 options->dev_type,
					 options->dev_node,
					 &gc);
    do_ifconfig(tt, guess, TUN_MTU_SIZE(frame), es);
  }

#ifdef TME_THREADS_SJLJ
  /* temporarily turn off ipv6 to disable protocol info being prepended to packets on Linux */
  tt->ipv6 = FALSE;
#endif
  /* open the tun device */
  open_tun(options->dev,
	   options->dev_type,
	   options->dev_node,
	   tt);

  tt->ipv6 = options->tun_ipv6;

  /* set the hardware address */
  if(options->lladdr)
    set_lladdr(tt->actual_name, options->lladdr, es);
  
  /* do ifconfig */
  if(ifconfig_order() == IFCONFIG_AFTER_TUN_OPEN) {
    do_ifconfig(tt, tt->actual_name, TUN_MTU_SIZE(frame), es);
  }
  
  /* run the up script */
  run_up_down(options->up_script,
	      NULL,
	      OPENVPN_PLUGIN_UP,
	      tt->actual_name,
	      dev_type_string(options->dev, options->dev_type),
	      TUN_MTU_SIZE(frame),
	      EXPANDED_SIZE(frame),
	      print_in_addr_t(tt->local, IA_EMPTY_IF_UNDEF, &gc),
	      print_in_addr_t(tt->remote_netmask, IA_EMPTY_IF_UNDEF, &gc),
	      "init",
	      NULL,
	      "up",
	      es);

  return tt;
}

static inline
struct link_socket *setup_link_socket(struct frame *frame, struct link_socket_addr *lsa, struct options *options) {
  struct link_socket *link_socket = link_socket_new();
  unsigned int sockflags = options->sockflags;

#if PORT_SHARE
  if (options->port_share_host && options->port_share_port)
    sockflags |= SF_PORT_SHARE;
#endif

  link_socket_init_phase1 (link_socket,
			   connection_list_defined(options),
			   options->ce.local,
			   options->ce.local_port,
			   options->ce.remote,
			   options->ce.remote_port,
			   options->ce.proto,
			   LS_MODE_DEFAULT,
			   accept_from,
#ifdef ENABLE_HTTP_PROXY
			   http_proxy,
#endif
#ifdef ENABLE_OPENVPNS
			   socks_proxy,
#endif
#ifdef ENABLE_DEBUG
			   options->gremlin,
#endif
			   options->ce.bind_local,
			   options->ce.remote_float,
			   options->inetd,
			   lsa,
			   options->ipchange,
			   NULL,
			   options->resolve_retry_seconds,
			   options->ce.connect_retry_seconds,
			   options->ce.connect_timeout,
			   options->ce.connect_retry_max,
			   options->ce.mtu_discover_type,
			   options->rcvbuf,
			   options->sndbuf,
			   options->mark,
			   sockflags);
  
  link_socket_init_phase2(link_socket, frame,
			   &sig->signal_received);

  return link_socket;
}

/*
 * Fast I/O setup.  Fast I/O is an optimization which only works
 * if all of the following are true:
 *
 * (1) The platform is not Windows
 * (2) --proto udp is enabled
 * (3) --shaper is disabled
 */
static bool
do_setup_fast_io(struct options *options) {
  if (options->fast_io) {
#ifdef WIN32
    msg (M_INFO, "NOTE: --fast-io is disabled since we are running on Windows");
#else
    if (!proto_is_udp(options->ce.proto))
      msg (M_INFO, "NOTE: --fast-io is disabled since we are not using UDP");
    else {
#ifdef ENABLE_FEATURE_SHAPER
      if (options->shaper)
	msg (M_INFO, "NOTE: --fast-io is disabled since we are using --shaper");
      else
#endif
	return true;
    }
#endif
  }
  return false;
}

struct env_set *openvpn_setup(const char *args[], int argc, struct options *options) {
  struct options _options;
  /* initialize environmental variable store */
  struct env_set *es = env_set_create(NULL);
  
  if(!options) options = &_options;
  
  /* initialize options to default state */
  init_options(options, true);

  /* parse command line options, and read configuration file */
  if(argc)
    parse_argv(options, argc, args, M_USAGE, OPT_P_DEFAULT, NULL, es);

  if(!options->dev)
    options->dev = "null";
  
  /* set verbosity and mute levels */
  set_check_status(D_LINK_ERRORS, D_READ_WRITE);
  set_debug_level(options->verbosity, SDL_CONSTRAIN);
  set_mute_cutoff(options->mute);

  /* set dev options */
  init_options_dev(options);

  /* sanity check on options */
  options_postprocess(options);

  /* show all option settings */
  show_settings(options);

  /* print version number */
  //msg(M_INFO, "%s", title_string);
  show_library_versions(M_INFO);

  /* misc stuff */
  pre_setup(options);

  /* set certain options as environmental variables */
  setenv_settings(es, options);

  return es;
}

struct frame *openvpn_setup_frame(struct options *options, struct tuntap **tt, struct link_socket **sock, struct env_set *es, u_char *flags, tme_event_set_t **event_set) {
  struct frame *frame = tme_new0(struct frame, 1);
  struct link_socket_addr *lsa = tme_new0(struct link_socket_addr, 1);

  /*
   * Adjust frame size based on the --tun-mtu-extra parameter.
   */
  if(options->ce.tun_mtu_extra_defined)
    tun_adjust_frame_parameters(frame, options->ce.tun_mtu_extra);

  /*
   * Adjust frame size based on link socket parameters.
   * (Since TCP is a stream protocol, we need to insert
   * a packet length uint16_t in the buffer.)
   */
  socket_adjust_frame_parameters(frame, options->ce.proto);

  /* See frame_finalize_options (struct context *c, const struct options *o) */
  frame_align_to_extra_frame(frame);
  frame_or_align_flags(frame,
		       FRAME_HEADROOM_MARKER_FRAGMENT
		       |FRAME_HEADROOM_MARKER_READ_LINK
		       |FRAME_HEADROOM_MARKER_READ_STREAM);
  
  frame_finalize(frame,
		 options->ce.link_mtu_defined,
		 options->ce.link_mtu,
		 options->ce.tun_mtu_defined,
		 options->ce.tun_mtu);

  /* packets with peer-id (P_DATA_V2) need 3 extra bytes in frame (on client)
   * and need link_mtu+3 bytes on socket reception (on server).
   *
   * accomodate receive path in f->extra_link
   *            send path in f->extra_buffer (+leave room for alignment)
   *
   * f->extra_frame is adjusted when peer-id option is push-received
   */
  frame_add_to_extra_link(frame, 3);
  frame_add_to_extra_buffer(frame, 8);

  if(do_setup_fast_io(options))
    *flags |= OPENVPN_FAST_IO;
  
  if(event_set) {
    int maxevents = 0;
    if(sock) maxevents++;
    if(tt) maxevents++;
    *event_set = tme_event_set_init(&maxevents, EVENT_METHOD_FAST);
  }

  sig = &siginfo_static;
  
  if(sock) *sock = setup_link_socket(frame, lsa, options);
  
  /* tun/tap persist command? */
  if(!do_persist_tuntap(options) && tt)
    *tt = setup_tuntap(frame, lsa, options, es);

  return frame;
}
