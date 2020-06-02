/*
 * Copyright (C) 2019 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef AUSF_TIMER_H
#define AUSF_TIMER_H

#include "ogs-core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* forward declaration */
typedef enum {
    AUSF_TIMER_BASE = 0,

    AUSF_TIMER_NF_INSTANCE_REGISTRATION_INTERVAL,
    AUSF_TIMER_NF_INSTANCE_HEARTBEAT_INTERVAL,
    AUSF_TIMER_NF_INSTANCE_HEARTBEAT,
    AUSF_TIMER_NF_INSTANCE_VALIDITY,
    AUSF_TIMER_SUBSCRIPTION_VALIDITY,
    AUSF_TIMER_SBI_CLIENT_WAIT,

    MAX_NUM_OF_AUSF_TIMER,

} ausf_timer_e;

typedef struct ausf_timer_cfg_s {
    int max_count;
    ogs_time_t duration;
} ausf_timer_cfg_t;

ausf_timer_cfg_t *ausf_timer_cfg(ausf_timer_e id);

const char *ausf_timer_get_name(ausf_timer_e id);

void ausf_timer_nf_instance_registration_interval(void *data);
void ausf_timer_nf_instance_heartbeat_interval(void *data);
void ausf_timer_nf_instance_heartbeat(void *data);
void ausf_timer_nf_instance_validity(void *data);
void ausf_timer_subscription_validity(void *data);
void ausf_timer_sbi_client_wait_expire(void *data);

#ifdef __cplusplus
}
#endif

#endif /* AUSF_TIMER_H */
