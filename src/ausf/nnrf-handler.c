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

#include "sbi-path.h"
#include "nnrf-handler.h"

void ausf_nnrf_handle_nf_register(
        ogs_sbi_nf_instance_t *nf_instance, ogs_sbi_message_t *message)
{
    OpenAPI_nf_profile_t *NFProfile = NULL;
    ogs_sbi_client_t *client = NULL;

    ogs_assert(message);
    ogs_assert(nf_instance);
    client = nf_instance->client;
    ogs_assert(client);

    NFProfile = message->NFProfile;
    if (!NFProfile) {
        ogs_error("No NFProfile");
        return;
    }

    /* TIME : Update heartbeat from NRF */
    nf_instance->time.heartbeat = NFProfile->heart_beat_timer;
}

void ausf_nnrf_handle_nf_status_subscribe(
        ogs_sbi_subscription_t *subscription, ogs_sbi_message_t *message)
{
    OpenAPI_subscription_data_t *SubscriptionData = NULL;
    ogs_sbi_client_t *client = NULL;

    ogs_assert(message);
    ogs_assert(subscription);
    client = subscription->client;
    ogs_assert(client);

    SubscriptionData = message->SubscriptionData;
    if (!SubscriptionData) {
        ogs_error("No SubscriptionData");
        return;
    }

    if (!SubscriptionData->subscription_id) {
        ogs_error("No SubscriptionId");
        return;
    }
    ogs_sbi_subscription_set_id(
        subscription, SubscriptionData->subscription_id);

    if (SubscriptionData->validity_time) {
        struct timeval tv;
        struct tm local, next;
        ogs_time_t diff, duration;
        if (ogs_strptime(SubscriptionData->validity_time,
                OGS_TIME_ISO8601_FORMAT, &next)) {
            ogs_gettimeofday(&tv);
            ogs_localtime(tv.tv_sec, &local);
            diff = ogs_mktime(&next) - ogs_mktime(&local);
#define VALIDITY_MARGIN 5 /* 5 seconds */
#define VALIDITY_MINIMUM 60 /* 60 seconds */
            duration = diff - (int)VALIDITY_MARGIN;

            if (duration < (int)VALIDITY_MINIMUM) {
                ogs_warn("Validation period [%d seconds, %s] is too small",
                        (int)diff, SubscriptionData->validity_time);
                duration = VALIDITY_MINIMUM;
                ogs_warn("Forced to %d seconds", VALIDITY_MINIMUM);
            }

            subscription->t_validity = ogs_timer_add(ausf_self()->timer_mgr,
                ausf_timer_subscription_validity, subscription);
            ogs_assert(subscription->t_validity);
            ogs_timer_start(
                    subscription->t_validity, ogs_time_from_sec(duration));
        }
    }
}

bool ausf_nnrf_handle_nf_status_notify(ogs_sbi_server_t *server,
        ogs_sbi_session_t *session, ogs_sbi_message_t *message)
{
    ogs_sbi_response_t *response = NULL;
    OpenAPI_notification_data_t *NotificationData = NULL;
    OpenAPI_nf_profile_t *NFProfile = NULL;
    ogs_sbi_nf_instance_t *nf_instance = NULL;
    bool handled;

    ogs_assert(session);
    ogs_assert(message);

    NotificationData = message->NotificationData;
    if (!NotificationData) {
        ogs_error("No NotificationData");
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                message, "No NotificationData", NULL);
        return false;
    }

    NFProfile = NotificationData->nf_profile;
    if (!NFProfile) {
        ogs_error("No NFProfile");
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                message, "No NFProfile", NULL);
        return false;
    }

    if (!NFProfile->nf_instance_id) {
        ogs_error("No NFProfile.NFInstanceId");
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                message, "No NFProfile", "NFInstanceId");
        return false;
    }

    if (!NFProfile->nf_instance_id) {
        ogs_error("No NFProfile.NFInstanceId");
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                message, "No NFProfile", "NFInstanceId");
        return false;
    }

    if (NF_INSTANCE_IS_SELF(NFProfile->nf_instance_id)) {
        ogs_error("The notification is not allowed [%s]",
                NFProfile->nf_instance_id);
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_FORBIDDEN,
                message, "The notification is not allowed",
                NFProfile->nf_instance_id);
        return false;
    }

    if (NotificationData->event ==
            OpenAPI_notification_event_type_NF_REGISTERED) {

        nf_instance = ogs_sbi_nf_instance_find(NFProfile->nf_instance_id);
        if (!nf_instance) {
            nf_instance = ogs_sbi_nf_instance_add(NFProfile->nf_instance_id);
            ogs_assert(nf_instance);

            ausf_nf_fsm_init(nf_instance);

            ogs_info("(NRF-notify) NF registered [%s]", nf_instance->id);
        } else {
            OGS_FSM_TRAN(&nf_instance->sm, ausf_nf_state_registered);
            ogs_fsm_dispatch(&nf_instance->sm, NULL);

            ogs_warn("(NRF-notify) NF [%s] has already been added",
                    NFProfile->nf_instance_id);
        }

        handled = ogs_sbi_nnrf_handle_nf_profile(
                    nf_instance, NFProfile, session, message);
        if (!handled) {
            AUSF_NF_INSTANCE_CLEAR("NRF-notify", nf_instance);
            return false;
        }

        ogs_info("(NRF-notify) NF Profile updated [%s]", nf_instance->id);

        handled = ogs_sbi_client_associate(nf_instance);
        if (!handled) {
            ogs_error("Cannot associate NF EndPoint [%s]", nf_instance->id);
            ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                    message, "Cannot find NF EndPoint", nf_instance->id);
            AUSF_NF_INSTANCE_CLEAR("NRF-notify", nf_instance);
            return false;
        }

        ausf_sbi_setup_client_callback(nf_instance);

    } else if (NotificationData->event ==
            OpenAPI_notification_event_type_NF_DEREGISTERED) {
        nf_instance = ogs_sbi_nf_instance_find(NFProfile->nf_instance_id);
        if (nf_instance) {
            AUSF_NF_INSTANCE_CLEAR("NRF-notify", nf_instance);
        } else {
            ogs_warn("(NRF-notify) Not found [%s]", NFProfile->nf_instance_id);
            ogs_sbi_server_send_error(session,
                OGS_SBI_HTTP_STATUS_NOT_FOUND,
                message, "Not found", message->h.resource.id);
            return false;
        }
    } else {
        char *eventstr = OpenAPI_notification_event_type_ToString(
                            NotificationData->event);
        ogs_error("Not supported event [%d:%s]",
                NotificationData->event, eventstr ? eventstr : "Unknown");
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                message, "Not supported event",
                eventstr ? eventstr : "Unknown");
        return false;
    }

    response = ogs_sbi_build_response(message, OGS_SBI_HTTP_STATUS_NO_CONTENT);
    ogs_assert(response);
    ogs_sbi_server_send_response(session, response);

    return true;
}

