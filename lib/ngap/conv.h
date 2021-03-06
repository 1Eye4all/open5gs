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

#if !defined(OGS_NGAP_INSIDE) && !defined(OGS_NGAP_COMPILATION)
#error "This header cannot be included directly."
#endif

#ifndef OGS_NGAP_CONV_H
#define OGS_NGAP_CONV_H

#ifdef __cplusplus
extern "C" {
#endif

void ogs_ngap_uint32_to_GNB_ID(uint32_t gnb_id, NGAP_GNB_ID_t *gNB_ID);
void ogs_ngap_GNB_ID_to_uint32(NGAP_GNB_ID_t *gNB_ID, uint32_t *gnb_id);

void ogs_ngap_uint8_to_AMFRegionID(
        uint8_t region, NGAP_AMFRegionID_t *aMFRegionID);
void ogs_ngap_uint16_to_NGAP_AMFSetID(
        uint16_t set, NGAP_AMFSetID_t *aMFSetID);
void ogs_ngap_uint8_to_NGAP_NGAP_AMFPointer(
        uint8_t pointer, NGAP_AMFPointer_t *aMFPointer);

#ifdef __cplusplus
}
#endif

#endif /* OGS_NGAP_CONV_H */

