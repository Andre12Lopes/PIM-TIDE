#include <assert.h>

#include <stdio.h>

#include "../common/app_common.h"
#include "alloc.h"
#include "application.h"
#include "avl_tree.h"
#include "byte_utils.h"
#include "tm.h"
#include "dpu_random.h"

MRAM warehouse_t *warehouse;
MRAM avl_tree_t *district_table;
MRAM avl_tree_t *customer_table;
MRAM avl_tree_t *history_table;
MRAM avl_tree_t *item_table;
MRAM avl_tree_t *stock_table;
MRAM avl_tree_t *new_order_table;
MRAM avl_tree_t *order_table;
MRAM avl_tree_t *order_line_table;

void
application_init(uint32_t dpu_id)
{
    mram_malloc_init();

    TM_INIT();
    warehouse_init(dpu_id);
    TM_INIT(); // Init warehouse uses tm. Cleanup after

    mram_malloc_rebalance();
}

void
exec(unsigned char *params, uint8_t type, uint8_t mode, uint32_t ts)
{
    switch (type)
    {
    case PAYMENT:
        payment(params, mode, ts);
        break;

    case PAYMENT_REMOTE:
        payment_remote(params, mode, ts);
        break;

    case NEW_ORDER:
        new_order(params, mode, ts);
        break;

    case ORDER_STATUS:
        order_status(params, mode, ts);
        break;

    default:
        break;
    }
}

void
payment(unsigned char *params, uint8_t mode, uint32_t ts)
{
    uint16_t w_id;
    uint16_t d_id;
    uint16_t c_w_id;
    uint16_t c_d_id;
    uint16_t c_id;
    float h_amount;
    char now[15];

    uintptr_t tmp_w_ytd;
    uintptr_t tmp_d_ytd;
    uintptr_t tmp_c_balance;
    uintptr_t tmp_c_ytd_payment;
    uintptr_t tmp_c_payment_cnt;

    MRAM warehouse_t *warehouse = NULL;
    MRAM district_t *district   = NULL;
    MRAM customer_t *customer   = NULL;

    bytes_to_uint16(&params, &w_id);
    bytes_to_uint16(&params, &d_id);
    bytes_to_uint16(&params, &c_w_id);
    bytes_to_uint16(&params, &c_d_id);
    bytes_to_uint16(&params, &c_id);
    bytes_to_float(&params, &h_amount);
    bytes_to_char(&params, now, 15);

    TM_START(ts, PAYMENT, mode);

    warehouse = find_warehouse();

    tmp_w_ytd = TM_LOAD(&warehouse->w_ytd);
    TM_STORE(&warehouse->w_ytd, float2uintp(uintp2float(tmp_w_ytd) + h_amount));

    district = find_district_tm(w_id, d_id);
    AFTER_TM_FUNCTION();

    tmp_d_ytd = TM_LOAD(&district->d_ytd);
    TM_STORE(&district->d_ytd, float2uintp(uintp2float(tmp_d_ytd) + h_amount));

    if (w_id == c_w_id && d_id == c_d_id)
    {
        // Local payment
        customer = find_customer_tm(c_w_id, c_d_id, c_id);
        AFTER_TM_FUNCTION();

        tmp_c_balance = TM_LOAD(&customer->c_balance);
        TM_STORE(&customer->c_balance, float2uintp(uintp2float(tmp_c_balance) - h_amount));

        tmp_c_ytd_payment = TM_LOAD(&customer->c_ytd_payment);
        TM_STORE(&customer->c_ytd_payment, float2uintp(uintp2float(tmp_c_ytd_payment) + h_amount));

        tmp_c_payment_cnt = TM_LOAD(&customer->c_payment_cnt);
        TM_STORE(&customer->c_payment_cnt, tmp_c_payment_cnt + 1);
    }

    TM_COMMIT();
}

