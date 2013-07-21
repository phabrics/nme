/* $Id: keyboard.c,v 1.9 2010/06/05 14:14:08 fredette Exp $ */

/* generic/keyboard.c - generic keyboard implementation support: */

/*
 * Copyright (c) 2003 Matt Fredette
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
_TME_RCSID("$Id: keyboard.c,v 1.9 2010/06/05 14:14:08 fredette Exp $");

/* includes: */
#include <tme/generic/keyboard.h>
#include <tme/hash.h>
#include <tme/misc.h>
#include <stdlib.h>
#include <errno.h>

/* macros: */

/* the shortest possible time, in milliseconds, between two human
   transitions on the same key: */
#define TME_KEYBOARD_SHORTEST_DOUBLE_MSEC	(80)

/* input stage zero uses a slightly expanded set of event types: */
#define TME_KEYBOARD_EVENT_IN0_RELEASE_USER	(0)
#define TME_KEYBOARD_EVENT_IN0_PRESS_USER	(1)
#define TME_KEYBOARD_EVENT_IN0_RELEASE_AUTO	(2)
#define TME_KEYBOARD_EVENT_IN0_PRESS_AUTO	(3)
#if TME_KEYBOARD_EVENT_RELEASE != TME_KEYBOARD_EVENT_IN0_RELEASE_USER
#error "TME_KEYBOARD_EVENT_RELEASE must be 0"
#endif
#if TME_KEYBOARD_EVENT_PRESS != TME_KEYBOARD_EVENT_IN0_PRESS_USER
#error "TME_KEYBOARD_EVENT_PRESS must be 1"
#endif

/* this macro turns an input stage zero pressed value into the
   corresponding release event type: */
#define TME_KEYBOARD_IN0_RELEASE_EVENT(pressed)	((pressed) ^ 1)

/* these macros evaluate to nonzero iff a keyval is pressed in the
   different stages: */
#define TME_KEYBOARD_PRESSED_IN0(keysym)	\
  ((keysym)->tme_keysym_state_in0_pressed)
#define TME_KEYBOARD_PRESSED_IN1(keysym)	\
  ((keysym)->tme_keysym_state_in1_keymode.tme_keymode_state_pressed)
#define _TME_KEYBOARD_PRESSED_IN2(keysym, prev)	\
  ((keysym)->tme_keysym_state_in2_pressed	\
   || (!(keysym)->tme_keysym_state_in2_released	\
       && prev))
#define TME_KEYBOARD_PRESSED_IN2(keysym)	\
  _TME_KEYBOARD_PRESSED_IN2(keysym, TME_KEYBOARD_PRESSED_IN1(keysym))
#define _TME_KEYBOARD_PRESSED_OUT0(keysym, prev)\
  ((keysym)->tme_keysym_state_out0_pressed	\
   || (!(keysym)->tme_keysym_state_out0_released\
       && prev))
#define TME_KEYBOARD_PRESSED_OUT0(keysym)	\
  _TME_KEYBOARD_PRESSED_OUT0(keysym, TME_KEYBOARD_PRESSED_IN2(keysym))
#define TME_KEYBOARD_PRESSED_OUT1(keycode)	\
  ((keycode)->tme_keycode_state_keymode.tme_keymode_state_pressed)

/* types: */

struct tme_keyboard_buffer_int;
struct tme_keysym_state;

/* keymode state: */
struct tme_keymode_state {

  /* keys that may be autorepeating are kept on a linked list: */
  struct tme_keymode_state *tme_keymode_state_next;

  /* the state for this keysym.  technically, since output stage one
     deals in keycodes, this really should be a void * and point to
     the keysym state for input stage one, and point to the keycode
     state for output stage one.  

     however, avoiding the void * allows us to avoid some function
     pointer casting, and in the output stage one case we don't care
     which of the many keysyms that may map to the same keycode is
     stored here - we just immediately grab the keycode state out of
     that keysym: */
  struct tme_keysym_state *tme_keymode_state_keysym;

  /* the keymode mode: */
  int tme_keymode_state_mode;

  /* this is nonzero iff the key is pressed in the physical sense: */
  int tme_keymode_state_pressed;

  /* the last time this key was released: */
  tme_uint32_t tme_keymode_state_last_release;

  /* this is nonzero iff a genuine release should be ignored: */
  int tme_keymode_state_ignore_release;
};

/* a keymode stage: */
struct tme_keymode_stage {

  /* the global keymode: */
  int tme_keymode_stage_global_mode;

  /* the list of keymode states for keys that must not autorepeat: */
  struct tme_keymode_state *tme_keymode_stage_no_autorepeats;

  /* the next stage: */
  int (*tme_keymode_stage_next) _TME_P((struct tme_keyboard_buffer_int *,
					struct tme_keysym_state *,
					tme_uint32_t));
};

/* keycode state: */
struct tme_keycode_state {

  /* the keycode: */
  tme_keyboard_keyval_t tme_keycode_state_keycode;

  /* the keycode keymode state: */
  struct tme_keymode_state tme_keycode_state_keymode;
};

/* keysym state.  one of these is kept for every keysym controlled by
   one or more input or output stages: */
struct tme_keysym_state {

  /* this keysym: */
  tme_keyboard_keyval_t tme_keysym_state_keysym;

  /* input stage zero: */

  /* if greater than TME_KEYBOARD_MODIFIER_NONE, this is the modifier
     that this keysym is attached to in input stage zero: */
  int tme_keysym_state_in0_modifier;

  /* the keysym states for all keys attached to the same modifier in
     input stage zero are kept on a linked list: */
  struct tme_keysym_state *tme_keysym_state_in0_modifier_next;

  /* this is nonzero iff the keysym is being pressed in input stage
     zero.  it's really either FALSE, or
     TME_KEYBOARD_EVENT_IN0_PRESS_USER, or
     TME_KEYBOARD_EVENT_IN0_PRESS_AUTO, which is why the last two are
     nonzero: */
  unsigned int tme_keysym_state_in0_pressed;

  /* the last time this keysym was pressed in input stage zero.  this
     is a time in milliseconds: */
  tme_uint32_t tme_keysym_state_in0_press_time;

  /* input stage one: */
  
  /* the input stage one keymode state: */
  struct tme_keymode_state tme_keysym_state_in1_keymode;

  /* input stage two: */

  /* this is nonzero iff the keysym is being released in input stage
     two: */
  unsigned int tme_keysym_state_in2_released;

  /* this is nonzero iff the keysym is being pressed in input stage
     two: */
  unsigned int tme_keysym_state_in2_pressed;

  /* output stage zero: */

  /* if non-NULL, this is the keycode that this keysym is mapped to in
     output stage zero: */
  struct tme_keycode_state *tme_keysym_state_out0_keycode;

  /* if this keysym is not attached to any modifier on the output
     stage zero, it may require that certain output side modifiers be
     set or clear in order for the keycode to mean the given keysym: */
  tme_keyboard_modifiers_t tme_keysym_state_out0_modifiers_set;
  tme_keyboard_modifiers_t tme_keysym_state_out0_modifiers_clear;

  /* iff greater than TME_KEYBOARD_MODIFIER_NONE, this is the modifier
     that this keysym is attached to in output stage zero: */
  int tme_keysym_state_out0_modifier;

  /* the keysym states for all keys attached to the same modifier in
     output stage zero are kept on a linked list: */
  struct tme_keysym_state *tme_keysym_state_out0_modifier_next;

  /* this is nonzero iff the keysym is being released in output stage
     zero: */
  unsigned int tme_keysym_state_out0_released;

  /* this is nonzero iff the keysym is being pressed in output stage
     zero: */
  unsigned int tme_keysym_state_out0_pressed;

  /* if this keysym is pressed in the output stage zero but required
     output stage zero modifier changes to be so, this is the list of
     those changes.  since most keyboards generate the same keysym
     using the same modifier(s), this list will usually be empty: */
  struct tme_keysym_state **tme_keysym_state_out0_keysyms;
  unsigned int *tme_keysym_state_out0_press_flags;

  /* output stage one: */
  
  /* this is nonzero iff the next release seen by output stage one 
     will not affect the output modifiers mask: */
  int tme_keysym_state_out1_ignore_release;
};

/* keyboard macros are necessary because it's almost certain that the
   keyboard you want to emulate has keysyms that your keyboard doesn't
   have.  one keyboard macro takes a *sequence* of one or more pressed
   keysyms to a *set* of one or more released and pressed keysyms.

   all keysym macros are kept in a single tree, where a single branch
   represents the next keysym in the sequences for one or more macros.

   a node in the macros tree is active iff the keysyms on the path
   from the root node have been pressed in sequence and remain
   pressed.  if there are any macros at all, the root node is always
   active: */
struct tme_keyboard_macro {

  /* a pointer up to our parent node, and the keysym on the branch
     from our parent to us.  for the root node, these are NULL and
     TME_KEYBOARD_KEYVAL_UNDEF, respectively: */
  struct tme_keyboard_macro *tme_keyboard_macro_parent;
  tme_keyboard_keyval_t tme_keyboard_macro_keysym;

  /* all active nodes are on a list.  the root node is always active,
     and it must be the last node on this list - making the is-active
     test for all other nodes as simple as testing this pointer
     against NULL: */
  struct tme_keyboard_macro *tme_keyboard_macro_active_next;

  /* non-leaf nodes branch out by keysym: */
  tme_hash_t tme_keyboard_macro_branches;

  /* leaf nodes contain the set of keysyms that the recognized
     sequence maps to.  in addition to presses of one or more new
     keysyms, this will normally include releases of some or all of
     the keysyms in the original sequence: */
  unsigned int tme_keyboard_macro_length;
  struct tme_keysym_state **tme_keyboard_macro_keysyms;
  unsigned int *tme_keyboard_macro_press_flags;
};

/* an internal keyboard buffer: */
struct tme_keyboard_buffer_int {

  /* the public keyboard buffer.  this must be first: */
  struct tme_keyboard_buffer tme_keyboard_buffer;
#define tme_keyboard_buffer_int_size tme_keyboard_buffer.tme_keyboard_buffer_size
#define tme_keyboard_buffer_int_head tme_keyboard_buffer.tme_keyboard_buffer_head
#define tme_keyboard_buffer_int_tail tme_keyboard_buffer.tme_keyboard_buffer_tail
#define tme_keyboard_buffer_int_events tme_keyboard_buffer.tme_keyboard_buffer_events
#define tme_keyboard_buffer_int_log_handle tme_keyboard_buffer.tme_keyboard_buffer_log_handle

  /* the keysyms state, common to all stages: */
  tme_hash_t tme_keyboard_buffer_int_keysyms_state;

  /* input stage zero: */

  /* this is nonzero iff input stage zero has modifier information,
     and it's actually the mask of modifiers that we have keysyms for: */
  unsigned int tme_keyboard_buffer_int_in0_have_modifiers;

  /* the lists of keysyms that are attached to input stage zero
     modifiers: */
  struct tme_keysym_state *tme_keyboard_buffer_int_in0_modkeys[TME_KEYBOARD_MODIFIER_MAX + 1];

  /* the current input stage zero modifiers mask: */
  tme_keyboard_modifiers_t tme_keyboard_buffer_int_in0_modifiers;

  /* the current input stage zero pressed keycodes, mapped to their
     corresponding struct tme_keysym_states: */
  tme_hash_t tme_keyboard_buffer_int_in0_keycodes;

  /* input stage one: */

  /* the input stage one keymode stage: */
  struct tme_keymode_stage tme_keyboard_buffer_int_in1_keymode_stage;

  /* input stage two: */

  /* this is NULL iff input stage two is a passthrough, else this is
     the list of active nodes in the input stage two keysym macros
     tree: */
  struct tme_keyboard_macro *tme_keyboard_buffer_int_in2_macros_active;

  /* the root of the input stage two keysym macros tree: */
  struct tme_keyboard_macro tme_keyboard_buffer_int_in2_macros_root;

  /* output stage zero: */

  /* this is nonzero iff output stage zero is just a passthrough: */
  unsigned int tme_keyboard_buffer_int_out0_passthrough;

  /* the output stage zero keycodes: */
  tme_hash_t tme_keyboard_buffer_int_out0_keycodes;

  /* this is nonzero iff the output stage zero lock modifier is to be
     treated as caps lock: */
  int tme_keyboard_buffer_int_out0_lock_is_caps;

  /* any output stage zero modifier that the Num_Lock keysym is
     attached to: */
  int tme_keyboard_buffer_int_out0_mod_num_lock;

  /* the lists of keysyms that are output stage zero modifiers: */
  struct tme_keysym_state *tme_keyboard_buffer_int_out0_modkeys[TME_KEYBOARD_MODIFIER_MAX + 1];

  /* the current output stage zero modifiers mask: */
  tme_keyboard_modifiers_t tme_keyboard_buffer_int_out0_modifiers;

  /* output stage one: */

  /* the output stage one keymode stage: */
  struct tme_keymode_stage tme_keyboard_buffer_int_out1_keymode_stage;
};

