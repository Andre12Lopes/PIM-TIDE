#include "tpcc_client.h"
#include "../common/app_common.h"
#include "sub_transaction.h"
#include "transaction.h"

TPCCClient::TPCCClient(std::uint32_t n_warehouses)
    : n_warehouses_(n_warehouses)
{
}

void
TPCCClient::payment(System &system)
{
    std::uint32_t x, y;
    std::uint16_t w_id, c_w_id;
    std::uint16_t d_id, c_d_id;
    std::uint16_t c_id;
    float h_amount;
    char now[DATETIME_SIZE + 1];

    Transaction tx;
    SubTransaction sub_tx_1, sub_tx_2;

    x = random_.generate(1, 100);
    y = random_.generate(1, 100);

    w_id = random_.generate(1, n_warehouses_);
    d_id = random_.generate(1, NUM_DISTRICTS_PER_WAREHOUSE);

    if (n_warehouses_ == 1 || x <= (100 - DIST_PAYMENT))
    {
        c_w_id = w_id;
        c_d_id = d_id;
    }
    else
    {
        // Case where customer resident warehouse is remote
        do
        {
            c_w_id = random_.generate(1, n_warehouses_);
        } while (c_w_id == w_id);

        c_d_id = random_.generate(1, NUM_DISTRICTS_PER_WAREHOUSE);
    }

    h_amount = random_.generate(MIN_PAYMENT, MAX_PAYMENT);

    clock_.getDateTimestamp(now);

    if (y <= 0 /* y <= 60 */)
    {
        // Currently not supported
    }
    else
    {
        c_id = random_.generate(1, NUM_CUSTOMERS_PER_DISTRICT);

        sub_tx_1.set_type(PAYMENT);
        sub_tx_1.add_param(w_id);
        sub_tx_1.add_param(d_id);
        sub_tx_1.add_param(c_w_id);
        sub_tx_1.add_param(c_d_id);
        sub_tx_1.add_param(c_id);
        sub_tx_1.add_param(h_amount);
        sub_tx_1.add_param(now, DATETIME_SIZE + 1);

        tx.add(w_id - 1, sub_tx_1);

        if (c_w_id != w_id)
        {
            sub_tx_2.set_type(PAYMENT_REMOTE);
            sub_tx_2.add_param(w_id);
            sub_tx_2.add_param(d_id);
            sub_tx_2.add_param(c_w_id);
            sub_tx_2.add_param(c_d_id);
            sub_tx_2.add_param(c_id);
            sub_tx_2.add_param(h_amount);
            sub_tx_2.add_param(now, DATETIME_SIZE + 1);

            tx.add(c_w_id - 1, sub_tx_2);
        }

        system.exec(tx);
    }
}

void
TPCCClient::new_order(System &system)
{
    std::uint16_t w_id;
    std::uint16_t d_id;
    std::uint16_t c_id;
    std::uint16_t ol_cnt;
    char now[DATETIME_SIZE + 1];

    Transaction tx;
    SubTransaction sub_tx;

    w_id = random_.generate(1, n_warehouses_);
    d_id = random_.generate(1, NUM_DISTRICTS_PER_WAREHOUSE);
    c_id = random_.generate(1, NUM_CUSTOMERS_PER_DISTRICT);

    ol_cnt = random_.generate(ORDER_MIN_OL_COUNT, ORDER_MAX_OL_COUNT);

    std::vector<new_order_item_t> items(ol_cnt);
    for (std::uint16_t i = 0; i < ol_cnt; ++i)
    {
        items[i].i_id           = random_.generate(1, NUM_ITEMS);
        items[i].ol_supply_w_id = w_id;
        items[i].ol_quantity    = random_.generate(1, MAX_OL_QUANTITY);
    }

    clock_.getDateTimestamp(now);

    sub_tx.set_type(NEW_ORDER);
    sub_tx.add_param(w_id);
    sub_tx.add_param(d_id);
    sub_tx.add_param(c_id);
    sub_tx.add_param(ol_cnt);

    for (std::uint32_t i = 0; i < ol_cnt; ++i)
    {
        sub_tx.add_param(items[i].i_id);
        sub_tx.add_param(items[i].ol_supply_w_id);
        sub_tx.add_param(items[i].ol_quantity);
    }

    sub_tx.add_param(now, DATETIME_SIZE + 1);

    tx.add(w_id - 1, sub_tx);

    system.exec(tx);
}

void
TPCCClient::order_status(System &system)
{
    std::uint32_t y;
    std::uint32_t w_id;
    std::uint32_t d_id;
    std::uint32_t c_id;

    Transaction tx;
    SubTransaction sub_tx;

    y = random_.generate(1, 100);
    if (y <= 0 /* y <= 60 */)
    {
        // Currently not supported
    }
    else
    {
        w_id = random_.generate(1, n_warehouses_);
        d_id = random_.generate(1, NUM_DISTRICTS_PER_WAREHOUSE);
        c_id = random_.generate(1, NUM_CUSTOMERS_PER_DISTRICT);
    }

    sub_tx.set_type(ORDER_STATUS);
    sub_tx.add_param(w_id);
    sub_tx.add_param(d_id);
    sub_tx.add_param(c_id);

    tx.add(w_id - 1, sub_tx);

    system.exec(tx);
}
