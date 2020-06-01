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

#include "nausf-handler.h"
#include "nas-path.h"

bool amf_nausf_auth_handle_authenticate(
        amf_ue_t *amf_ue, ogs_sbi_message_t *message)
{
    OpenAPI_ue_authentication_ctx_t *UeAuthenticationCtx = NULL;
    OpenAPI_av5g_aka_t *AV5G_AKA = NULL;
    OpenAPI_links_value_schema_t *LinksValueSchemeValue = NULL;
    OpenAPI_map_t *LinksValueScheme = NULL;
    OpenAPI_lnode_t *node = NULL;

    ogs_assert(amf_ue);
    ogs_assert(message);

    UeAuthenticationCtx = message->UeAuthenticationCtx;
    if (!UeAuthenticationCtx) {
        ogs_error("[%s] No UeAuthenticationCtx", amf_ue->id);
        nas_5gs_send_nas_reject_from_sbi(
                amf_ue, OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return false;
    }

    if (UeAuthenticationCtx->auth_type != OpenAPI_auth_type_5G_AKA) {
        ogs_error("[%s] Not supported Auth Method [%d]",
            amf_ue->id, UeAuthenticationCtx->auth_type);
        nas_5gs_send_nas_reject_from_sbi(
                amf_ue, OGS_SBI_HTTP_STATUS_FORBIDDEN);
        return false;
    }

    AV5G_AKA = UeAuthenticationCtx->_5g_auth_data;
    if (!AV5G_AKA) {
        ogs_error("[%s] No Av5gAka", amf_ue->id);
        nas_5gs_send_nas_reject_from_sbi(
                amf_ue, OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return false;
    }

    if (!AV5G_AKA->rand) {
        ogs_error("[%s] No Av5gAka.rand", amf_ue->id);
        nas_5gs_send_nas_reject_from_sbi(
                amf_ue, OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return false;
    }

    if (!AV5G_AKA->hxres_star) {
        ogs_error("[%s] No Av5gAka.hxresStar", amf_ue->id);
        nas_5gs_send_nas_reject_from_sbi(
                amf_ue, OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return false;
    }

    if (!AV5G_AKA->autn) {
        ogs_error("[%s] No Av5gAka.autn", amf_ue->id);
        nas_5gs_send_nas_reject_from_sbi(
                amf_ue, OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return false;
    }

    if (!UeAuthenticationCtx->_links) {
        ogs_error("[%s] No _links", amf_ue->id);
        nas_5gs_send_nas_reject_from_sbi(
                amf_ue, OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return false;
    }

    OpenAPI_list_for_each(UeAuthenticationCtx->_links, node) {
        LinksValueScheme = node->data;
        if (LinksValueScheme) {
            if (strcmp(LinksValueScheme->key, "5g-aka") == 0) {
                LinksValueSchemeValue = LinksValueScheme->value;
                break;
            }
        }
    }

    if (!LinksValueSchemeValue) {
        ogs_error("[%s] No _links.5g-aka", amf_ue->id);
        nas_5gs_send_nas_reject_from_sbi(
                amf_ue, OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return false;
    }

    if (!LinksValueSchemeValue->href) {
        ogs_error("[%s] No _links.5g-aka.href", amf_ue->id);
        nas_5gs_send_nas_reject_from_sbi(
                amf_ue, OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return false;
    }

    ogs_ascii_to_hex(AV5G_AKA->rand, strlen(AV5G_AKA->rand),
        amf_ue->rand, sizeof(amf_ue->rand));
    ogs_ascii_to_hex(AV5G_AKA->hxres_star, strlen(AV5G_AKA->hxres_star),
        amf_ue->hxres_star, sizeof(amf_ue->hxres_star));
    ogs_ascii_to_hex(AV5G_AKA->autn, strlen(AV5G_AKA->autn),
        amf_ue->autn, sizeof(amf_ue->autn));

    return true;
}