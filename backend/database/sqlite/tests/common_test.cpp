#include <backend/database/sqlite/sqlite_database.h>

#include <gtest/gtest.h>

using namespace cxx;

namespace {

    class SQLiteDatabaseTest: public ::testing::Test {
    public:
        void SetUp() override {
            // Создаем in-memory базу данных для каждого теста
            ASSERT_TRUE(db_.connect(SQLiteDatabase::InMemory{}));
        }

        void TearDown() override {
            db_.disconnect();
        }

        static std::vector< cxx::Col > getTestTableColumns() {
            return {
                { "id", cxx::Col::EDataType::INTEGER, cxx::Col::EConstraint::PRIMARY_KEY },
                { "name", cxx::Col::EDataType::TEXT, cxx::Col::EConstraint::NOT_NULL },
                { "age", cxx::Col::EDataType::INTEGER },
                { "salary", cxx::Col::EDataType::REAL },
                { "active", cxx::Col::EDataType::BOOLEAN }
            };
        }

    protected:
        cxx::SQLiteDatabase db_;
    };

} // unnamed namespace

TEST_F(SQLiteDatabaseTest, ConnectInMemory) {
    ASSERT_TRUE(db_.isReady());
    db_.disconnect();
    ASSERT_FALSE(db_.isReady());
    ASSERT_TRUE(db_.connect(SQLiteDatabase::InMemory{}));
    ASSERT_TRUE(db_.isReady());
}

TEST_F(SQLiteDatabaseTest, ConnectFile) {
    db_.disconnect();
    ASSERT_TRUE(db_.connect(":memory:"));
    ASSERT_TRUE(db_.isReady());
}

TEST_F(SQLiteDatabaseTest, CreateAndDropTable) {
    std::vector< cxx::Col > cols = getTestTableColumns();

    ASSERT_TRUE(db_.createTable("test_table", cols));

    auto result = db_.executeQuery("SELECT name FROM sqlite_master WHERE type='table' AND name='test_table'");
    ASSERT_TRUE(result.has_value());
    ASSERT_FALSE(result->empty());

    ASSERT_TRUE(db_.dropTable("test_table"));

    result = db_.executeQuery("SELECT name FROM sqlite_master WHERE type='table' AND name='test_table'");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->empty());
}

TEST_F(SQLiteDatabaseTest, InsertData) {
    std::vector< cxx::Col > cols = getTestTableColumns();
    ASSERT_TRUE(db_.createTable("test_table", cols));

    ASSERT_TRUE(db_.insert("test_table", { "id", "name", "age", "salary", "active" }, { "1", "Alice", "30", "50000.50", "1" }));

    auto result = db_.select("test_table", { "id", "name", "age", "salary", "active" });
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1);

    const auto & row = result->at(0);
    ASSERT_EQ(std::get< int >(row.at(0)), 1);                  // id
    ASSERT_EQ(std::get< std::string >(row.at(1)), "Alice");    // name
    ASSERT_EQ(std::get< int >(row.at(2)), 30);                 // age
    ASSERT_DOUBLE_EQ(std::get< double >(row.at(3)), 50000.50); // salary
    ASSERT_EQ(std::get< bool >(row.at(4)), true);              // active
}

TEST_F(SQLiteDatabaseTest, SelectData) {
    std::vector< cxx::Col > cols = getTestTableColumns();
    ASSERT_TRUE(db_.createTable("test_table", cols));

    // Multiple insert
    ASSERT_TRUE(db_.insert("test_table", { "id", "name", "age", "salary", "active" }, { "1", "Alice", "30", "50000.50", "1" }));
    ASSERT_TRUE(db_.insert("test_table", { "id", "name", "age", "salary", "active" }, { "2", "Bob", "25", "45000.75", "0" }));
    ASSERT_TRUE(db_.insert("test_table", { "id", "name", "age", "salary", "active" }, { "3", "Charlie", "35", "55000.25", "1" }));

    // Check data with column list
    auto result = db_.select("test_table", { "id", "name", "age" });
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3);

    // Check data with empty columns list
    result = db_.select("test_table", {});
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3);
    ASSERT_EQ(result->at(0).size(), 5); // 5 cols
}

TEST_F(SQLiteDatabaseTest, UpdateData) {
    std::vector< cxx::Col > cols = getTestTableColumns();
    ASSERT_TRUE(db_.createTable("test_table", cols));

    ASSERT_TRUE(db_.insert("test_table", { "id", "name", "age", "salary", "active" }, { "1", "Alice", "30", "50000.50", "1" }));

    // Update date
    std::vector< std::pair< std::string, std::string > > updates = {
        {   "name", "Alice Updated" },
        {    "age",            "31" },
        { "active",             "0" }
    };

    ASSERT_TRUE(db_.update("test_table", updates, "id = 1"));

    // Check data updated
    auto result = db_.select("test_table", { "name", "age", "active" });
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1);

    const auto & row = result->at(0);
    ASSERT_EQ(std::get< std::string >(row.at(0)), "Alice Updated"); // name
    ASSERT_EQ(std::get< int >(row.at(1)), 31);                      // age
    ASSERT_EQ(std::get< bool >(row.at(2)), false);                  // active
}

