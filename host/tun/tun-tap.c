/* $Id: tun-tap.c,v 1.9 2007/02/21 01:24:50 fredette Exp $ */

/* host/tun/tun-tap.c - TUN TAP Ethernet support: */

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
_TME_RCSID("$Id: tun-tap.c,v 1.9 2007/02/21 01:24:50 fredette Exp $");

/* includes: */
#include <tme/generic/ethernet.h>
#include <tme/threads.h>
#include <tme/misc.h>
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include "eth-if.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>	/* for offsetof */
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/param.h>
#ifdef HAVE_SYS_LINKER_H
#include <sys/linker.h>
#endif
#include <sys/socket.h>
#include <sys/stat.h>
#include <net/if.h>
#ifdef HAVE_NET_IF_VAR_H
#include <net/if_var.h>
#endif
#ifdef HAVE_NET_IF_TYPES_H
#include <net/if_types.h>
#endif
#include <netinet/in_systm.h>
#include <netinet/in.h>
#ifdef HAVE_NETINET_IN_VAR_H
#include <netinet/in_var.h>
#endif
#if defined(HAVE_SYS_SOCKIO_H)
#include <sys/sockio.h>
#elif defined(HAVE_SYS_SOCKETIO_H)
#include <sys/socketio.h> 
#endif /* HAVE_SYS_SOCKETIO_H */
#include <sys/ioctl.h>
#ifdef HAVE_IOCTLS_H
#include <ioctls.h>
#endif /* HAVE_IOCTLS_H */
#ifdef HAVE_NET_IF_ARP_H
#include <net/if_arp.h>
#endif
#ifdef HAVE_NETINET_IF_ETHER_H
#include <netinet/if_ether.h>
#endif /* HAVE_NET_IF_ETHER_H */
#ifdef HAVE_NETPACKET_PACKET_H
#include <netpacket/packet.h>
#endif /* HAVE_NETPACKET_PACKET_H */
#ifdef HAVE_NET_ETHERNET_H
#include <net/ethernet.h>
#endif /* HAVE_NET_ETHERNET_H */
#include <netinet/ip.h>
#ifdef HAVE_NET_IF_DL_H
#include <net/if_dl.h>
#endif /* HAVE_NET_IF_DL_H */
#include <arpa/inet.h>
#ifdef HAVE_NET_IF_TAP_H
#include <net/if_tap.h>
#endif
#ifdef HAVE_NET_TAP_IF_TAP_H
#include <net/tap/if_tap.h>
#endif
#ifdef HAVE_NET_IF_TUN_H
#include <net/if_tun.h>
#endif
#ifdef HAVE_LINUX_IF_TUN_H
#include <linux/if_tun.h>
#define TME_TUN_TAP_INSN tme_uint8_t
#define TME_TUN_TAP_PROG struct tun_filter
#define TME_TUN_TAP_INSNS(x) (x)->addr
#define TME_TUN_TAP_LEN(x) (x)->count
#endif
#ifdef HAVE_LINUX_NETFILTER_H
#include <linux/netfilter.h>
#endif
#ifdef HAVE_LINUX_NETFILTER_NF_TABLES_H
#include <linux/netfilter/nf_tables.h>
#endif
#ifdef HAVE_LIBMNL_LIBMNL_H
#include <libmnl/libmnl.h>
#endif
#ifdef HAVE_LIBNFTNL_TABLE_H
#include <libnftnl/table.h>
#include <libnftnl/chain.h>
#include <libnftnl/rule.h>
#include <libnftnl/expr.h>
#endif
#ifdef HAVE_NPF_H
#include <npf.h>
#endif

/* macros: */
/* interface types: */
#define TME_IF_TYPE_TAP (0)
#define TME_IF_TYPE_NAT (1)
#define TME_IF_TYPE_TOTAL (2)

/* interface addresses: */
#define TME_IP_ADDRS_INET (0)
#define TME_IP_ADDRS_NETMASK (1)
#define TME_IP_ADDRS_BCAST (2)
#define TME_IP_ADDRS_TOTAL (3)

#ifdef HAVE_LIBNFTNL_TABLE_H
/* nat types: */
#define TME_NAT_TABLE (0)
#define TME_NAT_CHAIN (1)
#define TME_NAT_RULE (2)
#define TME_NAT_TOTAL (3)

#define IPFWDFILE "/proc/sys/net/ipv4/ip_forward"

struct tme_nat {
  uint8_t type;
  void *msg;
};

typedef struct nlmsghdr *(*nat_hdr_builder)(char *buf, uint16_t cmd, uint16_t family,
					    uint16_t type, uint32_t seq);

typedef void (*nat_msg_builder)(struct nlmsghdr *nlh, void *msg);

typedef uint32_t (*nat_attr_get)(void *msg, uint16_t attr);

struct nat_family {
  nat_attr_get get_family;
  uint16_t family_attr;
};

