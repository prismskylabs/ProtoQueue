#include <gtest/gtest.h>

#include <future>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include <pq.h>

#include "basic.pb.h"


TEST(BindTests, DoubleBindTest) {
    auto bound = pq::Bind<Basic>();
    auto thrown = false;
    try {
        auto another_bound = pq::Bind<Basic>(bound.get_port());
    } catch (const std::exception& e) {
        thrown = true;
        EXPECT_EQ(std::string{"Address already in use"}, std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST(BindTests, DoubleBindAddressTest) {
    auto bound = pq::Bind<Basic>();
    auto thrown = false;
    try {
        auto another_bound = pq::Bind<Basic>(bound.get_address());
    } catch (const std::exception& e) {
        thrown = true;
        EXPECT_EQ(std::string{"Address already in use"}, std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST(BindTests, BindPortTest) {
    auto bound = pq::Bind<Basic>(Port{43561});
    EXPECT_EQ(Port{43561}.value, bound.get_port().value);
}

TEST(BindTests, BindAddressTest) {
    auto bound = pq::Bind<Basic>(Address{"tcp://0.0.0.0:43562"});
    EXPECT_EQ(Address{"tcp://0.0.0.0:43562"}.value, bound.get_address().value);
}

TEST(ConnectTests, ConnectBadPortTest) {
    auto thrown = false;
    try {
        auto connected = pq::Connect<Basic>(Port{0});
    } catch (const std::exception& e) {
        thrown = true;
        EXPECT_EQ(std::string{"Socket port must be above 0"}, std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST(ConnectTests, ConnectBadNegativePortTest) {
    auto thrown = false;
    try {
        auto connected = pq::Connect<Basic>(Port{-12345});
    } catch (const std::exception& e) {
        thrown = true;
        EXPECT_EQ(std::string{"Socket port must be above 0"}, std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST(PairTests, DefaultPairTest) {
    auto bound = pq::Bind<Basic>(); // Defaults to Pair
    auto connected = pq::Connect<Basic>(bound.get_port()); // Defaults to Pair
    {
        auto basic = connected.Receive(false);
        EXPECT_FALSE(basic.IsInitialized());
    }
}

TEST(PairTests, PairNoneTest) {
    auto bound = pq::Bind<Basic>(Type{ZMQ_PAIR});
    auto connected = pq::Connect<Basic>(bound.get_port(), Type{ZMQ_PAIR});
    {
        auto basic = connected.Receive(false);
        EXPECT_FALSE(basic.IsInitialized());
    }
}

TEST(PairTests, PairReverseNoneTest) {
    auto bound = pq::Bind<Basic>(Type{ZMQ_PAIR});
    auto connected = pq::Connect<Basic>(bound.get_port(), Type{ZMQ_PAIR});
    {
        auto basic = bound.Receive(false);
        EXPECT_FALSE(basic.IsInitialized());
    }
}

TEST(PairTests, PairTest) {
    auto bound = pq::Bind<Basic>(Type{ZMQ_PAIR});
    auto connected = pq::Connect<Basic>(bound.get_port(), Type{ZMQ_PAIR});
    {
        Basic basic;
        basic.set_value("Hello world");
        bound.Send(basic);
    }
    {
        auto basic = connected.Receive();
        EXPECT_EQ(std::string{"Hello world"}, basic.value());
    }
}

TEST(PairTests, PairReverseTest) {
    auto bound = pq::Bind<Basic>(Type{ZMQ_PAIR});
    auto connected = pq::Connect<Basic>(bound.get_port(), Type{ZMQ_PAIR});
    {
        Basic basic;
        basic.set_value("Hello world");
        connected.Send(basic);
    }
    {
        auto basic = bound.Receive();
        EXPECT_EQ(std::string{"Hello world"}, basic.value());
    }
}

TEST(PairTests, PairNonblockingTest) {
    auto bound = pq::Bind<Basic>(Type{ZMQ_PAIR});
    auto connected = pq::Connect<Basic>(bound.get_port(), Type{ZMQ_PAIR});
    {
        Basic basic;
        basic.set_value("Hello world");
        bound.Send(basic);
    }
    {
        while (true) {
            auto basic = connected.Receive(false);
            if (basic.IsInitialized()) {
                EXPECT_EQ(std::string{"Hello world"}, basic.value());
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

TEST(PairTests, PairReverseNonblockingTest) {
    auto bound = pq::Bind<Basic>(Type{ZMQ_PAIR});
    auto connected = pq::Connect<Basic>(bound.get_port(), Type{ZMQ_PAIR});
    {
        Basic basic;
        basic.set_value("Hello world");
        connected.Send(basic);
    }
    {
        while (true) {
            auto basic = bound.Receive(false);
            if (basic.IsInitialized()) {
                EXPECT_EQ(std::string{"Hello world"}, basic.value());
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

TEST(PushPullTests, PushPullTest) {
    auto push = pq::Bind<Basic>(Type{ZMQ_PUSH});
    auto pull = pq::Connect<Basic>(push.get_port(), Type{ZMQ_PULL});
    {
        Basic basic;
        basic.set_value("Hello world");
        push.Send(basic);
    }
    {
        auto basic = pull.Receive();
        EXPECT_EQ(std::string{"Hello world"}, basic.value());
    }
}

TEST(PushPullTests, PushNoReceiveTest) {
    auto push = pq::Bind<Basic>(Type{ZMQ_PUSH});
    auto pull = pq::Connect<Basic>(push.get_port(), Type{ZMQ_PULL});
    auto thrown = false;
    try {
        auto basic = push.Receive();
    } catch (const std::exception& e) {
        thrown = true;
        EXPECT_EQ(std::string{"Operation not supported"},
                  std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST(PushPullTests, PullNoSendTest) {
    auto push = pq::Bind<Basic>(Type{ZMQ_PUSH});
    auto pull = pq::Connect<Basic>(push.get_port(), Type{ZMQ_PULL});
    auto thrown = false;
    Basic basic;
    basic.set_value("Hello world");
    try {
        pull.Send(basic);
    } catch (const std::exception& e) {
        thrown = true;
        EXPECT_EQ(std::string{"Operation not supported"}, std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST(PubSubTests, PubSubTest) {
    auto pub = pq::Bind<Basic>(Type{ZMQ_PUB});
    auto sub = pq::Connect<Basic>(pub.get_port(), Type{ZMQ_SUB});

    auto response = std::async(std::launch::async,
            [] (ProtoQueue<Basic>& sub) {
                return sub.Receive().value();
            }, std::ref(sub));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    Basic basic;
    basic.set_value("Hello world");
    pub.Send(basic);
    EXPECT_EQ(std::string{"Hello world"}, response.get());
}

TEST(PubSubTests, PubManySubTest) {
    auto pub = pq::Bind<Basic>(Type{ZMQ_PUB});
    auto sub1 = pq::Connect<Basic>(pub.get_port(), Type{ZMQ_SUB});
    auto sub2 = pq::Connect<Basic>(pub.get_port(), Type{ZMQ_SUB});
    auto sub3 = pq::Connect<Basic>(pub.get_port(), Type{ZMQ_SUB});

    auto lambda = [] (ProtoQueue<Basic>& sub) {
                      return sub.Receive().value();
                  };
    auto response1= std::async(std::launch::async, lambda, std::ref(sub1));
    auto response2= std::async(std::launch::async, lambda, std::ref(sub2));
    auto response3= std::async(std::launch::async, lambda, std::ref(sub3));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    Basic basic;
    basic.set_value("Hello world");
    pub.Send(basic);
    EXPECT_EQ(std::string{"Hello world"}, response1.get());
    EXPECT_EQ(std::string{"Hello world"}, response2.get());
    EXPECT_EQ(std::string{"Hello world"}, response3.get());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    return RUN_ALL_TESTS();
}
