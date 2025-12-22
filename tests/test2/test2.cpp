//
// Created by Remilia on 25-12-21.
//
#include <utils/glogWrapper.h>
#include "TestApplication.h"

int main(int argc, char *argv[])
{
    GlogWrapper glogW = GlogWrapper(argv[0]);
    LOG(INFO) << "Program Start";
    TestApplication testApp = TestApplication(argv[0]);
    testApp.init();
    testApp.run();
}