nat_hdr_builder nat_hdr_builders[] = {
  nft_table_nlmsg_build_hdr,
  nft_chain_nlmsg_build_hdr,
  nft_rule_nlmsg_build_hdr
};

uint16_t nat_cmds[] = {
  NFT_MSG_NEWTABLE,
  NFT_MSG_NEWCHAIN,
  NFT_MSG_NEWRULE
};

struct nat_family nat_families[] = {
  { (nat_attr_get)nft_table_attr_get_u32, NFT_TABLE_ATTR_FAMILY },
  { (nat_attr_get)nft_chain_attr_get_u32, NFT_CHAIN_ATTR_FAMILY },
  { (nat_attr_get)nft_rule_attr_get_u32, NFT_RULE_ATTR_FAMILY }
};

nat_msg_builder nat_msg_builders[] = {
  (nat_msg_builder)nft_table_nlmsg_build_payload,
  (nat_msg_builder)nft_chain_nlmsg_build_payload,
  (nat_msg_builder)nft_rule_nlmsg_build_payload
};

#endif

/* this is called when the ethernet configuration changes: */
static int
_tme_tun_tap_config(struct tme_ethernet_connection *conn_eth, 
		    struct tme_ethernet_config *config)
{
  struct tme_ethernet *tap;
#ifdef HAVE_LINUX_IF_TUN_H
  TME_TUN_TAP_INSN *tap_filter;
#endif
  int tap_filter_size;
  int rc;

  /* recover our data structures: */
  tap = conn_eth->tme_ethernet_connection.tme_connection_element->tme_element_private;

  /* assume we will succeed: */
  rc = TME_OK;

#ifdef HAVE_LINUX_IF_TUN_H
  /* lock the mutex: */
  tme_mutex_lock(&tap->tme_eth_mutex);

  /* allocate space for the worst-case filter: one insn for the packet
     accept, one insn for the packet reject, and TME_ETHERNET_ADDR_SIZE
     * 2 insns for each address - one insn to load an address byte and
     one insn to test it and branch: */
  tap_filter_size = TME_ETHERNET_ADDR_SIZE * config->tme_ethernet_config_addr_count;
  tap_filter = tme_new0(TME_TUN_TAP_INSN, tap_filter_size + sizeof(TME_TUN_TAP_PROG));

  /* if this Ethernet is promiscuous, we will accept all packets: */
  if (config->tme_ethernet_config_flags & TME_ETHERNET_CONFIG_PROMISC) {
    tap_filter_size = 0;
  }

  /* if this Ethernet does have a set of addresses, we will accept all
     packets for one of those addresses: */
  else if (config->tme_ethernet_config_addr_count > 0) {
    memcpy(tap_filter + sizeof(TME_TUN_TAP_PROG), config->tme_ethernet_config_addrs, tap_filter_size);
  }

  /* otherwise this filter doesn't need to accept any packets: */
  else {
    // how do we reject all packets???
  }

  /* set the filter on the TAP device: */
  TME_TUN_TAP_LEN((TME_TUN_TAP_PROG *)tap_filter) = tap_filter_size;
  if (ioctl(tap->tme_eth_fd, TUNSETTXFILTER, tap_filter) < 0) {
    tme_log(&tap->tme_eth_element->tme_element_log_handle, 0, errno,
	    (&tap->tme_eth_element->tme_element_log_handle,
	     _("failed to set the filter")));
    rc = errno;
  }

  tme_log(&tap->tme_eth_element->tme_element_log_handle, 0, TME_OK,
	  (&tap->tme_eth_element->tme_element_log_handle,
	   _("set the filter")));

  /* free the filter: */
  tme_free(tap_filter);

  /* unlock the mutex: */
  tme_mutex_unlock(&tap->tme_eth_mutex);

#endif
  /* done: */
  return (rc);
}

/* this makes a new connection side for a TAP: */
static int
_tme_tun_tap_connections_new(struct tme_element *element, 
			     const char * const *args, 
			     struct tme_connection **_conns,
			     char **_output)
{
  struct tme_ethernet_connection *conn_eth;

  tme_eth_connections_new(element, args, _conns);
  conn_eth = (struct tme_ethernet_connection *) (*_conns);

  /* fill in the Ethernet connection: */
  conn_eth->tme_ethernet_connection_config = _tme_tun_tap_config;

  /* done: */
  return (TME_OK);
}

