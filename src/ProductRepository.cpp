#include "ProductRepository.h"

#include <pqxx/pqxx>
#include <utility>
#include <vector>

namespace market {

  namespace {
    model::ProductResponse productFromRow(const pqxx::row& row);
}

model::ProductPage ProductRepository::findPage(
    int32_t page,
    int32_t size,
    const std::optional<std::string>& status,
    const std::optional<std::string>& category
) const {
    pqxx::connection connection{connectionString_};
    pqxx::work transaction{connection};

    transaction.exec(
        "SET TRANSACTION ISOLATION LEVEL REPEATABLE READ READ ONLY"
    );

    const int64_t offset =
        static_cast<int64_t>(page) * size;

    const auto countResult = transaction.exec_params(
        R"(
            SELECT COUNT(*)
            FROM products
            WHERE ($1::text IS NULL OR status = $1::text)
              AND ($2::text IS NULL OR category = $2::text)
        )",
        status,
        category
    );

    const auto totalElements =
        countResult[0][0].as<int64_t>();

    const auto rows = transaction.exec_params(
        R"(
            SELECT
                id,
                name,
                description,
                price,
                stock,
                category,
                status,
                to_char(
                    created_at AT TIME ZONE 'UTC',
                    'YYYY-MM-DD"T"HH24:MI:SS.US"Z"'
                ) AS created_at_text,
                to_char(
                    updated_at AT TIME ZONE 'UTC',
                    'YYYY-MM-DD"T"HH24:MI:SS.US"Z"'
                ) AS updated_at_text
            FROM products
            WHERE ($1::text IS NULL OR status = $1::text)
              AND ($2::text IS NULL OR category = $2::text)
            ORDER BY id
            LIMIT $3 OFFSET $4
        )",
        status,
        category,
        size,
        offset
    );

    std::vector<model::ProductResponse> products;
    products.reserve(rows.size());

    for (const auto& row : rows) {
        products.push_back(productFromRow(row));
    }

    model::ProductPage result;

    result.setContent(products);
    result.setTotalElements(totalElements);
    result.setPage(page);
    result.setSize(size);

    transaction.commit();

    return result;
}

  std::optional<model::ProductResponse>
ProductRepository::update(
    int64_t id,
    const model::ProductUpdate& request
) const {
    pqxx::connection connection{connectionString_};
    pqxx::work transaction{connection};

    std::optional<std::string> description;

    if (request.descriptionIsSet()) {
        description = request.getDescription();
    }

    const auto result = transaction.exec_params(
        R"(
            UPDATE products
            SET
                name = $2,
                description = $3,
                price = $4,
                stock = $5,
                category = $6,
                status = $7
            WHERE id = $1
            RETURNING
                id,
                name,
                description,
                price,
                stock,
                category,
                status,
                to_char(
                    created_at AT TIME ZONE 'UTC',
                    'YYYY-MM-DD"T"HH24:MI:SS.US"Z"'
                ) AS created_at_text,
                to_char(
                    updated_at AT TIME ZONE 'UTC',
                    'YYYY-MM-DD"T"HH24:MI:SS.US"Z"'
                ) AS updated_at_text
        )",
        id,
        request.getName(),
        description,
        request.getPrice(),
        request.getStock(),
        request.getCategory(),
        request.getStatus()
    );

    if (result.empty()) {
        transaction.commit();
        return std::nullopt;
    }

    auto product = productFromRow(result[0]);

    transaction.commit();

    return product;
}
bool ProductRepository::archive(int64_t id) const {
    pqxx::connection connection{connectionString_};
    pqxx::work transaction{connection};

    const auto result = transaction.exec_params(
        R"(
            UPDATE products
            SET status = 'ARCHIVED'
            WHERE id = $1
            RETURNING id
        )",
        id
    );

    const bool found = !result.empty();

    transaction.commit();

    return found;
}

namespace {

model::ProductResponse productFromRow(const pqxx::row& row) {
    model::ProductResponse product;

    product.setId(row["id"].as<int64_t>());
    product.setName(row["name"].as<std::string>());
    product.setPrice(row["price"].as<double>());
    product.setStock(row["stock"].as<int32_t>());
    product.setCategory(row["category"].as<std::string>());
    product.setStatus(row["status"].as<std::string>());

    if (!row["description"].is_null()) {
        product.setDescription(
            row["description"].as<std::string>()
        );
    }

    product.setCreatedAt(
        row["created_at_text"].as<std::string>()
    );

    product.setUpdatedAt(
        row["updated_at_text"].as<std::string>()
    );

    return product;
}

}

ProductRepository::ProductRepository(std::string connectionString)
    : connectionString_(std::move(connectionString))
{
}

model::ProductResponse ProductRepository::create(
    const model::ProductCreate& request
) const {
    pqxx::connection connection{connectionString_};
    pqxx::work transaction{connection};

    std::optional<std::string> description;

    if (request.descriptionIsSet()) {
        description = request.getDescription();
    }

    const auto result = transaction.exec_params(
        R"(
            INSERT INTO products (
                name,
                description,
                price,
                stock,
                category,
                status
            )
            VALUES ($1, $2, $3, $4, $5, $6)
            RETURNING
                id,
                name,
                description,
                price,
                stock,
                category,
                status,
                to_char(
                    created_at AT TIME ZONE 'UTC',
                    'YYYY-MM-DD"T"HH24:MI:SS.US"Z"'
                ) AS created_at_text,
                to_char(
                    updated_at AT TIME ZONE 'UTC',
                    'YYYY-MM-DD"T"HH24:MI:SS.US"Z"'
                ) AS updated_at_text
        )",
        request.getName(),
        description,
        request.getPrice(),
        request.getStock(),
        request.getCategory(),
        request.getStatus()
    );

    auto product = productFromRow(result[0]);

    transaction.commit();

    return product;
}

std::optional<model::ProductResponse>
ProductRepository::findById(int64_t id) const {
    pqxx::connection connection{connectionString_};
    pqxx::work transaction{connection};

    const auto result = transaction.exec_params(
        R"(
            SELECT
                id,
                name,
                description,
                price,
                stock,
                category,
                status,
                to_char(
                    created_at AT TIME ZONE 'UTC',
                    'YYYY-MM-DD"T"HH24:MI:SS.US"Z"'
                ) AS created_at_text,
                to_char(
                    updated_at AT TIME ZONE 'UTC',
                    'YYYY-MM-DD"T"HH24:MI:SS.US"Z"'
                ) AS updated_at_text
            FROM products
            WHERE id = $1
        )",
        id
    );

    if (result.empty()) {
        transaction.commit();
        return std::nullopt;
    }

    auto product = productFromRow(result[0]);

    transaction.commit();

    return product;
}

}
