
#include "json/json.h"

#include "thirdparty/gtest/gtest.h"

TEST(JsonCpp, Const) {
  const Json::Value json;
  const Json::Value& jsonApi = json["api"];
  EXPECT_EQ(true, jsonApi.isNull());
}
