//
// Created by skybcyang on 2020/12/19.
//


#include "catch.hpp"
#include <iostream>
#include "maco/meta/ro_meta_data_.h"

namespace {
    using namespace ro_meta_data_;
    __CUB_2_stage_meta_table_(rw, ro,
                             (a, int),
                              (b, double[2]));
    TEST_CASE("test meta_") {
        rw meta_data;
        REQUIRE(meta_data.a_rw().present() == false);
        REQUIRE(meta_data.b_rw().present() == false);
        meta_data.a_rw().set(1);
        meta_data.b_rw().set({1, 2});
        REQUIRE(meta_data.a_rw().present() == true);
        REQUIRE(meta_data.b_rw().present() == true);
        {
            auto a = meta_data.a_rw().get();
            auto&& [b, len] = meta_data.b_rw().get();
            REQUIRE(a == 1);
            REQUIRE(len == 2);
            REQUIRE(b[0] == 1);
            REQUIRE(b[1] == 2);
        }
        const ro& meta = meta_data;
        {
            auto a = meta.a_ro().get();
            auto&& [b, len] = meta.b_ro().get();
            REQUIRE(a == 1);
            REQUIRE(len == 2);
            REQUIRE(b[0] == 1);
            REQUIRE(b[1] == 2);
        }
    }
}