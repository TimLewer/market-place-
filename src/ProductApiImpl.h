#pragma once

#include "ProductApi.h"
#include "ProductRepository.h"
#include <exception>

namespace market {
  
  namespace api = org::openapitools::server::api;
  namespace model = org::openapitools::server::model;

  class ProductApiImpl : public api::ProductApi {
  public:
    explicit ProductApiImpl(const std::shared_ptr<Pistache::Rest::Router>& router, std::string connectionString);
  
  private:
    ProductRepository repository_;
    void create_product(const model::ProductCreate& productCreate,Pistache::Http::ResponseWriter& response) override;

    void delete_product(const int64_t& id, Pistache::Http::ResponseWriter& response) override;

    void get_product_by_id(const int64_t& id, Pistache::Http::ResponseWriter& response) override;

    void get_products(
        const std::optional<int32_t>& page,
        const std::optional<int32_t>& size, const std::optional<std::string>& status,
        const std::optional<std::string>& category,
        Pistache::Http::ResponseWriter& response
    ) override;

    void update_product(const int64_t& id, const model::ProductUpdate& productUpdate, Pistache::Http::ResponseWriter& response) override;
    
    void handleParsingException(const std::exception& error,Pistache::Http::ResponseWriter& response) const noexcept override;

    void handleOperationException(const std::exception& error, Pistache::Http::ResponseWriter& response) const noexcept override;

  };
}

