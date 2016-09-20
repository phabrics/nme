/* host/eth/eth-impl.c - Common Ethernet host interface implementation */

/*
 * Copyright (c) 2014, 2015, 2016 Ruben Agin
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
_TME_RCSID("$Id: eth-if.c,v 1.3 2003/10/16 02:48:23 fredette Exp $");

/* includes: */
#include "eth-if.h"

/* macros: */
/* the callout flags: */
#define TME_ETH_CALLOUT_CHECK	(0)
#define TME_ETH_CALLOUT_RUNNING	TME_BIT(0)
#define TME_ETH_CALLOUTS_MASK	(-2)
#define TME_ETH_CALLOUT_CTRL	TME_BIT(1)
#define TME_ETH_CALLOUT_READ	TME_BIT(2)
#define TME_ETH_CALLOUT_CONFIG	TME_BIT(3)

/* ARP and RARP opcodes: */
#define TME_NET_ARP_OPCODE_REQUEST	(0x0001)
#define TME_NET_ARP_OPCODE_REPLY	(0x0002)
#define TME_NET_ARP_OPCODE_REV_REQUEST	(0x0003)
#define TME_NET_ARP_OPCODE_REV_REPLY	(0x0004)

/* this macro helps us size a struct ifreq: */
#ifdef HAVE_SOCKADDR_SA_LEN
#define SIZEOF_IFREQ(ifr) \
  ( sizeof(ifr->ifr_name) + (ifr->ifr_addr.sa_len > sizeof(ifr->ifr_ifru) ? ifr->ifr_addr.sa_len : sizeof(ifr->ifr_ifru)) )
#else  /* !HAVE_SOCKADDR_SA_LEN */
//#define SIZEOF_IFREQ(ifr) (sizeof(ifr->ifr_name) + sizeof(struct sockaddr))
#define SIZEOF_IFREQ(ifr) (sizeof(struct ifreq))
#endif /* !HAVE_SOCKADDR_SA_LEN */

/* a crude ARP header: */
struct tme_net_arp_header {
  tme_uint8_t tme_net_arp_header_hardware[2];
  tme_uint8_t tme_net_arp_header_protocol[2];
  tme_uint8_t tme_net_arp_header_hardware_length;
  tme_uint8_t tme_net_arp_header_protocol_length;
  tme_uint8_t tme_net_arp_header_opcode[2];
};

/* a crude partial IPv4 header: */
struct tme_net_ipv4_header {
  tme_uint8_t tme_net_ipv4_header_v_hl;
  tme_uint8_t tme_net_ipv4_header_tos;
  tme_uint8_t tme_net_ipv4_header_length[2];
};