/* prototypes: */
static int _tme_keyboard_buffer_in2 _TME_P((struct tme_keyboard_buffer_int *, 
					    struct tme_keysym_state *,
					    tme_uint32_t));
static int _tme_keyboard_buffer_out1_bottom _TME_P((struct tme_keyboard_buffer_int *,
						    struct tme_keysym_state *,
						    tme_uint32_t));

/* this is for debugging only: */
#if 0
static void
_tme_keyboard_debug(const struct tme_keyboard_buffer_int *buffer,
		    const char *stage,
		    tme_keyboard_keyval_t keyval,
		    int is_press,
		    tme_uint32_t event_time)
{
  struct tme_log_handle *handle;
  const char *string;
  extern const char *_tme_gtk_keyboard_keyval_name _TME_P((tme_keyboard_keyval_t));

  handle = buffer->tme_keyboard_buffer_int_log_handle;
  if (handle == NULL) {
    return;
  }

  string = _tme_gtk_keyboard_keyval_name(keyval);
  if (string == NULL) {
    string = "???";
  }

  tme_log(handle, 100, TME_OK,
	  (handle,
	   "%s event: time %lu key %lu (%s) %s",
	   stage,
	   (unsigned long) event_time,
	   (unsigned long) keyval,
	   string,
	   (is_press
	    ? "press"
	    : "release")));
}
#else
#define _tme_keyboard_debug(b, s, k, p, t) \
  do { } while(/* CONSTCOND */ 0)	  
#endif

/* this creates a new keyboard buffer: */
struct tme_keyboard_buffer *
tme_keyboard_buffer_new(unsigned int size)
{
  struct tme_keyboard_buffer_int *buffer;
  struct tme_keymode_stage *stage;
  struct tme_keyboard_macro *root;
  int modifier;

  /* round the buffer size up to a power of two: */
  if (size & (size - 1)) {
    do {
      size &= (size - 1);
    } while (size & (size - 1));
    size <<= 1;
  }

  /* allocate the buffer: */
  buffer = tme_new0(struct tme_keyboard_buffer_int, 1);

  /* set the buffer size: */
  buffer->tme_keyboard_buffer_int_size = size;

  /* set the head and tail pointers: */
  buffer->tme_keyboard_buffer_int_head = 0;
  buffer->tme_keyboard_buffer_int_tail = 0;

  /* allocate the buffer events: */
  buffer->tme_keyboard_buffer_int_events
    = tme_new(struct tme_keyboard_event, size);

  /* for now there is no log handle: */
  buffer->tme_keyboard_buffer_int_log_handle = NULL;

  /* create the common keysyms state: */
  buffer->tme_keyboard_buffer_int_keysyms_state
    = tme_hash_new(tme_direct_hash,
		   tme_direct_compare,
		   (tme_hash_data_t) NULL);

  /* input stage zero begins with no modifiers: */
  buffer->tme_keyboard_buffer_int_in0_have_modifiers = FALSE;

  /* initialize the input stage zero modifier keys lists: */
  for (modifier = 0;
       modifier <= TME_KEYBOARD_MODIFIER_MAX;
       modifier++) {
    buffer->tme_keyboard_buffer_int_in0_modkeys[modifier] = NULL;
  }

  /* initialize the input stage zero modifiers mask: */
  buffer->tme_keyboard_buffer_int_in0_modifiers = 0;

  /* initialize the input stage one keycodes hash: */
  buffer->tme_keyboard_buffer_int_in0_keycodes
    = tme_hash_new(tme_direct_hash,
		   tme_direct_compare,
		   (tme_hash_data_t) NULL);

  /* initialize the input stage one keymode stage: */
  stage = &buffer->tme_keyboard_buffer_int_in1_keymode_stage;
  stage->tme_keymode_stage_global_mode = 0;
  stage->tme_keymode_stage_no_autorepeats = NULL;
  stage->tme_keymode_stage_next = _tme_keyboard_buffer_in2;

  /* input stage two begins with no macros, so not even the root of
     the macros tree is active: */
  buffer->tme_keyboard_buffer_int_in2_macros_active = NULL;
  
  /* create the root of the input stage two keysym macros tree: */
  root = &buffer->tme_keyboard_buffer_int_in2_macros_root;
  root->tme_keyboard_macro_parent = NULL;
  root->tme_keyboard_macro_keysym = TME_KEYBOARD_KEYVAL_UNDEF;
  root->tme_keyboard_macro_active_next = NULL;
  root->tme_keyboard_macro_branches
    = tme_hash_new(tme_direct_hash,
		   tme_direct_compare,
		   (tme_hash_data_t) NULL);

  /* output stage zero begins as a passthrough: */
  buffer->tme_keyboard_buffer_int_out0_passthrough = TRUE;

  /* initialize the output stage zero keycodes: */
  buffer->tme_keyboard_buffer_int_out0_keycodes
    = tme_hash_new(tme_direct_hash,
		   tme_direct_compare,
		   (tme_hash_data_t) NULL);

  /* the output stage zero lock modifier is assumed to be a Shift lock: */
  buffer->tme_keyboard_buffer_int_out0_lock_is_caps
    = FALSE;

  /* initialize the output stage zero modifier that the Num_Lock
     keysym is attached to: */
  buffer->tme_keyboard_buffer_int_out0_mod_num_lock
    = TME_KEYBOARD_MODIFIER_NONE;

  /* initialize the output stage zero modifier keys lists: */
  for (modifier = 0;
       modifier <= TME_KEYBOARD_MODIFIER_MAX;
       modifier++) {
    buffer->tme_keyboard_buffer_int_out0_modkeys[modifier] = NULL;
  }

  /* initialize the output stage zero modifiers mask: */
  buffer->tme_keyboard_buffer_int_out0_modifiers = 0;

  /* initialize the output stage one keymode stage: */
  stage = &buffer->tme_keyboard_buffer_int_out1_keymode_stage;
  stage->tme_keymode_stage_global_mode = 0;
  stage->tme_keymode_stage_no_autorepeats = NULL;
  stage->tme_keymode_stage_next = _tme_keyboard_buffer_out1_bottom;

  /* done: */
  return (&buffer->tme_keyboard_buffer);
}

/* this destroys an entry in the common keysyms state: */
static void
_tme_keysym_state_destroy(tme_hash_data_t __keysym,
			  tme_hash_data_t _keysym,
			  void *_junk)
{
  struct tme_keysym_state *keysym;

  /* recover the keysym state: */
  keysym = (struct tme_keysym_state *) _keysym;

  /* if this keysym state has output stage zero keysym changes, free
     them: */
  if (keysym->tme_keysym_state_out0_keysyms != NULL) {
    tme_free(keysym->tme_keysym_state_out0_keysyms);
    tme_free(keysym->tme_keysym_state_out0_press_flags);
  }

  /* free the state itself: */
  tme_free(keysym);
}

/* this recursively destroys the input stage two keysym macros tree: */
static void
_tme_keyboard_macro_destroy(tme_hash_data_t _keysym,
			    tme_hash_data_t _macro, 
			    void *_junk)
{
  struct tme_keyboard_macro *macro;

  /* get this macro: */
  macro = (struct tme_keyboard_macro *) _macro;

  /* if this is a leaf node: */
  if (macro->tme_keyboard_macro_branches == NULL) {

    /* free the keysyms and flags: */
    tme_free(macro->tme_keyboard_macro_keysyms);
    tme_free(macro->tme_keyboard_macro_press_flags);
  }

  /* otherwise, recurse: */
  else {
    tme_hash_foreach(macro->tme_keyboard_macro_branches,
		     _tme_keyboard_macro_destroy,
		     NULL);
    tme_hash_destroy(macro->tme_keyboard_macro_branches);
  }

  /* free this tree node: */
  tme_free(macro);
}

/* this destroys an entry in the output stage zero keycodes state: */
static void
_tme_keycode_state_destroy(tme_hash_data_t __keycode,
			   tme_hash_data_t _keycode,
			   void *_junk)
{
  struct tme_keycode_state *keycode;

  /* recover the keycode state: */
  keycode = (struct tme_keycode_state *) _keycode;

  /* free the state itself: */
  tme_free(keycode);
}

/* this destroys a keyboard buffer: */
void
tme_keyboard_buffer_destroy(struct tme_keyboard_buffer *_buffer)
{
  struct tme_keyboard_buffer_int *buffer;

  /* recover our data structure: */
  buffer = (struct tme_keyboard_buffer_int *) _buffer;

  /* free the events: */
  tme_free(buffer->tme_keyboard_buffer_int_events);

  /* destroy the common keysyms state: */
  tme_hash_foreach(buffer->tme_keyboard_buffer_int_keysyms_state,
		   _tme_keysym_state_destroy,
		   NULL);
  tme_hash_destroy(buffer->tme_keyboard_buffer_int_keysyms_state);

  /* destroy the input stage two keysym macros tree: */
  tme_hash_foreach(buffer->tme_keyboard_buffer_int_in2_macros_root.tme_keyboard_macro_branches,
		   _tme_keyboard_macro_destroy,
		   NULL);
  tme_hash_destroy(buffer->tme_keyboard_buffer_int_in2_macros_root.tme_keyboard_macro_branches);

  /* destroy the output stage zero keycodes: */
  tme_hash_foreach(buffer->tme_keyboard_buffer_int_out0_keycodes,
		   _tme_keycode_state_destroy,
		   NULL);
  tme_hash_destroy(buffer->tme_keyboard_buffer_int_out0_keycodes);

  /* destroy the buffer itself: */
  tme_free(buffer);
}

/* this gets the state for a keysym, creating a new state if one
   doesn't exists yet: */
static struct tme_keysym_state *
_tme_keysym_state_get(struct tme_keyboard_buffer_int *buffer,
		      tme_keyboard_keyval_t _keysym)
{
  struct tme_keysym_state *keysym;

  /* look up the state for this keysym: */
  keysym
    = ((struct tme_keysym_state *)
       tme_hash_lookup(buffer->tme_keyboard_buffer_int_keysyms_state,
		       tme_keyboard_hash_data_from_keyval(_keysym)));

  /* if the state doesn't exist, allocate it: */
  if (keysym == NULL) {
    keysym = tme_new0(struct tme_keysym_state, 1);

    /* initialize all fields that might not be properly initialized as
       all-bits-zero: */
    keysym->tme_keysym_state_keysym = _keysym;
    keysym->tme_keysym_state_in0_modifier = TME_KEYBOARD_MODIFIER_NONE;
    keysym->tme_keysym_state_in1_keymode.tme_keymode_state_keysym = keysym;
    keysym->tme_keysym_state_out0_keycode = NULL;
    keysym->tme_keysym_state_out0_modifier = TME_KEYBOARD_MODIFIER_NONE;
    keysym->tme_keysym_state_out0_keysyms = NULL;
    keysym->tme_keysym_state_out0_press_flags = NULL;

    /* insert this state into the hash: */
    tme_hash_insert(buffer->tme_keyboard_buffer_int_keysyms_state,
		    tme_keyboard_hash_data_from_keyval(_keysym),
		    (tme_hash_data_t) keysym);
  }

  /* done: */
  return (keysym);
}

/* this changes the set of keysyms that are attached to an input stage
   zero modifier: */
int
tme_keyboard_buffer_in_modifier(struct tme_keyboard_buffer *_buffer,
				int modifier,
				const tme_keyboard_keyval_t *modkeys)
{
  struct tme_keyboard_buffer_int *buffer;
  struct tme_keysym_state *mod_keysym, **_mod_keysym;
  tme_keyboard_keyval_t keysym;

  /* recover our data structure: */
  buffer = (struct tme_keyboard_buffer_int *) _buffer;

  /* this must be a valid modifier: */
  assert (modifier > TME_KEYBOARD_MODIFIER_NONE
	  && modifier <= TME_KEYBOARD_MODIFIER_MAX);

  /* remove all currently attached keysyms from this modifier: */
  for (mod_keysym = buffer->tme_keyboard_buffer_int_in0_modkeys[modifier];
       mod_keysym != NULL;
       mod_keysym = mod_keysym->tme_keysym_state_in0_modifier_next) {
    mod_keysym->tme_keysym_state_in0_modifier
      = TME_KEYBOARD_MODIFIER_NONE;
  }

  /* attach all of these new keysyms to this modifier: */
  _mod_keysym = &buffer->tme_keyboard_buffer_int_in0_modkeys[modifier];
  for (; (keysym = *(modkeys++)) != TME_KEYBOARD_KEYVAL_UNDEF; ) {
    mod_keysym = _tme_keysym_state_get(buffer, keysym);
    mod_keysym->tme_keysym_state_in0_modifier = modifier;
    *_mod_keysym = mod_keysym;
    _mod_keysym = &mod_keysym->tme_keysym_state_in0_modifier_next;
  }
  *_mod_keysym = NULL;

  /* input stage zero now has modifier information: */
  buffer->tme_keyboard_buffer_int_in0_have_modifiers
    |= (1 << modifier);

  return (TME_OK);
}

