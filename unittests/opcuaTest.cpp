#include <gtest/gtest.h>
#include <iostream>

extern "C"
{
#include "opcua_manager.h"
#include "opcua_common.h"
#include "edge_identifier.h"
#include "edge_utils.h"
#include "edge_logger.h"
#include "edge_malloc.h"
#include "open62541.h"
}

#define TAG "TC"

#define LOCALHOST "localhost"
#define ENDPOINT_URI  "opc:tcp://%s:12686/edge-opc-server"
#define IPADDRESS "opc.tcp://localhost:12686"

#define TEST_STR1_R "test1"
#define TEST_STR2_R "test2"
#define TEST_STR3_R "test3"
#define TEST_DOUBLE_R 50.4
#define TEST_INT32_R 40
#define TEST_UINT16_R 30
#define TEST_METHOD_SR 5

#define TEST_STR1_W "apple"
#define TEST_STR2_W "banana"
#define TEST_STR3_W "mango"
#define TEST_DOUBLE_W 33.9
#define TEST_INT32_W 44
#define TEST_UINT16_W 77

#define PRINT(str) std::cout<<str<<std::endl
#define PRINT_ARG(str, arg) std::cout<<str<<" "<<arg<<std::endl

static char ipAddress[128];
static char endpointUri[512];
static EdgeEndPointInfo *epInfo = NULL;
static EdgeConfigure *config = NULL;

static uint8_t supportedApplicationTypes = EDGE_APPLICATIONTYPE_SERVER | EDGE_APPLICATIONTYPE_DISCOVERYSERVER |
    EDGE_APPLICATIONTYPE_CLIENTANDSERVER;

static bool startServerFlag = false;
static bool startClientFlag = false;
static bool readNodeFlag = true;
static bool browseNodeFlag = false;
static bool methodCallFlag = false;

static char node_arr[10][25] =
{ "{2;S;v=12}String1", "{2;S;v=12}String2", "{2;S;v=12}String3", "{2;S;v=11}Double", "{2;S;v=6}Int32",
        "{2;S;v=5}UInt16", "{2;S;v=15}ByteString", "{2;S;v=3}Byte", "Error", "{2;S;v=14}Guid" };

static int method_arr[5] =
{ 105, 205, 305, 405, 505 };

typedef struct BrowseNextData
{
    EdgeBrowseParameter browseParam;
    int count;
    int last_used;
    EdgeContinuationPoint *cp; // Continuation point List. Size of list = last_used.
    EdgeNodeId **srcNodeId; // Id of source node of every continuation point. Size of list = last_used.
} BrowseNextData;

BrowseNextData *browseNextData = NULL;