/* the ethernet callout function.  it must be called with the mutex locked: */
static void
_tme_eth_callout(struct tme_ethernet *eth, int new_callouts)
{
  struct tme_ethernet_connection *conn_eth;
  int callouts, later_callouts;
  unsigned int ctrl;
  int rc;
  int status;
  tme_ethernet_fid_t frame_id;
  struct tme_ethernet_frame_chunk frame_chunk_buffer;
  struct tme_ethernet_config config;
  const tme_uint8_t *config_addrs[2];
  
  /* add in any new callouts: */
  eth->tme_eth_callout_flags |= new_callouts;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (eth->tme_eth_callout_flags & TME_ETH_CALLOUT_RUNNING) {
    return;
  }
  
  /* callouts are now running: */
  eth->tme_eth_callout_flags |= TME_ETH_CALLOUT_RUNNING;

  /* assume that we won't need any later callouts: */
  later_callouts = 0;

  /* loop while callouts are needed: */
  for (; (callouts = eth->tme_eth_callout_flags) & TME_ETH_CALLOUTS_MASK; ) {

    /* clear the needed callouts: */
    eth->tme_eth_callout_flags = callouts & ~TME_ETH_CALLOUTS_MASK;
    callouts &= TME_ETH_CALLOUTS_MASK;

    /* get our Ethernet connection: */
    conn_eth = eth->tme_eth_eth_connection;

    /* if we need to call out new config information: */
    if (callouts & TME_ETH_CALLOUT_CONFIG) {
      
      /* form the new config: */
      memset(&config, 0, sizeof(config));

      /* if we're in promiscuous mode or any bits the logical address
	 filter are nonzero: */
      if (!eth->tme_eth_addr) {
	/* we have to be in promiscuous mode: */
	config.tme_ethernet_config_flags |= TME_ETHERNET_CONFIG_PROMISC;
      }

      /* otherwise, we only need to see packets addressed to our
	 physical address, and broadcast packets: */
      else {
      
	/* our Ethernet addresses: */
	config.tme_ethernet_config_addr_count = 2;
	config_addrs[0] = eth->tme_eth_addr;
	config_addrs[1] = &tme_ethernet_addr_broadcast[0];
	config.tme_ethernet_config_addrs = config_addrs;
      }

      /* unlock the mutex: */
      tme_mutex_unlock(&eth->tme_eth_mutex);
      
      /* do the callout: */
      rc = (conn_eth == NULL
	    ? TME_OK
	    : ((*conn_eth->tme_ethernet_connection_config)
	       (conn_eth,
		&config)));
      assert (rc == TME_OK);
      
      /* lock the mutex: */
      tme_mutex_lock(&eth->tme_eth_mutex);
    }
    
    /* if we need to call out new control information: */
    if (callouts & TME_ETH_CALLOUT_CTRL) {

      /* form the new ctrl: */
      ctrl = 0;
      if (eth->tme_eth_buffer_offset
	  < eth->tme_eth_buffer_end) {
	ctrl |= TME_ETHERNET_CTRL_OK_READ;
      }

      /* unlock the mutex: */
      tme_mutex_unlock(&eth->tme_eth_mutex);
      
      /* do the callout: */
      rc = (conn_eth != NULL
	    ? ((*conn_eth->tme_ethernet_connection_ctrl)
	       (conn_eth,
		ctrl))
	    : TME_OK);
	
      /* lock the mutex: */
      tme_mutex_lock(&eth->tme_eth_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_ETH_CALLOUT_CTRL;
      }
    }
      
    /* if the Ethernet is readable: */
    if (callouts & TME_ETH_CALLOUT_READ) {

      /* unlock the mutex: */
      tme_mutex_unlock(&eth->tme_eth_mutex);
      
      /* make a frame chunk to receive this frame: */
      frame_chunk_buffer.tme_ethernet_frame_chunk_next = NULL;

      frame_chunk_buffer.tme_ethernet_frame_chunk_bytes
	= eth->tme_eth_out;

      frame_chunk_buffer.tme_ethernet_frame_chunk_bytes_count =
	eth->tme_eth_buffer_size;
      
      /* do the callout: */
      rc = (conn_eth == NULL
	    ? TME_OK
	    : ((*conn_eth->tme_ethernet_connection_read)
	       (conn_eth,
		&frame_id,
		&frame_chunk_buffer,
		TME_ETHERNET_READ_NEXT)));
      
      /* lock the mutex: */
      tme_mutex_lock(&eth->tme_eth_mutex);
      
      /* if the read was successful: */
      if (rc > 0) {

	/* check the size of the frame: */
	assert(rc <= eth->tme_eth_buffer_size);

	/* set the length: */
	eth->tme_eth_data_length = rc;
	
	/* do the write: */
	status =
	  (eth->tme_eth_handle != TME_INVALID_HANDLE) ?
	  (tme_thread_write_yield(eth->tme_eth_handle, eth->tme_eth_out, rc, &eth->tme_eth_mutex)) :
	  (eth->tme_ethernet_write(eth->tme_eth_data));
      
	/* writes must succeed: */
	assert (status == rc);

	/* mark that we need to loop to callout to read more frames: */
	eth->tme_eth_callout_flags |= TME_ETH_CALLOUT_READ;
      }

      /* otherwise, the read failed.  convention dictates that we
	 forget that the connection was readable, which we already
	 have done by clearing the CALLOUT_READ flag: */
    }

  }

  /* put in any later callouts, and clear that callouts are running: */
  eth->tme_eth_callout_flags = later_callouts;
}

