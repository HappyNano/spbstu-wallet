#ifdef POSTGRES_TEST
#include <utils/database/postgres/psql_database.h>
#endif
#include <utils/database/sqlite/sqlite_database.h>

#include <gtest/gtest.h>

#include <memory>

using namespace cxx;

namespace {

    // Base test class
    class DatabaseTest: public ::testing::TestWithParam< std::function< std::shared_ptr< IDatabase >() > > {
    protected:
        std::shared_ptr< IDatabase > db_;

        void SetUp() override {
            db_ = GetParam()();
            ASSERT_TRUE(db_ != nullptr);
        }

        void TearDown() override {
            if (db_) {
                db_->disconnect();
            }
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
    };

    std::shared_ptr< IDatabase > createSQLiteInMemory() {
        auto db = std::make_shared< SQLiteDatabase >();
        db->connect(SQLiteDatabase::InMemory{});
        return db;
    }

#ifdef POSTGRES_TEST
    std::shared_ptr< IDatabase > createPostgreSQL() {
        auto db = std::make_shared< cxx::PsqlDatabase >();

        PsqlDatabase::ConnectionInfo connInfo;
        connInfo.dbname = "test_db";
        connInfo.user = "postgres";
        connInfo.password = "postgres";
        connInfo.host = "localhost";
        connInfo.port = "5432";

        db->connect("dbname=postgres");
        return db;
    }
#endif

} // unnamed namespace

TEST_P(DatabaseTest, CreateAndDropTable) {
    ASSERT_TRUE(db_->createTable("test_table", getTestTableColumns()));

    auto result = db_->executeQuery("SELECT name FROM sqlite_master WHERE type='table' AND name='test_table'");
    ASSERT_TRUE(result.has_value());
    ASSERT_FALSE(result->empty());

    ASSERT_TRUE(db_->dropTable("test_table"));

    result = db_->executeQuery("SELECT name FROM sqlite_master WHERE type='table' AND name='test_table'");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->empty());
}

TEST_P(DatabaseTest, InsertData) {
    ASSERT_TRUE(db_->createTable("test_table", getTestTableColumns()));

    ASSERT_TRUE(db_->insert("test_table", { "id", "name", "age", "salary", "active" }, { "1", "Alice", "30", "50000.50", "1" }));

    auto result = db_->select("test_table", { "id", "name", "age", "salary", "active" });
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1);

    const auto & row = result->at(0);
    ASSERT_EQ(std::get< int >(row.at(0)), 1);                  // id
    ASSERT_EQ(std::get< std::string >(row.at(1)), "Alice");    // name
    ASSERT_EQ(std::get< int >(row.at(2)), 30);                 // age
    ASSERT_DOUBLE_EQ(std::get< double >(row.at(3)), 50000.50); // salary
    ASSERT_EQ(std::get< bool >(row.at(4)), true);              // active
}

TEST_P(DatabaseTest, SelectData) {
    ASSERT_TRUE(db_->createTable("test_table", getTestTableColumns()));

    // Multiple insert
    ASSERT_TRUE(db_->insert("test_table", { "id", "name", "age", "salary", "active" }, { "1", "Alice", "30", "50000.50", "1" }));
    ASSERT_TRUE(db_->insert("test_table", { "id", "name", "age", "salary", "active" }, { "2", "Bob", "25", "45000.75", "0" }));
    ASSERT_TRUE(db_->insert("test_table", { "id", "name", "age", "salary", "active" }, { "3", "Charlie", "35", "55000.25", "1" }));

    // Check data with column list
    auto result = db_->select("test_table", { "id", "name", "age" });
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3);

    // Check data with empty columns list
    result = db_->select("test_table", {});
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3);
    ASSERT_EQ(result->at(0).size(), 5); // 5 cols
}

TEST_P(DatabaseTest, UpdateData) {
    ASSERT_TRUE(db_->createTable("test_table", getTestTableColumns()));

    ASSERT_TRUE(db_->insert("test_table", { "id", "name", "age", "salary", "active" }, { "1", "Alice", "30", "50000.50", "1" }));

    // Update date
    std::vector< std::pair< std::string, std::string > > updates = {
        {   "name", "Alice Updated" },
        {    "age",            "31" },
        { "active",             "0" }
    };

    ASSERT_TRUE(db_->update("test_table", updates, "id = 1"));

    // Check data updated
    auto result = db_->select("test_table", { "name", "age", "active" });
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1);

    const auto & row = result->at(0);
    ASSERT_EQ(std::get< std::string >(row.at(0)), "Alice Updated"); // name
    ASSERT_EQ(std::get< int >(row.at(1)), 31);                      // age
    ASSERT_EQ(std::get< bool >(row.at(2)), false);                  // active
}

TEST_P(DatabaseTest, DeleteData) {
    ASSERT_TRUE(db_->createTable("test_table", getTestTableColumns()));

    // Multiple instert
    ASSERT_TRUE(db_->insert("test_table", { "id", "name", "age" }, { "1", "Alice", "30" }));
    ASSERT_TRUE(db_->insert("test_table", { "id", "name", "age" }, { "2", "Bob", "25" }));
    ASSERT_TRUE(db_->insert("test_table", { "id", "name", "age" }, { "3", "Charlie", "35" }));

    auto result = db_->select("test_table", { "id" });
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3);

    // Delete one
    ASSERT_TRUE(db_->deleteFrom("test_table", "id = 2"));
    // Check deletion
    result = db_->select("test_table", { "id" });
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2);

    // Delete all
    ASSERT_TRUE(db_->deleteFrom("test_table", ""));

    // Check empty table
    result = db_->select("test_table", { "id" });
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 0);
}

