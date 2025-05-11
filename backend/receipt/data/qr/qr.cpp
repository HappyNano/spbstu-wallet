#include "qr.h"

#include <regex>

bool wallet::isValidQRData(const std::string & data) {
    // Проверяем общий формат фискального QR-кода
    std::regex fiscalQRRegex(
     "t=\\d{8}T\\d{4}&" // Дата и время (t)
     "s=\\d+\\.?\\d*&"  // Сумма (s)
     "fn=\\d+&"         // Номер фискального накопителя (fn)
     "i=\\d+&"          // Номер фискального документа (i)
     "fp=\\d+&"         // Фискальный признак (fp)
     "n=\\d+"           // Тип документа (n)
    );

    return std::regex_match(data, fiscalQRRegex);
}

wallet::Receipt wallet::parseQRDataFromString(const std::string & data) {
    // Парсим параметры QR-кода
    std::map< std::string, std::string > params;
    std::regex paramRegex("([^=&]+)=([^=&]+)");
    auto paramBegin = std::sregex_iterator(data.begin(), data.end(), paramRegex);
    auto paramEnd = std::sregex_iterator();

    for (auto i = paramBegin; i != paramEnd; ++i) {
        std::smatch match = *i;
        params[match[1].str()] = match[2].str();
    }

    // Проверяем наличие всех необходимых полей
    const std::vector< std::string > requiredFields = { "t", "s", "fn", "i", "fp", "n" };
    std::vector< std::string > missingFields;

    for (const auto & field: requiredFields) {
        if (params.find(field) == params.end()) {
            missingFields.push_back(field);
        }
    }

    if (!missingFields.empty()) {
        std::string details = "Missing fields: ";
        for (size_t i = 0; i < missingFields.size(); ++i) {
            details += missingFields[i];
            if (i < missingFields.size() - 1) {
                details += ", ";
            }
        }
        throw std::runtime_error(details);
    }

    // Проверка суммы
    double amount = std::stod(params["s"]);
    if (amount < 0) {
        throw std::runtime_error("Invalid amount (s): " + params["s"]);
    }

    // Проверка типа документа
    int docType = std::stoi(params["n"]);
    if (docType < 1 || docType > 4) {
        throw std::runtime_error("Invalid document type (n): " + params["n"]);
    }

    Receipt receipt;

    receipt.set_t(params["t"]);
    receipt.set_s(amount);
    receipt.set_fn(std::stoull(params["fn"]));
    receipt.set_i(std::stoull(params["i"]));
    receipt.set_fp(std::stoull(params["fp"]));
    receipt.set_n(docType);

    return receipt;
}