/* this changes a keysym's input stage one keymode: */
int
tme_keyboard_buffer_in_mode(struct tme_keyboard_buffer *_buffer,
			    tme_keyboard_keyval_t _keysym, int mode)
{
  struct tme_keyboard_buffer_int *buffer;
  struct tme_keysym_state *keysym;
  
  /* recover our data structure: */
  buffer = (struct tme_keyboard_buffer_int *) _buffer;

  /* there's no such thing as a global input keymode: */
  assert (_keysym != TME_KEYBOARD_KEYVAL_UNDEF);

  /* TME_KEYBOARD_MODE_UNLOCK and TME_KEYBOARD_MODE_LOCK cannot be
     combined with any other bits: */
  if ((mode
       & (TME_KEYBOARD_MODE_UNLOCK
	  | TME_KEYBOARD_MODE_LOCK))
      && (mode
	  & (mode - 1))) {
    return (EINVAL);
  }
  
  /* none of the TME_KEYBOARD_MODE_FLAG_NO_AUTOREPEATS,
     TME_KEYBOARD_MODE_FLAG_NO_RELEASES, and
     TME_KEYBOARD_MODE_FLAG_LOCK_SOFT flags can be set
     without TME_KEYBOARD_MODE_PASSTHROUGH: */
  if ((mode
       & (TME_KEYBOARD_MODE_FLAG_NO_AUTOREPEATS
	  | TME_KEYBOARD_MODE_FLAG_NO_RELEASES
	  | TME_KEYBOARD_MODE_FLAG_LOCK_SOFT))
      && !(mode
	   & TME_KEYBOARD_MODE_PASSTHROUGH)) {
    return (EINVAL);
  }

  /* you cannot specify that an input key must not release, or
     that it soft locks: */
  if (mode
      & (TME_KEYBOARD_MODE_FLAG_NO_RELEASES
	 | TME_KEYBOARD_MODE_FLAG_LOCK_SOFT)) {
    return (EINVAL);
  }

  /* look up this keysym and set the input stage one mode: */
  keysym = _tme_keysym_state_get(buffer, _keysym);
  keysym->tme_keysym_state_in1_keymode.tme_keymode_state_mode = mode;
  return (TME_OK);
}

/* this adds an input stage two keysym macro: */
int
tme_keyboard_buffer_in_macro(struct tme_keyboard_buffer *_buffer,
			     const tme_keyboard_keyval_t *keysyms_lhs,
			     const tme_keyboard_keyval_t *keysyms_rhs)
{
  struct tme_keyboard_buffer_int *buffer;
  unsigned int count_lhs, count_rhs;
  unsigned int keysym_i, keysym_j, keysym_count;
  tme_keyboard_keyval_t keysym;
  struct tme_keysym_state **keysyms;
  unsigned int *press_flags;
  int rc;
  struct tme_keyboard_macro *macro, *macro_next;

  /* recover our data structure: */
  buffer = (struct tme_keyboard_buffer_int *) _buffer;

  /* count the number of keysyms on both sides: */
  for (count_lhs = 0;
       keysyms_lhs[count_lhs] != TME_KEYBOARD_KEYVAL_UNDEF;
       count_lhs++);
  for (count_rhs = 0;
       keysyms_rhs[count_rhs] != TME_KEYBOARD_KEYVAL_UNDEF;
       count_rhs++);

  /* there must be some left-hand side and some right-hand side: */
  if (count_lhs == 0
      || count_rhs == 0) {
    return (EINVAL);
  }

  /* create the final keysyms and press-flags arrays for the macro.  any
     keysym on the left hand side that is also on the right hand side
     becomes a press, else a release, and any keysym on the right hand
     side that isn't on the left hand side becomes a press: */
  keysyms = tme_new(struct tme_keysym_state *, count_lhs + count_rhs);
  press_flags = tme_new(unsigned int, count_lhs + count_rhs);
  keysym_count = 0;
  for (keysym_i = 0; 
       keysym_i < count_lhs; 
       keysym_i++) {
    keysym = keysyms_lhs[keysym_i];
  
    /* see if this keysym is on the right hand side: */
    for (keysym_j = 0;
	 keysym_j < count_rhs;
	 keysym_j++) {
      if (keysym == keysyms_rhs[keysym_j]) {
	break;
      }
    }

    /* set this keysym and press-flags: */
    keysyms[keysym_count] = _tme_keysym_state_get(buffer, keysym);
    press_flags[keysym_count] = (keysym_j < count_rhs);
    keysym_count++;
  }
  for (keysym_j = 0;
       keysym_j < count_rhs;
       keysym_j++) {
    keysym = keysyms_rhs[keysym_j];

    /* see if this keysym is on the left hand side: */
    for (keysym_i = 0;
	 keysym_i < count_lhs;
	 keysym_i++) {
      if (keysym == keysyms_lhs[keysym_i]) {
	break;
      }
    }

    /* set this keysym and press-flags: */
    if (keysym_i == count_lhs) {
      keysyms[keysym_count] = _tme_keysym_state_get(buffer, keysym);
      press_flags[keysym_count] = TRUE;
      keysym_count++;
    }
  }
  
  /* the last keysym in any macro's right hand side must be a press: */
  if (!press_flags[keysym_count - 1]) {
    tme_free(keysyms);
    tme_free(press_flags);
    return (EINVAL);
  }
  
  /* add this keysym macro to the macros tree.  this macro's sequence
     (i.e., its left-hand side) cannot be strictly longer, or strictly
     shorter, or the same as any existing macro's sequence: */
  macro = &buffer->tme_keyboard_buffer_int_in2_macros_root;
  rc = TME_OK;
  for (keysym_i = 0;
       ;
       keysym_i++) {

    /* if we handled all left-hand side keysyms: */
    if (keysym_i == count_lhs) {
      
      /* if this node is already a non-leaf node, then this macro's
	 sequence is strictly shorter than an existing macro's
	 sequence: */
      if (macro->tme_keyboard_macro_branches != NULL) {
	rc = EEXIST;
      }

      /* otherwise, if this node is already a leaf node, then this
	 macro's sequence is the same as an existing macro's sequence: */
      else if (macro->tme_keyboard_macro_length > 0) {
	rc = EEXIST;
      }

      /* stop no matter what: */
      break;
    }

    /* if this node has no branch set: */
    if (macro->tme_keyboard_macro_branches == NULL) {

      /* if this node is already a leaf node, then this macro's
	 sequence is strictly longer than an existing macro's
	 sequence: */
      if (macro->tme_keyboard_macro_length > 0) {
	rc = EEXIST;
	break;
      }

      /* otherwise, create a branch set for this node: */
      macro->tme_keyboard_macro_branches
	= tme_hash_new(tme_direct_hash,
		       tme_direct_compare,
		       (tme_hash_data_t) NULL);
    }
      
    /* get the keysym: */
    keysym = keysyms_lhs[keysym_i];

    /* look up this keysym in the branch set for this node: */
    macro_next
      = ((struct tme_keyboard_macro *)
	 tme_hash_lookup(macro->tme_keyboard_macro_branches,
			 tme_keyboard_hash_data_from_keyval(keysym)));

    /* if this keysym is a new branch, create a new macros tree node: */
    if (macro_next == NULL) {
      macro_next = tme_new0(struct tme_keyboard_macro, 1);
      macro_next->tme_keyboard_macro_parent = macro;
      macro_next->tme_keyboard_macro_keysym = keysym;
      tme_hash_insert(macro->tme_keyboard_macro_branches, 
		      tme_keyboard_hash_data_from_keyval(keysym),
		      (tme_hash_data_t) macro_next);
    }

    /* advance in the tree: */
    macro = macro_next;
  }

  /* if this sequence couldn't be added to the sequences tree: */
  if (rc != TME_OK) {
    tme_free(keysyms);
    tme_free(press_flags);
    return (rc);
  }

  /* finish this leaf node in the macros tree: */
  macro->tme_keyboard_macro_length = keysym_count;
  macro->tme_keyboard_macro_keysyms = keysyms;
  macro->tme_keyboard_macro_press_flags = press_flags;

  /* if this is the first keysym macro added, set the root of the
     keysym macros tree as active, making input stage two no longer a
     passthrough: */
  if (buffer->tme_keyboard_buffer_int_in2_macros_active
      == NULL) {
    buffer->tme_keyboard_buffer_int_in2_macros_active
      = &buffer->tme_keyboard_buffer_int_in2_macros_root;
  }

  return (TME_OK);
}

/* this adds a single output stage zero keysym map entry: */
int
tme_keyboard_buffer_out_map(struct tme_keyboard_buffer *_buffer,
			    _tme_const struct tme_keyboard_map *map)
{
  struct tme_keyboard_buffer_int *buffer;
  struct tme_keysym_state *keysym;
  struct tme_keycode_state *keycode;
  struct tme_keymode_state *keymode;
  int modifier;
  tme_keyboard_modifiers_t modifiers_set, modifiers_clear;
  
  /* recover our data structure: */
  buffer = (struct tme_keyboard_buffer_int *) _buffer;

  /* the keysym must be defined: */
  assert (map->tme_keyboard_map_keysym
	  != TME_KEYBOARD_KEYVAL_UNDEF);

  /* get the state for this keysym: */
  keysym = _tme_keysym_state_get(buffer, map->tme_keyboard_map_keysym);

  /* this keysym must not already have an output side keycode: */
  if (keysym->tme_keysym_state_out0_keycode != NULL) {
    return (EEXIST);
  }

  /* lookup this keycode: */
  keycode
    = ((struct tme_keycode_state *)
       tme_hash_lookup(buffer->tme_keyboard_buffer_int_out0_keycodes,
		       tme_keyboard_hash_data_from_keyval(map->tme_keyboard_map_keycode)));

  /* if this keycode is new, allocate, initialize and add a structure
     for it: */
  if (keycode == NULL) {

    /* allocate the keycode state: */
    keycode = tme_new0(struct tme_keycode_state, 1);

    /* initialize any parts of the keycode state that might not be
       properly initialized as all-bits-zero.  note that the keysym
       stored in the keycode keymode state is the first keysym mapped
       to the keycode: */
    keycode->tme_keycode_state_keycode = map->tme_keyboard_map_keycode;
    keymode = &keycode->tme_keycode_state_keymode;
    keymode->tme_keymode_state_keysym = keysym;

    /* add the keycode structure: */
    tme_hash_insert(buffer->tme_keyboard_buffer_int_out0_keycodes,
		    tme_keyboard_hash_data_from_keyval(map->tme_keyboard_map_keycode),
		    (tme_hash_data_t) keycode);
  }

  /* set this keycode on this keysym: */
  keysym->tme_keysym_state_out0_keycode = keycode;
     
  /* if this keysym is attached to an output side modifier: */
  modifier = map->tme_keyboard_map_modifier;
  if (modifier != TME_KEYBOARD_MODIFIER_NONE) {

    /* attach this keysym to the output side modifier: */
    keysym->tme_keysym_state_out0_modifier
      = modifier;
    keysym->tme_keysym_state_out0_modifier_next
      = buffer->tme_keyboard_buffer_int_out0_modkeys[modifier];
    buffer->tme_keyboard_buffer_int_out0_modkeys[modifier]
      = keysym;

    /* dispatch on any special keysym note: */
    switch (map->tme_keyboard_map_keysym_note) {
    default: assert(FALSE);
    case TME_KEYBOARD_KEYSYM_NOTE_UNDEF:
      break;
    case TME_KEYBOARD_KEYSYM_NOTE_CAPS_LOCK:
      if (modifier == TME_KEYBOARD_MODIFIER_LOCK) {
	buffer->tme_keyboard_buffer_int_out0_lock_is_caps = TRUE;
      }
      break;
    case TME_KEYBOARD_KEYSYM_NOTE_SHIFT_LOCK:
      break;
    case TME_KEYBOARD_KEYSYM_NOTE_NUM_LOCK:
      buffer->tme_keyboard_buffer_int_out0_mod_num_lock = modifier;
      break;
    }

    /* this keysym cannot require any output side modifiers to be set
       or clear: */
    assert (map->tme_keyboard_map_modifiers_set == 0
	    && map->tme_keyboard_map_modifiers_clear == 0);
  }

  /* remember the output stage zero modifiers that must be set or
     clear for this mapping to work: */
  modifiers_set = map->tme_keyboard_map_modifiers_set;
  modifiers_clear = map->tme_keyboard_map_modifiers_clear;
  assert ((modifiers_set & modifiers_clear) == 0);

  /* if this keysym is lowercase, it also requires the shift modifier
     to be clear: */
  if (modifiers_clear
      & (1 << TME_KEYBOARD_MODIFIER_LOCK)) {
    modifiers_clear
      |= (1 << TME_KEYBOARD_MODIFIER_SHIFT);
  }

  keysym->tme_keysym_state_out0_modifiers_set = modifiers_set;
  keysym->tme_keysym_state_out0_modifiers_clear = modifiers_clear;

  /* output stage zero is no longer a passthrough: */
  buffer->tme_keyboard_buffer_int_out0_passthrough = FALSE;

  return (TME_OK);
}

