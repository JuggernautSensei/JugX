#include <gtest/gtest.h>

#include <JugX/ConsoleLogger.h>
#include <JugX/EnumArray.h>
#include <JugX/FileLogger.h>
#include <JugX/MemoryLogger.h>
#include <JugX/Scoped.h>
#include <JugX/GroupLogger.h>
using namespace jug;

int main()
{
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}