void ausf_nnrf_handle_nf_discover(
        ogs_sbi_session_t *session, ogs_sbi_message_t *message)
{
    ogs_sbi_nf_instance_t *nf_instance = NULL;
    ausf_ue_t *ausf_ue = NULL;

    OpenAPI_search_result_t *SearchResult = NULL;
    OpenAPI_lnode_t *node = NULL;
    bool handled;

    ogs_assert(session);
    ausf_ue = ogs_sbi_session_get_data(session);
    ogs_assert(ausf_ue);
    ogs_assert(message);

    SearchResult = message->SearchResult;
    if (!SearchResult) {
        ogs_error("No SearchResult");
        return;
    }

    OpenAPI_list_for_each(SearchResult->nf_instances, node) {
        OpenAPI_nf_profile_t *NFProfile = NULL;

        if (!node->data) continue;

        NFProfile = node->data;

        nf_instance = ogs_sbi_nf_instance_find(NFProfile->nf_instance_id);
        if (!nf_instance) {
            nf_instance = ogs_sbi_nf_instance_add(NFProfile->nf_instance_id);
            ogs_assert(nf_instance);

            ausf_nf_fsm_init(nf_instance);

            ogs_info("(NF-discover) NF registered [%s]", nf_instance->id);
        } else {
            OGS_FSM_TRAN(&nf_instance->sm, ausf_nf_state_registered);
            ogs_fsm_dispatch(&nf_instance->sm, NULL);

            ogs_warn("(NF-discover) NF [%s] has already been added",
                    NFProfile->nf_instance_id);
        }

        if (NF_INSTANCE_IS_OTHERS(nf_instance->id)) {
            handled = ogs_sbi_nnrf_handle_nf_profile(
                        nf_instance, NFProfile, NULL, NULL);
            if (!handled) {
                ogs_error("ogs_sbi_nnrf_handle_nf_profile() failed [%s]",
                        nf_instance->id);
                AUSF_NF_INSTANCE_CLEAR("NRF-discover", nf_instance);
                continue;
            }

            handled = ogs_sbi_client_associate(nf_instance);
            if (!handled) {
                ogs_error("Cannot assciate NF EndPoint [%s]", nf_instance->id);
                AUSF_NF_INSTANCE_CLEAR("NRF-discover", nf_instance);
                continue;
            }

            ausf_sbi_setup_client_callback(nf_instance);

            if (!OGS_SBI_NF_INSTANCE_GET(
                        ausf_ue->nf_types, nf_instance->nf_type))
                ogs_sbi_nf_types_associate(ausf_ue->nf_types,
                        nf_instance->nf_type, ausf_nf_state_registered);

            /* TIME : Update validity from NRF */
            if (SearchResult->validity_period) {
                nf_instance->time.validity = SearchResult->validity_period;

                ogs_assert(nf_instance->t_validity);
                ogs_timer_start(nf_instance->t_validity,
                        ogs_time_from_sec(nf_instance->time.validity));

            } else
                ogs_warn("NF Instance validity-time should not 0");

            ogs_info("(NF-discover) NF Profile updated [%s]", nf_instance->id);
        }
    }

    if (OGS_FSM_CHECK(&ausf_ue->sm, ausf_ue_state_will_authenticate)) {
        nf_instance = OGS_SBI_NF_INSTANCE_GET(
                ausf_ue->nf_types, OpenAPI_nf_type_UDM);
        if (!nf_instance) {
            ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_SERVICE_UNAVAILABLE, NULL,
                    "Cannot receive SBI message", ausf_ue->id);
        } else {
            ausf_nudm_ueau_send_get(ausf_ue, nf_instance);
        }
    } else {
        ogs_fatal("Should implement other case");
        ogs_assert_if_reached();
    }
}
