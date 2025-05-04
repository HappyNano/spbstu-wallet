#include "utils/http/client/interface/i_http_client.h"
#include <utils/http/client/curl/curl_http_client.h>
#include <utils/http/server/test/test_http_server.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <map>
#include <string>

using namespace cxx;

namespace {

    // Test fixture for CurlHttpClient tests
    class CurlHttpClientTest: public ::testing::Test {
    protected:
        void SetUp() override {
            testServer_ = std::make_unique< TestHttpServer >(8080);

            // Set up default response
            testServer_->setResponse(
             200,
             "Test Response",
             {
              {  "Content-Type", "text/plain" },
              { "X-Test-Header", "test-value" }
            });

            // Create client with test settings
            CurlHttpClient::Settings settings;
            settings.followRedirects = true;
            settings.verifySSL = false; // Disable SSL verification for tests
            settings.connectTimeout = 2;
            settings.timeout = 5;
            settings.verbose = false;

            IHttpClient::Headers initialHeaders = {
                { "User-Agent", "TestAgent/1.0" }
            };
            client_ = std::make_unique< CurlHttpClient >(settings, initialHeaders);
        }

        void TearDown() override {
            client_.reset();
            testServer_.reset();
        }

    protected:
        std::unique_ptr< TestHttpServer > testServer_;
        std::unique_ptr< CurlHttpClient > client_;
    };

} // unnamed namespace

// Tests for GET method
TEST_F(CurlHttpClientTest, GetRequestWithNoHeaders) {
    // Perform GET request
    auto response = client_->get(testServer_->getBaseUrl() + "/test");

    // Verify response
    EXPECT_EQ(200, response.statusCode);
    EXPECT_EQ("Test Response", response.body);
    EXPECT_EQ("text/plain", response.headers["Content-Type"]);
    EXPECT_EQ("test-value", response.headers["X-Test-Header"]);

    // Verify request
    EXPECT_TRUE(testServer_->wasRequestReceived());
    EXPECT_EQ("GET", testServer_->getLastRequestMethod());
    EXPECT_EQ("/test", testServer_->getLastRequestPath());
    EXPECT_TRUE(testServer_->getLastRequestBody().empty());
    EXPECT_EQ("TestAgent/1.0", testServer_->getLastRequestHeaders().at("User-Agent"));
}

TEST_F(CurlHttpClientTest, GetRequestWithCustomHeaders) {
    // Perform GET request with custom headers
    IHttpClient::Headers headers = {
        { "X-Custom-Header",     "custom-value" },
        {          "Accept", "application/json" }
    };

    auto response = client_->get(testServer_->getBaseUrl() + "/test", headers);

    // Verify response
    EXPECT_EQ(200, response.statusCode);

    // Verify request headers
    EXPECT_TRUE(testServer_->wasRequestReceived());
    EXPECT_EQ("custom-value", testServer_->getLastRequestHeaders().at("X-Custom-Header"));
    EXPECT_EQ("application/json", testServer_->getLastRequestHeaders().at("Accept"));
    EXPECT_EQ("TestAgent/1.0", testServer_->getLastRequestHeaders().at("User-Agent"));
}

// Tests for POST method
TEST_F(CurlHttpClientTest, PostRequestWithJsonBody) {
    // Set up test server
    testServer_->setResponse(201, "{\"id\": 123}", {
                                                    { "Content-Type", "application/json" }
    });

    // Perform POST request
    std::string jsonBody = R"({"name": "Test", "value": 42})";
    IHttpClient::Headers headers = {
        { "Content-Type", "application/json" }
    };

    auto response = client_->post(testServer_->getBaseUrl() + "/resource", jsonBody, headers);

    // Verify response
    EXPECT_EQ(201, response.statusCode);
    EXPECT_EQ("{\"id\": 123}", response.body);
    EXPECT_EQ("application/json", response.headers["Content-Type"]);

    // Verify request
    EXPECT_TRUE(testServer_->wasRequestReceived());
    EXPECT_EQ("POST", testServer_->getLastRequestMethod());
    EXPECT_EQ("/resource", testServer_->getLastRequestPath());
    EXPECT_EQ(jsonBody, testServer_->getLastRequestBody());
    EXPECT_EQ("application/json", testServer_->getLastRequestHeaders().at("Content-Type"));
}

// Tests for PUT method
TEST_F(CurlHttpClientTest, PutRequestWithBody) {
    // Set up test server
    testServer_->setResponse(200, "Updated", {
                                              { "Content-Type", "text/plain" }
    });

    // Perform PUT request
    std::string body = "Updated resource data";

    auto response = client_->put(testServer_->getBaseUrl() + "/resource/123", body);

    // Verify response
    EXPECT_EQ(200, response.statusCode);
    EXPECT_EQ("Updated", response.body);

    // Verify request
    EXPECT_TRUE(testServer_->wasRequestReceived());
    EXPECT_EQ("PUT", testServer_->getLastRequestMethod());
    EXPECT_EQ("/resource/123", testServer_->getLastRequestPath());
    EXPECT_EQ(body, testServer_->getLastRequestBody());
}