/* this changes a keycode's output stage one mode: */
int
tme_keyboard_buffer_out_mode(struct tme_keyboard_buffer *_buffer,
			     tme_keyboard_keyval_t _keycode, int mode)
{
  struct tme_keyboard_buffer_int *buffer;
  struct tme_keycode_state *keycode;
  
  /* recover our data structure: */
  buffer = (struct tme_keyboard_buffer_int *) _buffer;

  /* TME_KEYBOARD_MODE_UNLOCK and TME_KEYBOARD_MODE_LOCK cannot be
     combined with any other bits: */
  if ((mode
       & (TME_KEYBOARD_MODE_UNLOCK
	  | TME_KEYBOARD_MODE_LOCK))
      && (mode
	  & (mode - 1))) {
    return (EINVAL);
  }
  
  /* none of the TME_KEYBOARD_MODE_FLAG_NO_AUTOREPEATS,
     TME_KEYBOARD_MODE_FLAG_NO_RELEASES, and
     TME_KEYBOARD_MODE_FLAG_LOCK_SOFT flags can be set
     without TME_KEYBOARD_MODE_PASSTHROUGH: */
  if ((mode
       & (TME_KEYBOARD_MODE_FLAG_NO_AUTOREPEATS
	  | TME_KEYBOARD_MODE_FLAG_NO_RELEASES
	  | TME_KEYBOARD_MODE_FLAG_LOCK_SOFT))
      && !(mode
	   & TME_KEYBOARD_MODE_PASSTHROUGH)) {
    return (EINVAL);
  }

  /* you cannot specify that an output key must be unlocked: */
  if (mode & TME_KEYBOARD_MODE_UNLOCK) {
    return (EINVAL);
  }

  /* if we are setting the mode on a particular keycode: */
  if (_keycode != TME_KEYBOARD_KEYVAL_UNDEF) {
    keycode
      = ((struct tme_keycode_state *)
	 tme_hash_lookup(buffer->tme_keyboard_buffer_int_out0_keycodes,
			 tme_keyboard_hash_data_from_keyval(_keycode)));
    if (keycode == NULL) {
      return (ENOENT);
    }
    keycode->tme_keycode_state_keymode.tme_keymode_state_mode = mode;
  }

  /* otherwise, we are setting the mode on the whole keyboard: */
  else {

    /* you can't set TME_KEYBOARD_MODE_GLOBAL at the global level: */
    if (mode == TME_KEYBOARD_MODE_GLOBAL) {
      return (EINVAL);
    }

    /* set the output stage one global mode: */
    buffer->tme_keyboard_buffer_int_out1_keymode_stage
      .tme_keymode_stage_global_mode = mode;
  }

  return (TME_OK);
}

/* this fixes the keyboard's output stage zero modifiers when they get
   out of sync with the emulated software reading the output keyboard: */
tme_keyboard_modifiers_t
tme_keyboard_buffer_out_modifiers(struct tme_keyboard_buffer *_buffer,
				  tme_keyboard_modifiers_t modifiers_clear,
				  tme_keyboard_modifiers_t modifiers_set)
{
  struct tme_keyboard_buffer_int *buffer;
  
  /* recover our data structure: */
  buffer = (struct tme_keyboard_buffer_int *) _buffer;

  /* update the modifiers: */
  return (buffer->tme_keyboard_buffer_int_out0_modifiers
	  = ((buffer->tme_keyboard_buffer_int_out0_modifiers
	      & ~modifiers_clear)
	     | modifiers_set));
}

/* this parses a single keysym macro: */
int
tme_keyboard_parse_macro(const char *string,
			 tme_keyboard_keysym_lookup_t keysym_lookup,
			 void *keysym_lookup_private,
			 tme_keyboard_keyval_t **_keysyms_lhs,
			 tme_keyboard_keyval_t **_keysyms_rhs)
{
  char **tokens;
  int tokens_count, equals_token;
  int token_i;
  tme_keyboard_keyval_t keysym, *keysyms_lhs, *keysyms_rhs;
  struct tme_keyboard_lookup lookup;
  unsigned int count_lhs, count_rhs;
  int rc;

  /* tokenize this line: */
  tokens = tme_misc_tokenize(string, '#', &tokens_count);
  keysyms_lhs = tme_new(tme_keyboard_keyval_t, tokens_count);
  keysyms_rhs = tme_new(tme_keyboard_keyval_t, tokens_count);
  count_lhs = 0;
  count_rhs = 0;

  /* start the lookup structure: */
  lookup.tme_keyboard_lookup_context_length = 0;
  lookup.tme_keyboard_lookup_context = NULL;

  /* all of the tokens must be valid keysyms, except for a single
     mandatory "=" token, which must not be the first or last token: */
  equals_token = -1;
  rc = TME_OK;
  for (token_i = 0;
       token_i < tokens_count;
       token_i++) {
    
    /* check for an "=" token: */
    if (!strcmp(tokens[token_i], "=")) {
      if (equals_token >= 0
	  || token_i == 0
	  || token_i + 1 == tokens_count) {
	rc = EINVAL;
	break;
      }
      equals_token = token_i;
      continue;
    }

    /* a token on the left hand side must be a keysym that the 
       caller can generate directly: */
    if (equals_token < 0) {

      /* get the keysym for this token: */
      lookup.tme_keyboard_lookup_string = tokens[token_i];
      lookup.tme_keyboard_lookup_flags = TME_KEYBOARD_LOOKUP_FLAG_OK_DIRECT;
      keysym = (*keysym_lookup)(keysym_lookup_private, &lookup);
      if (keysym == TME_KEYBOARD_KEYVAL_UNDEF) {
	rc = ENOENT;
	break;
      }
      keysyms_lhs[count_lhs++] = keysym;
    }

    /* otherwise, a token on the right hand side is either a keysym
       that the caller can generate directly, or the caller must
       be able to allocate a unique value for it: */
    else {

      /* get the keysym for this token: */
      lookup.tme_keyboard_lookup_string = tokens[token_i];
      lookup.tme_keyboard_lookup_flags = (TME_KEYBOARD_LOOKUP_FLAG_OK_DIRECT
					  | TME_KEYBOARD_LOOKUP_FLAG_OK_ALLOC
					  | TME_KEYBOARD_LOOKUP_FLAG_OK_ALLOC_NOW);
      keysym = (*keysym_lookup)(keysym_lookup_private, &lookup);
      assert (keysym != TME_KEYBOARD_KEYVAL_UNDEF);
      keysyms_rhs[count_rhs++] = keysym;
    }
  }

  /* if this macro didn't parse correctly: */
  if (rc != TME_OK) {
    tme_free_string_array(tokens, -1);
    tme_free(keysyms_lhs);
    tme_free(keysyms_rhs);
    return (rc);
  }

  /* finish the sides of the macro: */
  keysyms_lhs[count_lhs] = TME_KEYBOARD_KEYVAL_UNDEF;
  keysyms_rhs[count_rhs] = TME_KEYBOARD_KEYVAL_UNDEF;

  /* done: */
  *_keysyms_lhs = keysyms_lhs;
  *_keysyms_rhs = keysyms_rhs;
  tme_free_string_array(tokens, -1);
  return (TME_OK);
}

/* this parses a single keysym map entry: */
int
tme_keyboard_parse_map(const char *string,
		       tme_keyboard_keysym_lookup_t keysym_lookup,
		       void *keysym_lookup_private,
		       struct tme_keyboard_map *map)
{
  char **tokens, *p1, c;
  int tokens_count;
  int token_i;
  tme_keyboard_keyval_t keycode;
  int modifier, attached_modifier;
  tme_keyboard_modifiers_t modifiers_set, modifiers_clear;
  struct tme_keyboard_lookup lookup;
  int rc;

  /* tokenize this line: */
  tokens = tme_misc_tokenize(string, '#', &tokens_count);
  rc = TME_OK;

  /* there must be at least three tokens.  the second token must be an
     equals sign, and the third token must be an integer that isn't
     TME_KEYBOARD_KEYVAL_UNDEF: */
  if (tokens_count < 3
      || strcmp(tokens[1], "=")
      || ((keycode = strtoul(tokens[2], &p1, 0))
	  == TME_KEYBOARD_KEYVAL_UNDEF)
      || p1 == tokens[2]
      || *p1 != '\0') {
    rc = EINVAL;
  }
  
  /* any tokens after the third must all be modifier names: */  
  else {
    attached_modifier = TME_KEYBOARD_MODIFIER_NONE;
    modifiers_set = 0;
    modifiers_clear = 0;
    for (token_i = 3;
	 token_i < tokens_count;
	 token_i++) {

      /* a token might be prefixed with '+' or '!': */
      p1 = tokens[token_i];
      c = *p1;
      if (c == '+'
	  || c == '!') {
	p1++;
      }

      /* turn this token into a real modifier: */
      if (!strcmp(p1, "shift")) {
	modifier = TME_KEYBOARD_MODIFIER_SHIFT;
      }
      else if (!strcmp(p1, "lock")) {
	modifier = TME_KEYBOARD_MODIFIER_LOCK;
      }
      else if (!strcmp(p1, "control")) {
	modifier = TME_KEYBOARD_MODIFIER_CONTROL;
      }
      else if (!strcmp(p1, "mod1")) {
	modifier = TME_KEYBOARD_MODIFIER_MOD1;
      }
      else if (!strcmp(p1, "mod2")) {
	modifier = TME_KEYBOARD_MODIFIER_MOD2;
      }
      else if (!strcmp(p1, "mod3")) {
	modifier = TME_KEYBOARD_MODIFIER_MOD3;
      }
      else if (!strcmp(p1, "mod4")) {
	modifier = TME_KEYBOARD_MODIFIER_MOD4;
      }
      else if (!strcmp(p1, "mod5")) {
	modifier = TME_KEYBOARD_MODIFIER_MOD5;
      }
      else {
	rc = EINVAL;
	break;
      }
    
      /* if a modifier is prefixed with '+', it must be the only
	 modifier token in the map entry, and it indicates that the
	 keycode is attached to that modifier in this map: */
      if (c == '+') {

	/* if this is not the only modifier token in the map entry: */
	if (tokens_count != 4) {
	  rc = EINVAL;
	  break;
	}

	attached_modifier = modifier;
      }
      
      /* otherwise, if a modifier name is prefixed with '!', this
	 modifier must be clear for this keycode to mean this keysym: */
      else if (c == '!') {
	modifiers_clear |= (1 << modifier);
      }

      /* otherwise, this modifier must be set for this keycode to mean
	 this keysym: */
      else {
	modifiers_set |= (1 << modifier);
      }
    }

    /* the modifiers that must be set and the modifiers that must be
       clear cannot overlap: */
    if (modifiers_set & modifiers_clear) {
      rc = EINVAL;
    }

    /* if no error has been encountered yet: */
    if (rc == TME_OK) {

      /* make the keyboard map entry.  we very deliberately set all
	 bytes not allocated to structure members to all-bits-zero, so
	 that identical keysym contexts truly are identical: */
      memset(map, 0, sizeof(*map));
      map->tme_keyboard_map_keycode = keycode;
      map->tme_keyboard_map_modifier = attached_modifier;
      map->tme_keyboard_map_modifiers_set = modifiers_set;
      map->tme_keyboard_map_modifiers_clear = modifiers_clear;

      /* take note of a special keysym: */
      if (!strcmp(tokens[0], "Caps_Lock")) {
	map->tme_keyboard_map_keysym_note
	  = TME_KEYBOARD_KEYSYM_NOTE_CAPS_LOCK;
      }
      else if (!strcmp(tokens[0], "Shift_Lock")) {
	map->tme_keyboard_map_keysym_note
	  = TME_KEYBOARD_KEYSYM_NOTE_SHIFT_LOCK;
      }
      else if (!strcmp(tokens[0], "Num_Lock")) {
	map->tme_keyboard_map_keysym_note
	  = TME_KEYBOARD_KEYSYM_NOTE_NUM_LOCK;
      }
      else {
	map->tme_keyboard_map_keysym_note
	  = TME_KEYBOARD_KEYSYM_NOTE_UNDEF;
      }

      /* the caller must be able to either directly generate this
	 keysym or have allocated a value for it already (because a
	 macro was previously added than can generate it).  if neither
	 is true, the lookup function must return
	 TME_KEYBOARD_KEYVAL_UNDEF, but this function still does not
	 fail.  it is up to the caller to not add a map entry with an
	 undefined keysym: */
      lookup.tme_keyboard_lookup_string
	= tokens[0];
      lookup.tme_keyboard_lookup_flags
	= (TME_KEYBOARD_LOOKUP_FLAG_OK_DIRECT
	   | TME_KEYBOARD_LOOKUP_FLAG_OK_ALLOC);
      lookup.tme_keyboard_lookup_context_length
	= sizeof(*map);
      lookup.tme_keyboard_lookup_context
	= (tme_uint8_t *) map;
      map->tme_keyboard_map_keysym
	= (*keysym_lookup)(keysym_lookup_private, &lookup);
    }
  }

