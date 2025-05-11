#include "receipt_database.h"
#include <stdexcept>

using namespace wallet;
using namespace cxx;

bool ReceiptDatabase::Config::isValid() const noexcept {
    return !tableName.empty();
}

ReceiptDatabase::ReceiptDatabase(Config config, std::shared_ptr< cxx::IDatabase > database)
  : config_{ std::move(config) }
  , database_{ std::move(database) } {
    if (not config_.isValid()) {
        throw std::logic_error("Invalid config");
    }
}

void ReceiptDatabase::init() {
    auto transaction = database_->makeTransaction();
    if (not transaction->isTableExist(config_.tableName)) {
        transaction->createTable(
         config_.tableName,
         {
          { "id", Col::EDataType::INTEGER, Col::EConstraint::PRIMARY_KEY },
          { "qrdata", Col::EDataType::INTEGER },
          { "id", Col::EDataType::INTEGER },
        });
    }
}

void ReceiptDatabase::insertReceiptData(const std::string & qrCode) {
    database_->makeTransaction()->insert(config_.tableName, { "qrdata" }, { qrCode });
}

void ReceiptDatabase::insertReceiptData(const ReceiptData & receipt) {
}
