#pragma once

#include <stdint.h>

#include "../common/app_common.h"
#include "util.h"

typedef struct warehouse
{
    char w_name[MAX_NAME + 1];
    char w_street_1[MAX_STREET + 1];
    char w_street_2[MAX_STREET + 1];
    char w_city[MAX_CITY + 1];
    char w_state[STATE + 1];
    char w_zip[ZIP + 1];
    float w_tax;
    float w_ytd;
    uint16_t w_id;
} warehouse_t;

typedef struct district
{
    char d_name[MAX_NAME + 1];
    char d_street_1[MAX_STREET + 1];
    char d_street_2[MAX_STREET + 1];
    char d_city[MAX_CITY + 1];
    char d_state[STATE + 1];
    char d_zip[ZIP + 1];
    float d_tax;
    float d_ytd;
    uint32_t d_next_o_id;
    uint16_t d_w_id;
    uint8_t d_id;
} district_t;

typedef struct customer
{
    char c_first[MAX_FIRST + 1];
    char c_middle[MIDDLE + 1];
    char c_last[MAX_LAST + 1];
    char c_street_1[MAX_STREET + 1];
    char c_street_2[MAX_STREET + 1];
    char c_city[MAX_CITY + 1];
    char c_state[STATE + 1];
    char c_zip[ZIP + 1];
    char c_phone[PHONE + 1];
    char c_since[DATETIME_SIZE + 1];
    char c_credit[CREDIT + 1];
    float c_credit_lim;
    float c_discount;
    float c_balance;
    float c_ytd_payment;
    uint32_t c_id;
    uint16_t c_w_id;
    uint8_t c_d_id;
    uint8_t c_payment_cnt;
    uint8_t c_delivery_cnt;
    // char c_data[MAX_CUSTOMER_DATA + 1];
} customer_t;

typedef struct history
{
    char h_data[MAX_HISTORY_DATA + 1];
    char h_date[DATETIME_SIZE + 1];
    float h_amount;
    uint32_t h_c_id;
    uint16_t h_c_w_id;
    uint16_t h_w_id;
    uint8_t h_c_d_id;
    uint8_t h_d_id;
} history_t;

typedef struct new_order
{
    uint32_t no_o_id;
    uint16_t no_w_id;
    uint8_t no_d_id;
} new_order_t;

typedef struct order
{
    char o_entry_d[DATETIME_SIZE + 1];
    uint32_t o_id;
    uint32_t o_c_id;
    uint16_t o_w_id;
    uint8_t o_d_id;
    uint8_t o_carrier_id;
    uint8_t o_ol_cnt;
    uint8_t o_all_local;
} order_t;

typedef struct order_line
{
    char ol_delivery_d[DATETIME_SIZE + 1];
    float ol_amount;
    uint32_t ol_o_id;
    uint32_t ol_i_id;
    uint32_t ol_quantity;
    uint16_t ol_w_id;
    uint16_t ol_supply_w_id;
    uint8_t ol_d_id;
    uint8_t ol_number;
    // char ol_dist_info[Stock::DIST+1];
} order_line_t;

typedef struct item
{
    char i_name[ITEM_MAX_NAME + 1];
    float i_price;
    uint32_t i_id;
    uint32_t i_im_id;
    // char i_data[ITEM_MAX_DATA + 1];
} item_t;

typedef struct stock
{
    uint32_t s_i_id;
    uint32_t s_ytd;
    uint32_t s_order_cnt;
    uint32_t s_remote_cnt;
    uint16_t s_w_id;
    uint16_t s_quantity;
    // char s_dist[District::NUM_PER_WAREHOUSE][DIST+1];
    // char s_data[MAX_DATA + 1];
} stock_t;

void application_init(uint32_t dpu_id);
void exec(unsigned char *params, uint8_t type, uint8_t mode, uint32_t ts);
void payment(unsigned char *params, uint8_t mode, uint32_t ts);
void payment_remote(unsigned char *params, uint8_t mode, uint32_t ts);
void new_order(unsigned char *params, uint8_t mode, uint32_t ts);
void order_status(unsigned char *params, uint8_t mode, uint32_t ts);

// ####################################### TPC-C #######################################

void warehouse_init(uint32_t dpu_id);

uint32_t make_district_key(uint16_t w_id, uint8_t d_id);
uint32_t make_customer_key(uint16_t w_id, uint8_t d_id, uint32_t c_id);
uint32_t make_stock_key(uint16_t w_id, uint32_t i_id);
uint32_t make_new_order_key(uint16_t w_id, uint8_t d_id, uint32_t o_id);
uint32_t make_order_key(uint16_t w_id, uint8_t d_id, uint32_t o_id);
uint32_t make_order_line_key(uint16_t w_id, uint8_t d_id, uint32_t o_id, uint8_t ol_number);

MRAM warehouse_t *generate_warehouse(uint16_t w_id);
MRAM district_t *generate_district(uint16_t w_id, uint8_t d_id);
MRAM customer_t *generate_customer(uint16_t w_id, uint8_t d_id, uint32_t c_id);
MRAM item_t *generate_item(uint32_t i_id);
MRAM stock_t *generate_stock(uint16_t w_id, uint32_t i_id);
MRAM new_order_t *generate_new_order(uint16_t w_id, uint8_t d_id, uint32_t o_id);
MRAM order_t *generate_order(uint16_t w_id, uint8_t d_id, uint32_t c_id, uint32_t o_id);
MRAM order_line_t *generate_order_line(uint16_t w_id, uint8_t d_id, uint32_t o_id, uint8_t ol_number);

MRAM warehouse_t *find_warehouse();
MRAM district_t *find_district_tm(uint32_t w_id, uint32_t d_id);
MRAM customer_t *find_customer_tm(uint32_t w_id, uint32_t d_id, uint32_t c_id);
MRAM order_t *find_order_tm(uint32_t w_id, uint32_t d_id, uint32_t o_id);
MRAM new_order_t *find_new_order_tm(uint32_t w_id, uint32_t d_id, uint32_t o_id);
MRAM order_line_t *find_order_line_tm(uint32_t w_id, uint32_t d_id, uint32_t o_id, uint32_t ol_number);

void insert_order_tm(MRAM order_t *order);
void insert_new_order_tm(MRAM new_order_t *new_order);
void insert_order_line_tm(MRAM order_line_t *order_line);
