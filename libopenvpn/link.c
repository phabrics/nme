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

/* includes: */
#ifdef HAVE_CONFIG_H
#include "config.h"
#elif defined(_MSC_VER)
#include "config-msvc.h"
#endif

#include "syshead.h"
#include "link.h"

const struct link_socket *accept_from; /* possibly do accept() on a parent link_socket */

struct link_socket *setup_link_socket(struct frame *frame, struct link_socket_addr *lsa, struct options *options, struct signal_info *sig) {

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
bool do_setup_fast_io(struct options *options) {
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
