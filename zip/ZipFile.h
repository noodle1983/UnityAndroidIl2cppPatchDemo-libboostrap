/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// General-purpose Zip archive access.  This class allows both reading and
// writing to Zip archives, including deletion of existing entries.
//
#ifndef __LIBS_ZIPFILE_H
#define __LIBS_ZIPFILE_H

#include <utils/Errors.h>
#include <stdio.h>

#include "ZipEntry.h"

namespace android {

class EndOfCentralDir {
public:
    EndOfCentralDir(void) :
        mDiskNumber(0),
        mDiskWithCentralDir(0),
        mNumEntries(0),
        mTotalNumEntries(0),
        mCentralDirSize(0),
        mCentralDirOffset(0),
        mCommentLen(0),
        mComment(NULL)
        {}
    virtual ~EndOfCentralDir(void) {
        delete[] mComment;
    }

    status_t readBuf(const unsigned char* buf, int len);
    status_t write(FILE* fp);

    //unsigned long   mSignature;
    unsigned short  mDiskNumber;
    unsigned short  mDiskWithCentralDir;
    unsigned short  mNumEntries;
    unsigned short  mTotalNumEntries;
    unsigned long   mCentralDirSize;
    unsigned long   mCentralDirOffset;      // offset from first disk
    unsigned short  mCommentLen;
    unsigned char*  mComment;

    enum {
        kSignature      = 0x06054b50,
        kEOCDLen        = 22,       // EndOfCentralDir len, excl. comment

        kMaxCommentLen  = 65535,    // longest possible in ushort
        kMaxEOCDSearch  = kMaxCommentLen + EndOfCentralDir::kEOCDLen,

    };

    void dump(void) const;
};

}; // namespace android

#endif // __LIBS_ZIPFILE_H
