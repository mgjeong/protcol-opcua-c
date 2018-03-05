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
 * @file cmd_util.h
 *
 * @brief This file contains the Utility functions for commands.
 */

#ifndef EDGE_CMD_UTIL_H
#define EDGE_CMD_UTIL_H

#include <open62541.h>

/**
 * @brief Get the data type of the response message
 * @param[in]  type UA_DataType .
 * @return response data type
 */
int get_response_type(const UA_DataType *type);


#endif // EDGE_CMD_UTIL_H