/* the ETH reader thread: */
static _tme_thret
_tme_eth_th_reader(struct tme_ethernet *eth)
{
  ssize_t buffer_end;
  unsigned long sleep_usec;
  const struct tme_ethernet_header *ethernet_header;
  
  tme_thread_enter(&eth->tme_eth_mutex);

  /* loop forever: */
  for (;;) {

    /* if the delay sleeping flag is set: */
    if (eth->tme_eth_delay_sleeping) {

      /* clear the delay sleeping flag: */
      eth->tme_eth_delay_sleeping = FALSE;
      
      /* call out that we can be read again: */
      _tme_eth_callout(eth, TME_ETH_CALLOUT_CTRL);
    }

    /* if a delay has been requested: */
    sleep_usec = eth->tme_eth_delay_sleep;
    if (sleep_usec > 0) {

      /* clear the delay sleep time: */
      eth->tme_eth_delay_sleep = 0;

      /* set the delay sleeping flag: */
      eth->tme_eth_delay_sleeping = TRUE;

      /* sleep for the delay sleep time: */
      tme_thread_sleep_yield(0, sleep_usec, &eth->tme_eth_mutex);
      
      continue;
    }

    /* if the buffer is not empty, wait until either it is,
       or we're asked to do a delay: */
    if (eth->tme_eth_buffer_offset
	< eth->tme_eth_buffer_end) {
      tme_cond_wait_yield(&eth->tme_eth_cond_reader,
			  &eth->tme_eth_mutex);
      continue;
    }

    /* read the ETH socket: */
    tme_log(&eth->tme_eth_element->tme_element_log_handle, 1, TME_OK,
	    (&eth->tme_eth_element->tme_element_log_handle,
	     _("calling read")));

    buffer_end =
      (eth->tme_eth_handle != TME_INVALID_HANDLE) ?
      (tme_thread_read_yield(eth->tme_eth_handle,
			     eth->tme_eth_buffer,
			     eth->tme_eth_buffer_size,
			     &eth->tme_eth_mutex)) :
      (eth->tme_ethernet_read(eth->tme_eth_data));
    
    /* if the read failed: */
    if(buffer_end <= 0) {
      tme_log(&eth->tme_eth_element->tme_element_log_handle, 1, errno,
	      (&eth->tme_eth_element->tme_element_log_handle,
	       _("failed to read/write packets")));
      continue;
    }
    
    /* Filter out multicast packets we sent or unicast packets not destined for us. 
       This should remove all duplicate packets on, i.e., tap interfaces...
     */
    ethernet_header = (struct tme_ethernet_header *) (eth->tme_eth_buffer);
    
    if(!eth->tme_eth_addr ||
       (((ethernet_header->tme_ethernet_header_dst[0] & 0x1)
	 && memcmp(eth->tme_eth_addr,
		   ethernet_header->tme_ethernet_header_src,
		   TME_ETHERNET_ADDR_SIZE))
	||!memcmp(eth->tme_eth_addr,
		  ethernet_header->tme_ethernet_header_dst,
		  TME_ETHERNET_ADDR_SIZE))) {
      /* the read succeeded: */
      tme_log(&eth->tme_eth_element->tme_element_log_handle, 1, TME_OK,
	      (&eth->tme_eth_element->tme_element_log_handle,
	       _("read %ld bytes of packets"), (long) buffer_end));
      eth->tme_eth_buffer_offset = 0;
      eth->tme_eth_buffer_end = buffer_end;
      _tme_eth_callout(eth, TME_ETH_CALLOUT_CTRL);
    }
  }
  /* NOTREACHED */
  tme_thread_exit();
}