void
payment_remote(unsigned char *params, uint8_t mode, uint32_t ts)
{
    uint16_t c_w_id;
    uint16_t c_d_id;
    uint16_t c_id;
    float h_amount;

    uintptr_t tmp_c_balance;
    uintptr_t tmp_c_ytd_payment;
    uintptr_t tmp_c_payment_cnt;

    MRAM customer_t *customer = NULL;

    bytes_to_uint16(&params, &c_w_id);
    bytes_to_uint16(&params, &c_d_id);
    bytes_to_uint16(&params, &c_id);
    bytes_to_float(&params, &h_amount);

    TM_START(ts, PAYMENT_REMOTE, mode);

    customer = find_customer_tm(c_w_id, c_d_id, c_id);
    AFTER_TM_FUNCTION();

    tmp_c_balance = TM_LOAD(&customer->c_balance);
    TM_STORE(&customer->c_balance, float2uintp(uintp2float(tmp_c_balance) - h_amount));

    tmp_c_ytd_payment = TM_LOAD(&customer->c_ytd_payment);
    TM_STORE(&customer->c_ytd_payment, float2uintp(uintp2float(tmp_c_ytd_payment) + h_amount));

    tmp_c_payment_cnt = TM_LOAD(&customer->c_payment_cnt);
    TM_STORE(&customer->c_payment_cnt, tmp_c_payment_cnt + 1);

    TM_COMMIT();
}

void
new_order(unsigned char *params, uint8_t mode, uint32_t ts)
{
    uint16_t w_id;
    uint16_t d_id;
    uint16_t c_id;
    uint16_t ol_cnt;
    new_order_item_t items[ORDER_MAX_OL_COUNT];

    MRAM warehouse_t *warehouse   = NULL;
    MRAM district_t *district     = NULL;
    MRAM customer_t *customer     = NULL;
    MRAM order_t *order           = NULL;
    MRAM new_order_t *new_order   = NULL;
    MRAM order_line_t *order_line = NULL;

    uint32_t tmp_d_next_o_id;

    bytes_to_uint16(&params, &w_id);
    bytes_to_uint16(&params, &d_id);
    bytes_to_uint16(&params, &c_id);
    bytes_to_uint16(&params, &ol_cnt);

    for (uint16_t i = 0; i < ol_cnt; ++i)
    {
        bytes_to_uint32(&params, &items[i].i_id);
        bytes_to_uint16(&params, &items[i].ol_supply_w_id);
        bytes_to_uint16(&params, &items[i].ol_quantity);
    }

    TM_START(ts, NEW_ORDER, mode);

    district = find_district_tm(w_id, d_id);
    AFTER_TM_FUNCTION();

    customer = find_customer_tm(w_id, d_id, c_id);
    AFTER_TM_FUNCTION();

    for (uint16_t i = 0; i < ol_cnt; ++i)
    {
        if (items[i].ol_supply_w_id != w_id)
        {
            assert(0);
        }
    }

    tmp_d_next_o_id = TM_LOAD(&district->d_next_o_id);
    TM_STORE(&district->d_next_o_id, tmp_d_next_o_id + 1);

    warehouse = find_warehouse();

    order               = generate_order(w_id, d_id, c_id, tmp_d_next_o_id);
    order->o_w_id       = w_id;
    order->o_carrier_id = 0;
    order->o_ol_cnt     = ol_cnt;
    order->o_all_local  = 1;

    insert_order_tm(order);
    AFTER_TM_FUNCTION();

    new_order = generate_new_order(w_id, d_id, tmp_d_next_o_id);
    insert_new_order_tm(new_order);
    AFTER_TM_FUNCTION();

    for (uint32_t ol_number = 1; ol_number <= ol_cnt; ++ol_number)
    {
        order_line = generate_order_line(w_id, d_id, tmp_d_next_o_id, 0);
        memcpy(order_line->ol_delivery_d, "00000000000000", 15);

        order_line->ol_number      = ol_number;
        order_line->ol_i_id        = items[ol_number].i_id;
        order_line->ol_supply_w_id = items[ol_number].ol_supply_w_id;
        order_line->ol_quantity    = items[ol_number].ol_quantity;

        insert_order_line_tm(order_line);
        AFTER_TM_FUNCTION_LOOP();
    }
    AFTER_TM_FUNCTION();

    TM_COMMIT();
}

