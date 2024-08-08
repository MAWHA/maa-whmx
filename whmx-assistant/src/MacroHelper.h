/* Copyright 2024 周上行Ryer

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#define MH_EXPAND(x) x

#define MH_STRINGIFY_IMPL(x)    #x
#define MH_STRINGIFY_WRAPPER(x) MH_STRINGIFY_IMPL(x)
#define MH_STRINGIFY(x)         MH_STRINGIFY_WRAPPER(x)

#define MH_CONCAT_IMPL(x, y)    x##y
#define MH_CONCAT_WRAPPER(x, y) MH_CONCAT_IMPL(x, y)
#define MH_CONCAT(x, y)         MH_CONCAT_WRAPPER(x, y)

#define MH_NARG_IMPL( \
    _0,               \
    _1,               \
    _2,               \
    _3,               \
    _4,               \
    _5,               \
    _6,               \
    _7,               \
    _8,               \
    _9,               \
    _10,              \
    _11,              \
    _12,              \
    _13,              \
    _14,              \
    _15,              \
    _16,              \
    _17,              \
    _18,              \
    _19,              \
    _20,              \
    _21,              \
    _22,              \
    _23,              \
    _24,              \
    _25,              \
    _26,              \
    _27,              \
    _28,              \
    _29,              \
    _30,              \
    _31,              \
    _32,              \
    N,                \
    ...)              \
    N
#define MH_NARG(...)        \
    MH_EXPAND(MH_NARG_IMPL( \
        0,                  \
        ##__VA_ARGS__,      \
        32,                 \
        31,                 \
        30,                 \
        29,                 \
        28,                 \
        27,                 \
        26,                 \
        25,                 \
        24,                 \
        23,                 \
        22,                 \
        21,                 \
        20,                 \
        19,                 \
        18,                 \
        17,                 \
        16,                 \
        15,                 \
        14,                 \
        13,                 \
        12,                 \
        11,                 \
        10,                 \
        9,                  \
        8,                  \
        7,                  \
        6,                  \
        5,                  \
        4,                  \
        3,                  \
        2,                  \
        1,                  \
        0))

#define MH_FOREACH_IMPL0(pred, ...)
#define MH_FOREACH_IMPL1(pred, x, ...)  pred(x)
#define MH_FOREACH_IMPL2(pred, x, ...)  pred(x) MH_EXPAND(MH_FOREACH_IMPL1(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL3(pred, x, ...)  pred(x) MH_EXPAND(MH_FOREACH_IMPL2(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL4(pred, x, ...)  pred(x) MH_EXPAND(MH_FOREACH_IMPL3(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL5(pred, x, ...)  pred(x) MH_EXPAND(MH_FOREACH_IMPL4(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL6(pred, x, ...)  pred(x) MH_EXPAND(MH_FOREACH_IMPL5(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL7(pred, x, ...)  pred(x) MH_EXPAND(MH_FOREACH_IMPL6(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL8(pred, x, ...)  pred(x) MH_EXPAND(MH_FOREACH_IMPL7(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL9(pred, x, ...)  pred(x) MH_EXPAND(MH_FOREACH_IMPL8(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL10(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL9(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL11(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL10(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL12(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL11(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL13(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL12(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL14(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL13(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL15(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL14(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL16(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL15(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL17(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL16(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL18(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL17(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL19(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL18(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL20(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL19(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL21(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL20(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL22(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL21(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL23(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL22(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL24(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL23(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL25(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL24(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL26(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL25(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL27(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL26(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL28(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL27(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL29(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL28(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL30(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL29(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL31(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL30(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL32(pred, x, ...) pred(x) MH_EXPAND(MH_FOREACH_IMPL31(pred, __VA_ARGS__))
#define MH_FOREACH_IMPL(N, pred, ...)   MH_EXPAND(MH_CONCAT(MH_FOREACH_IMPL, N)(pred, __VA_ARGS__))
#define MH_FOREACH(pred, ...)           MH_EXPAND(MH_FOREACH_IMPL(MH_EXPAND(MH_NARG(__VA_ARGS__)), pred, __VA_ARGS__))