/* this makes a new Ethernet connection: */
static int
_tme_eth_connection_make(struct tme_connection *conn, unsigned int state)
{
  struct tme_ethernet *eth;
  struct tme_ethernet_connection *conn_eth;
  struct tme_ethernet_connection *conn_eth_other;

  /* recover our data structures: */
  eth = conn->tme_connection_element->tme_element_private;
  conn_eth = (struct tme_ethernet_connection *) conn;
  conn_eth_other = (struct tme_ethernet_connection *) conn->tme_connection_other;

  /* both sides must be Ethernet connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_ETHERNET);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_ETHERNET);

  /* we're always set up to answer calls across the connection, so we
     only have to do work when the connection has gone full, namely
     taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock our mutex: */
    tme_mutex_lock(&eth->tme_eth_mutex);

    /* save our connection: */
    eth->tme_eth_eth_connection = conn_eth_other;

    /* unlock our mutex: */
    tme_mutex_unlock(&eth->tme_eth_mutex);
  }

  return (TME_OK);
}

/* this breaks a connection: */
static int
_tme_eth_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this is called when the ethernet configuration changes: */
static int
_tme_eth_config(struct tme_ethernet_connection *conn_eth, 
		    struct tme_ethernet_config *config)
{
  struct tme_ethernet *eth;

  /* recover our data structures: */
  eth = conn_eth->tme_ethernet_connection.tme_connection_element->tme_element_private;

  /* if this Ethernet is promiscuous, we will accept all packets: */
  if(!(config->tme_ethernet_config_flags & TME_ETHERNET_CONFIG_PROMISC ||
       (eth->tme_eth_addr && (config->tme_ethernet_config_addr_count > 0) &&
	memcmp(eth->tme_eth_addr,
	       config->tme_ethernet_config_addrs[0],
	       TME_ETHERNET_ADDR_SIZE))))
    return TME_OK;
  
  tme_free(eth->tme_eth_addr);
  eth->tme_eth_addr = NULL;    
  
  return TME_OK;
}

/* this is called when control lines change: */
static int
_tme_eth_ctrl(struct tme_ethernet_connection *conn_eth, 
		  unsigned int ctrl)
{
  struct tme_ethernet *eth;
  int new_callouts;

  /* recover our data structures: */
  eth = conn_eth->tme_ethernet_connection.tme_connection_element->tme_element_private;

  /* assume that we won't need any new callouts: */
  new_callouts = 0;

  /* lock the mutex: */
  tme_mutex_lock(&eth->tme_eth_mutex);

  /* if this connection is readable, call out a read: */
  if (ctrl & TME_ETHERNET_CTRL_OK_READ) {
    new_callouts |= TME_ETH_CALLOUT_READ;
  }

  /* make any new callouts: */
  _tme_eth_callout(eth, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&eth->tme_eth_mutex);

  return (TME_OK);
}