void
order_status(unsigned char *params, uint8_t mode, uint32_t ts)
{
    uint32_t w_id;
    uint32_t d_id;
    uint32_t c_id;

    uintptr_t tmp_c_balance;
    uintptr_t tmp_o_carrier_id;

    MRAM customer_t *customer     = NULL;
    MRAM order_t *order           = NULL;
    MRAM order_line_t *order_line = NULL;

    bytes_to_uint32(&params, &w_id);
    bytes_to_uint32(&params, &d_id);
    bytes_to_uint32(&params, &c_id);

    TM_START(ts, 0, mode);

    customer = find_customer_tm(w_id, d_id, c_id);
    AFTER_TM_FUNCTION();

    tmp_c_balance = TM_LOAD(&customer->c_balance);

    order = find_order_tm(customer->c_w_id, customer->c_d_id, customer->c_id);
    AFTER_TM_FUNCTION();

    tmp_o_carrier_id = TM_LOAD(&order->o_carrier_id);

    for (uint8_t ol_number = 1; ol_number <= order->o_ol_cnt; ++ol_number)
    {
        order_line = find_order_line_tm(customer->c_w_id, customer->c_d_id, order->o_id, ol_number);
        AFTER_TM_FUNCTION_LOOP();

        if (order_line == NULL)
        {
            break;
        }

        if (order_line->ol_amount < 0.0)
        {
            customer->c_credit_lim = order_line->ol_amount;
        }
    }
    AFTER_TM_FUNCTION();

    TM_COMMIT();
}

// ####################################### TPC-C #######################################

void
warehouse_init(uint32_t dpu_id)
{
    MRAM item_t *item;
    MRAM stock_t *stock;
    MRAM district_t *district;
    MRAM customer_t *customer;
    MRAM order_t *order;
    MRAM order_line_t *order_line;
    uint32_t stock_key, district_key, customer_key, order_key, order_line_key;

    warehouse        = generate_warehouse(dpu_id);
    item_table       = avl_tree_alloc();
    stock_table      = avl_tree_alloc();
    district_table   = avl_tree_alloc();
    customer_table   = avl_tree_alloc();
    history_table    = avl_tree_alloc();
    new_order_table  = avl_tree_alloc();
    order_table      = avl_tree_alloc();
    order_line_table = avl_tree_alloc();

    for (uint32_t i_id = 1; i_id <= NUM_ITEMS; ++i_id)
    {
        item = generate_item(i_id);

        TM_START(0, 0, OPTIMISTIC);
        avl_tree_insert(item_table, item->i_id, (MRAM void *)item);
        TM_COMMIT();

        stock     = generate_stock(warehouse->w_id, item->i_id);
        stock_key = make_stock_key(stock->s_w_id, stock->s_i_id);

        TM_START(0, 0, OPTIMISTIC);
        avl_tree_insert(stock_table, stock_key, (MRAM void *)stock);
        TM_COMMIT();
    }

    for (uint8_t d_id = 1; d_id <= NUM_DISTRICTS_PER_WAREHOUSE; ++d_id)
    {
        district     = generate_district(warehouse->w_id, d_id);
        district_key = make_district_key(district->d_w_id, district->d_id);

        TM_START(0, 0, OPTIMISTIC);
        avl_tree_insert(district_table, district_key, (MRAM void *)district);
        TM_COMMIT();

        for (uint32_t c_id = 1; c_id <= NUM_CUSTOMERS_PER_DISTRICT; ++c_id)
        {
            customer     = generate_customer(warehouse->w_id, d_id, c_id);
            customer_key = make_customer_key(customer->c_w_id, customer->c_d_id, customer->c_id);

            TM_START(0, 0, OPTIMISTIC);
            avl_tree_insert(customer_table, customer_key, (MRAM void *)customer);
            TM_COMMIT();
        }

        for (uint32_t o_id = 1; o_id <= NUM_CUSTOMERS_PER_DISTRICT; ++o_id)
        {
            order     = generate_order(warehouse->w_id, d_id, o_id, o_id);
            order_key = make_order_key(warehouse->w_id, d_id, o_id);

            TM_START(0, 0, OPTIMISTIC);
            avl_tree_insert(order_table, order_key, (MRAM void *)order);
            TM_COMMIT();

            for (uint8_t ol_number = 1; ol_number <= order->o_ol_cnt; ++ol_number)
            {
                order_line     = generate_order_line(warehouse->w_id, d_id, o_id, ol_number);
                order_line_key = make_order_line_key(warehouse->w_id, d_id, o_id, ol_number);

                TM_START(0, 0, OPTIMISTIC);
                avl_tree_insert(order_line_table, order_line_key, (MRAM void *)order_line);
                TM_COMMIT();
            }
        }
    }
}