// Тест multiple types
TEST_P(DatabaseTest, DataTypes) {
    std::vector< cxx::Col > cols = {
        {       "int_val",   cxx::Col::EDataType::INTEGER },
        {      "real_val",      cxx::Col::EDataType::REAL },
        {      "text_val",      cxx::Col::EDataType::TEXT },
        {      "bool_val",   cxx::Col::EDataType::BOOLEAN },
        {      "date_val",      cxx::Col::EDataType::DATE },
        { "timestamp_val", cxx::Col::EDataType::TIMESTAMP }
    };

    ASSERT_TRUE(db_->createTable("types_test", cols));

    // Check insert multiple types
    ASSERT_TRUE(db_->insert(
     "types_test",
     { "int_val", "real_val", "text_val", "bool_val", "date_val", "timestamp_val" },
     { "42", "3.14", "hello", "1", "2023-07-15", "2023-07-15 14:30:00" }));

    // Check data
    auto result = db_->select("types_test", {});
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

TEST_P(DatabaseTest, Constraints) {
    // Testing PRIMARY KEY constraint
    std::vector< cxx::Col > pkCols = {
        { "id", cxx::Col::EDataType::INTEGER, cxx::Col::EConstraint::PRIMARY_KEY },
        { "name", cxx::Col::EDataType::TEXT }
    };

    ASSERT_TRUE(db_->createTable("pk_test", pkCols));
    ASSERT_TRUE(db_->insert("pk_test", { "id", "name" }, { "1", "Test" }));

    // Try insert dublicate key
    ASSERT_FALSE(db_->insert("pk_test", { "id", "name" }, { "1", "Test Duplicate" }));

    // Testing NOT NULL
    std::vector< cxx::Col > nnCols = {
        { "id", cxx::Col::EDataType::INTEGER },
        { "name", cxx::Col::EDataType::TEXT, cxx::Col::EConstraint::NOT_NULL }
    };

    ASSERT_TRUE(db_->createTable("nn_test", nnCols));
    // Try insert without NOT NULL
    ASSERT_FALSE(db_->insert("nn_test", { "id" }, { "1" }));

    // Testing UNIQUE
    std::vector< cxx::Col > uqCols = {
        { "id", cxx::Col::EDataType::INTEGER },
        { "code", cxx::Col::EDataType::TEXT, cxx::Col::EConstraint::UNIQUE }
    };

    ASSERT_TRUE(db_->createTable("uq_test", uqCols));
    ASSERT_TRUE(db_->insert("uq_test", { "id", "code" }, { "1", "ABC" }));
    ASSERT_FALSE(db_->insert("uq_test", { "id", "code" }, { "2", "ABC" }));
}

TEST_P(DatabaseTest, ErrorHandling) {
    // Select non existent table
    auto result = db_->select("non_existent_table", {});
    ASSERT_FALSE(result.has_value());

    // Drop non existent table
    ASSERT_FALSE(db_->dropTable("non_existent_table"));

    // Temp table
    std::vector< cxx::Col > cols = {
        { "id", cxx::Col::EDataType::INTEGER, cxx::Col::EConstraint::PRIMARY_KEY },
        { "name", cxx::Col::EDataType::TEXT }
    };
    ASSERT_TRUE(db_->createTable("error_test", cols));

    // Update with wrong condition
    std::vector< std::pair< std::string, std::string > > updates = {
        { "name", "Test" }
    };
    ASSERT_FALSE(db_->update("error_test", updates, "non_existent_column = 1"));

    // Delete with wrong condition
    ASSERT_FALSE(db_->deleteFrom("error_test", "non_existent_column = 1"));
}

TEST_P(DatabaseTest, CheckExistence) {
    ASSERT_TRUE(db_->createTable("test_table", getTestTableColumns()));

    EXPECT_TRUE(db_->isTableExist("test_table"));
    EXPECT_FALSE(db_->isTableExist("non_existent_table"));
}

INSTANTIATE_TEST_SUITE_P(
 SQLite,
 DatabaseTest,
 ::testing::Values(createSQLiteInMemory));

#ifdef POSTGRES_TEST
INSTANTIATE_TEST_SUITE_P(
 PostgreSQL,
 DatabaseTest,
 ::testing::Values(createPostgreSQL));
#endif

class SQLiteDatabaseTest: public DatabaseTest {
};

TEST_P(SQLiteDatabaseTest, ConnectInMemory) {
    auto db = std::reinterpret_pointer_cast< SQLiteDatabase >(db_);
    ASSERT_TRUE(db->isReady());
    db->disconnect();
    ASSERT_FALSE(db->isReady());
    ASSERT_TRUE(db->connect(SQLiteDatabase::InMemory{}));
    ASSERT_TRUE(db->isReady());
}

TEST_P(SQLiteDatabaseTest, ConnectFile) {
    auto db = std::reinterpret_pointer_cast< SQLiteDatabase >(db_);
    db->disconnect();
    ASSERT_TRUE(db->connect(":memory:"));
    ASSERT_TRUE(db->isReady());
}

INSTANTIATE_TEST_SUITE_P(
 SQLiteOnly,
 SQLiteDatabaseTest,
 ::testing::Values(createSQLiteInMemory));
