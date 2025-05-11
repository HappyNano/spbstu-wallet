-- 1. Таблица пользователей и токенов
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    token VARCHAR(255) NOT NULL
);

-- 2. Таблица данных пользователей
CREATE TABLE users_data (
    user_id INTEGER REFERENCES users(id),
    created_at TIMESTAMP NOT NULL DEFAULT NOW()
);

-- 3. Таблица чеков Receipt
CREATE TABLE receipts (
    id SERIAL PRIMARY KEY,
    t VARCHAR(15) NOT NULL, -- Дата и время продажи (ггггммддTччммcc)
    s INTEGER NOT NULL, -- Сумма чека (в копейках)
    fn BIGINT NOT NULL, -- Номер фискального накопителя
    i BIGINT NOT NULL, -- Номер фискального документа
    fp BIGINT NOT NULL, -- Фискальный признак документа
    n INTEGER NOT NULL, -- Вид документа
    created_at TIMESTAMP NOT NULL DEFAULT NOW(),
    UNIQUE (fn, i, fp) -- Составной уникальный ключ для чека
);

-- 4. Таблица запросов данных чека
CREATE TABLE receipt_requests (
    id SERIAL PRIMARY KEY,
    receipt_id INTEGER REFERENCES receipts(id),
    ofd_data JSONB,
    request_time TIMESTAMP NOT NULL DEFAULT NOW()
);

-- 5.1. Таблица уникальных товаров
CREATE TABLE unique_items (
    id SERIAL PRIMARY KEY,
    name VARCHAR(255) NOT NULL
);

-- 5.2. Таблица данных чеков ReceiptData
CREATE TABLE receipt_data (
    id SERIAL PRIMARY KEY,
    receipt_id INTEGER NOT NULL REFERENCES receipts(id),
    request_id INTEGER REFERENCES receipt_requests(id),
    retailer_name VARCHAR(255),
    retailer_place VARCHAR(255),
    retailer_inn VARCHAR(50),
    retailer_address TEXT,
    additional_info JSONB,
    created_at TIMESTAMP NOT NULL DEFAULT NOW()
);

-- 5.3. Таблица receipt_items
CREATE TABLE receipt_items (
    id SERIAL PRIMARY KEY,
    receipt_data_id INTEGER NOT NULL REFERENCES receipt_data(id),
    unique_item_id INTEGER REFERENCES unique_items(id) DEFAULT NULL,
    name VARCHAR(255) NOT NULL,
    price INTEGER NOT NULL,
    quantity NUMERIC(12, 3) NOT NULL,
    amount INTEGER NOT NULL,
    nds_type SMALLINT NOT NULL,
    payment_type SMALLINT NOT NULL,
    product_type SMALLINT NOT NULL,
    measurement_unit SMALLINT NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT NOW() 
);

-- 6. Таблица соотношений id чека и id пользователя
CREATE TABLE user_receipts (
    user_id INTEGER NOT NULL REFERENCES users(id),
    receipt_id INTEGER NOT NULL REFERENCES receipts(id),
    UNIQUE (user_id, receipt_id)
);

-- 7. Таблица названий категорий
CREATE TABLE categories (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL UNIQUE
);

-- 8. Таблица доп. персонажей у пользователя
CREATE TABLE user_characters (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(id),
    name VARCHAR(100) NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT NOW(),
    UNIQUE (user_id, name)
);

-- 9. Таблица доходов/расходов (транзакций) пользователя
CREATE TABLE transactions (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(id),
    tr_time TIMESTAMP NOT NULL,
    type SMALLINT NOT NULL CHECK (type IN (0, 1)), -- 0-доход, 1-расход
    amount INTEGER NOT NULL, -- сумма в копейках
    category_id INTEGER REFERENCES categories(id) DEFAULT NULL,
    receipt_id INTEGER REFERENCES receipts(id) DEFAULT NULL,
    comment TEXT,
    created_at TIMESTAMP NOT NULL DEFAULT NOW()
);

-- 10. Таблица делений транзакций пользователя
CREATE TABLE transaction_splits (
    id SERIAL PRIMARY KEY,
    transaction_id INTEGER NOT NULL REFERENCES transactions(id),
    character_id INTEGER NOT NULL REFERENCES user_characters(id),
    amount INTEGER NOT NULL, -- сумма в копейках
    comment TEXT,
    created_at TIMESTAMP NOT NULL DEFAULT NOW(),
    UNIQUE (transaction_id, character_id)
);


-- Индексы
CREATE INDEX idx_receipts_fn_i_fp ON receipts(fn, i, fp);
CREATE INDEX idx_receipt_items_receipt_data_id ON receipt_items(receipt_data_id);
CREATE INDEX idx_receipt_items_unique_item_uid ON receipt_items(unique_item_id);
CREATE INDEX idx_user_receipts_user_id ON user_receipts(user_id);
CREATE INDEX idx_user_receipts_receipt_id ON user_receipts(receipt_id);
CREATE INDEX idx_receipt_data_receipt_id ON receipt_data(receipt_id);

CREATE INDEX idx_user_characters_user_id ON user_characters(user_id);
CREATE INDEX idx_transactions_user_id ON transactions(user_id);
CREATE INDEX idx_transactions_category_id ON transactions(category_id);
CREATE INDEX idx_transactions_receipt_id ON transactions(receipt_id);
CREATE INDEX idx_transaction_splits_transaction_id ON transaction_splits(transaction_id);
CREATE INDEX idx_transaction_splits_character_id ON transaction_splits(character_id);
