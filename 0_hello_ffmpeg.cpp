
//
// Created by William.Hua on 2020/9/12.
//

#if defined(__cplusplus)
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#if defined(__cplusplus)
}
#endif

#include <iostream>

using namespace std;

int main()
{
    cout << "avcodec_configuration : " << avcodec_configuration() << endl;
    return 0;
}