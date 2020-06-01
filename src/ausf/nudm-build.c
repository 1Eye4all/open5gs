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

#include "nudm-build.h"

ogs_sbi_request_t *ausf_nudm_ueau_build_get(ausf_ue_t *ausf_ue)
{
    ogs_sbi_message_t message;
    ogs_sbi_request_t *request = NULL;

    OpenAPI_authentication_info_request_t *AuthenticationInfoRequest = NULL;

    ogs_assert(ausf_ue);

    memset(&message, 0, sizeof(message));
    message.h.method = (char *)OGS_SBI_HTTP_METHOD_POST;
    message.h.service.name = (char *)OGS_SBI_SERVICE_NAME_NUDM_UEAU;
    message.h.api.version = (char *)OGS_SBI_API_VERSION;
    message.h.resource.component[0] = ausf_ue->id;
    message.h.resource.component[1] =
        (char *)OGS_SBI_RESOURCE_NAME_SECURITY_INFORMATION;
    message.h.resource.component[2] =
        (char *)OGS_SBI_RESOURCE_NAME_GENERATE_AUTH_DATA;

    AuthenticationInfoRequest =
        ogs_calloc(1, sizeof(*AuthenticationInfoRequest));
    ogs_assert(AuthenticationInfoRequest);

    AuthenticationInfoRequest->serving_network_name =
        ausf_ue->serving_network_name;
    AuthenticationInfoRequest->ausf_instance_id =
        ogs_sbi_self()->nf_instance_id;

    message.AuthenticationInfoRequest = AuthenticationInfoRequest;

    request = ogs_sbi_build_request(&message);
    ogs_assert(request);

    ogs_free(AuthenticationInfoRequest);

    return request;
}