  /* free the tokens: */
  tme_free_string_array(tokens, -1);

  /* return the parsed map: */
  return (rc);
}

/* this adds an event to the event buffer: */
static int
_tme_keyboard_buffer_copyin(struct tme_keyboard_buffer_int *buffer,
			    _tme_const struct tme_keyboard_event *event)
{
  unsigned int buffer_head, buffer_size_mask;

  buffer_head = buffer->tme_keyboard_buffer_int_head;
  buffer_size_mask = buffer->tme_keyboard_buffer_int_size - 1;

  /* if the buffer is full: */
  if (((buffer_head + 1) & buffer_size_mask)
      == buffer->tme_keyboard_buffer_int_tail) {
    return (EAGAIN);
  }

  /* put this event into the buffer: */
  buffer->tme_keyboard_buffer_int_events[buffer_head]
    = *event;

  /* advance the head: */
  buffer->tme_keyboard_buffer_int_head
    = (buffer_head + 1) & buffer_size_mask;

  return (TME_OK);
}

/* this adds a difference to an event time, avoiding a result of
   TME_KEYBOARD_EVENT_TIME_UNDEF: */
static tme_uint32_t
_tme_keyboard_event_time_diff(tme_uint32_t event_time,
			      tme_int32_t diff)
{
  event_time += (tme_uint32_t) diff;
  if (event_time == TME_KEYBOARD_EVENT_TIME_UNDEF) {
    assert (diff != 0);
    event_time += (diff < 0 ? -1 : 1);
  }
  return (event_time);
}

/* this subtracts event_time1 from event_time0, handling the wrapping
   of the tme_uint32_t milliseconds values as best as possible: */
static tme_int32_t
_tme_keyboard_event_time_subtract(tme_uint32_t event_time0,
				  tme_uint32_t event_time1)
{
  tme_uint32_t event_time0_less_least;
  tme_uint32_t event_time_diff_unwrapped;
  tme_uint32_t event_time_diff_wrapped;
  
  /* if the two times are equal: */
  if (event_time0 == event_time1) {
    return (0);
  }

  /* calculate the unwrapped and wrapped differences between the
     two times: */
  if (event_time0 < event_time1) {
    event_time_diff_unwrapped = event_time1 - event_time0;
    event_time_diff_wrapped = 0 - event_time_diff_unwrapped;
  }
  else {
    event_time_diff_unwrapped = event_time0 - event_time1;
    event_time_diff_wrapped = 0 - event_time_diff_unwrapped;
  }

  /* calculate the event time that is the most in the past without
     being confused as being later than event_time0: */
  event_time0_less_least
    = (event_time0 - (((tme_uint32_t) -1) >> 1));

  /* if event_time0_less_least is literally greater than event_time0,
     the event time space that represents "less than event_time0" is
     not a single region in the tme_uint32_t space: */
  if (event_time0_less_least < event_time0) {
    return ((event_time0_less_least <= event_time1
	     && event_time1 < event_time0)
	    /* event_time1 is less than event_time0: */
	    ? event_time_diff_unwrapped
	    /* event_time1 is greater than event_time0: */
	    : (event_time1 > event_time0
	       ? 0 - event_time_diff_unwrapped
	       : 0 - event_time_diff_wrapped));
  }

  /* otherwise, the event time space that represents "less than
     event_time0" is two regions in the tme_uint32_t space: */
  else {
    return ((event_time0_less_least <= event_time1
	     || event_time1 < event_time0)
	    /* event_time1 is less than event_time0: */
	    ? (event_time1 < event_time0
	       ? event_time_diff_unwrapped
	       : event_time_diff_wrapped)
	    /* event_time1 is greater than event_time0: */
	    : 0 - event_time_diff_unwrapped);
  }
}

/* this runs a keymode stage.  this is used for input stage one,
   and output stage one.  roughly, a keymode stage tries to
   guarantee that a key has a specific press/release behavior
   (or "mode") for later stages.  the particular modes are described
   in the case below: */
static int
_tme_keymode_stage(struct tme_keyboard_buffer_int *buffer,
		   struct tme_keymode_stage *stage,
		   struct tme_keymode_state *keymode,
		   int is_press,
		   tme_uint32_t event_time)
{
  struct tme_keymode_state **_auto_keymode, *auto_keymode;
  int pressed_old, mode;
  int rc;

  /* check all keys on the no-autorepeats list: */
  for (_auto_keymode = &stage->tme_keymode_stage_no_autorepeats;
       (auto_keymode = *_auto_keymode) != NULL; ) {

    /* if the time of the last release from the earlier stages is
       undefined, that means that the last event from the earlier
       stages was a press: */
    if (auto_keymode->tme_keymode_state_last_release
	== TME_KEYBOARD_EVENT_TIME_UNDEF) {

      /* if the key we're checking happens to be the key that the
	 earlier stages are calling us for: */
      if (auto_keymode == keymode) {

	/* this must be a release on this key: */
	assert (!is_press);

	/* set the release time on this key: */
	auto_keymode->tme_keymode_state_last_release = event_time;

	/* we've taken care of this call: */
	keymode = NULL;
      }
    }

    /* otherwise, the last event from the earlier stages was a release.
       if enough time has elapsed since then without a press from
       earlier stages: */
    else if (_tme_keyboard_event_time_subtract(event_time,
					       auto_keymode->tme_keymode_state_last_release)
	     > TME_KEYBOARD_SHORTEST_DOUBLE_MSEC) {

      /* now we're sure that the key has genuinely released.
	 remove this key from the autorepeats list: */
      *_auto_keymode = auto_keymode->tme_keymode_state_next;
      auto_keymode->tme_keymode_state_next = NULL;

      /* if we're supposed to ignore the genuine release: */
      if (auto_keymode->tme_keymode_state_ignore_release) {
	auto_keymode->tme_keymode_state_ignore_release = FALSE;
      }

      /* otherwise, we're not supposed to ignore the genuine release: */
      else {

	/* flip the pressed state of the key: */
	auto_keymode->tme_keymode_state_pressed
	  = !auto_keymode->tme_keymode_state_pressed;

	/* if this key is now pressed, or if we're allowed to
	   pass releases, run the next stage: */
	mode = auto_keymode->tme_keymode_state_mode;
	if (mode == TME_KEYBOARD_MODE_GLOBAL) {
	  mode = stage->tme_keymode_stage_global_mode;
	}
	if (auto_keymode->tme_keymode_state_pressed
	    || !(mode
		 & TME_KEYBOARD_MODE_FLAG_NO_RELEASES)) {
	  rc = (*stage->tme_keymode_stage_next)
	    (buffer,
	     auto_keymode->tme_keymode_state_keysym,
	     _tme_keyboard_event_time_diff(event_time, -1));
	  assert (rc == TME_OK);
	}
      }

      /* continue now - by removing this key from the autorepeats
	 list we've already advanced our position in the list for
	 the next iteration: */
      continue;
    }

    /* otherwise, if the key we're checking happens to be the key that
       the earlier stages are calling us for: */
    else if (auto_keymode == keymode) {

      /* this must be a press on this key: */
      assert (is_press);

      /* now we're sure that this key is autorepeating.  clear
	 the last-release time: */
      auto_keymode->tme_keymode_state_last_release
	= TME_KEYBOARD_EVENT_TIME_UNDEF;

      /* we've taken care of this call: */
      keymode = NULL;
    }
      
    /* continue: */
    _auto_keymode = &auto_keymode->tme_keymode_state_next;
  }
      
  /* return now if we already finished processing this event: */
  if (keymode == NULL) {
    return (TME_OK);
  }

  /* get this key's mode: */
  mode = keymode->tme_keymode_state_mode;
  if (mode == TME_KEYBOARD_MODE_GLOBAL) {
    mode = stage->tme_keymode_stage_global_mode;
  }
  
  /* remember if this key was pressed in this stage before: */
  pressed_old = keymode->tme_keymode_state_pressed;

  /* unlock mode unlocks a key by turning every transition into a
     press transition and a release transition.  think of a Caps Lock
     key on an input keyboard that *physically* locks down when you
     press it down - this is a nice property, because there's no
     mistaking that when you get a key press event, you know the key
     is both physically and *semantically* down until you get a key
     release, at which time you know just the opposite.
     
     however, this is a best case.  many keyboards have Caps Lock, Num
     Lock, and other keys where their semantic pressed/release state
     doesn't match their physical pressed/release state.  to make
     everything uniform, we need to bring the best-case keyboards down
     to the worst-case level: */
  if (mode == TME_KEYBOARD_MODE_UNLOCK) {

    /* we must not have this key as pressed: */
    assert (!pressed_old);

    /* press this key and run the next stage: */
    keymode->tme_keymode_state_pressed = TRUE;
    rc = (*stage->tme_keymode_stage_next)
      (buffer,
       keymode->tme_keymode_state_keysym,
       _tme_keyboard_event_time_diff(event_time, -1));
    assert (rc == TME_OK);
    
    /* release this key, and make sure the next stage
       gets run at the end of this function: */
    keymode->tme_keymode_state_pressed = FALSE;
    pressed_old = TRUE;
  }

  /* lock mode makes the key lock in the physical sense, like the
     best-case Caps Lock described above.  since our events from
     earlier stages are worst-case, we need to ignore autorepeats, and
     each press/release pair must toggle our notion of whether or not
     the key is physically pressed: */
  else if (mode == TME_KEYBOARD_MODE_LOCK) {

    /* this must be a press: */
    assert (is_press);

    /* put this key on the no-autorepeats list: */
    keymode->tme_keymode_state_last_release
      = TME_KEYBOARD_EVENT_TIME_UNDEF;
    keymode->tme_keymode_state_next
      = stage->tme_keymode_stage_no_autorepeats;
    stage->tme_keymode_stage_no_autorepeats = keymode;
    
    /* if we have the key as pressed (locked): */
    if (pressed_old) {
      
      /* don't ignore the genuine release, when it is determined to
	 have happened.  if this mode has
	 TME_KEYBOARD_MODE_FLAG_NO_RELEASES set, the genuine release
	 code will handle it then: */
      keymode->tme_keymode_state_ignore_release = FALSE;
    }
    
    /* otherwise, we have the key as released (unlocked): */
    else {
      
      /* press this key: */
      keymode->tme_keymode_state_pressed = TRUE;
      
      /* ignore the genuine release, when it is determined
	 to have happened: */
      keymode->tme_keymode_state_ignore_release = TRUE;
    }
  }

  /* passthrough mode generally passes events through: */
  else {

    assert (!pressed_old != !is_press);

    /* if this is a press: */
    if (is_press) {

      /* press this key: */
      keymode->tme_keymode_state_pressed = TRUE;

      /* if we must not pass through autorepeats: */
      if (mode & TME_KEYBOARD_MODE_FLAG_NO_AUTOREPEATS) {

	/* put this key on the no-autorepeats list: */
	keymode->tme_keymode_state_last_release
	  = TME_KEYBOARD_EVENT_TIME_UNDEF;
	keymode->tme_keymode_state_next
	  = stage->tme_keymode_stage_no_autorepeats;
	stage->tme_keymode_stage_no_autorepeats = keymode;

	/* don't ignore the genuine release, when it is
	   determined to have happened.  if this mode has
	   TME_KEYBOARD_MODE_FLAG_NO_RELEASES set, the 
	   genuine release code will handle it then: */
	keymode->tme_keymode_state_ignore_release = FALSE;
      }
    }

    /* otherwise, this is a release: */
    else {

      /* release this key: */
      keymode->tme_keymode_state_pressed = FALSE;

      /* if this mode has TME_KEYBOARD_MODE_FLAG_NO_RELEASES set,
	 we'll handle it below when we go to run the next stage: */
    }
  }

  /* stop processing this event now if the key's pressed state
     in this stage has not changed, or if the key is now released
     and we're not allowed to pass releases.  otherwise, run the
     next stage: */
  return ((keymode->tme_keymode_state_pressed
	   ? pressed_old
	   : (!pressed_old
	      || (mode
		  & TME_KEYBOARD_MODE_FLAG_NO_RELEASES)))
	  ? TME_OK
	  : ((*stage->tme_keymode_stage_next)
	     (buffer,
	      keymode->tme_keymode_state_keysym,
	      event_time)));
}

/* this is the bottom half of output stage one.  this half does
   nothing except finally buffer an event for a keycode, and update
   the output modifiers mask: */