/* this is called to read a frame: */
static int
_tme_eth_read(struct tme_ethernet_connection *conn_eth, 
	      tme_ethernet_fid_t *_frame_id,
	      struct tme_ethernet_frame_chunk *frame_chunks,
	      unsigned int flags)
{
  struct tme_ethernet *eth;
  struct tme_ethernet_frame_chunk frame_chunk_buffer;
  size_t buffer_offset_next;
  unsigned int count;
  int rc;

  /* recover our data structure: */
  eth = conn_eth->tme_ethernet_connection.tme_connection_element->tme_element_private;

  /* lock our mutex: */
  tme_mutex_lock(&eth->tme_eth_mutex);

  /* assume that we won't be able to return a packet: */
  rc = -ENOENT;

  /* loop until we have a good captured packet or until we 
     exhaust the buffer: */
  for (;;) {
    buffer_offset_next = eth->tme_eth_buffer_end;
    /* if there's not enough for a ETH header, flush the buffer: */
    if (eth->tme_eth_buffer_offset >= eth->tme_eth_buffer_end)
    {
      if (eth->tme_eth_buffer_offset
	  != eth->tme_eth_buffer_end) {
	tme_log(&eth->tme_eth_element->tme_element_log_handle, 1, TME_OK,
		(&eth->tme_eth_element->tme_element_log_handle,
		 _("flushed garbage ETH header bytes")));
	eth->tme_eth_buffer_offset = eth->tme_eth_buffer_end;
      }
      break;
    }

    /* form the single frame chunk: */
    frame_chunk_buffer.tme_ethernet_frame_chunk_next = NULL;
    frame_chunk_buffer.tme_ethernet_frame_chunk_bytes
      = eth->tme_eth_buffer + eth->tme_eth_buffer_offset;
    frame_chunk_buffer.tme_ethernet_frame_chunk_bytes_count
      = buffer_offset_next;

    /* filter out the frame: */
    count = tme_ethernet_chunks_copy(frame_chunks, &frame_chunk_buffer);

    /* if this is a peek: */
    if (flags & TME_ETHERNET_READ_PEEK) {
    }

    /* otherwise, this isn't a peek: */
    else {

      /* update the buffer pointer: */
      eth->tme_eth_buffer_offset = buffer_offset_next;
    }

    /* success: */
    rc = count;
    break;
  }

  /* if the buffer is empty, or if we failed to read a packet,
     wake up the reader: */
  if ((eth->tme_eth_buffer_offset
       >= eth->tme_eth_buffer_end)
      || rc <= 0) {
    tme_cond_notify(&eth->tme_eth_cond_reader, TRUE);
  }

  /* unlock our mutex: */
  tme_mutex_unlock(&eth->tme_eth_mutex);

  /* done: */
  return (rc);
}

