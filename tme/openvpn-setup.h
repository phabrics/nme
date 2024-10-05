/* host/openvpn/openvpn-setup.h - OpenVPN Ethernet setup: */

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
#include <tme/threads.h>
#include <libopenvpn/syshead.h>
#include <libopenvpn/event.h>
#include <libopenvpn/tun.h>
#include <libopenvpn/link.h>

#define OPENVPN_CAN_WRITE 1
#define OPENVPN_FAST_IO 2

#ifdef TME_THREADS_FIBER
/* Events: */
typedef struct tme_fiber_event_set tme_event_set_t;
#else
/* Events: */
typedef struct event_set tme_event_set_t;
#endif

extern tme_event_set_t *(*tme_event_set_init) _TME_P((int *maxevents, unsigned int flags));
extern void (*tme_event_free) _TME_P((tme_event_set_t *es));
extern void (*tme_event_reset) _TME_P((tme_event_set_t *es));
extern int (*tme_event_del) _TME_P((tme_event_set_t *es, event_t event));
extern int (*tme_event_ctl) _TME_P((tme_event_set_t *es, event_t event, unsigned int rwflags, void *arg));
extern int (*tme_event_wait) _TME_P((tme_event_set_t *es, const struct timeval *tv, struct event_set_return *out, int outlen, tme_mutex_t *mutex));

struct env_set *openvpn_setup _TME_P((const char *args[], int argc, struct options *options));

struct frame *openvpn_setup_frame _TME_P((struct options *options, struct tuntap **tt, struct link_socket **sock, struct env_set *es, u_char *flags, tme_event_set_t **event_set));