static int
_tme_keyboard_buffer_out1_bottom(struct tme_keyboard_buffer_int *buffer,
				 struct tme_keysym_state *keysym,
				 tme_uint32_t event_time)
{
  struct tme_keycode_state *keycode;
  struct tme_keyboard_event event_buffer;
  int modifier;
  int is_press;

  /* get the keycode: */
  keycode = keysym->tme_keysym_state_out0_keycode;

  /* get whether or not this is a press: */
  is_press = TME_KEYBOARD_PRESSED_OUT1(keycode);

  _tme_keyboard_debug(buffer,
		      "out1-bottom",
		      keysym->tme_keysym_state_keysym,
		      is_press,
		      event_time);

  /* if this keysym is attached to a modifier, update the output
     modifiers mask: */
  /* XXX there might be a cleaner way to work the output modifier
     mask.  it's suspicious that stages zero and one need to share the
     modifiers mask and the keysym to modifier mapping: */
  modifier = keysym->tme_keysym_state_out0_modifier;
  if (modifier != TME_KEYBOARD_MODIFIER_NONE) {

    /* if this is a press: */
    if (is_press) {

      /* if the modifier is currently clear: */
      if (!(buffer->tme_keyboard_buffer_int_out0_modifiers
	    & (1 << modifier))) {

	/* set the modifier: */
	buffer->tme_keyboard_buffer_int_out0_modifiers
	  |= (1 << modifier);

	/* iff this keysym soft-locks, the next release will not
	   affect the output modifiers mask: */
	keysym->tme_keysym_state_out1_ignore_release
	  = (keycode->tme_keycode_state_keymode.tme_keymode_state_mode
	     & TME_KEYBOARD_MODE_FLAG_LOCK_SOFT);
      }

      /* otherwise, the modifier is currently set: */
      else {
	
	/* nothing to do.  even if this keysym soft-locks, we won't
	   clear the modifier until the release: */
      }
    }

    /* otherwise, this is a release: */
    else {

      /* if we're supposed to ignore this release: */
      if (keysym->tme_keysym_state_out1_ignore_release) {
	keysym->tme_keysym_state_out1_ignore_release = FALSE;
      }

      /* otherwise, if the modifier is currently set: */
      else if (buffer->tme_keyboard_buffer_int_out0_modifiers
	       & (1 << modifier)) {

	/* clear the modifier: */
	buffer->tme_keyboard_buffer_int_out0_modifiers
	  &= ~(1 << modifier);
      }

      /* otherwise, the modifier is currently clear: */
      else {

	/* nothing to do.  a release would never set a modifier: */
      }
    }
  }

  /* finally buffer this keycode: */
  event_buffer.tme_keyboard_event_type
    = (is_press
       ? TME_KEYBOARD_EVENT_PRESS
       : TME_KEYBOARD_EVENT_RELEASE);
  event_buffer.tme_keyboard_event_keyval
    = keycode->tme_keycode_state_keycode;
  event_buffer.tme_keyboard_event_keycode = 0;
  event_buffer.tme_keyboard_event_time
    = event_time;
  event_buffer.tme_keyboard_event_modifiers
    = buffer->tme_keyboard_buffer_int_out0_modifiers;
  return (_tme_keyboard_buffer_copyin(buffer, &event_buffer));
}

/* this is the top half of output stage one.  it is the last output
   stage, that gives each keycode the specific behavior that it must
   have on the output keyboard: */
static int
_tme_keyboard_buffer_out1(struct tme_keyboard_buffer_int *buffer,
			  struct tme_keysym_state *keysym,
			  tme_uint32_t event_time)
{
  struct tme_keycode_state *keycode;

  _tme_keyboard_debug(buffer,
		      "out1-top",
		      keysym->tme_keysym_state_keysym,
		      TME_KEYBOARD_PRESSED_OUT0(keysym),
		      event_time);

  /* get the keycode state: */
  keycode = keysym->tme_keysym_state_out0_keycode;

  /* run the keymode stage function: */
  return (_tme_keymode_stage(buffer,
			     &buffer->tme_keyboard_buffer_int_out1_keymode_stage,
			     &keycode->tme_keycode_state_keymode,
			     TME_KEYBOARD_PRESSED_OUT0(keysym),
			     event_time));
}

/* this is output stage zero.  at this point, we have the final
   keysyms from the input keyboard, and this stage maps those keysyms
   to keycodes on the output keyboard.  

   sometimes, the input keyboard may generate keysyms for which the
   output keyboard requires that certain modifiers be set or clear, at
   a time when the output modifiers mask isn't suitable.  for example,
   this can happen when the input keyboard can generate a certain
   keysym without shifting, when the output keyboard requires
   shifting.

   when this happens, this stage is responsible for simulating presses
   or releases of modifiers on the output keyboard as needed to make
   sure that the keysym is properly obtained when the mapped keycode
   is pressed, and for undoing those changes when the mapped keycode
   is released: */
static int
_tme_keyboard_buffer_out0(struct tme_keyboard_buffer_int *buffer,
			  struct tme_keysym_state *keysym,
			  tme_uint32_t event_time)
{
  struct tme_keyboard_event event_buffer;
  tme_keyboard_modifiers_t modifiers, modifiers_set, modifiers_clear;
  int num_lock_on;
  int is_press, mod_pressed_old, modifier;
  struct tme_keysym_state *mod_keysym, **keysyms;
  unsigned int *press_flags;
  int keysym_count;
  int modifier_stuck;
  int rc;

  _tme_keyboard_debug(buffer,
		      "out0",
		      keysym->tme_keysym_state_keysym,
		      TME_KEYBOARD_PRESSED_IN2(keysym),
		      event_time);

  /* get whether or not this is a press from earlier stages: */
  is_press = TME_KEYBOARD_PRESSED_IN2(keysym);

  /* if output stage zero is a passthrough, just buffer an event for
     the keysym and we're done processing this event: */
  if (buffer->tme_keyboard_buffer_int_out0_passthrough) {

    /* otherwise, simply buffer this event: */
    event_buffer.tme_keyboard_event_type
      = (is_press
	 ? TME_KEYBOARD_EVENT_PRESS
	 : TME_KEYBOARD_EVENT_RELEASE);
    event_buffer.tme_keyboard_event_keyval
      = keysym->tme_keysym_state_keysym;
    event_buffer.tme_keyboard_event_keycode = 0;
    event_buffer.tme_keyboard_event_time
      = event_time;
    event_buffer.tme_keyboard_event_modifiers
      = 0;
    return (_tme_keyboard_buffer_copyin(buffer, &event_buffer));
  }

  /* if this keysym is not mapped to a keycode on the output keyboard,
     we don't have to process this event any more: */
  if (keysym->tme_keysym_state_out0_keycode == NULL) {
    return (TME_OK);
  }
  
  /* if this is a press from earlier stages: */
  if (is_press) {

    /* this keysym can't have any changes attached to it already: */
    assert (keysym->tme_keysym_state_out0_keysyms == NULL);
    keysyms = NULL;
    press_flags = NULL;
    keysym_count = 0;

    /* get the current output stage zero modifiers: */
    modifiers = buffer->tme_keyboard_buffer_int_out0_modifiers;

    /* get the sets of modifiers that must be clear and set for this
       keysym: */
    modifiers_clear = keysym->tme_keysym_state_out0_modifiers_clear;
    modifiers_set = keysym->tme_keysym_state_out0_modifiers_set;
    assert ((modifiers_clear & modifiers_set) == 0);
    assert ((modifiers_clear | modifiers_set) == 0
	    || (keysym->tme_keysym_state_out0_modifier
		== TME_KEYBOARD_MODIFIER_NONE));

    /* if this keysym is uppercase, but the current modifiers are such
       that a lowercase keysym *might* be generated: */
    if ((modifiers_set & (1 << TME_KEYBOARD_MODIFIER_LOCK))
	&& !(modifiers & (1 << TME_KEYBOARD_MODIFIER_LOCK))) {

      /* if shift is down, we must be generating uppercase keysyms, so
	 forget about the lock modifier in this instance: */
      if (modifiers & (1 << TME_KEYBOARD_MODIFIER_SHIFT)) {
	modifiers_set &= ~(1 << TME_KEYBOARD_MODIFIER_LOCK);
      }

      /* otherwise, neither shift nor lock are down, so we are
	 generating lowercase keysyms.  since this keyboard might not
	 have a lock modifier at all (and since it seems more
	 reasonable to do it this way anyways), require the shift
	 modifier to be set instead of the lock modifier in this
	 instance: */
      else {
	modifiers_set &= ~(1 << TME_KEYBOARD_MODIFIER_LOCK);
	modifiers_set |= (1 << TME_KEYBOARD_MODIFIER_SHIFT);
      }
    }
      
    /* if this output keyboard has a Num_Lock key, and this keysym is
       sensitive to the Num_Lock setting, but the appropriate Num_Lock
       setting is already in effect, forget about Num_Lock for this
       instance: */
    modifier = buffer->tme_keyboard_buffer_int_out0_mod_num_lock;
    if (modifier != TME_KEYBOARD_MODIFIER_NONE
	&& ((modifiers_clear
	     | modifiers_set)
	    & (1 << modifier))) {

      /* determine what Num_Lock setting is currently in effect.  it
	 is active if Num_Lock is pressed and there is no shifting
	 active, or if Num_Lock is released and there is shifting
	 active.  NB that if the lock modifier is attached only to a
	 Shift_Lock, that modifier also counts as shifting: */
      num_lock_on
	= (

	   /* this term is 0 iff Num_Lock is pressed, else 1: */
	   !(modifiers
	     & (1 << modifier))

	   !=

	   /* this term is 0 iff shifting is active, else 1: */
	   !(modifiers
	     & ((1 << TME_KEYBOARD_MODIFIER_SHIFT)
		| (buffer->tme_keyboard_buffer_int_out0_lock_is_caps
		   ? 0
		   : (1 << TME_KEYBOARD_MODIFIER_LOCK)))));

      /* if this keysym requires Num_Lock to be clear, but it is
	 not in effect, forget about Num_Lock for this instance: */
      if ((modifiers_clear & (1 << modifier))
	  && !num_lock_on) {
	modifiers_clear &= ~(1 << modifier);
      }

      /* if this keysym requires Num_Lock to be set, but it is
	 in effect, forget about Num_Lock for this instance: */
      if ((modifiers_set & (1 << modifier))
	  && num_lock_on) {
	modifiers_set &= ~(1 << modifier);
      }
    }

    /* finish the set of modifiers that must be clear and set for this
       keysym, but aren't: */
    modifiers_clear &= modifiers;
    modifiers_set &= ~modifiers;

    /* try to generate release events for all keysyms attached to
       output modifiers that are set, but that need to be clear: */
    if (modifiers_clear != 0) {
      
      /* loop over the modifiers that need clearing: */
      for (modifier = 0;
	   modifier <= TME_KEYBOARD_MODIFIER_MAX;
	   modifier++) {
	if (!(modifiers_clear & (1 << modifier))) {
	  continue;
	}
	
	/* try to release all of the keysyms attached to this modifier
	   that are pressed: */
	modifier_stuck = FALSE;
	for (mod_keysym = buffer->tme_keyboard_buffer_int_out0_modkeys[modifier];
	     mod_keysym != NULL;
	     mod_keysym = mod_keysym->tme_keysym_state_out0_modifier_next) {
	
	  /* XXX this is broken for modifiers that soft-lock. */

	  /* ignore this keysym if it isn't pressed: */
	  if (!TME_KEYBOARD_PRESSED_OUT0(mod_keysym)) {
	    continue;
	  }

	  /* grow the keysyms and press flags arrays.  the keysyms
	     array always has one extra entry, which will be filled
	     with NULL to terminate the array: */
	  if (keysym_count == 0) {
	    keysyms = tme_new(struct tme_keysym_state *, 2);
	    press_flags = tme_new(unsigned int, 1);
	  }
	  else {
	    keysyms = tme_renew(struct tme_keysym_state *, keysyms, keysym_count + 2);
	    press_flags = tme_renew(unsigned int, press_flags, keysym_count + 1);
	  }
	  
	  /* add this keysym to those arrays: */
	  keysyms[keysym_count] = mod_keysym;
	  press_flags[keysym_count] = FALSE;
	  keysym_count++;

	  /* release this modifier key.  if this actually
	     releases the key, run the next stage: */
	  mod_keysym->tme_keysym_state_out0_released++;
	  if (!TME_KEYBOARD_PRESSED_OUT0(mod_keysym)) {
	    rc = _tme_keyboard_buffer_out1(buffer,
					   mod_keysym,
					   _tme_keyboard_event_time_diff(event_time, -1));
	    assert (rc == TME_OK);
	  }

	  /* otherwise, this modifier is stuck on: */
	  else {
	    modifier_stuck = TRUE;
	  }
	}

	/* if we were able to release all keysyms attached to this
	   modifier, clear the modifier in the output keyboard mask: */
	if (!modifier_stuck) {
	  buffer->tme_keyboard_buffer_int_out0_modifiers
	    &= ~(1 << modifier);
	}
      }
    }

    /* try to generate press events for single keysyms attached to
       output modifiers that are clear, but that need to be set: */
    if (modifiers_set != 0) {
      
      /* loop over the modifiers that need setting: */
      for (modifier = 0;
	   modifier <= TME_KEYBOARD_MODIFIER_MAX;
	   modifier++) {
	if (!(modifiers_set & (1 << modifier))) {
	  continue;
	}
	
	/* try to press a single keysym attached to this modifier: */
	mod_keysym = buffer->tme_keyboard_buffer_int_out0_modkeys[modifier];
	assert (mod_keysym != NULL);
	
	/* this keysym can't pressed - if it is, the modifier
	   should be set: */
	assert (!TME_KEYBOARD_PRESSED_OUT0(mod_keysym));

	/* grow the keysyms and press flags arrays.  the keysyms
	   array always has one extra entry, which will be filled
	   with NULL to terminate the array: */
	if (keysym_count == 0) {
	  keysyms = tme_new(struct tme_keysym_state *, 2);
	  press_flags = tme_new(unsigned int, 1);
	}
	else {
	  keysyms = tme_renew(struct tme_keysym_state *, keysyms, keysym_count + 2);
	  press_flags = tme_renew(unsigned int, press_flags, keysym_count + 1);
	}
	  
	/* add this keysym to those arrays: */
	keysyms[keysym_count] = mod_keysym;
	press_flags[keysym_count] = TRUE;
	keysym_count++;

	/* press this modifier key.  this must actually press the key,
	   so run the next stage: */
	mod_keysym->tme_keysym_state_out0_pressed++;
	assert (TME_KEYBOARD_PRESSED_OUT0(mod_keysym));
	rc = _tme_keyboard_buffer_out1(buffer,
				       mod_keysym,
				       _tme_keyboard_event_time_diff(event_time, -1));
	assert (rc == TME_OK);

	/* set the modifier: */
	buffer->tme_keyboard_buffer_int_out0_modifiers
	  |= (1 << modifier);
      }
    }

    /* remember any changes we made: */
    if (keysyms != NULL) {
      keysyms[keysym_count] = NULL;
    }
    keysym->tme_keysym_state_out0_keysyms = keysyms;
    keysym->tme_keysym_state_out0_press_flags = press_flags;

    /* run the next stage: */
    return (_tme_keyboard_buffer_out1(buffer,
				      keysym,
				      event_time));
  }

  /* otherwise, this is a release: */
  else {
  
    /* run the next stage: */
    rc = _tme_keyboard_buffer_out1(buffer,
				   keysym,
				   event_time);
    assert (rc == TME_OK);

    /* if this keysym tried to make any modifier changes, undo them: */
    keysyms = keysym->tme_keysym_state_out0_keysyms;
    press_flags = keysym->tme_keysym_state_out0_press_flags;
    if (keysyms != NULL) {

      /* loop over all of the modifiers we changed: */
      for (; (mod_keysym = *(keysyms++)) != NULL; ) {

	/* see if this modifier is pressed now: */
	mod_pressed_old = TME_KEYBOARD_PRESSED_OUT0(mod_keysym);

	/* undo the change we made to this modifier: */
	if (*(press_flags++)) {
	  mod_keysym->tme_keysym_state_out0_pressed--;
	}
	else {
	  mod_keysym->tme_keysym_state_out0_released--;
	}

	/* if the state of this modifier has changed, run the next
	   stage: */
	if (TME_KEYBOARD_PRESSED_OUT0(mod_keysym) != mod_pressed_old) {
	  rc = _tme_keyboard_buffer_out1(buffer,
					 mod_keysym,
					 _tme_keyboard_event_time_diff(event_time, +1));
	  assert (rc == TME_OK);
	}
      }

      /* forget these modifier changes: */
      tme_free(keysym->tme_keysym_state_out0_keysyms);
      tme_free(keysym->tme_keysym_state_out0_press_flags);
      keysym->tme_keysym_state_out0_keysyms = NULL;
      keysym->tme_keysym_state_out0_press_flags = NULL;
    }
  }
      
  /* success: */
  return (TME_OK);
}