/* retrieve ethernet arguments */
int _tme_tun_tap_args(const char * const args[], 
		      struct ifreq *ifr,
		      struct in_addr *ip_addrs,
		      char **_output)
{
  int arg_i;
  int usage;
  
  /* check our arguments: */
  usage = 0;

  memset(ifr, 0, TME_IF_TYPE_TOTAL * sizeof(struct ifreq));
  memset(ip_addrs, 0, TME_IP_ADDRS_TOTAL * sizeof(struct in_addr));

  arg_i = 1;

#define TAPIF (*(ifr + TME_IF_TYPE_TAP))
#define NATIF (*(ifr + TME_IF_TYPE_NAT))

  for (;;) {
    /* the interface we're supposed to use: */
    if (TME_ARG_IS(args[arg_i + 0], "interface")
	&& args[arg_i + 1] != NULL) {
      strncpy(TAPIF.ifr_name, args[arg_i + 1], sizeof(TAPIF.ifr_name));
    }

    /* the interface to nat to: */
    else if (TME_ARG_IS(args[arg_i + 0], "nat")
	&& args[arg_i + 1] != NULL) {
      strncpy(NATIF.ifr_name, args[arg_i + 1], sizeof(NATIF.ifr_name));
    }

    else if(TME_ARG_IS(args[arg_i + 0], "inet") 
	 && args[arg_i + 1] != NULL) {
      inet_aton(args[arg_i + 1], ip_addrs + TME_IP_ADDRS_INET);      
    }

    else if (TME_ARG_IS(args[arg_i + 0], "netmask")
	&& args[arg_i + 1] != NULL) {
      inet_aton(args[arg_i + 1], ip_addrs + TME_IP_ADDRS_NETMASK);
    }

    else if (TME_ARG_IS(args[arg_i + 0], "bcast")
	&& args[arg_i + 1] != NULL) {
      inet_aton(args[arg_i + 1], ip_addrs + TME_IP_ADDRS_BCAST);
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
			    "%s %s [ interface %s ] [ inet %s ] [ netmask %s ] [ bcast %s ] [ nat %s ]",
			    _("usage:"),
			    args[0],
			    _("INTERFACE"),
			    _("IPADDRESS"),
			    _("IPADDRESS"),
			    _("IPADDRESS"),
			    _("INTERFACE"));
    return (EINVAL);
  }
  return (TME_OK);
#undef TAPIF
#undef NATIF
}

#ifdef HAVE_LIBNFTNL_TABLE_H
static int _tme_nat_rule_add_payload(struct nft_rule *r, uint32_t base, uint32_t dreg,
				     uint32_t offset, uint32_t len)
{
  struct nft_rule_expr *e;

  e = nft_rule_expr_alloc("payload");
  if (e == NULL) {
    return -1;
  }

  nft_rule_expr_set_u32(e, NFT_EXPR_PAYLOAD_BASE, base);
  nft_rule_expr_set_u32(e, NFT_EXPR_PAYLOAD_DREG, dreg);
  nft_rule_expr_set_u32(e, NFT_EXPR_PAYLOAD_OFFSET, offset);
  nft_rule_expr_set_u32(e, NFT_EXPR_PAYLOAD_LEN, len);

  nft_rule_add_expr(r, e);
  return (TME_OK);
}

static int _tme_nat_rule_add_meta(struct nft_rule *r, uint32_t dreg, enum nft_meta_keys key)
{
  struct nft_rule_expr *e;

  e = nft_rule_expr_alloc("meta");
  if (e == NULL) {
    return -1;
  }

  nft_rule_expr_set_u32(e, NFT_EXPR_META_DREG, dreg);
  nft_rule_expr_set_u32(e, NFT_EXPR_META_KEY, key);
  nft_rule_add_expr(r, e);
  return (TME_OK);
}

static int _tme_nat_rule_add_bitwise(struct nft_rule *r, uint32_t dreg,
				     const void *mask, const void *xor, uint32_t len)
{
  struct nft_rule_expr *e;

  e = nft_rule_expr_alloc("bitwise");
  if (e == NULL) {
    return -1;
  }

  nft_rule_expr_set_u32(e, NFT_EXPR_BITWISE_SREG, dreg);
  nft_rule_expr_set_u32(e, NFT_EXPR_BITWISE_DREG, dreg);
  nft_rule_expr_set_u32(e, NFT_EXPR_BITWISE_LEN, len);
  nft_rule_expr_set(e, NFT_EXPR_BITWISE_MASK, mask, len);
  nft_rule_expr_set(e, NFT_EXPR_BITWISE_XOR, xor, len);

  nft_rule_add_expr(r, e);
  return (TME_OK);
}

static int _tme_nat_rule_add_cmp(struct nft_rule *r, uint32_t sreg, uint32_t op,
				  const void *data, uint32_t data_len)
{
  struct nft_rule_expr *e;

  e = nft_rule_expr_alloc("cmp");
  if (e == NULL) {
    return -1;
  }

  nft_rule_expr_set_u32(e, NFT_EXPR_CMP_SREG, sreg);
  nft_rule_expr_set_u32(e, NFT_EXPR_CMP_OP, op);
  nft_rule_expr_set(e, NFT_EXPR_CMP_DATA, data, data_len);

  nft_rule_add_expr(r, e);
  return (TME_OK);
}