uint32_t
make_district_key(uint16_t w_id, uint8_t d_id)
{
    return (w_id * NUM_DISTRICTS_PER_WAREHOUSE) + d_id;
}

uint32_t
make_customer_key(uint16_t w_id, uint8_t d_id, uint32_t c_id)
{
    return (w_id * NUM_DISTRICTS_PER_WAREHOUSE + d_id) * NUM_CUSTOMERS_PER_DISTRICT + c_id;
}

uint32_t
make_stock_key(uint16_t w_id, uint32_t i_id)
{
    return (w_id * NUM_STOCK_PER_WAREHOUSE) + i_id;
}

uint32_t
make_new_order_key(uint16_t w_id, uint8_t d_id, uint32_t o_id)
{
    return (o_id * NUM_DISTRICTS_PER_WAREHOUSE + d_id) * MAX_WAREHOUSE_ID + w_id;
}

uint32_t
make_order_key(uint16_t w_id, uint8_t d_id, uint32_t o_id)
{
    return (o_id * NUM_DISTRICTS_PER_WAREHOUSE + d_id) * MAX_WAREHOUSE_ID + w_id;
}

uint32_t
make_order_line_key(uint16_t w_id, uint8_t d_id, uint32_t o_id, uint8_t ol_number)
{
    return ((o_id * NUM_DISTRICTS_PER_WAREHOUSE + d_id) * MAX_WAREHOUSE_ID + w_id) * ORDER_MAX_OL_COUNT +
           ol_number;
}

MRAM warehouse_t *
generate_warehouse(uint16_t w_id)
{
    MRAM warehouse_t *warehouse;

    warehouse = (MRAM warehouse_t *)mram_malloc(sizeof(warehouse_t));

    if (warehouse)
    {
    	/*
    	 * Since DPUs do not support random number generation, 
    	 * due to kernel size limitaions, item fields are hard coded.
    	 * The folowing functions should be used
    	 * - generate_random_int()
    	 * - generate_random_float()
    	 * - generate_random_string()
    	 *
    	 * This does not interfere with results.
    	 */

        warehouse->w_id = w_id;
        memcpy(warehouse->w_name, "ABCDEFGHIJ", 11);
        memcpy(warehouse->w_street_1, "0123456789", 11);
        memcpy(warehouse->w_street_2, "0123456789", 11);
        memcpy(warehouse->w_city, "9876543210", 11);
        memcpy(warehouse->w_state, "CA", 3);
        memcpy(warehouse->w_zip, "123123123", 10);
        warehouse->w_tax = 0.1f;
        warehouse->w_ytd = WAREHOUSE_INITIAL_YTD;
    }

    return warehouse;
}