/* this is input stage two.  at this point, we have the input keyboard
   cleaned up to the point where we can have macros for generating
   keysyms that the input keyboard can't generate on its own: */
static int
_tme_keyboard_buffer_in2(struct tme_keyboard_buffer_int *buffer,
			 struct tme_keysym_state *keysym,
			 tme_uint32_t event_time)
{
  tme_keyboard_keyval_t _keysym;
  struct tme_keyboard_macro **_macro, *macro, *child, *parent;
  int is_press, pressed_old, sub_pressed_old;
  int update, keysym_i;
  struct tme_keysym_state *sub_keysym;
  int rc;

  _tme_keyboard_debug(buffer,
		      "in2",
		      keysym->tme_keysym_state_keysym,
		      TME_KEYBOARD_PRESSED_IN1(keysym),
		      event_time);

  /* get the keysym: */
  _keysym = keysym->tme_keysym_state_keysym;

  /* get whether or not this is a press from earlier stages: */
  is_press = TME_KEYBOARD_PRESSED_IN1(keysym);

  /* remember if this keysym was pressed in this stage before: */
  pressed_old = _TME_KEYBOARD_PRESSED_IN2(keysym, !is_press);

  /* visit all active macros tree nodes: */
  for (_macro = &buffer->tme_keyboard_buffer_int_in2_macros_active;
       (macro = *_macro) != NULL; ) {

    /* if this is a press, see if it advances this macro's sequence: */
    if (is_press) {

      /* ignore this macros tree node if it's a leaf, or if this
	 keysym is not a branch out of this node to some child node: */
      if (macro->tme_keyboard_macro_branches == NULL
	  || (child
	      = ((struct tme_keyboard_macro *)
		 tme_hash_lookup(macro->tme_keyboard_macro_branches,
				 tme_keyboard_hash_data_from_keyval(_keysym)))) == NULL) {
	_macro = &macro->tme_keyboard_macro_active_next;
	continue;
      }
	
      /* the child node cannot already be active.  if it is, that
	 means that earlier stages didn't give us a release for this
	 keysym: */
      assert (child->tme_keyboard_macro_active_next == NULL);
      
      /* make the child node active: */
      child->tme_keyboard_macro_active_next = macro;
      *_macro = child;
      _macro = &macro->tme_keyboard_macro_active_next;
      macro = child;
    }

    /* otherwise, this is a release, so see if this release cancels
       any sequence: */
    else {

      /* ignore this macros tree node if this is not a release of a
	 keysym somewhere on the path from the root to this node: */
      if (_keysym != macro->tme_keyboard_macro_keysym) {
	for (parent = macro->tme_keyboard_macro_parent;
	     parent != NULL;
	     parent = parent->tme_keyboard_macro_parent) {
	  if (_keysym == parent->tme_keyboard_macro_keysym) {
	    break;
	  }
	}
	if (parent == NULL) {
	  _macro = &macro->tme_keyboard_macro_active_next;
	  continue;
	}
      }
      
      /* make this macros tree node no longer active: */
      *_macro = macro->tme_keyboard_macro_active_next;
      macro->tme_keyboard_macro_active_next = NULL;
    }
    
    /* if this macro is not a leaf node, continue: */
    if (macro->tme_keyboard_macro_branches != NULL) {
      continue;
    }

    /* otherwise, this event has either activated or deactivated a
       macro.  either do or undo this macro's presses and releases: */
    update = (is_press ? 1 : -1);
    for (keysym_i = macro->tme_keyboard_macro_length;
	 keysym_i-- > 0; ) {

      /* get this keysym's state: */
      sub_keysym = macro->tme_keyboard_macro_keysyms[keysym_i];

      /* remember if this keysym was pressed in this stage before: */
      sub_pressed_old = TME_KEYBOARD_PRESSED_IN2(sub_keysym);

      /* update this keysym's state: */
      if (macro->tme_keyboard_macro_press_flags[keysym_i]) {
	sub_keysym->tme_keysym_state_in2_pressed += update;
      }
      else {
	sub_keysym->tme_keysym_state_in2_released += update;
      }

      /* if this keysym's pressed state in this stage has changed, 
	 run the next stage, unless this is the keysym that we
	 were called with, in which case we'll handle this later: */
      if (sub_keysym != keysym
	  && TME_KEYBOARD_PRESSED_IN2(sub_keysym) != sub_pressed_old) {
	rc = _tme_keyboard_buffer_out0(buffer, sub_keysym, event_time);
	assert (rc == TME_OK);
      }
    }
  }

  /* if this keysym's pressed state in this stage has changed, run
     the next stage, otherwise stop processing this event now: */
  return ((TME_KEYBOARD_PRESSED_IN2(keysym) != pressed_old)
	  ? _tme_keyboard_buffer_out0(buffer, keysym, event_time)
	  : TME_OK);
}

/* this is input stage one.  at this point, the input keyboard is
   slightly cleaned up - consecutive presses have had a release
   inserted in between, releases of unpressed keys have been dropped,
   and as many inferred lost events as possible have been generated.

   this stage finishes cleaning up the input keyboard to the point
   where macros are useful.  usually, the only keysyms that will have
   specific behaviors enforced here will be keysyms that are used like
   modifiers in input stage two macros, but that don't behave like
   modifiers on the input keyboard (i.e., they autorepeat) and so
   aren't good for use in multiple-key macros: */
static int
_tme_keyboard_buffer_in1(struct tme_keyboard_buffer_int *buffer,
			 struct tme_keysym_state *keysym,
			 tme_uint32_t event_time)
{

  _tme_keyboard_debug(buffer,
		      "in1",
		      keysym->tme_keysym_state_keysym,
		      TME_KEYBOARD_PRESSED_IN0(keysym),
		      event_time);

  /* run the keymode stage function: */
  return (_tme_keymode_stage(buffer,
			     &buffer->tme_keyboard_buffer_int_in1_keymode_stage,
			     &keysym->tme_keysym_state_in1_keymode,
			     TME_KEYBOARD_PRESSED_IN0(keysym),
			     event_time));
}

/* this is the bottom half of input stage zero.  at this point, events
   inferred by modifier changes have been generated, but otherwise the
   input keyboard hasn't been cleaned up.

   this half drops releases of keys that we don't think are pressed,
   generates a release in between two consecutive presses, and tracks
   the input modifier mask: */
static int
_tme_keyboard_buffer_in0_bottom(struct tme_keyboard_buffer_int *buffer,
				struct tme_keysym_state *keysym,
				const struct tme_keyboard_event *event)
{
  struct tme_keysym_state *mod_keysym;
  struct tme_keyboard_event event_pseudo;
  int modifier, pressed_old;
  int rc;

  _tme_keyboard_debug(buffer,
		      "in0-bottom",
		      keysym->tme_keysym_state_keysym,
		      (event->tme_keyboard_event_type
		       & TME_KEYBOARD_EVENT_IN0_PRESS_USER),
		      event->tme_keyboard_event_time);

  /* NB: event->tme_keyboard_event_modifiers is always the modifiers
     *mask from immediately before* the event, i.e., if this event is
     *for a modifier keysym, it does not reflect any change the press
     *or release of this keysym will cause: */

  /* remember if this keysym was pressed in this stage before: */
  pressed_old = TME_KEYBOARD_PRESSED_IN0(keysym);

  /* see if this keysym is attached to any known modifier: */
  modifier = (buffer->tme_keyboard_buffer_int_in0_have_modifiers
	      ? keysym->tme_keysym_state_in0_modifier
	      : TME_KEYBOARD_MODIFIER_NONE);

  /* dispatch on the event type: */
  switch (event->tme_keyboard_event_type) {
    
    /* an automatic input stage zero press: */
  case TME_KEYBOARD_EVENT_IN0_PRESS_AUTO:
    /* this keysym must not be pressed at all: */
    assert (!pressed_old);
    /* FALLTHROUGH */
    
    /* a user input stage zero press: */
  case TME_KEYBOARD_EVENT_IN0_PRESS_USER:

    /* if this keysym was already pressed, inject a release first -
       you can't press a key twice without releasing it in between: */
    if (pressed_old) {

      /* make the pseudoevent.  assume that the release we dropped
	 happened as soon as humanly possible after the press, but
	 never after this new press: */
      event_pseudo.tme_keyboard_event_type
	= TME_KEYBOARD_IN0_RELEASE_EVENT(keysym->tme_keysym_state_in0_pressed);
      event_pseudo.tme_keyboard_event_time
	= _tme_keyboard_event_time_diff(keysym->tme_keysym_state_in0_press_time,
					TME_KEYBOARD_SHORTEST_DOUBLE_MSEC);
      if (_tme_keyboard_event_time_subtract(event_pseudo.tme_keyboard_event_time,
					    event->tme_keyboard_event_time) <= 0) {
	event_pseudo.tme_keyboard_event_time
	  = _tme_keyboard_event_time_diff(event->tme_keyboard_event_time, -1);
      }
      event_pseudo.tme_keyboard_event_modifiers
	= buffer->tme_keyboard_buffer_int_in0_modifiers;

      /* recurse with this pseudoevent: */
      rc = _tme_keyboard_buffer_in0_bottom(buffer,
					   keysym,
					   &event_pseudo);
      assert (rc == TME_OK);
    }
    
    /* this keysym is now pressed: */
    keysym->tme_keysym_state_in0_pressed = event->tme_keyboard_event_type;
    keysym->tme_keysym_state_in0_press_time = event->tme_keyboard_event_time;

    /* set any modifier in the mask: */
    assert (buffer->tme_keyboard_buffer_int_in0_modifiers
	    == event->tme_keyboard_event_modifiers);
    if (modifier != TME_KEYBOARD_MODIFIER_NONE) {
      buffer->tme_keyboard_buffer_int_in0_modifiers |= (1 << modifier);
    }
    break;
	
    /* an automatic input stage zero release :*/
  case TME_KEYBOARD_EVENT_IN0_RELEASE_AUTO:
    assert (pressed_old == TME_KEYBOARD_EVENT_IN0_PRESS_AUTO);
    /* FALLTHROUGH */

    /* a user input stage zero release: */
  case TME_KEYBOARD_EVENT_IN0_RELEASE_USER:
      
    /* if this keysym wasn't already pressed, stop processing this
       event: */
    if (!pressed_old) {
      return (TME_OK);
    }

    /* this keysym is no longer pressed: */
    keysym->tme_keysym_state_in0_pressed = FALSE;

    /* if this keysym is attached to a modifier: */
    if (modifier != TME_KEYBOARD_MODIFIER_NONE) {

      /* if we can find no keysym attached to this modifier that is
	 still pressed, clear the modifier in the mask: */
      for (mod_keysym = buffer->tme_keyboard_buffer_int_in0_modkeys[modifier];
	   mod_keysym != NULL;
	   mod_keysym = mod_keysym->tme_keysym_state_in0_modifier_next) {
	if (mod_keysym->tme_keysym_state_in0_pressed) {
	  break;
	}
      }
      if (mod_keysym == NULL) {
	buffer->tme_keyboard_buffer_int_in0_modifiers &= ~(1 << modifier);
      }
    }
    break;
      
  default:
    abort();
  }

  /* run the next stage: */
  return (_tme_keyboard_buffer_in1(buffer,
				   keysym,
				   event->tme_keyboard_event_time));
}

