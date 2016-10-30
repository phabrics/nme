/* host/tun/tun-tap.c - Native TUN TAP Ethernet support: */

/*
 * Copyright (c) 2014, 2015 Ruben Agin
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
#include "eth-if.h"
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
#endif
#ifdef HAVE_LIBNFTNL_CHAIN_H
#include <libnftnl/chain.h>
#endif
#ifdef HAVE_LIBNFTNL_RULE_H
#include <libnftnl/rule.h>
#endif
#ifdef HAVE_LIBNFTNL_EXPR_H
#include <libnftnl/expr.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#ifdef HAVE_SYS_MODULE_H
#include <sys/module.h>
#endif
#ifdef TME_NAT_NPF
#ifdef HAVE_NPF_H
#define _NPF_PRIVATE
#define NPF_BPFCOP
#include <npf.h>
#ifdef HAVE_NET_NPF_NCODE_H
#include <net/npf_ncode.h>
#else
#include <net/bpf.h>
#define NPF_BPF_SUCCESS ((u_int)-1)
#define NPF_BPF_FAILURE 0
#ifdef HAVE_PCAP_PCAP_H
#include <pcap/pcap.h>
#endif
#endif // HAVE_NET_NPF_NCODE_H
#endif // HAVE_NPF_H
#endif // TME_NAT_NPF
#ifdef HAVE_NET_PFVAR_H
#include <net/pfvar.h>
#endif
#ifdef HAVE_NET_PF_PFVAR_H
#include <net/pf/pfvar.h>
#endif
#ifdef HAVE_NETINET_IP_COMPAT_H
#include <netinet/ip_compat.h>
#endif
#ifdef HAVE_NETINET_IP_FIL_H
#include <netinet/ip_fil.h>
#endif
#ifdef HAVE_NETINET_IP_NAT_H
#include <netinet/ip_nat.h>
#endif
#ifdef HAVE_NETINET_IP_PROXY_H
#include <netinet/ip_proxy.h>
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

#define TME_DO_NFT defined(HAVE_LIBNFTNL_TABLE_H) && defined(TME_NAT_NFT)
#define TME_DO_NPF defined(HAVE_NPF_H) && defined(TME_NAT_NPF)
#define TME_DO_PFV (defined(HAVE_NET_PFVAR_H) || defined(HAVE_NET_PF_PFVAR_H)) && defined(TME_NAT_PFV)
#define TME_DO_IPF defined(HAVE_NETINET_IP_NAT_H) && defined(TME_NAT_IPF)
#define TME_DO_APF TME_DO_NPF || TME_DO_PFV || TME_DO_IPF
#define TME_DO_NAT TME_DO_NFT || TME_DO_APF

/* IP forwarding variables */
// procfs (Linux)
#define IPFWDFILE "/proc/sys/net/ipv4/ip_forward"
// sysctl (*BSD)
#define SYSCTLNAME "net.inet.ip.forwarding"

#if TME_DO_NPF
#define DEV_IPF_FILENAME "/dev/npf"
#elif TME_DO_PFV
#define DEV_IPF_FILENAME "/dev/pf"
#else // TME_DO_IPF
#define DEV_IPF_FILENAME "/dev/ipnat"
#endif

#if TME_DO_NFT
/* nat types: */
#define TME_NAT_TABLE (0)
#define TME_NAT_CHAIN (1)
#define TME_NAT_RULE (2)
#define TME_NAT_TOTAL (3)

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

#endif // TME_DO_NFT

/* this is called when the ethernet configuration changes: */
static int
_tme_tun_tap_config(struct tme_ethernet_connection *conn_eth, 
		    struct tme_ethernet_config *config)
{
#ifdef HAVE_LINUX_IF_TUN_H
  struct tme_ethernet *tap;
  TME_TUN_TAP_INSN *tap_filter;
  int tap_filter_size;
#endif
  int rc;

