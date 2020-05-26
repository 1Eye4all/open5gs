/*
 * Copyright (C) 2019,2020 by Sukchan Lee <acetcom@gmail.com>
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

#include "context.h"

static amf_timer_cfg_t g_amf_timer_cfg[MAX_NUM_OF_AMF_TIMER] = {
    [AMF_TIMER_NF_INSTANCE_REGISTRATION_INTERVAL] =
        { .duration = ogs_time_from_sec(3) },

    /* Paging procedure for EPS services initiated */
    [AMF_TIMER_T3413] = 
        { .max_count = 2, .duration = ogs_time_from_sec(2) },

    /* DETACH REQUEST sent */
    [AMF_TIMER_T3422] = 
        { .max_count = 4, .duration = ogs_time_from_sec(3) },

    /* ATTACH ACCEPT sent
     * TRACKING AREA UPDATE ACCEPT sent with GUTI
     * TRACKING AREA UPDATE ACCEPT sent with TMSI
     * GUTI REALLOCATION COMMAND sent */
    [AMF_TIMER_T3450] = 
        { .max_count = 4, .duration = ogs_time_from_sec(6) },

    /* AUTHENTICATION REQUEST sent
     * SECURITY MODE COMMAND sent */
    [AMF_TIMER_T3460] = 
        { .max_count = 4, .duration = ogs_time_from_sec(3) },

    /* IDENTITY REQUEST sent */
    [AMF_TIMER_T3470] = 
        { .max_count = 4, .duration = ogs_time_from_sec(3) },

    /* 5GSM INFORMATION REQUEST sent */
    [AMF_TIMER_T3489] = 
        { .max_count = 2, .duration = ogs_time_from_sec(4) },

#if 0
    [AMF_TIMER_SGS_CLI_CONN_TO_SRV] = 
        { .duration = ogs_time_from_sec(3) },
#endif
};

static void gmm_timer_event_send(
        amf_timer_e timer_id, amf_ue_t *amf_ue);
static void gsm_timer_event_send(
        amf_timer_e timer_id, amf_bearer_t *bearer);

amf_timer_cfg_t *amf_timer_cfg(amf_timer_e id)
{
    ogs_assert(id < MAX_NUM_OF_AMF_TIMER);
    return &g_amf_timer_cfg[id];
}

const char *amf_timer_get_name(amf_timer_e id)
{
    switch (id) {
    case AMF_TIMER_NF_INSTANCE_REGISTRATION_INTERVAL:
        return "AMF_TIMER_NF_INSTANCE_REGISTRATION_INTERVAL";
    case AMF_TIMER_NF_INSTANCE_HEARTBEAT_INTERVAL:
        return "AMF_TIMER_NF_INSTANCE_HEARTBEAT_INTERVAL";
    case AMF_TIMER_NF_INSTANCE_HEARTBEAT:
        return "AMF_TIMER_NF_INSTANCE_HEARTBEAT";
    case AMF_TIMER_NF_INSTANCE_VALIDITY:
        return "AMF_TIMER_NF_INSTANCE_VALIDITY";
    case AMF_TIMER_SUBSCRIPTION_VALIDITY:
        return "AMF_TIMER_SUBSCRIPTION_VALIDITY";
    case AMF_TIMER_NG_DELAYED_SEND:
        return "AMF_TIMER_NG_DELAYED_SEND";
    case AMF_TIMER_T3413:
        return "AMF_TIMER_T3413";
    case AMF_TIMER_T3422:
        return "AMF_TIMER_T3422";
    case AMF_TIMER_T3450:
        return "AMF_TIMER_T3450";
    case AMF_TIMER_T3460:
        return "AMF_TIMER_T3460";
    case AMF_TIMER_T3470:
        return "AMF_TIMER_T3470";
    case AMF_TIMER_T3489:
        return "AMF_TIMER_T3489";
#if 0
    case AMF_TIMER_SGS_CLI_CONN_TO_SRV:
        return "AMF_TIMER_SGS_CLI_CONN_TO_SRV";
#endif
    default: 
       break;
    }

    return "UNKNOWN_TIMER";
}

void amf_timer_ng_delayed_send(void *data)
{
    int rv;
    amf_event_t *e = data;
    ogs_assert(e);

    e->timer_id = AMF_TIMER_NG_DELAYED_SEND;

    rv = ogs_queue_push(amf_self()->queue, e);
    if (rv != OGS_OK) {
        ogs_warn("ogs_queue_push() failed:%d", (int)rv);
        ogs_timer_delete(e->timer);
        amf_event_free(e);
    }
}

