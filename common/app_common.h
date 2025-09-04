#pragma once

#include <stdint.h>

// #define NUM_WAREHOUSES                     1
#define MAX_WAREHOUSE_ID                   2600
#define NUM_DISTRICTS_PER_WAREHOUSE        10
#define NUM_CUSTOMERS_PER_DISTRICT         3000

#define MAX_NAME                           10
#define MIN_STREET                         10
#define MAX_STREET                         20
#define MIN_CITY                           10
#define MAX_CITY                           20
#define STATE                              2
#define ZIP                                9
#define MAX_FIRST                          16
#define MIDDLE                             2
#define MAX_LAST                           16
#define PHONE                              16
#define DATETIME_SIZE                      14
#define CREDIT                             2
#define MAX_CUSTOMER_DATA                  500
#define MAX_HISTORY_DATA                   24

#define WAREHOUSE_INITIAL_YTD              300000.00
#define DISTRICT_INITIAL_YTD               30000.00
#define CUSTOMER_INITIAL_CREDIT_LIM        50000.00
#define CUSTOMER_INITIAL_BALANCE           -10.00
#define CUSTOMER_INITIAL_YTD_PAYMENT       10.00
#define CUSTOMER_INITIAL_PAYMENT_CNT       1
#define CUSTOMER_INITIAL_DELIVERY_CNT      0

#define MIN_PAYMENT                        1.00f
#define MAX_PAYMENT                        5000.00f
// #define ORDER_MIN_OL_COUNT                 5
#define ORDER_MIN_OL_COUNT                 5
// #define ORDER_MAX_OL_COUNT                 5
#define ORDER_MAX_OL_COUNT                 15
#define MAX_OL_QUANTITY                    10

#define NUM_ITEMS                          100000

#define ITEM_MAX_NAME                      24
#define ITEM_MAX_DATA                      50

#define NUM_STOCK_PER_WAREHOUSE            100000
#define STOCK_MIN_QUANTITY                 10
#define STOCK_MAX_QUANTITY                 10000

#define ORDER_MIN_CARRIER_ID               1
#define ORDER_MAX_CARRIER_ID               10
#define ORDER_INITIAL_ALL_LOCAL            1

#define ORDER_LINE_MIN_I_ID                1
#define ORDER_LINE_MAX_I_ID                NUM_ITEMS
#define ORDER_LINE_INITIAL_QUANTITY        5
#define ORDER_LINE_MIN_AMOUNT              0.01f
#define ORDER_LINE_MAX_AMOUNT              9999.99f

#define NEW_ORDER_INITIAL_NUM_PER_DISTRICT 900

#define PAYMENT                            1
#define PAYMENT_REMOTE                     2
#define NEW_ORDER                          3
#define ORDER_STATUS                       4

typedef struct new_order_item
{
    uint32_t i_id;
    uint16_t ol_supply_w_id;
    uint16_t ol_quantity;
} new_order_item_t;