#if 0
/* this finds a network interface via traditional ioctls: */
int
tme_eth_if_find(const char *ifr_name_user, struct ifreq **_ifreq, tme_uint8_t **_if_addr, unsigned int *_if_addr_size)
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

    /*
    if (ioctl(dummy_fd, SIOCGIFINDEX, ifr) < 0) {
      ifr = NULL;
      break;
    }
    */

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
	    : !(saved_flags & IFF_LOOPBACK))) {
      ifr_user = ifr;
      break;
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
#endif // 0

#ifdef HAVE_IFADDRS_H
/* this finds a network interface via the ifaddrs api: */
int
tme_eth_ifaddrs_find(const char *ifa_name_user, int family, struct ifaddrs **_ifaddr, tme_uint8_t **_if_addr, unsigned int *_if_addr_size)
{
  struct ifaddrs *ifaddr, *ifa;
  struct ifaddrs *ifa_user = NULL;
  char ifa_name[INET6_ADDRSTRLEN];
#if defined(HAVE_AF_LINK) || defined(HAVE_AF_PACKET)
  struct ifaddrs *link_ifaddrs[20];	/* FIXME - magic constant. */
  size_t link_ifaddrs_count;
  size_t link_ifaddrs_i;
#ifdef HAVE_AF_LINK
  #define AF_HWADDR AF_LINK
  struct sockaddr_dl *sadl;
#else				/* HAVE_AF_LINK */
#ifdef HAVE_AF_PACKET
  #define AF_HWADDR AF_PACKET
  struct sockaddr_ll *sadl;
#endif				/* HAVE_AF_PACKET */
#endif 
#endif 

  if (getifaddrs(&ifaddr) == -1) {
    return (-1);
  }

#ifdef AF_HWADDR
  /* start our list of link address ifaddrs: */
  link_ifaddrs_count = 0;
#endif 

  /* Walk through linked list, maintaining head pointer so we
     can free list later */

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;

#ifdef AF_HWADDR
    /* if this is a hardware address, save it: */
    if (ifa->ifa_addr->sa_family == AF_HWADDR) {
      if (link_ifaddrs_count < TME_ARRAY_ELS(link_ifaddrs)) {
	link_ifaddrs[link_ifaddrs_count++] = ifa;
      }
      continue;
    }
#endif 

    /* ignore this interface if it isn't up and running: */
    if ((ifa->ifa_flags & (IFF_UP | IFF_RUNNING))
	!= (IFF_UP | IFF_RUNNING)) {
      continue;
    }

    switch(family) {
    case AF_UNSPEC:
      strncpy(ifa_name, ifa->ifa_name, IFNAMSIZ);
      break;
    case AF_INET:
      if(ifa->ifa_addr->sa_family != AF_INET) continue;
      inet_ntop(family, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, ifa_name, INET_ADDRSTRLEN);
      break;
    case AF_INET6:
      if(ifa->ifa_addr->sa_family != AF_INET6) continue;
      inet_ntop(family, &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr, ifa_name, INET6_ADDRSTRLEN);
      break;
    default:
      return EAFNOSUPPORT;
    }
    
    /* if we don't have an interface yet, take this one depending on
       whether the user asked for an interface by name or not.  if he
       did, and this is it, take this one.  if he didn't, and this
       isn't a loopback interface, take this one: */
    if ((ifa_name_user != NULL && strlen(ifa_name_user))
	? !strncmp(ifa_name, ifa_name_user, strlen(ifa_name_user))
	: !(ifa->ifa_flags & IFF_LOOPBACK)) {
      ifa_user = ifa;
      break;
    }
    
  }
  
  /* if we don't have an interface to return: */
  if (ifa_user == NULL) {
    return ENOENT;
  }
  
  /* return this interface: */
  if (_ifaddr != NULL)
    *_ifaddr = (struct ifaddrs *) tme_memdup(ifa_user, sizeof(struct ifaddrs));
    
  /* assume that we can't find this interface's hardware address: */
  if (_if_addr != NULL) {
      *_if_addr = NULL;
  }
  if (_if_addr_size != NULL) {    
    *_if_addr_size = 0;
  }

#ifdef AF_HWADDR

  /* try to find an AF_LINK ifreq that gives us the interface's
     hardware address: */
  ifa = NULL;
  for (link_ifaddrs_i = 0;
       link_ifaddrs_i < link_ifaddrs_count;
       link_ifaddrs_i++) {
    if (!strncmp(link_ifaddrs[link_ifaddrs_i]->ifa_name,
		 ifa_user->ifa_name,
		 IFNAMSIZ)) {
      ifa = link_ifaddrs[link_ifaddrs_i];
      break;
    }
  }

  /* if we found one, return the hardware address: */
  if (ifa != NULL) {
#ifdef HAVE_AF_LINK
    sadl = (struct sockaddr_dl *) ifa->ifa_addr;
    if (_if_addr_size != NULL) {
      *_if_addr_size = sadl->sdl_alen;
    }
    if (_if_addr != NULL) {
      *_if_addr = tme_new(tme_uint8_t, sadl->sdl_alen);
      memcpy(*_if_addr, LLADDR(sadl), sadl->sdl_alen);
    }
#else
    sadl = (struct sockaddr_ll *) ifa->ifa_addr;
    if (_if_addr_size != NULL)
      *_if_addr_size = sadl->sll_halen;
    if (_if_addr != NULL) {
      *_if_addr = tme_new(tme_uint8_t, sadl->sll_halen);
      memcpy(*_if_addr, sadl->sll_addr, sadl->sll_halen);
    }
#endif
  }

#endif /* AF_HWADDR */

  //  freeifaddrs(ifaddr);

  /* done: */
  return (TME_OK);
}
#endif // HAVE_IFADDRS_H

