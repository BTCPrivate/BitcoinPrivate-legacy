#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "clientversion.h"
#include "deprecation.h"
#include "init.h"
#include "ui_interface.h"
#include "util.h"

using ::testing::StrictMock;

static const std::string CLIENT_VERSION_STR = FormatVersion(CLIENT_VERSION);
extern std::atomic<bool> fRequestShutdown;

class MockUIInterface {
public:
    MOCK_METHOD3(ThreadSafeMessageBox, bool(const std::string& message,
                                      const std::string& caption,
                                      unsigned int style));
};

static bool ThreadSafeMessageBox(MockUIInterface *mock,
                                 const std::string& message,
                                 const std::string& caption,
                                 unsigned int style)
{
    mock->ThreadSafeMessageBox(message, caption, style);
}

class DeprecationTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        uiInterface.ThreadSafeMessageBox.disconnect_all_slots();
        uiInterface.ThreadSafeMessageBox.connect(boost::bind(ThreadSafeMessageBox, &mock_, _1, _2, _3));
    }

    virtual void TearDown() {
        fRequestShutdown = false;
        mapArgs["-disabledeprecation"] = "";
    }

    StrictMock<MockUIInterface> mock_;
};

TEST_F(DeprecationTest, NonDeprecatedNodeKeepsRunning) {
    EXPECT_FALSE(ShutdownRequested());
    EnforceNodeDeprecation(DEPRECATION_HEIGHT - DEPRECATION_WARN_LIMIT - 1);
    EXPECT_FALSE(ShutdownRequested());
}

TEST_F(DeprecationTest, NodeNearDeprecationIsWarned) {
    EXPECT_FALSE(ShutdownRequested());
    EXPECT_CALL(mock_, ThreadSafeMessageBox(::testing::_, "", CClientUIInterface::MSG_WARNING));
    EnforceNodeDeprecation(DEPRECATION_HEIGHT - DEPRECATION_WARN_LIMIT);
    EXPECT_FALSE(ShutdownRequested());
}

TEST_F(DeprecationTest, NodeNearDeprecationWarningIsNotDuplicated) {
    EXPECT_FALSE(ShutdownRequested());
    EnforceNodeDeprecation(DEPRECATION_HEIGHT - DEPRECATION_WARN_LIMIT + 1);
    EXPECT_FALSE(ShutdownRequested());
}

TEST_F(DeprecationTest, NodeNearDeprecationWarningIsRepeatedOnStartup) {
    EXPECT_FALSE(ShutdownRequested());
    EXPECT_CALL(mock_, ThreadSafeMessageBox(::testing::_, "", CClientUIInterface::MSG_WARNING));
    EnforceNodeDeprecation(DEPRECATION_HEIGHT - DEPRECATION_WARN_LIMIT + 1, true);
    EXPECT_FALSE(ShutdownRequested());
}

TEST_F(DeprecationTest, DeprecatedNodeShutsDown) {
    EXPECT_FALSE(ShutdownRequested());
    EXPECT_CALL(mock_, ThreadSafeMessageBox(::testing::_, "", CClientUIInterface::MSG_ERROR));
    EnforceNodeDeprecation(DEPRECATION_HEIGHT);
    EXPECT_TRUE(ShutdownRequested());
}

TEST_F(DeprecationTest, DeprecatedNodeErrorIsNotDuplicated) {
    EXPECT_FALSE(ShutdownRequested());
    EnforceNodeDeprecation(DEPRECATION_HEIGHT + 1);
    EXPECT_TRUE(ShutdownRequested());
}

TEST_F(DeprecationTest, DeprecatedNodeErrorIsRepeatedOnStartup) {
    EXPECT_FALSE(ShutdownRequested());
    EXPECT_CALL(mock_, ThreadSafeMessageBox(::testing::_, "", CClientUIInterface::MSG_ERROR));
    EnforceNodeDeprecation(DEPRECATION_HEIGHT + 1, true);
    EXPECT_TRUE(ShutdownRequested());
}

TEST_F(DeprecationTest, DeprecatedNodeShutsDownIfOldVersionDisabled) {
    EXPECT_FALSE(ShutdownRequested());
    mapArgs["-disabledeprecation"] = "1.0.0";
    EXPECT_CALL(mock_, ThreadSafeMessageBox(::testing::_, "", CClientUIInterface::MSG_ERROR));
    EnforceNodeDeprecation(DEPRECATION_HEIGHT);
    EXPECT_TRUE(ShutdownRequested());
}

TEST_F(DeprecationTest, DeprecatedNodeKeepsRunningIfCurrentVersionDisabled) {
    EXPECT_FALSE(ShutdownRequested());
    mapArgs["-disabledeprecation"] = CLIENT_VERSION_STR;
    EXPECT_CALL(mock_, ThreadSafeMessageBox(::testing::_, "", CClientUIInterface::MSG_ERROR));
    EnforceNodeDeprecation(DEPRECATION_HEIGHT);
    EXPECT_FALSE(ShutdownRequested());
}