MRAM district_t *
generate_district(uint16_t w_id, uint8_t d_id)
{
    MRAM district_t *district;

    district = (MRAM district_t *)mram_malloc(sizeof(district_t));

    if (district)
    {
        district->d_id   = d_id;
        district->d_w_id = w_id;
        memcpy(district->d_name, "ABCDEFGHIJ", 11);
        memcpy(district->d_street_1, "0123456789", 11);
        memcpy(district->d_street_2, "0123456789", 11);
        memcpy(district->d_city, "9876543210", 11);
        memcpy(district->d_state, "CA", 3);
        memcpy(district->d_zip, "123123123", 10);
        district->d_tax       = 0.1f;
        district->d_ytd       = DISTRICT_INITIAL_YTD;
        district->d_next_o_id = NUM_CUSTOMERS_PER_DISTRICT + 1;
    }

    return district;
}

MRAM customer_t *
generate_customer(uint16_t w_id, uint8_t d_id, uint32_t c_id)
{
    MRAM customer_t *customer;

    customer = (MRAM customer_t *)mram_malloc(sizeof(customer_t));

    if (customer)
    {
        customer->c_id   = c_id;
        customer->c_d_id = d_id;
        customer->c_w_id = w_id;
        memcpy(customer->c_first, "ABCDE", 6);
        memcpy(customer->c_middle, "ZX", 3);
        memcpy(customer->c_last, "FGHIJ", 6);
        memcpy(customer->c_street_1, "0123456789", 11);
        memcpy(customer->c_street_2, "0123456789", 11);
        memcpy(customer->c_city, "9876543210", 11);
        memcpy(customer->c_state, "CA", 3);
        memcpy(customer->c_zip, "123123123", 10);
        memcpy(customer->c_phone, "0123456789987654", 17);
        memcpy(customer->c_since, "20240716122945", 15);
        memcpy(customer->c_credit, "GC", 3);
        customer->c_credit_lim = CUSTOMER_INITIAL_CREDIT_LIM;
        customer->c_discount     = 0.5000;
        customer->c_balance     = CUSTOMER_INITIAL_BALANCE;
        customer->c_ytd_payment = CUSTOMER_INITIAL_YTD_PAYMENT;
        customer->c_payment_cnt = CUSTOMER_INITIAL_PAYMENT_CNT;
        customer->c_delivery_cnt = CUSTOMER_INITIAL_DELIVERY_CNT;
    }

    return customer;
}

MRAM item_t *
generate_item(uint32_t i_id)
{
    MRAM item_t *item;

    item = (MRAM item_t *)mram_malloc(sizeof(item_t));

    if (item)
    {
        item->i_id = i_id;
        item->i_im_id = 1000;
        memcpy(item->i_name, "012345678998765432100123", 25);
        item->i_price = 12.5;
    }

    return item;
}

MRAM stock_t *
generate_stock(uint16_t w_id, uint32_t i_id)
{
    MRAM stock_t *stock;

    stock = (MRAM stock_t *)mram_malloc(sizeof(stock_t));

    if (stock)
    {
        stock->s_i_id = i_id;
        stock->s_w_id = w_id;
        stock->s_quantity   = STOCK_MIN_QUANTITY;
        stock->s_ytd        = 0;
        stock->s_order_cnt  = 0;
        stock->s_remote_cnt = 0;
    }

    return stock;
}

MRAM new_order_t *
generate_new_order(uint16_t w_id, uint8_t d_id, uint32_t o_id)
{
    MRAM new_order_t *new_order;

    new_order = (MRAM new_order_t *)mram_malloc(sizeof(new_order_t));

    if (new_order)
    {
        new_order->no_o_id = o_id;
        new_order->no_d_id = d_id;
        new_order->no_w_id = w_id;
    }

    return new_order;
}