/* Allocate an ethernet device */
int tme_eth_alloc(char *dev_filename, char **_output)
{
  int fd, minor;
  char dev_minor[4];
  char *dev_fn;

  /* loop trying to open a minor device: */
  minor = 0;
  dev_fn = tme_strdup(dev_filename);

  tme_output_append_error(_output, "trying %s\n", dev_filename);

  /* open the clone device */
  while( ((fd = open(dev_filename, O_RDWR)) < 0 )) {
    /* we failed to open this device.  if this device was simply
       busy, loop: */
    tme_output_append_error(_output, "failed opening %s: %d - %s\n", dev_filename, errno, strerror(errno));
    if(errno != EBUSY &&
       errno != EACCES &&
       errno != EPERM &&
       minor)
      break;

    /* form the name of the next device to try, then try opening
       it. if we succeed, we're done: */
    sprintf(dev_filename, "%s%d", dev_fn, minor++);
    tme_output_append_error(_output, "trying %s\n", dev_filename);
  }

  if(fd>=0) tme_output_append_error(_output, "successfully opened %s\n", dev_filename);

  tme_free(dev_fn);
  return fd;
}

/* this makes a new connection side for a ETH: */
int
tme_eth_connections_new(struct tme_element *element, 
			const char * const *args, 
			struct tme_connection **_conns)
{
  struct tme_ethernet *eth;
  struct tme_ethernet_connection *conn_eth;
  struct tme_connection *conn;

  /* recover our data structure: */
  eth = (struct tme_ethernet *) element->tme_element_private;

  /* if we already have an Ethernet connection, do nothing: */
  if (eth->tme_eth_eth_connection != NULL) {
    return (TME_OK);
  }

  /* allocate the new Ethernet connection: */
  conn_eth = tme_new0(struct tme_ethernet_connection, 1);
  conn = &conn_eth->tme_ethernet_connection;
  
  /* fill in the generic connection: */
  conn->tme_connection_next = *_conns;
  conn->tme_connection_type = TME_CONNECTION_ETHERNET;
  conn->tme_connection_score = tme_ethernet_connection_score;
  conn->tme_connection_make = _tme_eth_connection_make;
  conn->tme_connection_break = _tme_eth_connection_break;
  
  /* fill in the Ethernet connection: */
  conn_eth->tme_ethernet_connection_config = _tme_eth_config;
  conn_eth->tme_ethernet_connection_ctrl = _tme_eth_ctrl;
  conn_eth->tme_ethernet_connection_read = _tme_eth_read;

  /* return the connection side possibility: */
  *_conns = conn;

  /* done: */
  return (TME_OK);
}

int tme_eth_init(struct tme_element *element, 
		 tme_thread_handle_t hand,
		 unsigned int sz, 
		 void *data,
		 unsigned char *addr,
		 typeof(tme_eth_connections_new) eth_connections_new)
{
  struct tme_ethernet *eth;
  
  /* start our data structure: */
  eth = tme_new0(struct tme_ethernet, 1);
  eth->tme_eth_element = element;
  eth->tme_eth_handle = hand;
  if(hand != TME_INVALID_HANDLE) {
    eth->tme_eth_buffer = tme_new(tme_uint8_t, sz);
    eth->tme_eth_out = tme_new(tme_uint8_t, TME_ETHERNET_FRAME_MAX);
  }
  eth->tme_eth_buffer_size = sz;
  eth->tme_eth_buffer_offset = 0;
  eth->tme_eth_buffer_end = 0;
  eth->tme_eth_data = data;
  eth->tme_eth_addr = addr;
  eth->tme_eth_callout_flags = TME_ETH_CALLOUT_CONFIG;
  
  /* start the threads: */
  tme_mutex_init(&eth->tme_eth_mutex);
  tme_cond_init(&eth->tme_eth_cond_reader);
  tme_thread_create(&eth->tme_eth_thread, (tme_thread_t) _tme_eth_th_reader, eth);

  /* fill the element: */
  element->tme_element_private = eth;
  element->tme_element_connections_new =
    (eth_connections_new) ?
    (eth_connections_new) :
    (tme_eth_connections_new);

  return (TME_OK);
}