static int _tme_nat_rule_add_immediate(struct nft_rule *r, uint32_t dreg,
					const void *data, uint32_t data_len)
{
  struct nft_rule_expr *e;

  e = nft_rule_expr_alloc("immediate");
  if (e == NULL) {
    return -1;
  }

  nft_rule_expr_set_u32(e, NFT_EXPR_IMM_DREG, dreg);
  nft_rule_expr_set(e, NFT_EXPR_IMM_DATA, data, data_len);

  nft_rule_add_expr(r, e);
  return (TME_OK);
}

static int _tme_nat_rule_add_nat(struct nft_rule *r, uint32_t type,
				  uint32_t flags, uint32_t sreg)
{
  struct nft_rule_expr *e;
  uint32_t family;

  e = nft_rule_expr_alloc("nat");
  if (e == NULL) {
    return -1;
  }

  nft_rule_expr_set_u32(e, NFT_EXPR_NAT_TYPE, type);

  family = nft_rule_attr_get_u32(r, NFT_RULE_ATTR_FAMILY);
  nft_rule_expr_set_u32(e, NFT_EXPR_NAT_FAMILY, family);

  if (flags != 0)
    nft_rule_expr_set_u32(e, NFT_EXPR_NAT_FLAGS, flags);

  nft_rule_expr_set_u32(e, NFT_EXPR_NAT_REG_ADDR_MIN,
			sreg);

  nft_rule_add_expr(r, e);

  return (TME_OK);
}

static int _tme_nat_run(struct tme_nat *nat, int num)
{
  struct mnl_socket *nl;
  char buf[MNL_SOCKET_BUFFER_SIZE];
  struct nlmsghdr *nlh;
  uint32_t portid, seq, nat_seq;
  struct mnl_nlmsg_batch *batch;
  int ret, batching;
  uint8_t nat_type;
  nat_hdr_builder nat_build_hdr;
  uint16_t nat_cmd;
  struct nat_family *nat_family;
  nat_msg_builder nat_build_msg;
  int i;

  batching = nft_batch_is_supported();
  if (batching < 0) {
    return -1;
  }
    
  seq = time(NULL);
  
  for(i=0;i<num;i++) {
    batch = mnl_nlmsg_batch_start(buf, sizeof(buf));
    
    if (batching) {
      nft_batch_begin(mnl_nlmsg_batch_current(batch), seq++);
      mnl_nlmsg_batch_next(batch);
    }
    
    nat_type=(*(nat+i)).type;
    nat_build_hdr = nat_hdr_builders[nat_type];
    nat_cmd = nat_cmds[nat_type];
    nat_family = &nat_families[nat_type];
    nat_build_msg = nat_msg_builders[nat_type];
    nat_seq = seq;

    nlh = (*nat_build_hdr)(mnl_nlmsg_batch_current(batch),
			   nat_cmd, nat_family->get_family((*(nat + i)).msg, nat_family->family_attr),
			   NLM_F_CREATE | NLM_F_ACK, seq++);
    (*nat_build_msg)(nlh, (*(nat + i)).msg);
    mnl_nlmsg_batch_next(batch);
    
    if (batching) {
      nft_batch_end(mnl_nlmsg_batch_current(batch), seq++);
      mnl_nlmsg_batch_next(batch);
    }
    
    nl = mnl_socket_open(NETLINK_NETFILTER);
    if (nl == NULL) {
      return -1;
    }
    
    if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
      return -1;
    }
    portid = mnl_socket_get_portid(nl);
    
    if (mnl_socket_sendto(nl, mnl_nlmsg_batch_head(batch),
			  mnl_nlmsg_batch_size(batch)) < 0) {
      return -1;
    }
    
    mnl_nlmsg_batch_stop(batch);
    
    ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
    while (ret > 0) {
      ret = mnl_cb_run(buf, ret, nat_seq, portid, NULL, NULL);
      if (ret <= 0)
	break;
      ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
    }
    if (ret == -1) {
      return -1;
    }
    mnl_socket_close(nl);
  }
  return (TME_OK);
}
#endif

/* the new TAP function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_tun,tap) {
  struct tme_tun_tap *tap;
  int tap_fd, dummy_fd;
  char dev_tap_filename[sizeof(DEV_TAP_FILENAME) + 9];
  int saved_errno;
  u_int tap_opt;
  struct ifreq ifr[TME_IF_TYPE_TOTAL];
  struct in_addr ip_addrs[TME_IP_ADDRS_TOTAL];
  struct ifaddrs *ifa;
#ifdef SIOCAIFADDR
  struct in_aliasreq ifra;
#endif
#ifdef HAVE_STRUCT_STAT_ST_RDEV
  struct stat tap_buf;
#endif
#if defined(TUNGIFINFO) && !defined(TAPGIFINFO)
  struct tuninfo info;
#ifdef HAVE_NETINET_IF_ETHER_H
  struct ether_addr addr;
#endif
#endif
  int i, usage, rc;
  char host[NI_MAXHOST];
  char mask[NI_MAXHOST];
  struct in_addr nataddr;
  u_int natidx;
  in_addr_t zero = 0;
#ifdef HAVE_LIBNFTNL_TABLE_H
  FILE *f;
  int nating, forward = EOF;
  struct nft_table *table;
  struct nft_chain *prechain, *postchain;
  struct nft_rule *rule;
  struct tme_nat nat[4];
#endif
#ifdef HAVE_NPF_H
  int nating;
  nl_config_t *ncf;
  nl_nat_t *nt;
#endif

  /* get the arguments: */
  rc = _tme_tun_tap_args(args, ifr, ip_addrs, _output);