  /* assume we will succeed: */
  rc = TME_OK;

#ifdef HAVE_LINUX_IF_TUN_H
  /* recover our data structures: */
  tap = conn_eth->tme_ethernet_connection.tme_connection_element->tme_element_private;

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
  if (ioctl(tap->tme_eth_handle, TUNSETTXFILTER, tap_filter) < 0) {
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
			     struct tme_connection **_conns)
{
  struct tme_ethernet_connection *conn_eth;

  tme_eth_connections_new(element, args, _conns);
  conn_eth = (struct tme_ethernet_connection *) (*_conns);

  /* fill in the Ethernet connection: */
  //  conn_eth->tme_ethernet_connection_config = _tme_tun_tap_config;

  /* done: */
  return (TME_OK);
}

/* retrieve ethernet arguments */
int _tme_tun_tap_args(const char * const args[], 
		      char **if_names,
		      struct in_addr *ip_addrs,
		      char **_output)
{
  int arg_i;
  int usage;
  
  /* check our arguments: */
  usage = 0;

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif
  memset(if_names, 0, TME_IF_TYPE_TOTAL * IFNAMSIZ);
  memset(ip_addrs, 0, TME_IP_ADDRS_TOTAL * sizeof(struct in_addr));

  arg_i = 1;

#define TAPIF ((char *)if_names + TME_IF_TYPE_TAP * IFNAMSIZ)
#define NATIF ((char *)if_names + TME_IF_TYPE_NAT * IFNAMSIZ)

  for (;;) {
    /* the interface we're supposed to use: */
    if (TME_ARG_IS(args[arg_i + 0], "interface")
	&& args[arg_i + 1] != NULL) {
      strncpy(TAPIF, args[arg_i + 1], IFNAMSIZ);
    }

    /* the interface to nat to: */
    else if (TME_ARG_IS(args[arg_i + 0], "nat")
	&& args[arg_i + 1] != NULL) {
      strncpy(NATIF, args[arg_i + 1], IFNAMSIZ);
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

    else if (TME_ARG_IS(args[arg_i + 0], "bcast")
	&& args[arg_i + 1] != NULL) {
      inet_pton(AF_INET, args[arg_i + 1], ip_addrs + TME_IP_ADDRS_BCAST);
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

#if TME_DO_NFT
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

#if TME_DO_NPF
npf_netmask_t get_netbits(struct in_addr netmask)
{
  // Given a netmask address, return the CIDR bitmask representation (ie, number of bits in netmask)
  npf_netmask_t netbits = 0;
  for(;netmask.s_addr;netmask.s_addr>>=1)
    if (netmask.s_addr & 1) netbits++;

  return netbits;
}

#ifdef _NPF_PRIVATE
void
_tme_npf_print_error(const nl_error_t *ne, char **_output)
{
#ifdef HAVE_NET_NPF_NCODE_H
  static const char *ncode_errors[] = {
    [-NPF_ERR_OPCODE]	= "invalid instruction",
    [-NPF_ERR_JUMP]		= "invalid jump",
    [-NPF_ERR_REG]		= "invalid register",
    [-NPF_ERR_INVAL]	= "invalid argument value",
    [-NPF_ERR_RANGE]	= "processing out of range"
  };
#endif
  
  const int nc_err = ne->ne_ncode_error;
  const char *srcfile = ne->ne_source_file;

  if (srcfile) {
    tme_output_append_error(_output, "source %s line %d", srcfile, ne->ne_source_line);
  }
  if (nc_err) {
    tme_output_append_error(_output, "n-code error (%d): %s at offset 0x%x", nc_err, 
#ifdef HAVE_NET_NPF_NCODE_H
			    ncode_errors[-nc_err],
#else
			    "",
#endif
			    ne->ne_ncode_errat);
  }
  if (ne->ne_id) {
    tme_output_append_error(_output, "object: %d", ne->ne_id);
  }
}
#endif // _NPF_PRIVATE
#endif // _TME_DO_NPF

/* the new TAP function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_tun,tap) {
  int tap_fd, dummy_fd;
  int saved_errno, i, rc;
  char dev_tap_filename[sizeof(DEV_TAP_FILENAME) + 9];
  struct ifreq ifr;
  char if_names[TME_IF_TYPE_TOTAL][IFNAMSIZ];
  char tap_hosts[TME_IP_ADDRS_TOTAL][NI_MAXHOST];
  struct in_addr tap_addrs[TME_IP_ADDRS_TOTAL];
#ifdef SIOCAIFADDR
  struct in_aliasreq ifra;
#endif
  unsigned char *hwaddr;
  unsigned int hwaddr_len;
#if !defined(HAVE_FDEVNAME) && defined(HAVE_STRUCT_STAT_ST_RDEV)
  struct stat tap_buf;
#endif
#if TME_DO_NAT
  char nat_hosts[TME_IP_ADDRS_TOTAL][NI_MAXHOST];
  struct in_addr nat_addrs[TME_IP_ADDRS_TOTAL];
  u_int netnum;
  int forward = EOF;
#endif
#if TME_DO_NFT
  in_addr_t zero = 0;
  FILE *f;
  struct nft_table *table;
  struct nft_chain *prechain, *postchain;
  struct nft_rule *rule;
  struct tme_nat nat[4];
#elif TME_DO_APF
  int mib[4];
  size_t len;  
#if TME_DO_NPF
  modctl_load_t mod;
  int ver;
  npf_netmask_t netbits;
  nl_config_t *ncf;
  nl_nat_t *nt;
  nl_rule_t *ext, *def;
  nl_rule_t *rl, *rl2;
#elif TME_DO_PFV
  // using PF for NAT
  FILE *fp;
#else // TME_DO_IPF
  ipnat_t nat;
#endif
#endif
#ifdef HAVE_IFADDRS_H
  struct ifaddrs *ifa;
  int ifa_offs[TME_IP_ADDRS_TOTAL];

  ifa_offs[TME_IP_ADDRS_INET] = offsetof(struct ifaddrs, ifa_addr);
  ifa_offs[TME_IP_ADDRS_NETMASK] = offsetof(struct ifaddrs, ifa_netmask);
  ifa_offs[TME_IP_ADDRS_BCAST] = offsetof(struct ifaddrs, ifa_broadaddr);
#endif
  
  /* get the arguments: */
  _tme_tun_tap_args(args, if_names, tap_addrs, _output);

#define TAPIF if_names[TME_IF_TYPE_TAP]
  // Set/get ip addresses
#define TAPINET tap_addrs[TME_IP_ADDRS_INET]
#define TAPNETMASK tap_addrs[TME_IP_ADDRS_NETMASK]
#define TAPBCAST tap_addrs[TME_IP_ADDRS_BCAST]
  
  sprintf(dev_tap_filename, DEV_TAP_FILENAME);

#ifndef HAVE_LINUX_IF_TUN_H
  if(TAPIF[0] != '\0') {
    strncpy(dev_tap_filename + 5, TAPIF, sizeof(DEV_TAP_FILENAME) - 1);
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

  strncpy(ifr.ifr_name, TAPIF, sizeof(TAPIF));
  
#ifdef HAVE_LINUX_IF_TUN_H
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_ONE_QUEUE;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */
  
  /* try to create the device */
  if (ioctl(tap_fd, TUNSETIFF, (void *) &ifr) < 0 )
#elif defined(HAVE_NET_IF_TAP_H)
  if (ioctl(tap_fd, TAPGIFNAME, (void *) &ifr) < 0 )
#elif defined(HAVE_FDEVNAME)
  strncpy(TAPIF, fdevname(tap_fd), IFNAMSIZ);
#elif defined(HAVE_DEVNAME) && defined(HAVE_STRUCT_STAT_ST_RDEV)
  fstat(tap_fd, &tap_buf);
  strncpy(TAPIF, devname(tap_buf.st_rdev, S_IFCHR), IFNAMSIZ);
#endif

#if defined(HAVE_LINUX_IF_TUN_H) || defined(HAVE_NET_IF_TAP_H)
  {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("failed to set the TAP interface on %s"),
	     dev_tap_filename));
    _TME_TAP_RAW_OPEN_ERROR(close(tap_fd));
    return (errno);
  } else strncpy(TAPIF, ifr.ifr_name, sizeof(ifr.ifr_name));
#endif

  tme_log(&element->tme_element_log_handle, 0, TME_OK, 
	  (&element->tme_element_log_handle, 
	   "using tap interface %s",
	   TAPIF));

  /* make a dummy socket so we can configure the interface: */
  if ((dummy_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    tme_log(&element->tme_element_log_handle, 0, errno,
	    (&element->tme_element_log_handle,
	     _("couldn't open inet socket to set ip parameters for tap interface %s; try setting them manually using e.g., ifconfig or ip"),
	     TAPIF));
    goto exit_tap;
  }
  
#if defined(TUNGIFINFO) && !defined(TAPGIFINFO)
  // OpenBSD requires this to enable TAP mode on TUN interface
  ifr.ifr_flags = IFF_LINK0;
#else
  ifr.ifr_flags = IFF_UP;
#endif
  /* try to bring up the device */
  if (ioctl(dummy_fd, SIOCSIFFLAGS, (void *) &ifr) < 0 ) {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("failed to set the flags on iface %s"),
	     TAPIF));
  }
  
#ifdef SIOCAIFADDR
  if(TAPINET.s_addr
     || TAPBCAST.s_addr
     || TAPNETMASK.s_addr) {
    memset(&ifra, 0, sizeof(ifra));
    strncpy(ifra.ifra_name, TAPIF, IFNAMSIZ);
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

    ifra.ifra_addr.sin_addr = TAPINET;    
    ifra.ifra_broadaddr.sin_addr = TAPBCAST;    
    ifra.ifra_mask.sin_addr = TAPNETMASK;    
    if(ioctl(dummy_fd, SIOCAIFADDR, (char *) &ifra) < 0 ) {
      tme_log(&element->tme_element_log_handle, 1, errno,
	      (&element->tme_element_log_handle,
	       _("failed to set the addresses on tap interface %s"),
	       ifra.ifra_name));
    }
  }

  //tap_hwaddr = tme_new0(unsigned char, TME_ETHERNET_ADDR_SIZE);
  //memcpy(tap_hwaddr, LLADDR(satosdl(&ifla.addr)), TME_ETHERNET_ADDR_SIZE);
  
#else
  ifr.ifr_addr.sa_family = AF_INET;
  if(TAPINET.s_addr) {
    ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr = TAPINET;
    if(ioctl(dummy_fd, SIOCSIFADDR, (void *) &ifr) < 0 )
      tme_log(&element->tme_element_log_handle, 1, errno,
	      (&element->tme_element_log_handle,
	       _("failed to set the address on iface %s"),
	       TAPIF));
  }

  if(TAPNETMASK.s_addr) {
    ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr = TAPNETMASK;
    if(ioctl(dummy_fd, SIOCSIFNETMASK, (void *) &ifr) < 0 )
      tme_log(&element->tme_element_log_handle, 1, errno,
	      (&element->tme_element_log_handle,
	       _("failed to set the netmask on iface %s"),
	       TAPIF));
  }

  if(TAPBCAST.s_addr) {
    ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr = TAPBCAST;
    if(ioctl(dummy_fd, SIOCSIFBRDADDR, (void *) &ifr) < 0 )
      tme_log(&element->tme_element_log_handle, 1, errno,
	      (&element->tme_element_log_handle,
	       _("failed to set the broadcast address on iface %s"),
	       TAPIF));
  }

  /*
  if(ioctl(dummy_fd, SIOCGIFHWADDR, (void *) &ifr) < 0) {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("failed to get the hardware address on tap interface %s"),
	     TAPIF)); 	       
  } else {
    tap_hwaddr = tme_new0(unsigned char, TME_ETHERNET_ADDR_SIZE);
    memcpy(tap_hwaddr, ifr.ifr_hwaddr.sa_data, TME_ETHERNET_ADDR_SIZE);
  }
  */
#endif
  close(dummy_fd);

 exit_tap:
  /* find the interface we will use: */
  rc = tme_eth_ifaddrs_find(TAPIF, AF_UNSPEC, &ifa, &hwaddr, &hwaddr_len);
  
  for(i=0;i<TME_IP_ADDRS_TOTAL;i++) {
    tap_hosts[i][0]='\0';
    if((rc = getnameinfo(*(struct sockaddr **)((char *)ifa + ifa_offs[i]),
			 sizeof(struct sockaddr_in),
			 tap_hosts[i], NI_MAXHOST, NULL, 0, NI_NUMERICHOST)))
      tme_log(&element->tme_element_log_handle, 0, errno,
	      (&element->tme_element_log_handle,
	       _("getnameinfo() failed: %s\n"), gai_strerror(rc)));
    else
      inet_pton(AF_INET, tap_hosts[i], &tap_addrs[i]);
  }
  
  tme_log(&element->tme_element_log_handle, 0, TME_OK, 
	  (&element->tme_element_log_handle, 
	   "using tap interface %s(%d) with address %s, netmask %s, broadcast %s",
	   TAPIF, if_nametoindex(TAPIF),
	   tap_hosts[TME_IP_ADDRS_INET],
	   tap_hosts[TME_IP_ADDRS_NETMASK],
	   tap_hosts[TME_IP_ADDRS_BCAST]));

  if(hwaddr_len == TME_ETHERNET_ADDR_SIZE) {
    tme_log(&element->tme_element_log_handle, 0, TME_OK, 
	    (&element->tme_element_log_handle, 
	     "hardware address on tap interface %s set to %02x:%02x:%02x:%02x:%02x:%02x",
	     TAPIF, 
	     hwaddr[0],
	     hwaddr[1],
	     hwaddr[2],
	     hwaddr[3],
	     hwaddr[4],
	     hwaddr[5]));
  }
  // Perform network address translation, if available

#define NATIF if_names[TME_IF_TYPE_NAT]

#if TME_DO_NAT
  tme_log(&element->tme_element_log_handle, 0, TME_OK, 
	  (&element->tme_element_log_handle, 
	   "trying nat interface %s", NATIF));
	   
  /* find the interface we will use: */
  rc = tme_eth_ifaddrs_find(NATIF, AF_UNSPEC, &ifa, NULL, NULL);

  if (rc != TME_OK) {
    tme_output_append_error(_output, _("couldn't find an interface %s"), NATIF);
    goto exit_nat;
  } else {
    strncpy(NATIF, ifa->ifa_name, IFNAMSIZ);
    for(i=0;i<TME_IP_ADDRS_TOTAL;i++) {
      nat_hosts[i][0]='\0';
      if((rc = getnameinfo(*(struct sockaddr **)((char *)ifa + ifa_offs[i]),
			   sizeof(struct sockaddr_in),
			   nat_hosts[i], NI_MAXHOST, NULL, 0, NI_NUMERICHOST)))
	tme_log(&element->tme_element_log_handle, 0, errno,
		(&element->tme_element_log_handle,
		 _("getnameinfo() failed: %s\n"), gai_strerror(rc)));
      else
	inet_pton(AF_INET, nat_hosts[i], &nat_addrs[i]);
    }

    tme_log(&element->tme_element_log_handle, 0, TME_OK, 
	    (&element->tme_element_log_handle, 
	     "using nat interface %s(%d) with address %s, netmask %s, broadcast %s",
	     NATIF, if_nametoindex(NATIF),
	     nat_hosts[TME_IP_ADDRS_INET],
	     nat_hosts[TME_IP_ADDRS_NETMASK],
	     nat_hosts[TME_IP_ADDRS_BCAST]));
  }

  netnum = TAPINET.s_addr & TAPNETMASK.s_addr;
#endif

#if TME_DO_NFT
  /* NAT on Linux using NFTABLES */
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
  //_tme_nat_rule_add_cmp(rule, NFT_REG_1, NFT_CMP_EQ, TAPIF, IFNAMSIZ);
  _tme_nat_rule_add_payload(rule, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
			    offsetof(struct iphdr, saddr), sizeof(uint32_t));
  _tme_nat_rule_add_bitwise(rule, NFT_REG_1, &TAPNETMASK.s_addr, &zero, sizeof(in_addr_t));
  _tme_nat_rule_add_cmp(rule, NFT_REG_1, NFT_CMP_EQ, &netnum, sizeof(in_addr_t));
  _tme_nat_rule_add_meta(rule, NFT_REG_1, NFT_META_OIFNAME);
  _tme_nat_rule_add_cmp(rule, NFT_REG_1, NFT_CMP_EQ, NATIF, IFNAMSIZ);
  _tme_nat_rule_add_immediate(rule, NFT_REG_1, &nat_addrs[TME_IP_ADDRS_INET].s_addr, sizeof(in_addr_t));
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
	       "with forwarding from the tap interface, you may need to manually adjust the filter tables, or remove conflicting nat "
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
#elif TME_DO_APF // !TME_DO_NFT
  /* NAT on *BSD */

#if defined(TME_DO_PFV) && defined(HAVE_KLDFIND)
  // A helper step to automate loading of the necessary kernel module on FreeBSD-derived platforms
#define KLD_FILENAME "pf.ko"
  if((kldfind(KLD_FILENAME)<0) &&
     (kldload(KLD_FILENAME)<0))
    tme_log(&element->tme_element_log_handle, 0, errno,
	    (&element->tme_element_log_handle,
	     _("failed to load the TAP kernel module...\ntry \"kldload %s\" in a root console"),
	     KLD_FILENAME));
#undef KLD_FILENAME
#endif

  // Generate a NAT configuration & submit to kernel
  dummy_fd = open(DEV_IPF_FILENAME, O_RDWR);
  if (dummy_fd == -1) {
    tme_log(&element->tme_element_log_handle, 0, errno,
	    (&element->tme_element_log_handle,
	     _("couldn't open pf device %s; access to external network will be restricted."), 
	     DEV_IPF_FILENAME));
    goto exit_nat;
  }

#if TME_DO_NPF
  // Load kernel modules if needed
  memset(&mod, 0, sizeof(modctl_load_t));
  mod.ml_filename="npf";
  modctl(MODCTL_LOAD, &mod);
  mod.ml_filename="npf_alg_icmp";
  modctl(MODCTL_LOAD, &mod);
  // using NPF for NAT
  if (ioctl(dummy_fd, IOC_NPF_VERSION, &ver) == -1) {
    tme_log(&element->tme_element_log_handle, 0, -1,
	    (&element->tme_element_log_handle,
	     _("ioctl(IOC_NPF_VERSION)")));
    goto exit_nat;
  }
  if (ver != NPF_VERSION) {
    tme_log(&element->tme_element_log_handle, 0, -1,
	    (&element->tme_element_log_handle, 
	     _("incompatible NPF interface version (%d, kernel %d)\n"
	       "Hint: update userland?"), NPF_VERSION, ver));
    goto exit_nat;    
  }
  ncf = npf_config_create();

  netbits = get_netbits(TAPNETMASK);
  
#ifdef HAVE_NET_NPF_NCODE_H
  nt = npf_nat_create(NPF_NATOUT, 0, if_nametoindex(NATIF), &nat_addrs[TME_IP_ADDRS_INET], AF_INET, 0);
  tme_uint32_t ncode[] = {
 NPF_OPCODE_IP4MASK,
 1,
 netnum,
 netbits,
 NPF_OPCODE_RET,
 0,
 NPF_OPCODE_RET,
 0xff
  };
  npf_rule_setcode(nt, NPF_CODE_NC, ncode, sizeof(ncode));
#else
  nt = npf_nat_create(NPF_NATOUT, 0, NATIF, AF_INET, &nat_addrs[TME_IP_ADDRS_INET], NPF_NO_NETMASK /*get_netbits(natmask)*/, 0);
  uint32_t wordmask = 0xffffffff << (32 - netbits);
  
  struct bpf_insn incode[] = {  
    BPF_STMT(BPF_LD+BPF_W+BPF_MEM, BPF_MW_IPVER),
    BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, IPVERSION, 0, 4),
    BPF_STMT(BPF_LD+BPF_W+BPF_ABS, offsetof(struct ip, ip_src)),
    BPF_STMT(BPF_ALU+BPF_AND+BPF_K, wordmask),
    BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, ntohl(netnum), 0, 1),
    BPF_STMT(BPF_RET+BPF_K, NPF_BPF_SUCCESS),
    BPF_STMT(BPF_RET+BPF_K, NPF_BPF_FAILURE)
  };
  