TEST_F(SQLiteDatabaseTest, DeleteData) {
    std::vector< cxx::Col > cols = getTestTableColumns();
    ASSERT_TRUE(db_.createTable("test_table", cols));

    // Multiple instert
    ASSERT_TRUE(db_.insert("test_table", { "id", "name", "age" }, { "1", "Alice", "30" }));
    ASSERT_TRUE(db_.insert("test_table", { "id", "name", "age" }, { "2", "Bob", "25" }));
    ASSERT_TRUE(db_.insert("test_table", { "id", "name", "age" }, { "3", "Charlie", "35" }));

    auto result = db_.select("test_table", { "id" });
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3);

    // Delete one
    ASSERT_TRUE(db_.deleteFrom("test_table", "id = 2"));
    // Check deletion
    result = db_.select("test_table", { "id" });
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2);

    // Delete all
    ASSERT_TRUE(db_.deleteFrom("test_table", ""));

    // Check empty table
    result = db_.select("test_table", { "id" });
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 0);
}

// Тест multiple types
TEST_F(SQLiteDatabaseTest, DataTypes) {
    std::vector< cxx::Col > cols = {
        {       "int_val",   cxx::Col::EDataType::INTEGER },
        {      "real_val",      cxx::Col::EDataType::REAL },
        {      "text_val",      cxx::Col::EDataType::TEXT },
        {      "bool_val",   cxx::Col::EDataType::BOOLEAN },
        {      "date_val",      cxx::Col::EDataType::DATE },
        { "timestamp_val", cxx::Col::EDataType::TIMESTAMP }
    };

    ASSERT_TRUE(db_.createTable("types_test", cols));

    // Check insert multiple types
    ASSERT_TRUE(db_.insert("types_test", { "int_val", "real_val", "text_val", "bool_val", "date_val", "timestamp_val" }, { "42", "3.14", "hello", "1", "2023-07-15", "2023-07-15 14:30:00" }));

    // Check data
    auto result = db_.select("types_test", {});
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1);

    const auto & row = result->at(0);
    ASSERT_EQ(std::get< int >(row.at(0)), 42);
    ASSERT_DOUBLE_EQ(std::get< double >(row.at(1)), 3.14);
    ASSERT_EQ(std::get< std::string >(row.at(2)), "hello");
    ASSERT_EQ(std::get< bool >(row.at(3)), true);
    ASSERT_EQ(std::get< std::string >(row.at(4)), "2023-07-15");
    ASSERT_EQ(std::get< std::string >(row.at(5)), "2023-07-15 14:30:00");
}

TEST_F(SQLiteDatabaseTest, Constraints) {
    // Testing PRIMARY KEY constraint
    std::vector< cxx::Col > pkCols = {
        { "id", cxx::Col::EDataType::INTEGER, cxx::Col::EConstraint::PRIMARY_KEY },
        { "name", cxx::Col::EDataType::TEXT }
    };

    ASSERT_TRUE(db_.createTable("pk_test", pkCols));
    ASSERT_TRUE(db_.insert("pk_test", { "id", "name" }, { "1", "Test" }));

    // Try insert dublicate key
    ASSERT_FALSE(db_.insert("pk_test", { "id", "name" }, { "1", "Test Duplicate" }));

    // Testing NOT NULL
    std::vector< cxx::Col > nnCols = {
        { "id", cxx::Col::EDataType::INTEGER },
        { "name", cxx::Col::EDataType::TEXT, cxx::Col::EConstraint::NOT_NULL }
    };

    ASSERT_TRUE(db_.createTable("nn_test", nnCols));
    // Try insert without NOT NULL
    ASSERT_FALSE(db_.insert("nn_test", { "id" }, { "1" }));

    // Testing UNIQUE
    std::vector< cxx::Col > uqCols = {
        { "id", cxx::Col::EDataType::INTEGER },
        { "code", cxx::Col::EDataType::TEXT, cxx::Col::EConstraint::UNIQUE }
    };

    ASSERT_TRUE(db_.createTable("uq_test", uqCols));
    ASSERT_TRUE(db_.insert("uq_test", { "id", "code" }, { "1", "ABC" }));
    ASSERT_FALSE(db_.insert("uq_test", { "id", "code" }, { "2", "ABC" }));
}

TEST_F(SQLiteDatabaseTest, ErrorHandling) {
    // Select non existent table
    auto result = db_.select("non_existent_table", {});
    ASSERT_FALSE(result.has_value());

    // Drop non existent table
    ASSERT_FALSE(db_.dropTable("non_existent_table"));

    // Temp table
    std::vector< cxx::Col > cols = {
        { "id", cxx::Col::EDataType::INTEGER, cxx::Col::EConstraint::PRIMARY_KEY },
        { "name", cxx::Col::EDataType::TEXT }
    };
    ASSERT_TRUE(db_.createTable("error_test", cols));

    // Update with wrong condition
    std::vector< std::pair< std::string, std::string > > updates = {
        { "name", "Test" }
    };
    ASSERT_FALSE(db_.update("error_test", updates, "non_existent_column = 1"));

    // Delete with wrong condition
    ASSERT_FALSE(db_.deleteFrom("error_test", "non_existent_column = 1"));
}
