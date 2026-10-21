CREATE TABLE products (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

    name VARCHAR(255) NOT NULL,
    description VARCHAR(4000),

    price NUMERIC(12, 2) NOT NULL,
    stock INTEGER NOT NULL,

    category VARCHAR(100) NOT NULL,
    status VARCHAR(20) NOT NULL,

    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT products_name_length
        CHECK (char_length(name) >= 1),

    CONSTRAINT products_price_positive
        CHECK (price >= 0.01),

    CONSTRAINT products_stock_nonnegative
        CHECK (stock >= 0),

    CONSTRAINT products_category_length
        CHECK (char_length(category) >= 1),

    CONSTRAINT products_status_valid
        CHECK (status IN ('ACTIVE', 'INACTIVE', 'ARCHIVED'))
);

CREATE INDEX idx_products_status ON products (status);

CREATE FUNCTION set_products_updated_at()
RETURNS TRIGGER
LANGUAGE plpgsql
AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$;

CREATE TRIGGER products_updated_at_trigger
BEFORE UPDATE ON products
FOR EACH ROW
EXECUTE FUNCTION set_products_updated_at();