  uint32_t mwords[] = { 0, 1, AF_INET, 2, 6, AF_INET, netbits, netnum, 0, 0, 0 };
  if (npf_rule_setinfo(nt, &mwords, sizeof(mwords)) == -1) {
    tme_log(&element->tme_element_log_handle, 0, -1,
	    (&element->tme_element_log_handle,
	     _("npf_rule_setinfo failed")));
  }
  
  if (npf_rule_setcode(nt, NPF_CODE_BPF, incode, sizeof(incode)) == -1) {
    tme_log(&element->tme_element_log_handle, 0, -1,
	    (&element->tme_element_log_handle,
	     _("npf_rule_setcode failed")));
  }
#ifdef HAVE_PCAP_PCAP_H
  struct bpf_program bf;
  bf.bf_insns = &incode;
  bf.bf_len = sizeof(incode)/sizeof(struct bpf_insn);
  bpf_dump(&bf, 0);
#endif
#endif
  
  npf_nat_insert(ncf, nt, NPF_PRI_LAST);

  ext = npf_rule_create("external", NPF_RULE_GROUP, NATIF);
  npf_rule_setprio(ext, NPF_PRI_LAST);
  npf_rule_insert(ncf, NULL, ext);
  rl = npf_rule_create(NULL, NPF_RULE_PASS | NPF_RULE_STATEFUL | NPF_RULE_OUT | NPF_RULE_FINAL, 0);
  npf_rule_setprio(rl, NPF_PRI_LAST);
  npf_rule_insert(ncf, ext, rl);
  def = npf_rule_create(0, NPF_RULE_IN | NPF_RULE_OUT | NPF_RULE_GROUP, 0); 
  npf_rule_setprio(def, NPF_PRI_LAST);
  rl2 = npf_rule_create(NULL, NPF_RULE_PASS | NPF_RULE_IN | NPF_RULE_OUT | NPF_RULE_FINAL, 0);
  npf_rule_setprio(rl2, NPF_PRI_LAST);
  npf_rule_insert(ncf, def, rl2);
  npf_rule_insert(ncf, NULL, def);
  rc = npf_config_submit(ncf, dummy_fd);
  if (rc) {
#ifdef _NPF_PRIVATE
    nl_error_t ne;
    _npf_config_error(ncf, &ne);
    _tme_npf_print_error(&ne, _output);
#else
    tme_log(&element->tme_element_log_handle, 0, -1,
	    (&element->tme_element_log_handle,
	     _("Could not add nat rule to npf (%d).  Try manually adding via npfctl."), rc));
#endif
    goto exit_nat;
  }
  npf_config_destroy(ncf);
  i=1;
  rc = ioctl(dummy_fd, IOC_NPF_SWITCH, &i);
  if (rc) {
    tme_log(&element->tme_element_log_handle, 0, -1,
	    (&element->tme_element_log_handle,
	     "Could not turn on NPF.  Try 'npfctl start'."));
  }

