/*
 *  Copyright (C) 2004 Steve Harris
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation; either version 2.1
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  $Id: lo_macros.h,v 1.4 2009/02/27 00:15:50 rbd Exp $
 */

#ifndef LO_MACROS_H
#define LO_MACROS_H

/* macros that have to be defined after function signatures */

#ifdef __cplusplus
extern "C" {
#endif

/* \brief Maximum length of UDP messages in bytes
 */
#define LO_MAX_MSG_SIZE 32768

/* \brief A set of macros to represent different communications transports
 */
#define LO_DEFAULT 0x0
#define LO_UDP     0x1
#define LO_UNIX    0x2
#define LO_TCP     0x4

/* an internal value, ignored in transmission but check against LO_MARKER in the
 * argument list. Used to do primitive bounds checking */
#define LO_MARKER_A 0xdeadbeef
#define LO_MARKER_B 0xf00baa23
#define LO_ARGS_END LO_MARKER_A, LO_MARKER_B

#ifdef _WIN32
#define lo_message_add(msg, types, ...)                         \
    lo_message_add_internal(msg, __FILE__, __LINE__, types,  \
                            __VA_ARGS__, \
                            LO_MARKER_A, LO_MARKER_B)
#else
#define lo_message_add(msg, types...)                         \
    lo_message_add_internal(msg, __FILE__, __LINE__, types,   \
                            LO_MARKER_A, LO_MARKER_B)
#endif

#define lo_message_add_varargs(msg, types, list) \
        lo_message_add_varargs_internal(msg, types, list, __FILE__, __LINE__)


#ifdef _WIN32
#define lo_send(targ, path, types, ...) \
        lo_send_internal(targ, __FILE__, __LINE__, path, types, \
			 __VA_ARGS__, LO_MARKER_A, LO_MARKER_B)

#define lo_send_timestamped(targ, ts, path, types, ...) \
        lo_send_timestamped_internal(targ, __FILE__, __LINE__, ts, path, \
		       	             types, __VA_ARGS__, \
                                     LO_MARKER_A, LO_MARKER_B)

#define lo_send_from(targ, from, ts, path, types, ...) \
        lo_send_from_internal(targ, from, __FILE__, __LINE__, ts, path, \
                              types, __VA_ARGS__, \
		       	      LO_MARKER_A, LO_MARKER_B)
#else
#define lo_send(targ, path, types...) \
        lo_send_internal(targ, __FILE__, __LINE__, path, types, \
			 LO_MARKER_A, LO_MARKER_B)

#define lo_send_timestamped(targ, ts, path, types...) \
        lo_send_timestamped_internal(targ, __FILE__, __LINE__, ts, path, \
		       	             types, LO_MARKER_A, LO_MARKER_B)

#define lo_send_from(targ, from, ts, path, types...) \
        lo_send_from_internal(targ, from, __FILE__, __LINE__, ts, path, \
		       	             types, LO_MARKER_A, LO_MARKER_B)
#endif

#ifdef __cplusplus
}
#endif

#endif