static void sbi_timer_send_event(int timer_id, void *data)
{
    int rv;
    amf_event_t *e = NULL;
    ogs_assert(data);

    switch (timer_id) {
    case AMF_TIMER_NF_INSTANCE_REGISTRATION_INTERVAL:
    case AMF_TIMER_NF_INSTANCE_HEARTBEAT_INTERVAL:
    case AMF_TIMER_NF_INSTANCE_HEARTBEAT:
    case AMF_TIMER_NF_INSTANCE_VALIDITY:
    case AMF_TIMER_SUBSCRIPTION_VALIDITY:
        e = amf_event_new(AMF_EVT_SBI_TIMER);
        ogs_assert(e);
        e->timer_id = timer_id;
        e->sbi.data = data;
        break;
    default:
        ogs_fatal("Unknown timer id[%d]", timer_id);
        ogs_assert_if_reached();
        break;
    }

    rv = ogs_queue_push(amf_self()->queue, e);
    if (rv != OGS_OK) {
        ogs_warn("ogs_queue_push() failed [%d] in %s",
                (int)rv, amf_timer_get_name(e->timer_id));
        amf_event_free(e);
    }
}

void amf_timer_nf_instance_registration_interval(void *data)
{
    sbi_timer_send_event(AMF_TIMER_NF_INSTANCE_REGISTRATION_INTERVAL, data);
}

void amf_timer_nf_instance_heartbeat_interval(void *data)
{
    sbi_timer_send_event(AMF_TIMER_NF_INSTANCE_HEARTBEAT_INTERVAL, data);
}

void amf_timer_nf_instance_heartbeat(void *data)
{
    sbi_timer_send_event(AMF_TIMER_NF_INSTANCE_HEARTBEAT, data);
}

void amf_timer_nf_instance_validity(void *data)
{
    sbi_timer_send_event(AMF_TIMER_NF_INSTANCE_VALIDITY, data);
}

void amf_timer_subscription_validity(void *data)
{
    sbi_timer_send_event(AMF_TIMER_SUBSCRIPTION_VALIDITY, data);
}

static void gmm_timer_event_send(
        amf_timer_e timer_id, amf_ue_t *amf_ue)
{
    int rv;
    amf_event_t *e = NULL;
    ogs_assert(amf_ue);

    e = amf_event_new(AMF_EVT_5GMM_TIMER);
    e->timer_id = timer_id;
    e->amf_ue = amf_ue;

    rv = ogs_queue_push(amf_self()->queue, e);
    if (rv != OGS_OK) {
        ogs_warn("ogs_queue_push() failed:%d", (int)rv);
        amf_event_free(e);
    }
}

void amf_timer_t3413_expire(void *data)
{
    gmm_timer_event_send(AMF_TIMER_T3413, data);
}
void amf_timer_t3422_expire(void *data)
{
    gmm_timer_event_send(AMF_TIMER_T3422, data);
}
void amf_timer_t3450_expire(void *data)
{
    gmm_timer_event_send(AMF_TIMER_T3450, data);
}
void amf_timer_t3460_expire(void *data)
{
    gmm_timer_event_send(AMF_TIMER_T3460, data);
}
void amf_timer_t3470_expire(void *data)
{
    gmm_timer_event_send(AMF_TIMER_T3470, data);
}

static void gsm_timer_event_send(
        amf_timer_e timer_id, amf_bearer_t *bearer)
{
    int rv;
    amf_event_t *e = NULL;
    amf_ue_t *amf_ue = NULL;
    ogs_assert(bearer);
    amf_ue = bearer->amf_ue;
    ogs_assert(bearer);

    e = amf_event_new(AMF_EVT_5GSM_TIMER);
    e->timer_id = timer_id;
    e->amf_ue = amf_ue;
    e->bearer = bearer;

    rv = ogs_queue_push(amf_self()->queue, e);
    if (rv != OGS_OK) {
        ogs_warn("ogs_queue_push() failed:%d", (int)rv);
        amf_event_free(e);
    }
}

void amf_timer_t3489_expire(void *data)
{
    gsm_timer_event_send(AMF_TIMER_T3489, data);
}

#if 0
void amf_timer_sgs_cli_conn_to_srv(void *data)
{
    int rv;
    amf_event_t *e = NULL;
    ogs_assert(data);

    e = amf_event_new(AMF_EVT_SGSAP_TIMER);
    e->timer_id = AMF_TIMER_SGS_CLI_CONN_TO_SRV;
    e->vlr = data;

    rv = ogs_queue_push(amf_self()->queue, e);
    if (rv != OGS_OK) {
        ogs_warn("ogs_queue_push() failed:%d", (int)rv);
        amf_event_free(e);
    }
}
#endif