  tme_log(&element->tme_element_log_handle, 0, TME_OK,
	  (&element->tme_element_log_handle,
	   _("Added ip nat rules for tap interface network, "
	     "to npf.  Run 'npfctl show' to view the rules.  If you still have problems "
	     "with forwarding from the tap interface, you may need to manually adjust the filter rules, or remove conflicting nat "
	     "e.g., npfctl flush.  If you want ping, try 'modload npf_alg_icmp'. npf is the successor to pf (on NetBSD), so it may not be available on older systems.")));
  
#elif TME_DO_PFV
  // using PF for NAT
  fp = popen("pfctl -f -", "w");

  if(fp != NULL) {
#if defined(TUNGIFINFO) && !defined(TAPGIFINFO)
    // OpenBSD PF NAT syntax is different for PF >= 4.7
    fprintf(fp, "pass out on %s from %s:network to any nat-to %s\n", NATIF, TAPIF, NATIF);
#else
    fprintf(fp, 
	    "nat on %s from %s:network to any -> (%s)\n \
	    pass from %s:network to any keep state\n",
	    NATIF, TAPIF, NATIF, TAPIF);
    rc = ioctl(dummy_fd, DIOCSTART);
    if (rc) {
      tme_log(&element->tme_element_log_handle, 0, -1,
	      (&element->tme_element_log_handle,
	       "Could not turn on PF.  Try 'pfctl -e'."));
    }
#endif
    pclose(fp);
    tme_log(&element->tme_element_log_handle, 0, TME_OK,
	    (&element->tme_element_log_handle,
	     _("Added ip nat rules for tap interface network, "
	       "to pf.  Run 'pfctl -s rules' to view the rules.  If you still have problems "
	       "with forwarding from the tap interface, you may need to manually adjust the filter rules, or remove conflicting nat "
	       "e.g., pfctl -F rules.")));
  } else {
    tme_output_append_error(_output, _("couldn't open file pfctl; ip forwarding may not work."));
  }

#else // TME_DO_IPF

