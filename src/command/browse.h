/******************************************************************
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

/**
 * @file browse.h
 *
 * This file contains the definition, types and APIs for BROWSE command request.
 */

#ifndef EDGE_BROWSE_H
#define EDGE_BROWSE_H

#include "opcua_common.h"
#include "open62541.h"

#include "edge_utils.h"
#include "command_adapter.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Executes Browse operation
 * @param[in]  client Client Handle.
 * @param[in]  msg EdgeMessage request data
 * @param[in]  browseNext flag to indicate whether its browse or browseNext operation
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult executeBrowse(UA_Client *client, EdgeMessage *msg, bool browseNext);

/**
 * @brief Executes Browse View operation
 * @param[in]  client Client Handle.
 * @param[in]  msg EdgeMessage request data
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult executeBrowseViews(UA_Client *client, EdgeMessage *msg);

/**
 * @brief Register callback for browse response
 * @param[in]  callback response callback
 */
void resgisterBrowseResponseCallback(response_cb_t callback);

#ifdef __cplusplus
}
#endif

#endif
