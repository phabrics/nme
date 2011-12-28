/* $Id: bsd-if.c,v 1.3 2003/10/16 02:48:23 fredette Exp $ */

/* host/bsd/bsd-if.c - BSD interface support: */

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
_TME_RCSID("$Id: bsd-if.c,v 1.3 2003/10/16 02:48:23 fredette Exp $");

/* includes: */
#include "bsd-impl.h"
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <net/if.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#if defined(HAVE_SYS_SOCKIO_H)
#include <sys/sockio.h>
#elif defined(HAVE_SYS_SOCKETIO_H)
#include <sys/socketio.h> 
#endif /* HAVE_SYS_SOCKETIO_H */
#include <sys/ioctl.h>
#ifdef HAVE_IOCTLS_H
#include <ioctls.h>
#endif /* HAVE_IOCTLS_H */
#ifdef HAVE_NET_IF_ETHER_H
#include <net/if_ether.h>
#endif /* HAVE_NET_IF_ETHER_H */
#ifdef HAVE_NET_ETHERNET_H
#include <net/ethernet.h>
#endif /* HAVE_NET_ETHERNET_H */
#include <netinet/ip.h>
#ifdef HAVE_NET_IF_DL_H
#include <net/if_dl.h>
#endif /* HAVE_NET_IF_DL_H */
#include <arpa/inet.h>

/* this macro helps us size a struct ifreq: */
#ifdef HAVE_SOCKADDR_SA_LEN
#define SIZEOF_IFREQ(ifr) (sizeof(ifr->ifr_name) + ifr->ifr_addr.sa_len)
#else  /* !HAVE_SOCKADDR_SA_LEN */
#define SIZEOF_IFREQ(ifr) (sizeof(ifr->ifr_name) + sizeof(struct sockaddr))
#endif /* !HAVE_SOCKADDR_SA_LEN */

/* this finds a network interface: */
int
tme_bsd_if_find(const char *ifr_name_user, struct ifreq **_ifreq, tme_uint8_t **_if_addr, unsigned int *_if_addr_size)
{
  int saved_errno;
  int dummy_fd;
  char ifreq_buffer[16384];	/* FIXME - magic constant. */
  struct ifconf ifc;
  struct ifreq *ifr;
  struct ifreq *ifr_user;
  size_t ifr_offset;
  struct sockaddr_in saved_ip_address;
  short saved_flags;
#ifdef HAVE_AF_LINK
  struct ifreq *link_ifreqs[20];	/* FIXME - magic constant. */
  size_t link_ifreqs_count;
  size_t link_ifreqs_i;
  struct sockaddr_dl *sadl;
#endif				/* HAVE_AF_LINK */

  /* make a dummy socket so we can read the interface list: */
  if ((dummy_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    return (-1);
  }

  /* read the interface list: */
  ifc.ifc_len = sizeof(ifreq_buffer);
  ifc.ifc_buf = ifreq_buffer;
  if (ioctl(dummy_fd, SIOCGIFCONF, &ifc) < 0) {
    saved_errno = errno;
    close(dummy_fd);
    errno = saved_errno;
    return (-1);
  }

#ifdef HAVE_AF_LINK
  /* start our list of link address ifreqs: */
  link_ifreqs_count = 0;
#endif /* HAVE_AF_LINK */

  /* walk the interface list: */
  ifr_user = NULL;
  for (ifr_offset = 0;; ifr_offset += SIZEOF_IFREQ(ifr)) {

    /* stop walking if we have run out of space in the buffer.  note
       that before we can use SIZEOF_IFREQ, we have to make sure that
       there is a minimum number of bytes in the buffer to use it
       (namely, that there's a whole struct sockaddr available): */
    ifr = (struct ifreq *) (ifreq_buffer + ifr_offset);
    if (((ifr_offset
	 + sizeof(ifr->ifr_name)
	 + sizeof(struct sockaddr))
	 > (size_t) ifc.ifc_len)
	|| ((ifr_offset
	     + SIZEOF_IFREQ(ifr))
	    > (size_t) ifc.ifc_len)) {
      errno = ENOENT;
      break;
    }

#ifdef HAVE_AF_LINK
    /* if this is a hardware address, save it: */
    if (ifr->ifr_addr.sa_family == AF_LINK) {
      if (link_ifreqs_count < TME_ARRAY_ELS(link_ifreqs)) {
	link_ifreqs[link_ifreqs_count++] = ifr;
      }
      continue;
    }
#endif /* HAVE_AF_LINK */

    /* ignore this interface if it doesn't do IP: */
    /* XXX is this actually important? */
    if (ifr->ifr_addr.sa_family != AF_INET) {
      continue;
    }

    /* get the interface flags, preserving the IP address in the
       struct ifreq across the call: */
    saved_ip_address = *((struct sockaddr_in *) & ifr->ifr_addr);
    if (ioctl(dummy_fd, SIOCGIFFLAGS, ifr) < 0) {
      ifr = NULL;
      break;
    }
    saved_flags = ifr->ifr_flags;
    *((struct sockaddr_in *) & ifr->ifr_addr) = saved_ip_address;

    /* ignore this interface if it isn't up and running: */
    if ((saved_flags & (IFF_UP | IFF_RUNNING))
	!= (IFF_UP | IFF_RUNNING)) {
      continue;
    }

    /* if we don't have an interface yet, take this one depending on
       whether the user asked for an interface by name or not.  if he
       did, and this is it, take this one.  if he didn't, and this
       isn't a loopback interface, take this one: */
    if (ifr_user == NULL
	&& (ifr_name_user != NULL
	    ? !strncmp(ifr->ifr_name, ifr_name_user, sizeof(ifr->ifr_name))
	    : !(ifr->ifr_flags & IFF_LOOPBACK))) {
      ifr_user = ifr;
    }
  }

  /* close the dummy socket: */
  saved_errno = errno;
  close(dummy_fd);
  errno = saved_errno;

  /* if we don't have an interface to return: */
  if (ifr_user == NULL) {
    return (errno);
  }

  /* return this interface: */
  *_ifreq = (struct ifreq *) tme_memdup(ifr_user, SIZEOF_IFREQ(ifr_user));

  /* assume that we can't find this interface's hardware address: */
  if (_if_addr != NULL) {
    *_if_addr = NULL;
  }
  if (_if_addr_size != NULL) {
    *_if_addr_size = 0;
  }

#ifdef HAVE_AF_LINK

  /* try to find an AF_LINK ifreq that gives us the interface's
     hardware address: */
  ifr = NULL;
  for (link_ifreqs_i = 0;
       link_ifreqs_i < link_ifreqs_count;
       link_ifreqs_i++) {
    if (!strncmp(link_ifreqs[link_ifreqs_i]->ifr_name,
		 ifr_user->ifr_name,
		 sizeof(ifr_user->ifr_name))) {
      ifr = link_ifreqs[link_ifreqs_i];
      break;
    }
  }

  /* if we found one, return the hardware address: */
  if (ifr != NULL) {
    sadl = (struct sockaddr_dl *) &ifr->ifr_addr;
    if (_if_addr_size != NULL) {
      *_if_addr_size = sadl->sdl_alen;
    }
    if (_if_addr != NULL) {
      *_if_addr = tme_new(tme_uint8_t, sadl->sdl_alen);
      memcpy(*_if_addr, LLADDR(sadl), sadl->sdl_alen);
    }
  }

#endif /* HAVE_AF_LINK */

  /* done: */
  return (TME_OK);
}
