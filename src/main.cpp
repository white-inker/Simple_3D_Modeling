#include <utils/glogWrapper.h>

int main()
{
    GlogWrapper::Init("Simple_3D_Modeling");
    LOG(INFO) << "Program Start";

    GlogWrapper::Shutdown();
}