  memset(&nat, 0, sizeof(ipnat_t));
  nat.in_inip = netnum;
  nat.in_inmsk = TAPNETMASK.s_addr;
  nat.in_outip = nat_addrs[TME_IP_ADDRS_INET].s_addr;
  nat.in_outmsk = 0xffffffff;
  nat.in_redir = NAT_MAP;
  strncpy(nat.in_ifname, NATIF, IFNAMSIZ);
  rc = ioctl(dummy_fd, SIOCADNAT, &nat);
  if (rc) {
    tme_log(&element->tme_element_log_handle, 0, -1,
	    (&element->tme_element_log_handle,
	     _("Could not add nat rule to ipnat.  Try manually adding via 'ipnat %s %s/%d -> 0/32'."),
	     nat.in_ifname, inet_ntoa(TAPINET), netnum));
    goto exit_nat;
  }

  tme_log(&element->tme_element_log_handle, 0, TME_OK,
	  (&element->tme_element_log_handle,
	   _("Added ip nat rules for tap interface network, to ipnat. "
	     "Should be equivalent to 'ipnat %s %s/%d -> 0/32'."),
	   nat.in_ifname, inet_ntoa(TAPINET), netnum));

#endif

#ifdef HAVE_SYS_SYSCTL_H
  // Turn on forwarding if not already on
  forward = -1;
  len = sizeof(forward);
  mib[0] = CTL_NET;
  mib[1] = PF_INET;
  mib[2] = IPPROTO_IP;
  mib[3] = IPCTL_FORWARDING;
  rc = sysctl(mib, 4, &forward, &len, NULL, 0);
  if(!(rc || forward)) {
    forward = 1;
    rc = sysctl(mib, 4, NULL, 0, &forward, len);
    if(rc == -1) rc--;
  }

  if(rc < 0) {
    tme_log(&element->tme_element_log_handle, 0, errno,
	    (&element->tme_element_log_handle,
	     _("failed to set %s %s (%d); ip forwarding may not work properly"),
	     ((rc == -1) ? ("get") : ("set")),
	     SYSCTLNAME, forward));
  } else {
    tme_log(&element->tme_element_log_handle, 0, TME_OK,
	    (&element->tme_element_log_handle,
	     _("Enabled ipv4 forwarding!")));
  }
#endif

 exit_nat:
  close(dummy_fd);
#endif // TME_DO_APF

  return tme_eth_init(element, tap_fd, 4096, NULL, hwaddr, _tme_tun_tap_connections_new);

#undef TAPINET
#undef TAPNETMASK
#undef TAPBCAST
#undef TAPIF
#undef NATIF
#undef _TME_TAP_RAW_OPEN_ERROR
}