#define TAPIF ifr[TME_IF_TYPE_TAP]
#define NATIF ifr[TME_IF_TYPE_NAT]

#if defined(HAVE_LIBNFTNL_TABLE_H) || defined(HAVE_NPF_H)
  /* find the interface we will use: */
  rc = tme_eth_ifaddrs_find(NATIF.ifr_name, &ifa, NULL, NULL);

  if (rc != TME_OK) {
    nating = false;
    tme_output_append_error(_output, _("couldn't find an interface %s"), NATIF.ifr_name);
  } else {
    strncpy(NATIF.ifr_name, ifa->ifa_name, sizeof(NATIF.ifr_name));
    rc = getnameinfo(ifa->ifa_addr,
		     sizeof(struct sockaddr_in),
		     host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
    if (rc != 0) {
      tme_log(&element->tme_element_log_handle, 0, errno,
	      (&element->tme_element_log_handle,
	       _("getnameinfo() failed: %s\n"), gai_strerror(rc)));
    }
    
    rc = getnameinfo(ifa->ifa_netmask,
		     sizeof(struct sockaddr_in),
		     mask, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
    if (rc != 0) {
      tme_log(&element->tme_element_log_handle, 0, errno,
	      (&element->tme_element_log_handle,
	       _("getnameinfo() failed: %s\n"), gai_strerror(rc)));
    }

    nating = true;

    natidx = if_nametoindex(NATIF.ifr_name);

    tme_log(&element->tme_element_log_handle, 0, TME_OK, 
	    (&element->tme_element_log_handle, 
	     "using nat interface %s(%d) with address %s, netmask %s",
	     NATIF.ifr_name, natidx,
	     host, mask));
  }
#endif

#ifdef HAVE_KLDFIND
  // A helper step to automate loading of the necessary kernel module on FreeBSD-derived platforms
#define KLD_FILENAME "if_tap.ko"
  if((kldfind(KLD_FILENAME)<0) &&
     (kldload(KLD_FILENAME)<0))
    tme_log(&element->tme_element_log_handle, 0, errno,
	    (&element->tme_element_log_handle,
	     _("failed to load the TAP kernel module...\ntry \"kldload %s\" in a root console"),
	     KLD_FILENAME));
#undef KLD_FILENAME
#endif

  sprintf(dev_tap_filename, DEV_TAP_FILENAME);

#ifndef HAVE_LINUX_IF_TUN_H
  if(TAPIF.ifr_name[0] != '\0') {
    strncpy(dev_tap_filename + 5, TAPIF.ifr_name, sizeof(DEV_TAP_FILENAME) - 1);
  }
#endif
  tap_fd = tme_eth_alloc(dev_tap_filename, _output);

  if (tap_fd < 0) {
    saved_errno = errno;
    tme_log(&element->tme_element_log_handle, 1, saved_errno,
	    (&element->tme_element_log_handle,
	     _("failed to open TAP device %s"),
	     dev_tap_filename));
    return (saved_errno);
  }

  /* this macro helps in closing the TAP socket on error: */
#define _TME_TAP_RAW_OPEN_ERROR(x) saved_errno = errno; x; errno = saved_errno

#ifdef HAVE_LINUX_IF_TUN_H
  TAPIF.ifr_flags = IFF_TAP | IFF_NO_PI;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */
  
  /* try to create the device */
  if (ioctl(tap_fd, TUNSETIFF, (void *) ifr) < 0 )
#elif defined(HAVE_NET_IF_TAP_H)
  if (ioctl(tap_fd, TAPGIFNAME, (void *) ifr) < 0 )
#elif defined(HAVE_FDEVNAME)
  strncpy(TAPIF.ifr_name, fdevname(tap_fd), IFNAMSIZ);
#elif defined(HAVE_DEVNAME) && defined(HAVE_STRUCT_STAT_ST_RDEV)
  fstat(tap_fd, &tap_buf);
  strncpy(TAPIF.ifr_name, devname(tap_buf.st_rdev, S_IFCHR), IFNAMSIZ);
#endif
#if defined(HAVE_LINUX_IF_TUN_H) || defined(HAVE_NET_IF_TAP_H)
  {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("failed to set the TAP interface on %s"),
	     dev_tap_filename));
    _TME_TAP_RAW_OPEN_ERROR(close(tap_fd));
    return (errno);
  }
