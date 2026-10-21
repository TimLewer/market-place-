#pragma once

#include "ProductCreate.h"
#include "ProductResponse.h"
#include "ProductUpdate.h"
#include "ProductPage.h"

#include <cstdint>
#include <optional>
#include <string>

namespace market {

namespace model = org::openapitools::server::model;

class ProductRepository {
public:
  model::ProductPage findPage(
    int32_t page,
    int32_t size,
    const std::optional<std::string>& status,
    const std::optional<std::string>& category
) const;

    explicit ProductRepository(std::string connectionString);

    model::ProductResponse create(
        const model::ProductCreate& request
    ) const;

    std::optional<model::ProductResponse> findById(
        int64_t id
    ) const;

    std::optional<model::ProductResponse> update(
    int64_t id,
    const model::ProductUpdate& request
) const;

  bool archive(int64_t id) const;
private:
    std::string connectionString_;
};

}
