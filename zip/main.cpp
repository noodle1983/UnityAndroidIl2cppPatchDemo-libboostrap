#include "ZipFile.h"
#include "shadow_zip.h"
#include "ZipEntry.h"
#include <vector>
using namespace android;


int main()
{
    ShadowZip::init(".", "./UnityPlayerActivity.apk");
    ShadowZip::old_fopen = ::fopen;
    ShadowZip::old_fseek = ::fseek;
    ShadowZip::old_ftell = ::ftell;
    ShadowZip::old_rewind = ::rewind;
    ShadowZip::old_fread = ::fread;
    ShadowZip::old_fclose = ::fclose;

    ShadowZip test;
    FILE* fr = test.fopen();
    FILE* fw = fopen( "test.apk", "wb" );
    char buffer[1024] = {0};
    while(1)
    {
        int read_cnt = test.fread( buffer, 1, sizeof(buffer), fr );
        if (read_cnt <= 0){break;}
        int write_cnt = fwrite( buffer, 1, read_cnt, fw );
        if (write_cnt != read_cnt )
        {
            MY_LOG("write error %d != %d", write_cnt, read_cnt);
            break;
        }
    }
    MY_LOG("copy end at %d, %d", ftell(fw), (size_t)ShadowZip::get_eof());
    test.fclose(fr);
    fclose(fw);
    return 0;
}