MRAM order_t *
generate_order(uint16_t w_id, uint8_t d_id, uint32_t c_id, uint32_t o_id)
{
    MRAM order_t *order;

    order = (MRAM order_t *)mram_malloc(sizeof(order_t));

    if (order)
    {
        order->o_id   = o_id;
        order->o_d_id = d_id;
        order->o_w_id = w_id;
        order->o_c_id = c_id;
        memcpy(order->o_entry_d, "20240716122945", 15);
        order->o_carrier_id = ORDER_MIN_CARRIER_ID;
        order->o_ol_cnt     = ORDER_MIN_OL_COUNT;
        order->o_all_local  = ORDER_INITIAL_ALL_LOCAL;
    }

    return order;
}

MRAM order_line_t *
generate_order_line(uint16_t w_id, uint8_t d_id, uint32_t o_id, uint8_t ol_number)
{
    MRAM order_line_t *order_line;

    order_line = (MRAM order_line_t *)mram_malloc(sizeof(order_line_t));

    if (order_line)
    {
        order_line->ol_o_id        = o_id;
        order_line->ol_d_id        = d_id;
        order_line->ol_w_id        = w_id;
        order_line->ol_number      = ol_number;
        order_line->ol_i_id        = ORDER_LINE_MIN_I_ID;
        order_line->ol_supply_w_id = w_id;
        memcpy(order_line->ol_delivery_d, "20240716122945", 15);
        order_line->ol_quantity = ORDER_LINE_INITIAL_QUANTITY;
        order_line->ol_amount   = ORDER_LINE_MIN_AMOUNT;
    }

    return order_line;
}

MRAM warehouse_t *
find_warehouse()
{
    return warehouse;
}

MRAM district_t *
find_district_tm(uint32_t w_id, uint32_t d_id)
{
    return (MRAM district_t *)avl_tree_find(district_table, make_district_key(w_id, d_id));
}

MRAM customer_t *
find_customer_tm(uint32_t w_id, uint32_t d_id, uint32_t c_id)
{
    return (MRAM customer_t *)avl_tree_find(customer_table, make_customer_key(w_id, d_id, c_id));
}

MRAM order_t *
find_order_tm(uint32_t w_id, uint32_t d_id, uint32_t o_id)
{
    uint32_t order_key;

    order_key = make_order_key(w_id, d_id, o_id);

    return (MRAM order_t *)avl_tree_find(order_table, order_key);
}

MRAM new_order_t *
find_new_order_tm(uint32_t w_id, uint32_t d_id, uint32_t o_id)
{
    uint32_t new_order_key;

    new_order_key = make_new_order_key(w_id, d_id, o_id);

    return (MRAM new_order_t *)avl_tree_find(new_order_table, new_order_key);
}

MRAM order_line_t *
find_order_line_tm(uint32_t w_id, uint32_t d_id, uint32_t o_id, uint32_t ol_number)
{
    return (MRAM order_line_t *)avl_tree_find(
        order_line_table, make_order_line_key(w_id, d_id, o_id, ol_number));
}

void
insert_order_tm(MRAM order_t *order)
{
    uint32_t order_key;

    order_key = make_order_key(order->o_w_id, order->o_d_id, order->o_id);

    avl_tree_insert(order_table, order_key, (MRAM void *)order);
}

void
insert_new_order_tm(MRAM new_order_t *new_order)
{
    avl_tree_insert(
        new_order_table, make_new_order_key(new_order->no_w_id, new_order->no_d_id, new_order->no_o_id),
        (MRAM void *)new_order);
}

void
insert_order_line_tm(MRAM order_line_t *order_line)
{
    avl_tree_insert(
        order_line_table,
        make_order_line_key(
            order_line->ol_w_id, order_line->ol_d_id, order_line->ol_o_id, order_line->ol_number),
        (MRAM void *)order_line);
}