#endif

  tme_log(&element->tme_element_log_handle, 0, TME_OK, 
	  (&element->tme_element_log_handle, 
	   "using tap interface %s",
	   TAPIF.ifr_name));

  /* make a dummy socket so we can configure the interface: */
  if ((dummy_fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
#if defined(TUNGIFINFO) && !defined(TAPGIFINFO)
    // OpenBSD requires this to enable TAP mode on TUN interface
    TAPIF.ifr_flags = IFF_LINK0;
#else
    TAPIF.ifr_flags = IFF_UP;
#endif
    /* try to create the device */
    if (ioctl(dummy_fd, SIOCSIFFLAGS, (void *) ifr) < 0 ) {
      tme_log(&element->tme_element_log_handle, 1, errno,
	      (&element->tme_element_log_handle,
	       _("failed to set the flags on iface %s"),
	       TAPIF.ifr_name));
    }

#define IPAINET ip_addrs[TME_IP_ADDRS_INET]
#define IPANETMASK ip_addrs[TME_IP_ADDRS_NETMASK]
#define IPABCAST ip_addrs[TME_IP_ADDRS_BCAST]

#ifdef SIOCAIFADDR
    if(IPAINET.s_addr
       || IPABCAST.s_addr
       || IPANETMASK.s_addr) {
      memset(&ifra, 0, sizeof(ifra));
      strncpy(ifra.ifra_name, TAPIF.ifr_name, IFNAMSIZ);
      ifra.ifra_addr.sin_len = sizeof(ifra.ifra_addr);
      ifra.ifra_addr.sin_family = AF_INET;
      ifra.ifra_broadaddr.sin_len = sizeof(ifra.ifra_broadaddr);
      ifra.ifra_broadaddr.sin_family = AF_INET;
      ifra.ifra_mask.sin_len = sizeof(ifra.ifra_mask);
      ifra.ifra_mask.sin_family = AF_INET;

      /*
      if(ioctl(dummy_fd, SIOCDIFADDR, (char *) &ifra) < 0 ) {
	tme_log(&element->tme_element_log_handle, 1, errno,
		(&element->tme_element_log_handle,
		 _("failed to set the addresses on iface %s"),
		 ifra.ifra_name));
      }
      */

      ifra.ifra_addr.sin_addr = IPAINET;    
      ifra.ifra_broadaddr.sin_addr = IPABCAST;    
      ifra.ifra_mask.sin_addr = IPANETMASK;    
      if(ioctl(dummy_fd, SIOCAIFADDR, (char *) &ifra) < 0 ) {
	tme_log(&element->tme_element_log_handle, 1, errno,
		(&element->tme_element_log_handle,
		 _("failed to set the addresses on tap interface %s"),
		 ifra.ifra_name));
      } else {
	if(ifra.ifra_addr.sin_addr.s_addr)
         tme_log(&element->tme_element_log_handle, 0, TME_OK, 
		 (&element->tme_element_log_handle, 
		  "set address on tap interface %s to %s",
		  ifra.ifra_name, 
		  inet_ntoa(ifra.ifra_addr.sin_addr)));
	if(ifra.ifra_broadaddr.sin_addr.s_addr)
         tme_log(&element->tme_element_log_handle, 0, TME_OK, 
		 (&element->tme_element_log_handle, 
		  "set broadcast address on tap interface %s to %s",
		  ifra.ifra_name, 
		  inet_ntoa(ifra.ifra_broadaddr.sin_addr)));
	if(ifra.ifra_mask.sin_addr.s_addr)
         tme_log(&element->tme_element_log_handle, 0, TME_OK, 
		 (&element->tme_element_log_handle, 
		  "set netmask on tap interface %s to %s",
		  ifra.ifra_name, 
		  inet_ntoa(ifra.ifra_mask.sin_addr)));
      }
    }
#else
    TAPIF.ifr_addr.sa_family = AF_INET;
    if(IPAINET.s_addr) {
      ((struct sockaddr_in *)&TAPIF.ifr_addr)->sin_addr = IPAINET;
      if(ioctl(dummy_fd, SIOCSIFADDR, (void *) ifr) < 0 ) {
	tme_log(&element->tme_element_log_handle, 1, errno,
		(&element->tme_element_log_handle,
	       _("failed to set the address on iface %s"),
		 TAPIF.ifr_name));
      } else
         tme_log(&element->tme_element_log_handle, 0, TME_OK, 
		 (&element->tme_element_log_handle, 
		  "set address on tap interface %s to %s",
		  TAPIF.ifr_name, 
		  inet_ntoa(((struct sockaddr_in *)&TAPIF.ifr_addr)->sin_addr)));
    }

    if(IPANETMASK.s_addr) {
      ((struct sockaddr_in *)&TAPIF.ifr_addr)->sin_addr = IPANETMASK;
      if(ioctl(dummy_fd, SIOCSIFNETMASK, (void *) ifr) < 0 ) {
	tme_log(&element->tme_element_log_handle, 1, errno,
		(&element->tme_element_log_handle,
	       _("failed to set the netmask on iface %s"),
		 TAPIF.ifr_name));
      } else
         tme_log(&element->tme_element_log_handle, 0, TME_OK, 
		 (&element->tme_element_log_handle, 
		  "set netmask on tap interface %s to %s",
		  TAPIF.ifr_name, 
		  inet_ntoa(((struct sockaddr_in *)&TAPIF.ifr_addr)->sin_addr)));

    }

    if(IPABCAST.s_addr) {
      ((struct sockaddr_in *)&TAPIF.ifr_addr)->sin_addr = IPABCAST;
      if(ioctl(dummy_fd, SIOCSIFBRDADDR, (void *) ifr) < 0 ) {
	tme_log(&element->tme_element_log_handle, 1, errno,
		(&element->tme_element_log_handle,
	       _("failed to set the broadcast address on iface %s"),
		 TAPIF.ifr_name));
      } else
         tme_log(&element->tme_element_log_handle, 0, TME_OK, 
		 (&element->tme_element_log_handle, 
		  "set broadcast address on tap interface %s to %s",
		  TAPIF.ifr_name, 
		  inet_ntoa(((struct sockaddr_in *)&TAPIF.ifr_addr)->sin_addr)));

    }
#endif
    close(dummy_fd);
  }

  inet_aton(host, &nataddr);
  // Perform network address translation, if available
#ifdef HAVE_NPF_H
  dummy_fd = open("/dev/npf", O_RDWR);
  if (dummy_fd == -1) {
    tme_output_append_error(_output, _("couldn't open file /dev/npf; ip forwarding may not work."));
    goto exit_nat;    
  }
  
  ncf = npf_config_create();
  nt = npf_nat_create(NPF_NATOUT, 0, natidx, &nataddr, AF_INET, 0);
  npf_nat_insert(ncf, nt, NPF_PRI_LAST);
  npf_config_submit(ncf, dummy_fd);
  close(dummy_fd);
  npf_config_destroy(ncf);

 exit_nat:
  setuid(getuid());
#endif
#ifdef HAVE_LIBNFTNL_TABLE_H
  if(!nating) goto exit_nat;

  i=0;

  table = nft_table_alloc();
  if (table == NULL) {
    tme_log(&element->tme_element_log_handle, 0, errno,
	    (&element->tme_element_log_handle,
	     _("failed to allocate tme nat table")));
    goto exit_nat;
  }

  nft_table_attr_set_u32(table, NFT_TABLE_ATTR_FAMILY, NFPROTO_IPV4);
  nft_table_attr_set(table, NFT_TABLE_ATTR_NAME, "tme");
  nat[i].type = TME_NAT_TABLE;
  nat[i++].msg = table;

  prechain = nft_chain_alloc();
  if (prechain == NULL) {
    tme_log(&element->tme_element_log_handle, 0, errno,
	    (&element->tme_element_log_handle,
	     _("failed to allocate chain for tme nat table")));
    goto exit_nat;
  }

  nft_chain_attr_set_u32(prechain, NFT_CHAIN_ATTR_FAMILY, nft_table_attr_get_u32(table, NFT_TABLE_ATTR_FAMILY));
  nft_chain_attr_set(prechain, NFT_CHAIN_ATTR_TABLE, nft_table_attr_get_str(table, NFT_TABLE_ATTR_NAME));
  nft_chain_attr_set(prechain, NFT_CHAIN_ATTR_NAME, "prerouting");
  nft_chain_attr_set_u32(prechain, NFT_CHAIN_ATTR_HOOKNUM, NF_INET_PRE_ROUTING);
  nft_chain_attr_set_u32(prechain, NFT_CHAIN_ATTR_PRIO, 0);
  nft_chain_attr_set(prechain, NFT_CHAIN_ATTR_TYPE, "nat");
  nat[i].type = TME_NAT_CHAIN;
  nat[i++].msg = prechain;

  postchain = nft_chain_alloc();
  if (postchain == NULL) {
    tme_log(&element->tme_element_log_handle, 0, errno,
	    (&element->tme_element_log_handle,
	     _("failed to allocate chain for tme nat table")));
    goto exit_nat;
  }

  nft_chain_attr_set_u32(postchain, NFT_CHAIN_ATTR_FAMILY, nft_table_attr_get_u32(table, NFT_TABLE_ATTR_FAMILY));
  nft_chain_attr_set(postchain, NFT_CHAIN_ATTR_TABLE, nft_table_attr_get_str(table, NFT_TABLE_ATTR_NAME));
  nft_chain_attr_set(postchain, NFT_CHAIN_ATTR_NAME, "postrouting");
  nft_chain_attr_set_u32(postchain, NFT_CHAIN_ATTR_HOOKNUM, NF_INET_POST_ROUTING);
  nft_chain_attr_set_u32(postchain, NFT_CHAIN_ATTR_PRIO, 0);
  nft_chain_attr_set(postchain, NFT_CHAIN_ATTR_TYPE, "nat");
  nat[i].type = TME_NAT_CHAIN;
  nat[i++].msg = postchain;

  rule = nft_rule_alloc();
  if (rule == NULL) {
    tme_log(&element->tme_element_log_handle, 0, errno,
	    (&element->tme_element_log_handle,
	     _("failed to allocate rule for tme nat table")));
    goto exit_nat;
  }

  nft_rule_attr_set_u32(rule, NFT_RULE_ATTR_FAMILY, nft_chain_attr_get_u32(postchain, NFT_CHAIN_ATTR_FAMILY));
  nft_rule_attr_set(rule, NFT_RULE_ATTR_TABLE, nft_chain_attr_get_str(postchain, NFT_CHAIN_ATTR_TABLE));
  nft_rule_attr_set(rule, NFT_RULE_ATTR_CHAIN, nft_chain_attr_get_str(postchain, NFT_CHAIN_ATTR_NAME));
  nat[i].type = TME_NAT_RULE;
  nat[i++].msg = rule;
  
  //_tme_nat_rule_add_meta(rule, NFT_REG_1, NFT_META_IIFNAME);
  //_tme_nat_rule_add_cmp(rule, NFT_REG_1, NFT_CMP_EQ, TAPIF.ifr_name, IFNAMSIZ);
  _tme_nat_rule_add_payload(rule, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
			    offsetof(struct iphdr, saddr), sizeof(uint32_t));
  _tme_nat_rule_add_bitwise(rule, NFT_REG_1, &IPANETMASK.s_addr, &zero, sizeof(in_addr_t));
  IPAINET.s_addr &= IPANETMASK.s_addr;
  _tme_nat_rule_add_cmp(rule, NFT_REG_1, NFT_CMP_EQ, &IPAINET.s_addr, sizeof(in_addr_t));
  _tme_nat_rule_add_meta(rule, NFT_REG_1, NFT_META_OIFNAME);
  _tme_nat_rule_add_cmp(rule, NFT_REG_1, NFT_CMP_EQ, NATIF.ifr_name, IFNAMSIZ);
  _tme_nat_rule_add_immediate(rule, NFT_REG_1, &nataddr.s_addr, sizeof(in_addr_t));
  _tme_nat_rule_add_nat(rule, NFT_NAT_SNAT, 0, NFT_REG_1);

  if (_tme_nat_run(nat, i) < 0) {
    tme_log(&element->tme_element_log_handle, 0, errno,
	    (&element->tme_element_log_handle,
	     _("failed to set nat on tap interface; access to external network will be restricted")));
  } else {
    tme_log(&element->tme_element_log_handle, 0, TME_OK,
	    (&element->tme_element_log_handle,
	     _("Added tme table with ip nat rules for tap interface network, "
	       "to nftables.  Run 'nft list table tme' to view the table.  If you still have problems "
	       "with forwarding from the tap interface, you may need to manually adjust the filter tables, "
	       "e.g., iptables -F FORWARD.  nftables is the successor to iptables, so it may not be available on older systems.")));

    // Enable IP forwarding if we successfully added the tme nat table
    f = fopen(IPFWDFILE, "r");
    if(f != NULL) {
      forward = fgetc(f);
      fclose(f);
      if(forward == '0') {
	f = fopen(IPFWDFILE, "w");
	if(f != NULL) {
	  forward = fputc('1', f);
	  fclose(f);
	}
      }
    } else {
      tme_output_append_error(_output, _("couldn't open file %s; ip forwarding may not work."), IPFWDFILE);
    }

    if(forward != '1') {
      tme_log(&element->tme_element_log_handle, 0, errno,
	      (&element->tme_element_log_handle,
	       _("problem reading or writing to %s; ip forwarding may not work."), IPFWDFILE));
    } else {
      tme_log(&element->tme_element_log_handle, 0, TME_OK,
	      (&element->tme_element_log_handle,
	       _("Enabled ipv4 forwarding!")));
    }
  }

  nft_table_free(table);
  nft_chain_free(prechain);
  nft_chain_free(postchain);
  nft_rule_free(rule);

 exit_nat:
#endif
  return tme_eth_init(element, tap_fd, 4096, NULL, _tme_tun_tap_connections_new);

#undef IPAINET
#undef IPANETMASK
#undef IPABCAST
#undef TAPIF
#undef NATIF
#undef _TME_TAP_RAW_OPEN_ERROR
}
