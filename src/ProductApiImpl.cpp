#include "ProductApiImpl.h"
#include <utility>
#include <nlohmann/json.hpp>
#include "Helpers.h"
#include <sstream>
#include <iostream>


namespace market {

ProductApiImpl::ProductApiImpl(const std::shared_ptr<Pistache::Rest::Router>& router, std::string connectionString) : api::ProductApi(router), repository_(std::move(connectionString)) {

}


///////////////////////////////////////////////////////////////////////////////////////////////
void ProductApiImpl::create_product(const model::ProductCreate& productCreate,Pistache::Http::ResponseWriter& response) {
  const auto product = repository_.create(productCreate);

  const nlohmann::json body = product;

  response.headers()
      .add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json)
       );

  response.send(
      Pistache::Http::Code::Created,
      body.dump()
  );
}

void ProductApiImpl::delete_product(
    const int64_t& id,
    Pistache::Http::ResponseWriter& response
) {
    const bool found = repository_.archive(id);

    if (!found) {
        model::ErrorResponse error;

        error.setErrorCode("PRODUCT_NOT_FOUND");
        error.setMessage("Товар не найден по ID");

        const nlohmann::json body = error;

        response.headers()
            .add<Pistache::Http::Header::ContentType>(
                MIME(Application, Json)
            );

        response.send(
            Pistache::Http::Code::Not_Found,
            body.dump()
        );

        return;
    }

    response.send(Pistache::Http::Code::No_Content);
}

void ProductApiImpl::get_product_by_id(const int64_t& id, Pistache::Http::ResponseWriter& response) {
  const auto product = repository_.findById(id);

    response.headers()
        .add<Pistache::Http::Header::ContentType>(
            MIME(Application, Json)
        );

    if (!product.has_value()) {
        model::ErrorResponse error;

        error.setErrorCode("PRODUCT_NOT_FOUND");
        error.setMessage("Товар не найден по ID");

        const nlohmann::json body = error;

        response.send(
            Pistache::Http::Code::Not_Found,
            body.dump()
        );

        return;
    }

    const nlohmann::json body = product.value();

    response.send(
        Pistache::Http::Code::Ok,
        body.dump()
    );
}

void ProductApiImpl::get_products(
    const std::optional<int32_t>& page,
    const std::optional<int32_t>& size,
    const std::optional<std::string>& status,
    const std::optional<std::string>& category,
    Pistache::Http::ResponseWriter& response
) {
    const int32_t pageNumber = page.value_or(0);
    const int32_t pageSize = size.value_or(20);

    const auto result = repository_.findPage(
        pageNumber,
        pageSize,
        status,
        category
    );

    const nlohmann::json body = result;

    response.headers()
        .add<Pistache::Http::Header::ContentType>(
            MIME(Application, Json)
        );

    response.send(
        Pistache::Http::Code::Ok,
        body.dump()
    );
}

void ProductApiImpl::update_product(
    const int64_t& id,
    const model::ProductUpdate& productUpdate,
    Pistache::Http::ResponseWriter& response
) {
    const auto product = repository_.update(id, productUpdate);

    response.headers()
        .add<Pistache::Http::Header::ContentType>(
            MIME(Application, Json)
        );

    if (!product.has_value()) {
        model::ErrorResponse error;

        error.setErrorCode("PRODUCT_NOT_FOUND");
        error.setMessage("Товар не найден по ID");

        const nlohmann::json body = error;

        response.send(
            Pistache::Http::Code::Not_Found,
            body.dump()
        );

        return;
    }

    const nlohmann::json body = product.value();

    response.send(
        Pistache::Http::Code::Ok,
        body.dump()
    );
}

void ProductApiImpl::handleParsingException(
    const std::exception& error,
    Pistache::Http::ResponseWriter& response
) const noexcept {
    model::ErrorResponse result;

    result.setErrorCode("VALIDATION_ERROR");
    result.setMessage("Ошибка валидации входных данных");

    nlohmann::json fields = nlohmann::json::array();

    const auto* validationError =
        dynamic_cast<
            const org::openapitools::server::helpers::ValidationException*
        >(&error);

    if (validationError != nullptr) {
        std::stringstream messages{error.what()};
        std::string item;

        while (std::getline(messages, item, ';')) {
            if (item.empty()) {
                continue;
            }

            const auto separator = item.find(':');

            if (separator == std::string::npos) {
                fields.push_back({
                    {"field", "body"},
                    {"message", item}
                });
                continue;
            }

            std::string field = item.substr(0, separator);
            std::string message = item.substr(separator + 1);

            const auto first = message.find_first_not_of(" \t");
            if (first != std::string::npos) {
                message.erase(0, first);
            }

            // Сгенерированная validate() использует пути
            // вида ProductCreate.name или ProductUpdate.price.
            for (const std::string prefix : {
                std::string{"ProductCreate."},
                std::string{"ProductUpdate."}
            }) {
                if (field.compare(0, prefix.size(), prefix) == 0) {
                    field.erase(0, prefix.size());
                    break;
                }
            }

            fields.push_back({
                {"field", field},
                {"message", message}
            });
        }
    }

    if (fields.empty()) {
        fields.push_back({
            {"field", "body"},
            {"message", error.what()}
        });
    }

    nlohmann::json body = result;
    body["details"] = {{"fields", fields}};

    response.headers()
        .add<Pistache::Http::Header::ContentType>(
            MIME(Application, Json)
        );

    response.send(
        Pistache::Http::Code::Bad_Request,
        body.dump()
    );
}

void ProductApiImpl::handleOperationException(
    const std::exception& error,
    Pistache::Http::ResponseWriter& response
) const noexcept {
    const nlohmann::json log = {
        {"level", "error"},
        {"message", "API operation failed"},
        {"details", error.what()}
    };

    std::cerr << log.dump() << std::endl;

    model::ErrorResponse result;

    result.setErrorCode("INTERNAL_ERROR");
    result.setMessage("Внутренняя ошибка сервера");

    const nlohmann::json body = result;

    response.headers()
        .add<Pistache::Http::Header::ContentType>(
            MIME(Application, Json)
        );

    response.send(
        Pistache::Http::Code::Internal_Server_Error,
        body.dump()
    );
}

}