extern "C"
{

    static void startClient(char *addr, int port, char *securityPolicyUri);

    static void response_msg_cb(EdgeMessage *data)
    {
        if (data->type == GENERAL_RESPONSE)
        {
            int len = data->responseLength;
            int idx = 0;
            for (idx = 0; idx < len; idx++)
            {
                if (data->responses[idx]->message != NULL)
                {
                    if (data->command == CMD_READ || data->command == CMD_METHOD)
                    {
                        if (data->responses[idx]->message->isArray)
                        {
                            // Handle Output array
                            int arrayLen = data->responses[idx]->message->arrayLength;
                            if (data->responses[idx]->type == Boolean)
                            {
                                /* Handle Boolean output array */
                                PRINT_ARG("Boolean output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT_ARG("  ",
                                            ((bool * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == Byte)
                            {
                                /* Handle Byte output array */
                                PRINT_ARG("Byte output array length ::", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int8_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == SByte)
                            {
                                /* Handle SByte output array */
                                PRINT_ARG("SByte output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int8_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == Int16)
                            {
                                /* Handle int16 output array */
                                PRINT_ARG("Int16 output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int16_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == UInt16)
                            {
                                /* Handle UInt16 output array */
                                PRINT_ARG("UInt16 output array length ::", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int16_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == Int32)
                            {
                                /* Handle Int32 output array */
                                PRINT_ARG("Int32 output array length :: ", arrayLen);
                                if (methodCallFlag)
                                    EXPECT_EQ(arrayLen, 5);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int32_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                    if (methodCallFlag)
                                        EXPECT_EQ(
                                                ((int32_t * ) data->responses[idx]->message->value)[arrayIdx],
                                                method_arr[arrayIdx]);
                                }
                                methodCallFlag = false;
                            }
                            else if (data->responses[idx]->type == UInt32)
                            {
                                /* Handle UInt32 output array */
                                PRINT_ARG("UInt32 output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int32_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == Int64)
                            {
                                /* Handle Int64 output array */
                                PRINT_ARG("Int64 output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((long int * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == UInt64)
                            {
                                /* Handle UInt64 output array */
                                PRINT_ARG("UInt64 output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((long int * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == Float)
                            {
                                /* Handle Float output array */
                                PRINT_ARG("Float output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((float * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == Double)
                            {
                                /* Handle Double output array */
                                PRINT_ARG("Double output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((double * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == String
                                    || data->responses[idx]->type == ByteString
                                    || data->responses[idx]->type == Guid)
                            {
                                /* Handle String/ByteString/Guid output array */
                                PRINT_ARG("String/ByteString/Guid output array length :: ",
                                        arrayLen);
                                char **values = ((char **) data->responses[idx]->message->value);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(values[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == DateTime)
                            {
                                /* Handle DateTime output array */
                                PRINT_ARG("DateTime output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int64_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                        }
                        else
                        {
                            if (data->responses[idx]->type == Boolean)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        *((int * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == Byte)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        *((int8_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == SByte)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        *((int8_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == ByteString)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        (char * )data->responses[idx]->message->value);
                            else if (data->responses[idx]->type == DateTime)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        *((int64_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == Double)
                            {
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        *((double * )data->responses[idx]->message->value));
                                double temp = *((double *) data->responses[idx]->message->value);
                                if (readNodeFlag)
                                    EXPECT_EQ(temp, TEST_DOUBLE_R);
                                else if (methodCallFlag)
                                {
                                    EXPECT_EQ(temp, TEST_METHOD_SR);
                                    methodCallFlag = false;
                                }
                                else
                                    EXPECT_EQ(temp, TEST_DOUBLE_W);
                            }
                            else if (data->responses[idx]->type == Float)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        *((float * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == Int16)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        *((int16_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == UInt16)
                            {
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        *((int16_t * )data->responses[idx]->message->value));

                                int16_t temp = *((int16_t *) data->responses[idx]->message->value);
                                if (readNodeFlag)
                                    EXPECT_EQ(temp, TEST_UINT16_R);
                                else
                                    EXPECT_EQ(temp, TEST_UINT16_W);
                            }
                            else if (data->responses[idx]->type == Int32)
                            {
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        *((int32_t * )data->responses[idx]->message->value));

                                int temp = *((int *) data->responses[idx]->message->value);
                                if (readNodeFlag)
                                    EXPECT_EQ(temp, TEST_INT32_R);
                                else
                                    EXPECT_EQ(temp, TEST_INT32_W);
                            }
                            else if (data->responses[idx]->type == UInt32)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        *((int32_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == Int64)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        *((int64_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == UInt64)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        *((int64_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == String)
                            {
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        (char * )data->responses[idx]->message->value);

                                char *temp = ((char *) data->responses[idx]->message->value);
                                if (readNodeFlag)
                                    EXPECT_EQ(
                                            (strcmp(temp, TEST_STR1_R) && strcmp(temp, TEST_STR2_R) && strcmp(temp, TEST_STR3_R)),
                                            0);
                                else
                                    EXPECT_EQ(
                                            (strcmp(temp, TEST_STR1_W) && strcmp(temp, TEST_STR2_W) && strcmp(temp, TEST_STR3_W)),
                                            0);
                            }
                        }
                    }
                    else if (data->command == CMD_WRITE)
                    {
                        EXPECT_EQ(strcmp("Good", (char * ) data->responses[idx]->message->value),
                                0);
                        PRINT_ARG("[Application response Callback] Write response :: ",
                                (char * ) data->responses[idx]->message->value);
                    }
                }
                PRINT("=========");
            }
        }
    }

    static void monitored_msg_cb(EdgeMessage *data)
    {
        if (data->type == REPORT)
        {
            PRINT("[Application response Callback] Monitored Item Response received");
            int len = data->responseLength;
            int idx = 0;
            for (idx = 0; idx < len; idx++)
            {
                if (data->responses[idx]->message != NULL)
                {
                    if (data->responses[idx]->type == Int16)
                        PRINT_ARG(
                                "[MonitoredItem DataChange callback] Monitored Change Value read from node ===>> ",
                                *((int * )data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == UInt16)
                        PRINT_ARG(
                                "[MonitoredItem DataChange callback] Monitored Change Value read from node ===>> ",
                                *((int * )data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == Int32)
                        PRINT_ARG(
                                "[MonitoredItem DataChange callback] Monitored Change Value read from node ===>>  ",
                                *((int * )data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == UInt32)
                        PRINT_ARG(
                                "[MonitoredItem DataChange callbackk] Monitored Change Value read from node ===>> ",
                                *((int * )data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == Int64)
                        PRINT_ARG(
                                "[MonitoredItem DataChange callbackk] Monitored Change Value read from node ===>>  ",
                                *((long * )data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == UInt64)
                        PRINT_ARG(
                                "[MonitoredItem DataChange callbackk] Monitored Change Value read from node ===>>  ",
                                *((long * )data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == Float)
                        PRINT_ARG(
                                "[MonitoredItem DataChange callback] Monitored Change Value read from node  ===>>  ",
                                *((float * )data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == Double)
                        PRINT_ARG(
                                "[MonitoredItem DataChange callback] Monitored Change Value read from node  ===>>  ",
                                *((double * )data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == String)
                        PRINT_ARG(
                                "[MonitoredItem DataChange callback] Monitored Change Value read from node ===>>  ",
                                ((char * )data->responses[idx]->message->value));
                }
            }
        }
    }

    static void error_msg_cb(EdgeMessage *data)
    {
        PRINT_ARG("[error_msg_cb] EdgeStatusCode: ", data->result->code);
    }
    static void browse_msg_cb (EdgeMessage *data)
    {
        browseNodeFlag = true;
        if (data->browseResult)
        {
            //PRINT_ARG("\n", (unsigned char *)data->responses[0]->message->value);
        }
        else
        {
            if (data->cpList && data->cpList->count > 0)
            {
                // Do somethings
            }
        }
    }

    /* status callbacks */
    static void status_start_cb(EdgeEndPointInfo *epInfo, EdgeStatusCode status)
    {
        if (status == STATUS_SERVER_STARTED)
        {
            PRINT("\n[Application Callback] Server started");
            startServerFlag = true;
        }
        if (status == STATUS_CLIENT_STARTED)
        {
            PRINT("[Application Callback] Client connected\n");
            startClientFlag = true;
        }
    }

    static void status_stop_cb(EdgeEndPointInfo *epInfo, EdgeStatusCode status)
    {
        if (status == STATUS_STOP_SERVER)
        {
            PRINT("\n[Application Callback] Server stopped ");
            startServerFlag = false;
        }
        if (status == STATUS_STOP_CLIENT)
        {
            PRINT("[Application Callback] Client disconnected \n\n");
            startClientFlag = false;
        }
    }

    static void status_network_cb(EdgeEndPointInfo *epInfo, EdgeStatusCode status)
    {

    }

    /* discovery callback */
    static void endpoint_found_cb(EdgeDevice *device)
    {
        if (device)
        {
            int num_endpoints = device->num_endpoints;
            int idx = 0;
            for (idx = 0; idx < num_endpoints; idx++)
                startClient(device->address, device->port,
                        device->endpointsInfo[idx]->securityPolicyUri);
        }
    }

    static void device_found_cb(EdgeDevice *device)
    {

    }
}

static void square_method(int inpSize, void **input, int outSize, void **output)
{
    double *inp = (double *) input[0];
    double *sq = (double *) EdgeMalloc(sizeof(double));
    *sq = (*inp) * (*inp);
    output[0] = (void *) sq;
}

static void configureCallbacks()
{
    PRINT("-----INITIALIZING CALLBACKS-----");

    EXPECT_EQ(NULL == config, true);
    config = (EdgeConfigure *) EdgeMalloc(sizeof(EdgeConfigure));
    EXPECT_EQ(NULL == config, false);

    config->recvCallback = (ReceivedMessageCallback *) EdgeMalloc(sizeof(ReceivedMessageCallback));
    EXPECT_EQ(NULL == config->recvCallback, false);

    config->recvCallback->resp_msg_cb = response_msg_cb;
    config->recvCallback->monitored_msg_cb = monitored_msg_cb;
    config->recvCallback->error_msg_cb = error_msg_cb;
    config->recvCallback->browse_msg_cb = browse_msg_cb;

    config->statusCallback = (StatusCallback *) EdgeMalloc(sizeof(StatusCallback));
    EXPECT_EQ(NULL == config->statusCallback, false);

    config->statusCallback->start_cb = status_start_cb;
    config->statusCallback->stop_cb = status_stop_cb;
    config->statusCallback->network_cb = status_network_cb;

    config->discoveryCallback = (DiscoveryCallback *) EdgeMalloc(sizeof(DiscoveryCallback));
    EXPECT_EQ(NULL == config->discoveryCallback, false);

    config->discoveryCallback->endpoint_found_cb = endpoint_found_cb;
    config->discoveryCallback->device_found_cb = device_found_cb;

    config->supportedApplicationTypes = supportedApplicationTypes;

    configure(config);
}

static void cleanCallbacks()
{
    PRINT("-----CLEANING CALLBACKS-----");

    if (config->recvCallback != NULL)
    {
        free(config->recvCallback);
        config->recvCallback = NULL;
    }

    if (config->statusCallback != NULL)
    {
        free(config->statusCallback);
        config->statusCallback = NULL;
    }

    if (config->discoveryCallback != NULL)
    {
        free(config->discoveryCallback);
        config->discoveryCallback = NULL;
    }

    if (config != NULL)
    {
        free(config);
        config = NULL;
    }
}

static void deleteMessage(EdgeMessage *msg, EdgeEndPointInfo *ep)
{
    if (msg != NULL)
    {
        free(msg);
        msg = NULL;
    }

    if (ep != NULL)
    {
        free(ep);
        ep = NULL;
    }
}

static void subscribeAndModifyNodes()
{
    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;

    EdgeRequest **requests = (EdgeRequest **) malloc(sizeof(EdgeRequest *) * 2);
    EdgeSubRequest *subReq = (EdgeSubRequest *) EdgeMalloc(sizeof(EdgeSubRequest));
    subReq->subType = Edge_Create_Sub;
    subReq->samplingInterval = 1000.0;
    subReq->publishingInterval = 0.0;
    subReq->maxKeepAliveCount =
            (1 > (int) ((10000.0 / subReq->publishingInterval))) ?
                    1 : (int) (10000.0 / subReq->publishingInterval);
    subReq->lifetimeCount = 10000; //subReq->maxKeepAliveCount * 6;
    subReq->maxNotificationsPerPublish = 1;
    subReq->publishingEnabled = true;
    subReq->priority = 0;
    subReq->queueSize = 50;

    EdgeNodeInfo *nodeInfo1 = createEdgeNodeInfo(node_arr[1]);
    EdgeNodeInfo *nodeInfo2 = createEdgeNodeInfo(node_arr[0]);
    requests[0] = (EdgeRequest *) EdgeMalloc(sizeof(EdgeRequest));
    requests[0]->nodeInfo = nodeInfo1;
    requests[0]->subMsg = subReq;
    requests[1] = (EdgeRequest *) EdgeMalloc(sizeof(EdgeRequest));
    requests[1]->nodeInfo = nodeInfo2;
    requests[1]->subMsg = subReq;

    EdgeMessage *msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_SUB;
    msg->type = SEND_REQUESTS;
    msg->requests = requests;
    msg->requestLength = 2;

    EdgeResult result = handleSubscription(msg);
    EXPECT_EQ(result.code, STATUS_OK);

    sleep(3);

    free(requests[0]->nodeInfo);
    requests[0]->nodeInfo = NULL;
    free(requests[1]->nodeInfo);
    requests[1]->nodeInfo = NULL;
    free(requests[0]);
    requests[0] = NULL;
    free(requests[1]);
    requests[1] = NULL;
    free(requests);
    requests = NULL;
    free(subReq);
    subReq = NULL;
    free(msg);
    msg = NULL;
    free(ep);
    ep = NULL;

    //Deleting Subscription - node_arr[0]
    EdgeEndPointInfo *epDel1 = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    epDel1->endpointUri = endpointUri;

    EdgeSubRequest *subReqDel1 = (EdgeSubRequest *) EdgeMalloc(sizeof(EdgeSubRequest));
    subReqDel1->subType = Edge_Delete_Sub;

    EdgeNodeInfo *nodeInfoDel1 = createEdgeNodeInfo(node_arr[0]);

    EdgeRequest *requestDel1 = (EdgeRequest *) EdgeMalloc(sizeof(EdgeRequest));
    requestDel1->nodeInfo = nodeInfoDel1;
    requestDel1->subMsg = subReqDel1;

    EdgeMessage *msgDel1 = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msgDel1->endpointInfo = epDel1;
    msgDel1->command = CMD_SUB;
    msgDel1->request = requestDel1;

    result = handleSubscription(msgDel1);
    EXPECT_EQ(result.code, STATUS_OK);

    free(subReqDel1);
    subReqDel1 = NULL;
    free(nodeInfoDel1);
    nodeInfoDel1 = NULL;
    free(requestDel1);
    requestDel1 = NULL;
    deleteMessage(msgDel1, epDel1);

    sleep(2);

    EdgeEndPointInfo *epModify = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    epModify->endpointUri = endpointUri;

    EdgeSubRequest *subReqMod = (EdgeSubRequest *) EdgeMalloc(sizeof(EdgeSubRequest));
    subReqMod->subType = Edge_Modify_Sub;
    subReqMod->samplingInterval = 3000.0;
    subReqMod->publishingInterval = 0.0;
    subReqMod->maxKeepAliveCount =
            (1 > (int) ((10000.0 / subReqMod->publishingInterval))) ?
                    1 : (int) (10000.0 / subReqMod->publishingInterval);
    EDGE_LOG_V(TAG, "keepalive count :: %d\n", subReqMod->maxKeepAliveCount);
    subReqMod->lifetimeCount = 10000; //subReq->maxKeepAliveCount * 6;
    EDGE_LOG_V(TAG, "lifetimecount :: %d\n", subReqMod->lifetimeCount);
    subReqMod->maxNotificationsPerPublish = 1;
    subReqMod->publishingEnabled = true;
    subReqMod->priority = 0;
    subReqMod->queueSize = 50;

    EdgeNodeInfo *nodeInfoMod = createEdgeNodeInfo(node_arr[3]);

    EdgeRequest *requestMod = (EdgeRequest *) EdgeMalloc(sizeof(EdgeRequest));
    requestMod->nodeInfo = nodeInfoMod;
    requestMod->subMsg = subReqMod;

    EdgeMessage *msgMod = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msgMod->endpointInfo = epModify;
    msgMod->command = CMD_SUB;
    msgMod->request = requestMod;

    result = handleSubscription(msgMod);
    EXPECT_NE(result.code, STATUS_OK);

    destroyEdgeNodeInfo(requestMod->nodeInfo);
    requestMod->nodeInfo = createEdgeNodeInfo(node_arr[1]);

    result = handleSubscription(msgMod);
    EXPECT_EQ(result.code, STATUS_OK);
    sleep(2);

    free(nodeInfoMod);
    nodeInfoMod = NULL;
    free(subReqMod);
    subReqMod = NULL;
    free(requestMod);
    requestMod = NULL;
    deleteMessage(msgMod, epModify);

    EdgeEndPointInfo *epRepub = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    epRepub->endpointUri = endpointUri;

    EdgeSubRequest *subReqRepub = (EdgeSubRequest *) EdgeMalloc(sizeof(EdgeSubRequest));
    subReqRepub->subType = Edge_Republish_Sub;

    EdgeNodeInfo *nodeInfoRepub = createEdgeNodeInfo(node_arr[1]);

    EdgeRequest *requestRepub = (EdgeRequest *) EdgeMalloc(sizeof(EdgeRequest));
    requestRepub->nodeInfo = nodeInfoRepub;
    requestRepub->subMsg = subReqRepub;

    EdgeMessage *msgRepub = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msgRepub->endpointInfo = epRepub;
    msgRepub->command = CMD_SUB;
    msgRepub->request = requestRepub;

    result = handleSubscription(msgRepub);
    sleep(2);
    //EXPECT_EQ(result.code, STATUS_OK);

    free(nodeInfoRepub);
    nodeInfoRepub = NULL;
    free(subReqRepub);
    subReqRepub = NULL;
    free(requestRepub);
    requestRepub = NULL;
    deleteMessage(msgRepub, epRepub);
}

static void deleteSub()
{
    //Deleting Subscription - node_arr[1]
    EdgeEndPointInfo *epDel2 = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    epDel2->endpointUri = endpointUri;

    EdgeSubRequest *subReqDel2 = (EdgeSubRequest *) EdgeMalloc(sizeof(EdgeSubRequest));
    subReqDel2->subType = Edge_Delete_Sub;

    EdgeNodeInfo *nodeInfoDel2 = createEdgeNodeInfo(node_arr[1]);

    EdgeRequest *requestDel2 = (EdgeRequest *) EdgeMalloc(sizeof(EdgeRequest));
    requestDel2->nodeInfo = nodeInfoDel2;
    requestDel2->subMsg = subReqDel2;

    EdgeMessage *msgDel2 = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msgDel2->endpointInfo = epDel2;
    msgDel2->command = CMD_SUB;
    msgDel2->request = requestDel2;

    EdgeResult result = handleSubscription(msgDel2);
    EXPECT_EQ(result.code, STATUS_OK);

    free(subReqDel2);
    subReqDel2 = NULL;
    free(nodeInfoDel2);
    nodeInfoDel2 = NULL;
    free(requestDel2);
    requestDel2 = NULL;
    deleteMessage(msgDel2, epDel2);

    sleep(2);

}

static void browseNodes()
{
    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;

    EdgeMessage *msg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    msg->type = SEND_REQUEST;
    msg->endpointInfo = ep;
    msg->command = CMD_BROWSE;

    EdgeNodeInfo *nodeInfo = (EdgeNodeInfo *) EdgeCalloc(1, sizeof(EdgeNodeInfo));
    nodeInfo->nodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    nodeInfo->nodeId->type = INTEGER;
    nodeInfo->nodeId->integerNodeId = RootFolder;
    nodeInfo->nodeId->nameSpace = SYSTEM_NAMESPACE_INDEX;

    EdgeRequest *request = (EdgeRequest *) EdgeCalloc(1, sizeof(EdgeRequest));
    request->nodeInfo = nodeInfo;

    msg->request = request;
    msg->requestLength = 0;
    msg->browseParam = (EdgeBrowseParameter *) EdgeCalloc(1, sizeof(EdgeBrowseParameter));
    msg->browseParam->direction = DIRECTION_FORWARD;
    msg->browseParam->maxReferencesPerNode = 0;

    browseNextData = (BrowseNextData *)EdgeCalloc(1, sizeof(BrowseNextData));
    browseNextData->browseParam = *msg->browseParam;
    browseNextData->count = 1000;
    browseNextData->last_used = -1;
    browseNextData->cp = (EdgeContinuationPoint *)EdgeCalloc(browseNextData->count,
                         sizeof(EdgeContinuationPoint));
    browseNextData->srcNodeId = (EdgeNodeId **)calloc(browseNextData->count, sizeof(EdgeNodeId *));


    EXPECT_EQ(browseNodeFlag, false);
    browseNode(msg);
    EXPECT_EQ(browseNodeFlag, true);
    browseNodeFlag = false;

    free(nodeInfo);
    nodeInfo = NULL;

    nodeInfo = (EdgeNodeInfo *) EdgeCalloc(1, sizeof(EdgeNodeInfo));
    nodeInfo->nodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));

    nodeInfo->nodeId->type = INTEGER;
    nodeInfo->nodeId->integerNodeId = ObjectsFolder;
    nodeInfo->nodeId->nameSpace = SYSTEM_NAMESPACE_INDEX;

    request->nodeInfo = nodeInfo;
    msg->request = request;

    EXPECT_EQ(browseNodeFlag, false);
    browseNode(msg);
    EXPECT_EQ(browseNodeFlag, true);
    browseNodeFlag = false;

    free(nodeInfo);
    nodeInfo = NULL;
    free(ep);
    ep = NULL;
    free(request);
    request = NULL;
    free(msg);
    msg = NULL;
}

static void writeNodes(bool defaultFlag)
{
    PRINT("=============== Writting Nodes ==================");

    void *new_value = NULL;

    int id;
    double d_value = TEST_DOUBLE_R;
    int i_value = TEST_INT32_R;
    int id_value = TEST_UINT16_R;
    if (!defaultFlag)
    {
        d_value = TEST_DOUBLE_W;
        i_value = TEST_INT32_W;
        id_value = TEST_UINT16_W;
    }

    for (id = 0; id < 7; id++)
    {
        PRINT_ARG("*****  Writting the node with browse name  ", node_arr[id]);

        EdgeEndPointInfo *epWrite = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
        epWrite->endpointUri = endpointUri;

        EdgeRequest **requests = (EdgeRequest **) EdgeCalloc(1, sizeof(EdgeRequest *));
        requests[0] = (EdgeRequest *) EdgeCalloc(1, sizeof(EdgeRequest));
        requests[0]->nodeInfo = createEdgeNodeInfo(node_arr[id]);

        EdgeVersatility* message = (EdgeVersatility*) EdgeCalloc(1, sizeof(EdgeVersatility));
        message->isArray = false;

        switch (id)
        {
            case 0:
                requests[0]->type = String;
                if (defaultFlag)
                    message->value = (void *) TEST_STR1_R;
                else
                    message->value = (void *) TEST_STR1_W;
                break;
            case 1:
                requests[0]->type = String;
                if (defaultFlag)
                    message->value = (void *) TEST_STR2_R;
                else
                    message->value = (void *) TEST_STR2_W;
                break;
            case 2:
                requests[0]->type = String;
                if (defaultFlag)
                    message->value = (void *) TEST_STR3_R;
                else
                    message->value = (void *) TEST_STR3_W;
                break;
            case 3:
                requests[0]->type = Double;
                message->value = (void *) &d_value;
                break;
            case 4:
                requests[0]->type = Int32;
                message->value = (void *) &i_value;
                break;
            case 5:
                requests[0]->type = UInt16;
                message->value = (void *) &id_value;
                break;
            case 6:
                requests[0]->type = UInt16;
                message->value = (void *) &id_value;
                break;
        }
        requests[0]->value = message;

        EdgeMessage *msgWrite = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
        msgWrite->endpointInfo = epWrite;
        msgWrite->command = CMD_WRITE;
        msgWrite->type = SEND_REQUESTS;
        msgWrite->requests = requests;
        msgWrite->requestLength = 1;

        writeNode(msgWrite);

        destroyEdgeNodeInfo(requests[0]->nodeInfo);
        EdgeFree(requests[0]->value);
        EdgeFree(requests[0]);
        EdgeFree(requests);
        deleteMessage(msgWrite, epWrite);
    }
}

static void readNodes()
{
    PRINT("=============== Reading Nodes ==================");

    EdgeEndPointInfo *epRead = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    epRead->endpointUri = endpointUri;

    EdgeNodeInfo **nodeInfo = (EdgeNodeInfo **) EdgeCalloc(1, sizeof(EdgeNodeInfo *));

    for (int idx = 0; idx < 10; idx++)
    {
        PRINT_ARG("*****  Reading the node with browse name  ", node_arr[idx]);

        EdgeRequest **requests = (EdgeRequest **) EdgeCalloc(1, sizeof(EdgeRequest *));
        requests[0] = (EdgeRequest *) EdgeCalloc(1, sizeof(EdgeRequest));
        requests[0]->nodeInfo = createEdgeNodeInfo(node_arr[idx]);

        EdgeMessage *msgRead = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
        msgRead->endpointInfo = epRead;
        msgRead->command = CMD_READ;
        msgRead->type = SEND_REQUESTS;
        msgRead->requests = requests;
        msgRead->requestLength = 1;

        readNode(msgRead);

        destroyEdgeRequest(requests[0]);
        EdgeFree(requests);
        EdgeFree(msgRead);
    }

    free(nodeInfo);
    nodeInfo = NULL;

    free(epRead);
    epRead = NULL;
}

static void callClientMethods()
{
    PRINT("=============== Calling Methods ==================");
    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;

    EdgeNodeInfo *nodeInfo = createEdgeNodeInfo("{2;S;v=0}sqrt(x)");

    EdgeMethodRequestParams *methodParams = (EdgeMethodRequestParams *) EdgeMalloc(
            sizeof(EdgeMethodRequestParams));
    methodParams->num_inpArgs = 1;
    methodParams->inpArg = (EdgeArgument **) malloc(
            sizeof(EdgeArgument *) * methodParams->num_inpArgs);
    methodParams->inpArg[0] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
    methodParams->inpArg[0]->argType = Double;
    methodParams->inpArg[0]->valType = SCALAR;
    double d = 25.0;
    methodParams->inpArg[0]->scalarValue = (void *) &d;

    EdgeRequest *request = (EdgeRequest *) EdgeMalloc(sizeof(EdgeRequest));
    request->nodeInfo = nodeInfo;
    request->methodParams = methodParams;

    EdgeMessage *msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_METHOD;
    msg->request = request;

    methodCallFlag = true;

    callMethod(msg);

    for (int i = 0; i < methodParams->num_inpArgs; i++)
    {
        free(methodParams->inpArg[i]);
        methodParams->inpArg[i] = NULL;
    }
    free(methodParams->inpArg);
    methodParams->inpArg = NULL;
    free(methodParams);
    methodParams = NULL;

    free(nodeInfo);
    nodeInfo = NULL;
    free(request);
    request = NULL;
    deleteMessage(msg, ep);

    ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;

    nodeInfo = createEdgeNodeInfo("{2;S;v=0}incrementInc32Array(x,delta)");

    methodParams = (EdgeMethodRequestParams *) EdgeMalloc(sizeof(EdgeMethodRequestParams));
    methodParams->num_inpArgs = 2;
    methodParams->inpArg = (EdgeArgument **) malloc(
            sizeof(EdgeArgument *) * methodParams->num_inpArgs);
    methodParams->inpArg[0] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
    methodParams->inpArg[0]->argType = Int32;
    methodParams->inpArg[0]->valType = ARRAY_1D;
    int32_t array[5] =
    { 100, 200, 300, 400, 500 };
    methodParams->inpArg[0]->arrayData = (void *) array;
    methodParams->inpArg[0]->arrayLength = 5;

    methodParams->inpArg[1] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
    methodParams->inpArg[1]->argType = Int32;
    methodParams->inpArg[1]->valType = SCALAR;
    int delta = 5;
    methodParams->inpArg[1]->scalarValue = (void *) &delta;

    request = (EdgeRequest *) EdgeMalloc(sizeof(EdgeRequest));
    request->nodeInfo = nodeInfo;
    request->methodParams = methodParams;

    msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_METHOD;
    msg->request = request;

    methodCallFlag = true;

    callMethod(msg);

    for (int i = 0; i < methodParams->num_inpArgs; i++)
    {
        free(methodParams->inpArg[i]);
        methodParams->inpArg[i] = NULL;
    }
    free(methodParams->inpArg);
    methodParams->inpArg = NULL;
    free(methodParams);
    methodParams = NULL;
    free(nodeInfo);
    nodeInfo = NULL;
    free(request);
    request = NULL;
    deleteMessage(msg, ep);

}

static void startClient(char *addr, int port, char *securityPolicyUri)
{
    PRINT("                       Client connect            ");
    EdgeEndpointConfig *endpointConfigClient = (EdgeEndpointConfig *) EdgeCalloc(
            1, sizeof(EdgeEndpointConfig));
    EXPECT_EQ(NULL != endpointConfigClient, true);
    endpointConfigClient->bindAddress = ipAddress;
    EXPECT_EQ(strcmp(endpointConfigClient->bindAddress, ipAddress) == 0, true);
    endpointConfigClient->bindPort = port;
    endpointConfigClient->serverName = DEFAULT_SERVER_NAME_VALUE;
    endpointConfigClient->requestTimeout = 60000;
    EXPECT_EQ(endpointConfigClient->requestTimeout, 60000);

    EdgeApplicationConfig *endpointAppConfig = (EdgeApplicationConfig *) EdgeCalloc(
            1, sizeof(EdgeApplicationConfig));
    EXPECT_EQ(NULL != endpointAppConfig, true);
    endpointAppConfig->applicationName = DEFAULT_SERVER_APP_NAME_VALUE;
    endpointAppConfig->applicationUri = DEFAULT_SERVER_APP_URI_VALUE;
    endpointAppConfig->productUri = DEFAULT_PRODUCT_URI_VALUE;

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    EXPECT_EQ(NULL != ep, true);
    ep->endpointUri = endpointUri;
    EXPECT_EQ(strcmp(ep->endpointUri, ipAddress) == 0, true);
    ep->securityPolicyUri = securityPolicyUri;
    //EXPECT_EQ(ep->securityPolicyUri == NULL,  true);
    ep->endpointConfig = endpointConfigClient;
    ep->appConfig = endpointAppConfig;

    EdgeMessage *msg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    EXPECT_EQ(NULL != msg, true);
    msg->endpointInfo = ep;
    msg->command = CMD_START_CLIENT;
    EXPECT_EQ(msg->command, CMD_START_CLIENT);
    msg->type = SEND_REQUEST;
    EXPECT_EQ(msg->type, SEND_REQUEST);

    PRINT("********************** startClient **********************");
    connectClient(ep);

    EXPECT_EQ(startClientFlag, true);

    deleteMessage(msg, ep);

    free(endpointConfigClient);
    endpointConfigClient = NULL;

    free(endpointAppConfig);
    endpointAppConfig = NULL;
}

static void stopClient()
{
    EdgeEndPointInfo *ep_t = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep_t->endpointUri = endpointUri;

    EdgeMessage *msg_t = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg_t->endpointInfo = ep_t;
    msg_t->command = CMD_STOP_CLIENT;

    disconnectClient(ep_t);

    deleteMessage(msg_t, ep_t);
}

class OPC_serverTests: public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        strcpy(ipAddress, LOCALHOST);
        snprintf(endpointUri, sizeof(endpointUri), ENDPOINT_URI, ipAddress);

        epInfo = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));

    }

    virtual void TearDown()
    {
        if (epInfo != NULL)
        {
            free(epInfo);
            epInfo = NULL;
        }

    }

};

class OPC_clientTests: public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        int len = strlen(IPADDRESS);
        strcpy(ipAddress, IPADDRESS);
        ipAddress[len] = '\0';

        strcpy(endpointUri, ipAddress);
        endpointUri[len] = '\0';

        configureCallbacks();
    }

    virtual void TearDown()
    {
        cleanCallbacks();

        startClientFlag = false;
    }

};

//-----------------------------------------------------------------------------
//  Tests
//-----------------------------------------------------------------------------

TEST_F(OPC_serverTests , SetUPServer_P)
{
    EXPECT_EQ(strcmp(ipAddress, LOCALHOST) == 0, true);

    EXPECT_EQ(strcmp(endpointUri, ENDPOINT_URI) == 0, false);

    EXPECT_EQ(strcmp(endpointUri, "opc:tcp://localhost:12686/edge-opc-server") == 0, true);
}

TEST_F(OPC_serverTests , InitializeServer_P)
{
    EXPECT_EQ(NULL == epInfo, false);

    int len = strlen(endpointUri);
    epInfo->endpointUri = (char *) EdgeMalloc(len + 1);
    EXPECT_EQ(NULL == epInfo->endpointUri, false);

    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';

    EXPECT_EQ(strcmp(epInfo->endpointUri, endpointUri) == 0, true);

    if (epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , InitializeServer_N)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char *) EdgeMalloc(len + 1);
    EXPECT_EQ(NULL == epInfo->endpointUri, false);

    //strncpy(epInfo->endpointUri, endpointUri, len);
    //epInfo->endpointUri[len] = '\0';

    EXPECT_EQ(strcmp(epInfo->endpointUri, endpointUri) == 0, false);

    if (epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , ConfigureServer_P)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char *) EdgeMalloc(len + 1);

    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';

    configureCallbacks();

    cleanCallbacks();

    if (epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , StartServer_N)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char *) EdgeMalloc(len + 1);

    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';

    configureCallbacks();

    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    EXPECT_EQ(NULL != endpointConfig, true);

    endpointConfig->bindAddress = ipAddress;
    EXPECT_EQ(strcmp(endpointConfig->bindAddress, ipAddress) == 0, true);

    endpointConfig->bindPort = 12686;
    EXPECT_EQ(endpointConfig->bindPort, 12686);

    endpointConfig->serverName = (char *) DEFAULT_SERVER_NAME_VALUE;
    EXPECT_EQ(strcmp(endpointConfig->serverName, DEFAULT_SERVER_NAME_VALUE) == 0, true);

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1, sizeof(EdgeApplicationConfig));
    EXPECT_EQ(NULL != appConfig, true);

    appConfig->applicationName = (char *) DEFAULT_SERVER_APP_NAME_VALUE;
    EXPECT_EQ(strcmp(appConfig->applicationName, DEFAULT_SERVER_APP_NAME_VALUE) == 0, true);

    appConfig->applicationUri = (char *) DEFAULT_SERVER_URI_VALUE;
    EXPECT_EQ(strcmp(appConfig->applicationUri, DEFAULT_SERVER_URI_VALUE) == 0, true);

    appConfig->productUri = (char *) DEFAULT_PRODUCT_URI_VALUE;
    EXPECT_EQ(strcmp(appConfig->productUri, DEFAULT_PRODUCT_URI_VALUE) == 0, true);

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    EXPECT_EQ(NULL != ep, true);
    ep->endpointUri = endpointUri;
    ASSERT_TRUE(strcmp(ep->endpointUri, "opc:tcp://localhost:12686/edge-opc-server") == 0);
    ep->securityPolicyUri = NULL;
    EXPECT_EQ(ep->securityPolicyUri == NULL, true);
    ep->endpointConfig = endpointConfig;
    ep->appConfig = appConfig;

    EdgeMessage *msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    EXPECT_EQ(NULL != msg, true);
    msg->endpointInfo = ep;

    msg->command = CMD_START_SERVER;
    EXPECT_EQ(msg->command, CMD_START_SERVER);

    msg->type = SEND_REQUEST;
    EXPECT_EQ(msg->type, SEND_REQUEST);

    EXPECT_EQ(startServerFlag, false);

    PRINT("--- START SERVER ----");

    //createServer(ep);
    //::TODO - This case is faling need to put NULL checks
    createServer (NULL);

    EXPECT_EQ(startServerFlag, false);

    deleteMessage(msg, ep);

    cleanCallbacks();

    free(endpointConfig);
    free(appConfig);

    if (epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , StartServer_P)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char *) EdgeMalloc(len + 1);

    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';
    configureCallbacks();

    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    endpointConfig->bindAddress = ipAddress;
    endpointConfig->bindPort = 12686;
    endpointConfig->serverName = (char *) DEFAULT_SERVER_NAME_VALUE;

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1, sizeof(EdgeApplicationConfig));
    appConfig->applicationName = (char *) DEFAULT_SERVER_APP_NAME_VALUE;
    appConfig->applicationUri = (char *) DEFAULT_SERVER_URI_VALUE;
    appConfig->productUri = (char *) DEFAULT_PRODUCT_URI_VALUE;

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->endpointConfig = endpointConfig;
    ep->appConfig = appConfig;

    EdgeMessage *msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;

    msg->type = SEND_REQUEST;

    EXPECT_EQ(startServerFlag, false);

    PRINT("--- START SERVER ----");

    createServer(ep);

    EXPECT_EQ(startServerFlag, true);

    deleteMessage(msg, ep);

    cleanCallbacks();

    free(endpointConfig);
    free(appConfig);

    if (epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , ServerCreateNamespace_P)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char *) EdgeMalloc(len + 1);

    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';

    configureCallbacks();

    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    endpointConfig->bindAddress = ipAddress;
    endpointConfig->bindPort = 12686;
    endpointConfig->serverName = (char *) DEFAULT_SERVER_NAME_VALUE;

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1, sizeof(EdgeApplicationConfig));
    appConfig->applicationName = (char *) DEFAULT_SERVER_APP_NAME_VALUE;
    appConfig->applicationUri = (char *) DEFAULT_SERVER_URI_VALUE;
    appConfig->productUri = (char *) DEFAULT_PRODUCT_URI_VALUE;

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->endpointConfig = endpointConfig;
    ep->appConfig = appConfig;

    EdgeMessage *msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;
    msg->type = SEND_REQUEST;

    PRINT("--- START SERVER ----");

    createServer(ep);

    EXPECT_EQ(startServerFlag, true);

    EdgeResult result = createNamespace(DEFAULT_NAMESPACE_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE);

    EXPECT_EQ(result.code, STATUS_OK);

    deleteMessage(msg, ep);

    cleanCallbacks();

    free(endpointConfig);
    free(appConfig);

    if (epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , ServerAddNodes_P)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char *) EdgeMalloc(len + 1);

    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';

    configureCallbacks();

    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    endpointConfig->bindAddress = ipAddress;
    endpointConfig->bindPort = 12686;
    endpointConfig->serverName = (char *) DEFAULT_SERVER_NAME_VALUE;

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1, sizeof(EdgeApplicationConfig));
    appConfig->applicationName = (char *) DEFAULT_SERVER_APP_NAME_VALUE;
    appConfig->applicationUri = (char *) DEFAULT_SERVER_URI_VALUE;
    appConfig->productUri = (char *) DEFAULT_PRODUCT_URI_VALUE;

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->endpointConfig = endpointConfig;
    ep->appConfig = appConfig;

    EdgeMessage *msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;
    msg->type = SEND_REQUEST;

    PRINT("--- START SERVER ----");

    createServer(ep);

    EXPECT_EQ(startServerFlag, true);

    EdgeNodeItem *item = NULL;

    // VARIABLE NODE with string variant:
    item = createVariableNodeItem("String1", String, (void *)"test1", VARIABLE_NODE);
    EdgeResult result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    deleteNodeItem(item);

    // VARIABLE NODE with UInt16 variant:
    int value = 30;
    item = createVariableNodeItem("UInt16", UInt16, (void *) &value, VARIABLE_NODE);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    deleteNodeItem(item);

    //modify value for this node
    value = 43;
    char *name = "UInt16";
    EdgeVersatility *message = (EdgeVersatility *) EdgeMalloc(sizeof(EdgeVersatility));
    message->value = (void *) &value;
    result = modifyVariableNode(DEFAULT_NAMESPACE_VALUE, name, message);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(message);

    //Array Nodes with double values
    double *data = (double *) EdgeMalloc(sizeof(double) * 2);
    data[0] = 10.2;
    data[1] = 20.2;
    item = createVariableNodeItem("DoubleArray", Double, (void *) data, VARIABLE_NODE);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 2;
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(data);
    deleteNodeItem(item);

    //Array Nodes with ByteString values
    UA_ByteString **dataArray = (UA_ByteString **) malloc(sizeof(UA_ByteString *) * 5);
    dataArray[0] = (UA_ByteString *) EdgeMalloc(sizeof(UA_ByteString));
    *dataArray[0] = UA_BYTESTRING_ALLOC("abcde");
    dataArray[1] = (UA_ByteString *) EdgeMalloc(sizeof(UA_ByteString));
    *dataArray[1] = UA_BYTESTRING_ALLOC("fghij");
    dataArray[2] = (UA_ByteString *) EdgeMalloc(sizeof(UA_ByteString));
    *dataArray[2] = UA_BYTESTRING_ALLOC("klmno");
    dataArray[3] = (UA_ByteString *) EdgeMalloc(sizeof(UA_ByteString));
    *dataArray[3] = UA_BYTESTRING_ALLOC("pqrst");
    dataArray[4] = (UA_ByteString *) EdgeMalloc(sizeof(UA_ByteString));
    *dataArray[4] = UA_BYTESTRING_ALLOC("uvwxyz");
    item = createVariableNodeItem("ByteStringArray", ByteString, (void *) dataArray, VARIABLE_NODE);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 5;
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    deleteNodeItem(item);

    //Array Nodes with String values
    char **data1 = (char **) malloc(sizeof(char *) * 5);
    data1[0] = (char *) EdgeMalloc(10);
    strcpy(data1[0], "apple");
    data1[1] = (char *) EdgeMalloc(10);
    strcpy(data1[1], "ball");
    data1[2] = (char *) EdgeMalloc(10);
    strcpy(data1[2], "cats");
    data1[3] = (char *) EdgeMalloc(10);
    strcpy(data1[3], "dogs");
    data1[4] = (char *) EdgeMalloc(10);
    strcpy(data1[4], "elephant");
    item = createVariableNodeItem("CharArray", String, (void *) data1, VARIABLE_NODE);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 5;
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    deleteNodeItem(item);

    //OBJECT NODE
    EdgeNodeId *edgeNodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    item = createNodeItem("Object1", OBJECT_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);
    deleteNodeItem(item);

    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    edgeNodeId->nodeId = "Object1";
    item = createNodeItem("Object2", OBJECT_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);
    deleteNodeItem(item);

    //OBJECT TYPE NDOE
    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    edgeNodeId->nodeId = NULL; // no source node
    item = createNodeItem("ObjectType1", OBJECT_TYPE_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);
    deleteNodeItem(item);

    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    edgeNodeId->nodeId = "ObjectType1";
    item = createNodeItem("ObjectType2", OBJECT_TYPE_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);
    deleteNodeItem(item);

    //DATA TYPE NODE
    edgeNodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    item = createNodeItem("DataType1", DATA_TYPE_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);
    deleteNodeItem(item);

    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    edgeNodeId->nodeId = "DataType1";
    item = createNodeItem("DataType2", DATA_TYPE_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);
    deleteNodeItem(item);

    //VIEW NODE
    edgeNodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    item = createNodeItem("ViewNode1", VIEW_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);
    deleteNodeItem(item);

    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    edgeNodeId->nodeId = "ViewNode1";
    item = createNodeItem("ViewNode2", VIEW_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);
    deleteNodeItem(item);

    //REFERENCE TYPE NODE
    edgeNodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    item = createNodeItem("ReferenceTypeNode1", REFERENCE_TYPE_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);

    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    edgeNodeId->nodeId = "ReferenceTypeNode1";
    item = createNodeItem("ReferenceTypeNode2", REFERENCE_TYPE_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);

    deleteNodeItem(item);

    //METHOD NODE
    EdgeNodeItem *methodNodeItem = (EdgeNodeItem *) EdgeMalloc(sizeof(EdgeNodeItem));
    methodNodeItem->browseName = "square";
    methodNodeItem->sourceNodeId = NULL;

    EdgeMethod *method = (EdgeMethod *) EdgeMalloc(sizeof(EdgeMethod));
    method->description = "Calculate square";
    method->methodNodeName = "square";
    method->method_fn = square_method;
    method->num_inpArgs = 1;
    method->inpArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method->num_inpArgs);
    for (int idx = 0; idx < method->num_inpArgs; idx++)
    {
        method->inpArg[idx] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
        method->inpArg[idx]->argType = Double;
        method->inpArg[idx]->valType = SCALAR;
    }

    method->num_outArgs = 1;
    method->outArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method->num_outArgs);
    for (int idx = 0; idx < method->num_outArgs; idx++)
    {
        method->outArg[idx] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
        method->outArg[idx]->argType = Double;
        method->outArg[idx]->valType = SCALAR;
    }
    result = createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem, method);
    EXPECT_EQ(result.code, STATUS_OK);


    //REFERENCE NODE
    /*EdgeReference *reference = (EdgeReference *) EdgeMalloc(sizeof(EdgeReference));
    reference->forward = true;
    reference->sourceNamespace = DEFAULT_NAMESPACE_VALUE;
    reference->sourcePath = "ViewNode1";
    reference->targetNamespace = DEFAULT_NAMESPACE_VALUE;
    reference->targetPath = "ObjectType1";
    result = addReference(reference);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(reference);*/

    deleteMessage(msg, ep);

    cleanCallbacks();

    free(endpointConfig);
    free(appConfig);

    if (epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , StartStopServer_P)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char *) EdgeMalloc(len + 1);

    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';

    configureCallbacks();

    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    endpointConfig->bindAddress = ipAddress;
    endpointConfig->bindPort = 12686;
    endpointConfig->serverName = (char *) DEFAULT_SERVER_NAME_VALUE;

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1, sizeof(EdgeApplicationConfig));
    appConfig->applicationName = (char *) DEFAULT_SERVER_APP_NAME_VALUE;
    appConfig->applicationUri = (char *) DEFAULT_SERVER_URI_VALUE;
    appConfig->productUri = (char *) DEFAULT_PRODUCT_URI_VALUE;

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->endpointConfig = endpointConfig;
    ep->appConfig = appConfig;

    EdgeMessage *msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;
    msg->type = SEND_REQUEST;

    PRINT("--- START SERVER ----");

    createServer(ep);

    ASSERT_TRUE(startServerFlag);

    deleteMessage(msg, ep);

    PRINT("=============STOP SERVER===============");

    EdgeEndPointInfo *epStop = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    epStop->endpointUri = endpointUri;

    EdgeMessage *msgStop = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    ASSERT_TRUE(NULL != msgStop);
    msgStop->endpointInfo = epStop;
    msgStop->command = CMD_STOP_SERVER;
    EXPECT_EQ(msgStop->command, CMD_STOP_SERVER);
    msgStop->type = SEND_REQUEST;
    EXPECT_EQ(msgStop->type, SEND_REQUEST);

    closeServer(epStop);

    ASSERT_FALSE(startServerFlag);

    deleteMessage(msgStop, epStop);

    cleanCallbacks();

    free(endpointConfig);
    free(appConfig);

    if (epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , StartServerNew_P)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char *) EdgeMalloc(len + 1);

    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';
    configureCallbacks();

    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    endpointConfig->bindAddress = ipAddress;
    endpointConfig->bindPort = 12686;
    endpointConfig->serverName = (char *) DEFAULT_SERVER_NAME_VALUE;

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1, sizeof(EdgeApplicationConfig));
    appConfig->applicationName = (char *) DEFAULT_SERVER_APP_NAME_VALUE;
    appConfig->applicationUri = (char *) DEFAULT_SERVER_URI_VALUE;
    appConfig->productUri = (char *) DEFAULT_PRODUCT_URI_VALUE;

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->endpointConfig = endpointConfig;
    ep->appConfig = appConfig;

    EdgeMessage *msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;
    msg->type = SEND_REQUEST;

    PRINT("--- START SERVER ----");

    createServer(ep);

    ASSERT_TRUE(startServerFlag);

    deleteMessage(msg, ep);

    cleanCallbacks();

    free(endpointConfig);
    free(appConfig);

    if (epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_clientTests , ConfigureClient_P)
{
    EXPECT_EQ(NULL == config, false);

    EXPECT_EQ(NULL == config->recvCallback, false);

    EXPECT_EQ(NULL == config->statusCallback, false);

    EXPECT_EQ(NULL == config->discoveryCallback, false);
}

TEST_F(OPC_clientTests , InitializeClient_P)
{
    EXPECT_EQ(strcmp(ipAddress, IPADDRESS) == 0, true);

    EXPECT_EQ(strcmp(endpointUri, ENDPOINT_URI) == 0, false);

    EXPECT_EQ(strcmp(endpointUri, ipAddress) == 0, true);
}

TEST_F(OPC_clientTests , StartClient_P)
{
    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    EXPECT_EQ(NULL != endpointConfig, true);
    endpointConfig->serverName = (char *) DEFAULT_SERVER_NAME_VALUE;
    EXPECT_EQ(strcmp(endpointConfig->serverName, DEFAULT_SERVER_NAME_VALUE) == 0, true);

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1, sizeof(EdgeApplicationConfig));
    EXPECT_EQ(NULL != appConfig, true);
    appConfig->applicationName = (char *) DEFAULT_SERVER_APP_NAME_VALUE;
    EXPECT_EQ(strcmp(appConfig->applicationName, DEFAULT_SERVER_APP_NAME_VALUE) == 0, true);
    appConfig->applicationUri = (char *) DEFAULT_SERVER_APP_URI_VALUE;
    EXPECT_EQ(strcmp(appConfig->applicationUri, DEFAULT_SERVER_APP_URI_VALUE) == 0, true);
    appConfig->productUri = (char *) DEFAULT_PRODUCT_URI_VALUE;
    EXPECT_EQ(strcmp(appConfig->productUri, DEFAULT_PRODUCT_URI_VALUE) == 0, true);

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    EXPECT_EQ(NULL != ep, true);
    ep->endpointUri = endpointUri;
    EXPECT_EQ(strcmp(ep->endpointUri, ipAddress) == 0, true);
    ep->endpointConfig = endpointConfig;
    ep->appConfig = appConfig;

    EdgeMessage *msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    EXPECT_EQ(NULL != msg, true);
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;
    EXPECT_EQ(msg->command, CMD_START_SERVER);
    msg->type = SEND_REQUEST;
    EXPECT_EQ(msg->type, SEND_REQUEST);

    PRINT("=============== startClient ==================");
    EXPECT_EQ(startClientFlag, false);

    getEndpointInfo(ep);

    EXPECT_EQ(startClientFlag, true);

    deleteMessage(msg, ep);

    stopClient();

    EXPECT_EQ(startClientFlag, false);

    free(endpointConfig);
    free(appConfig);
}

TEST_F(OPC_clientTests , ClientBrowse_P)
{
    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    endpointConfig->serverName = (char *) DEFAULT_SERVER_NAME_VALUE;

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1, sizeof(EdgeApplicationConfig));
    appConfig->applicationName = (char *) DEFAULT_SERVER_APP_NAME_VALUE;
    appConfig->applicationUri = (char *) DEFAULT_SERVER_APP_URI_VALUE;
    appConfig->productUri = (char *) DEFAULT_PRODUCT_URI_VALUE;

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->endpointConfig = endpointConfig;
    ep->appConfig = appConfig;

    EdgeMessage *msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;
    msg->type = SEND_REQUEST;

    PRINT("=============== startClient ==================");

    getEndpointInfo(ep);

    EXPECT_EQ(startClientFlag, true);

    deleteMessage(msg, ep);

    browseNodes();

    EdgeEndPointInfo *ep_t = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep_t->endpointUri = endpointUri;

    EdgeMessage *msg_t = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg_t->endpointInfo = ep_t;
    msg_t->command = CMD_STOP_CLIENT;

    disconnectClient(ep_t);

    EXPECT_EQ(startClientFlag, false);

    deleteMessage(msg_t, ep_t);

    free(endpointConfig);
    free(appConfig);
}

TEST_F(OPC_clientTests , ClientRead_P)
{
    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    endpointConfig->serverName = (char *) DEFAULT_SERVER_NAME_VALUE;

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1, sizeof(EdgeApplicationConfig));
    appConfig->applicationName = (char *) DEFAULT_SERVER_APP_NAME_VALUE;
    appConfig->applicationUri = (char *) DEFAULT_SERVER_APP_URI_VALUE;
    appConfig->productUri = (char *) DEFAULT_PRODUCT_URI_VALUE;

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->endpointConfig = endpointConfig;
    ep->appConfig = appConfig;

    EdgeMessage *msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;
    msg->type = SEND_REQUEST;

    PRINT("=============== startClient ==================");

    getEndpointInfo(ep);

    EXPECT_EQ(startClientFlag, true);

    deleteMessage(msg, ep);

    readNodes();

    EdgeEndPointInfo *ep_t = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep_t->endpointUri = endpointUri;

    EdgeMessage *msg_t = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg_t->endpointInfo = ep_t;
    msg_t->command = CMD_STOP_CLIENT;

    disconnectClient(ep_t);

    EXPECT_EQ(startClientFlag, false);

    deleteMessage(msg_t, ep_t);

    free(endpointConfig);
    free(appConfig);
}

TEST_F(OPC_clientTests , ClientWrite_P)
{
    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    endpointConfig->serverName = (char *) DEFAULT_SERVER_NAME_VALUE;

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1, sizeof(EdgeApplicationConfig));
    appConfig->applicationName = (char *) DEFAULT_SERVER_APP_NAME_VALUE;
    appConfig->applicationUri = (char *) DEFAULT_SERVER_APP_URI_VALUE;
    appConfig->productUri = (char *) DEFAULT_PRODUCT_URI_VALUE;

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->endpointConfig = endpointConfig;
    ep->appConfig = appConfig;

    EdgeMessage *msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;
    msg->type = SEND_REQUEST;

    PRINT("=============== startClient ==================");

    getEndpointInfo(ep);

    EXPECT_EQ(startClientFlag, true);

    deleteMessage(msg, ep);

    writeNodes(false);

    // Verify written nodes values
    readNodeFlag = false;
    readNodes();

    writeNodes(true);

    EdgeEndPointInfo *ep_t = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep_t->endpointUri = endpointUri;

    EdgeMessage *msg_t = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg_t->endpointInfo = ep_t;
    msg_t->command = CMD_STOP_CLIENT;

    disconnectClient(ep_t);

    EXPECT_EQ(startClientFlag, false);

    deleteMessage(msg_t, ep_t);

    free(endpointConfig);
    free(appConfig);
}

TEST_F(OPC_clientTests , ClientMethodCall_P)
{
    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    endpointConfig->serverName = (char *) DEFAULT_SERVER_NAME_VALUE;

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1, sizeof(EdgeApplicationConfig));
    appConfig->applicationName = (char *) DEFAULT_SERVER_APP_NAME_VALUE;
    appConfig->applicationUri = (char *) DEFAULT_SERVER_APP_URI_VALUE;
    appConfig->productUri = (char *) DEFAULT_PRODUCT_URI_VALUE;

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->endpointConfig = endpointConfig;
    ep->appConfig = appConfig;

    EdgeMessage *msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;
    msg->type = SEND_REQUEST;

    PRINT("=============== startClient ==================");

    getEndpointInfo(ep);

    EXPECT_EQ(startClientFlag, true);

    deleteMessage(msg, ep);

    callClientMethods();

    EdgeEndPointInfo *ep_t = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep_t->endpointUri = endpointUri;

    EdgeMessage *msg_t = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg_t->endpointInfo = ep_t;
    msg_t->command = CMD_STOP_CLIENT;

    disconnectClient(ep_t);

    EXPECT_EQ(startClientFlag, false);

    deleteMessage(msg_t, ep_t);

    free(endpointConfig);
    free(appConfig);
}

TEST_F(OPC_clientTests , ClientSubscribe_P)
{
    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    endpointConfig->serverName = (char *) DEFAULT_SERVER_NAME_VALUE;

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1, sizeof(EdgeApplicationConfig));
    appConfig->applicationName = (char *) DEFAULT_SERVER_APP_NAME_VALUE;
    appConfig->applicationUri = (char *) DEFAULT_SERVER_APP_URI_VALUE;
    appConfig->productUri = (char *) DEFAULT_PRODUCT_URI_VALUE;

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->endpointConfig = endpointConfig;
    ep->appConfig = appConfig;

    EdgeMessage *msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;
    msg->type = SEND_REQUEST;

    PRINT("=============== startClient ==================");

    getEndpointInfo(ep);

    EXPECT_EQ(startClientFlag, true);

    deleteMessage(msg, ep);

    subscribeAndModifyNodes();

    //deleteSub();

    EdgeEndPointInfo *ep_t = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep_t->endpointUri = endpointUri;

    EdgeMessage *msg_t = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg_t->endpointInfo = ep_t;
    msg_t->command = CMD_STOP_CLIENT;

    disconnectClient(ep_t);

    EXPECT_EQ(startClientFlag, false);

    deleteMessage(msg_t, ep_t);

    free(endpointConfig);
    free(appConfig);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