/* this is the top half of input stage zero.  this gets raw
   events from the input keyboard.  if we get a modifiers mask with
   each event, and we know which input keyboard keysyms are attached
   to which modifiers, we can often infer presses and releases of
   these keysyms when our caller has missed those events.  

   For example, this works well under X11 when the Caps_Lock or
   Num_Lock key "locks" and its state changes while our window doesn't
   have focus.  When we do regain focus, we don't get press or release
   events for those changes, but we can get the current modifiers
   mask: */
static int
_tme_keyboard_buffer_in0(struct tme_keyboard_buffer_int *buffer,
			 const struct tme_keyboard_event *event)
{
  struct tme_keysym_state *keysym, *mod_keysym, *other_keysym;
  struct tme_keyboard_event event_pseudo;
  int modifier;
  tme_keyboard_modifiers_t modifiers, modifiers_set, modifiers_clear;
  int rc;

  _tme_keyboard_debug(buffer,
		      "in0-top",
		      event->tme_keyboard_event_keyval,
		      (event->tme_keyboard_event_type
		       == TME_KEYBOARD_EVENT_PRESS),
		      event->tme_keyboard_event_time);

  assert (event->tme_keyboard_event_time
	  != TME_KEYBOARD_EVENT_TIME_UNDEF);

  /* look up this keysym: */
  keysym
    = ((struct tme_keysym_state *)
       tme_hash_lookup(buffer->tme_keyboard_buffer_int_keysyms_state,
		       tme_keyboard_hash_data_from_keyval(event->tme_keyboard_event_keyval)));


  /* if input stage zero has modifier information: */
  if (buffer->tme_keyboard_buffer_int_in0_have_modifiers) {

    /* NB: event->tme_keyboard_event_modifiers is always the modifiers
       mask from immediately *before* the event, i.e., if this event
       is for a modifier keysym, it does not reflect any change this
       press or release of this keysym will cause to that mask: */
    modifiers = event->tme_keyboard_event_modifiers;

    /* get the mask of modifiers that been cleared and set unbeknownst
       to us before this event: */
    modifiers_clear =
      (buffer->tme_keyboard_buffer_int_in0_modifiers
       & ~modifiers
       & buffer->tme_keyboard_buffer_int_in0_have_modifiers);
    modifiers_set =
      (modifiers
       & ~buffer->tme_keyboard_buffer_int_in0_modifiers
       & buffer->tme_keyboard_buffer_int_in0_have_modifiers);

    /* generate the appropriate release events for all keysyms
       attached to modifiers that have been cleared: */
    if (modifiers_clear != 0) {

      /* loop over the modifiers that need clearing: */
      for (modifier = 0;
	   modifier <= TME_KEYBOARD_MODIFIER_MAX;
	   modifier++) {
	if (!(modifiers_clear & (1 << modifier))) {
	  continue;
	}

	/* release all of the keysyms: */
	for (mod_keysym = buffer->tme_keyboard_buffer_int_in0_modkeys[modifier];
	     mod_keysym != NULL;
	     mod_keysym = mod_keysym->tme_keysym_state_in0_modifier_next) {

	  /* ignore this keysym if it isn't pressed: */
	  if (!mod_keysym->tme_keysym_state_in0_pressed) {
	    continue;
	  }

	  /* make the pseudoevent: */
	  event_pseudo.tme_keyboard_event_type
	    = TME_KEYBOARD_IN0_RELEASE_EVENT(mod_keysym->tme_keysym_state_in0_pressed);
	  event_pseudo.tme_keyboard_event_time
	    = _tme_keyboard_event_time_diff(event->tme_keyboard_event_time, -1);
	  event_pseudo.tme_keyboard_event_modifiers
	    = buffer->tme_keyboard_buffer_int_in0_modifiers;

	  /* call the bottom half with this pseudoevent: */
	  rc = _tme_keyboard_buffer_in0_bottom(buffer,
					       mod_keysym,
					       &event_pseudo);
	  assert (rc == TME_OK);
	}
      }

      /* all of the modifiers that needed clearing must now be clear: */
      assert ((buffer->tme_keyboard_buffer_int_in0_modifiers
	       & modifiers_clear) == 0);
    }

    /* generate a press event for a single keysym attached to each
       modifier that has been set: */
    if (modifiers_set != 0) {

      /* loop over the modifiers that need setting: */
      for (modifier = 0;
	   modifier <= TME_KEYBOARD_MODIFIER_MAX;
	   modifier++) {
	if (!(modifiers_set & (1 << modifier))) {
	  continue;
	}

	/* press the first keysym attached to this modifier: */
	mod_keysym = buffer->tme_keyboard_buffer_int_in0_modkeys[modifier];
	assert (mod_keysym != NULL);
	
	/* make the pseudoevent: */
	event_pseudo.tme_keyboard_event_type
	  = TME_KEYBOARD_EVENT_IN0_PRESS_AUTO;
	event_pseudo.tme_keyboard_event_time
	  = _tme_keyboard_event_time_diff(event->tme_keyboard_event_time, -1);
	event_pseudo.tme_keyboard_event_modifiers
	  = buffer->tme_keyboard_buffer_int_in0_modifiers;

	/* call the bottom half with this pseudoevent: */
	rc = _tme_keyboard_buffer_in0_bottom(buffer,
					     mod_keysym,
					     &event_pseudo);
	assert (rc == TME_OK);
      }

      /* all of the modifiers that needed setting must now be set: */
      assert ((buffer->tme_keyboard_buffer_int_in0_modifiers
	       & modifiers_set) == modifiers_set);
    }

    /* if the keysym in the event is undefined, we don't need to
       process this event any more.  events with an undefined keysym
       can be used whenever the caller thinks that it has dropped
       keyboard events, and wants to do what it can to update the
       keyboard, and it can at least get the true current modifiers
       mask.  this is common under X11: */
    if (event->tme_keyboard_event_keyval
	== TME_KEYBOARD_KEYVAL_UNDEF) {
      return (TME_OK);
    }
  }

  /* we must have a defined keysym: */
  assert (event->tme_keyboard_event_keyval
	  != TME_KEYBOARD_KEYVAL_UNDEF);

  /* if we have no state for the keysym in this event, this keysym
     isn't controlled by any of the input stages: */
  if (keysym == NULL) {

    /* if there are output stages, having no state for this keysym
       means that this keysym doesn't exist on the output keyboard.
       we can stop processing this event now: */
    if (!buffer->tme_keyboard_buffer_int_out0_passthrough) {
      return (TME_OK);
    }

    /* otherwise, simply buffer this event: */
    event_pseudo = *event;
    switch (event->tme_keyboard_event_type) {
    case TME_KEYBOARD_EVENT_IN0_PRESS_USER:
    case TME_KEYBOARD_EVENT_IN0_PRESS_AUTO:
      event_pseudo.tme_keyboard_event_type = TME_KEYBOARD_EVENT_PRESS;
      break;
    case TME_KEYBOARD_EVENT_IN0_RELEASE_USER:
    case TME_KEYBOARD_EVENT_IN0_RELEASE_AUTO:
      event_pseudo.tme_keyboard_event_type = TME_KEYBOARD_EVENT_RELEASE;
      break;
    default: abort();
    }
    event_pseudo.tme_keyboard_event_modifiers = 0;
    return (_tme_keyboard_buffer_copyin(buffer, &event_pseudo));
  }

  /* if this event has a keycode: */
  if (event->tme_keyboard_event_keycode
      != TME_KEYBOARD_KEYVAL_UNDEF) {

    /* see if this keycode is already pressed in this stage: */
    other_keysym
      = ((struct tme_keysym_state *)
	 tme_hash_lookup(buffer->tme_keyboard_buffer_int_in0_keycodes,
			 tme_keyboard_hash_data_from_keyval(event->tme_keyboard_event_keycode)));

    /* if this keycode is already pressed by another keysym in this
       stage, release the other keysym first: */
    if (other_keysym != NULL
	&& other_keysym != keysym
	&& TME_KEYBOARD_PRESSED_IN0(other_keysym)) {
      
      /* make the pseudoevent: */
      event_pseudo.tme_keyboard_event_type
	= TME_KEYBOARD_IN0_RELEASE_EVENT(other_keysym->tme_keysym_state_in0_pressed);
      event_pseudo.tme_keyboard_event_time
	= _tme_keyboard_event_time_diff(event->tme_keyboard_event_time, -1);
      event_pseudo.tme_keyboard_event_modifiers
	= buffer->tme_keyboard_buffer_int_in0_modifiers;
      
      /* call the bottom half with this pseudoevent: */
      rc = _tme_keyboard_buffer_in0_bottom(buffer,
					   other_keysym,
					   &event_pseudo);
      assert (rc == TME_OK);
    }

    /* if this is a press, remember that this keycode is pressed,
       else forget that this keycode is pressed: */
    if (event->tme_keyboard_event_type == TME_KEYBOARD_EVENT_PRESS) {
      tme_hash_insert(buffer->tme_keyboard_buffer_int_in0_keycodes,
		      tme_keyboard_hash_data_from_keyval(event->tme_keyboard_event_keycode),
		      (tme_hash_data_t) keysym);
    }
    else {
      tme_hash_remove(buffer->tme_keyboard_buffer_int_in0_keycodes,
		      tme_keyboard_hash_data_from_keyval(event->tme_keyboard_event_keycode));
    }
  }

  /* call the bottom half with this event: */
  return (_tme_keyboard_buffer_in0_bottom(buffer,
					  keysym,
					  event));
}

/* this copies a keyboard event into the buffer: */
int
tme_keyboard_buffer_copyin(struct tme_keyboard_buffer *_buffer,
			   const struct tme_keyboard_event *event)
{
  struct tme_keyboard_buffer_int *buffer;

  /* recover our data structure: */
  buffer = (struct tme_keyboard_buffer_int *) _buffer;

  /* run input stage zero: */
  return (_tme_keyboard_buffer_in0(buffer, event));
}

/* this copies a keyval out of a keyboard buffer: */
int
tme_keyboard_buffer_copyout(struct tme_keyboard_buffer *buffer,
			    struct tme_keyboard_event *event)
{
  unsigned int buffer_tail, buffer_size_mask;

  buffer_tail = buffer->tme_keyboard_buffer_tail;
  buffer_size_mask = buffer->tme_keyboard_buffer_size - 1;

  /* if the buffer is empty: */
  if (buffer_tail == buffer->tme_keyboard_buffer_head) {
    return (EAGAIN);
  }

  /* get an event out of the buffer: */
  *event = buffer->tme_keyboard_buffer_events[buffer_tail];

  /* advance the tail: */
  buffer->tme_keyboard_buffer_tail = (buffer_tail + 1) & buffer_size_mask;
  return (TME_OK);
}