// Tests for DELETE method
TEST_F(CurlHttpClientTest, DeleteRequest) {
    // Set up test server
    testServer_->setResponse(204, "");

    // Perform DELETE request
    auto response = client_->del(testServer_->getBaseUrl() + "/resource/123");

    // Verify response
    EXPECT_EQ(204, response.statusCode);
    EXPECT_TRUE(response.body.empty());

    // Verify request
    EXPECT_TRUE(testServer_->wasRequestReceived());
    EXPECT_EQ("DELETE", testServer_->getLastRequestMethod());
    EXPECT_EQ("/resource/123", testServer_->getLastRequestPath());
    EXPECT_TRUE(testServer_->getLastRequestBody().empty());
}

// Test for error responses
TEST_F(CurlHttpClientTest, ErrorResponse) {
    // Set up test server to return an error
    testServer_->setResponse(404, "Not Found", {
                                                { "Content-Type", "text/plain" }
    });

    // Perform request
    auto response = client_->get(testServer_->getBaseUrl() + "/nonexistent");

    // Verify response
    EXPECT_EQ(404, response.statusCode);
    EXPECT_EQ("Not Found", response.body);
    EXPECT_EQ("text/plain", response.headers["Content-Type"]);
}

// Test for network errors
TEST_F(CurlHttpClientTest, NetworkError) {
    // Try to connect to a non-existent server
    bool exceptionThrown = false;
    try {
        client_->get("http://non-existent-server.invalid/test");
    } catch (const std::exception & e) {
        exceptionThrown = true;
        std::string errorMsg = e.what();
        EXPECT_TRUE(errorMsg.find("curl_easy_perform() failed") != std::string::npos);
    }
    EXPECT_TRUE(exceptionThrown);
}

// Test for initialHeaders being properly sent with every request
TEST_F(CurlHttpClientTest, InitialHeadersAreSentWithEveryRequest) {
    // First request
    client_->get(testServer_->getBaseUrl() + "/test1");
    EXPECT_TRUE(testServer_->wasRequestReceived());
    EXPECT_EQ("TestAgent/1.0", testServer_->getLastRequestHeaders().at("User-Agent"));

    // Reset test server state
    testServer_.reset();
    testServer_ = std::make_unique< TestHttpServer >(8080);
    testServer_->setResponse(200, "Test Response");

    // Second request
    client_->post(testServer_->getBaseUrl() + "/test2", "test body");
    EXPECT_TRUE(testServer_->wasRequestReceived());
    EXPECT_EQ("TestAgent/1.0", testServer_->getLastRequestHeaders().at("User-Agent"));

    // Reset test server state
    testServer_.reset();
    testServer_ = std::make_unique< TestHttpServer >(8080);
    testServer_->setResponse(200, "Test Response");

    // Third request with custom headers shouldn't override initialHeaders
    IHttpClient::Headers customHeaders = {
        { "X-Custom", "custom-value" }
    };
    client_->get(testServer_->getBaseUrl() + "/test3", customHeaders);
    EXPECT_TRUE(testServer_->wasRequestReceived());
    EXPECT_EQ("TestAgent/1.0", testServer_->getLastRequestHeaders().at("User-Agent"));
    EXPECT_EQ("custom-value", testServer_->getLastRequestHeaders().at("X-Custom"));
}

// Test for custom headers overriding initialHeaders
TEST_F(CurlHttpClientTest, CustomHeadersOverrideInitialHeaders) {
    // Create client with initial User-Agent header
    CurlHttpClient::Settings settings;
    IHttpClient::Headers initialHeaders = {
        { "User-Agent", "InitialAgent/1.0" }
    };
    auto testClient = std::make_unique< CurlHttpClient >(settings, initialHeaders);

    // Perform request with custom User-Agent header
    IHttpClient::Headers customHeaders = {
        { "User-Agent", "CustomAgent/1.0" }
    };
    testClient->get(testServer_->getBaseUrl() + "/test", customHeaders);

    // Verify the custom header overrode the initial header
    EXPECT_TRUE(testServer_->wasRequestReceived());
    EXPECT_EQ("CustomAgent/1.0", testServer_->getLastRequestHeaders().at("User-Agent"));
}

// Test with large response body
TEST_F(CurlHttpClientTest, LargeResponseBody) {
    // Generate a large response body (1MB)
    std::string largeBody(1024 * 1024, 'X');
    testServer_->setResponse(200, largeBody);

    // Perform request
    auto response = client_->get(testServer_->getBaseUrl() + "/large");

    // Verify response
    EXPECT_EQ(200, response.statusCode);
    EXPECT_EQ(largeBody.size(), response.body.size());
    EXPECT_EQ(largeBody, response.body);
}
