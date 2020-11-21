#include "shadow_zip.h"
#include "log.h"
#include "ZipEntry.h"
#include "ZipFile.h"
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <string.h>
#include <fstream>
#include <set>
#include <algorithm>

using namespace android;

class FileHandleReleaser
{
public:
	FileHandleReleaser(FILE* fp){fp_ = fp;}
	virtual ~FileHandleReleaser(){::fclose(fp_);}
private:
	FILE* fp_;	
};


static void get_files(const char* _apk_patch_path, std::vector<std::string>& _files)
{
    DIR* dir = opendir(_apk_patch_path);
    if (dir == NULL){ 
		MY_INFO("opendir failed:%d[%s]", errno, _apk_patch_path);
		return; 
	}

    struct dirent *ent = NULL;
    while((ent = readdir(dir)) != NULL) {  
        if(ent->d_type & DT_REG) {  
			std::string patch_file = std::string(_apk_patch_path) + "/" + ent->d_name;
            _files.push_back(patch_file);
			MY_INFO("patch file:[%s]", patch_file.c_str());
        }  
    }  
    closedir(dir);
}

static bool is_illegal_path_char(char ch){return ch == '\r' || ch == '\n' || ch == '"' || ch == '\'';}

static int parse_apk(const char* _path, std::vector<ZipEntry*>& _all_entries)
{
    off_t fileLength, seekStart;
    long readAmount;
    int i;
    EndOfCentralDir endOfCentralDir;

    FILE *fp = fopen(_path, "rb");
	if (fp == NULL){
		MY_ERROR("failure parse file %s", _path);
        return -1;
	}
	FileHandleReleaser autoDel(fp);
	
    fseek(fp, 0, SEEK_END);
    fileLength = ftell(fp);
    rewind(fp);

    /* too small to be a ZIP archive? */
    if (fileLength < EndOfCentralDir::kEOCDLen) {
        MY_ERROR("%s len is too small\n", _path);
        return -1;
    }

    unsigned char* buf = new unsigned char[EndOfCentralDir::kMaxEOCDSearch];
    if (buf == NULL) {
        MY_ERROR("failure allocating %d bytes", EndOfCentralDir::kMaxEOCDSearch);
        return -1;
    }

    if (fileLength > EndOfCentralDir::kMaxEOCDSearch) {
        seekStart = fileLength - EndOfCentralDir::kMaxEOCDSearch;
        readAmount = EndOfCentralDir::kMaxEOCDSearch;
    } else {
        seekStart = 0;
        readAmount = (long) fileLength;
    }
    if (fseek(fp, seekStart, SEEK_SET) != 0) {
        MY_ERROR("failure seeking to end of zip at %ld", (long) seekStart);
        delete[] buf;
        return -1;
    }

    /* read the last part of the file into the buffer */
    if (fread(buf, 1, readAmount, fp) != (size_t) readAmount) {
        MY_ERROR("read error! wanted %ld\n", readAmount);
        delete[] buf;
        return -1;
    }

    /* find the end-of-central-dir magic */
    for (i = readAmount - 4; i >= 0; i--) {
        if (buf[i] == 0x50 && ZipEntry::getLongLE(&buf[i]) == EndOfCentralDir::kSignature) {
            break;
        }
    }
    if (i < 0) {
        MY_ERROR("not zip:%s\n", _path);
        delete[] buf;
        return -1;
    }

    /* extract eocd values */
    int result = endOfCentralDir.readBuf(buf + i, readAmount - i);
    if (result != NO_ERROR) {
        MY_ERROR("failure reading %ld bytes for end of centoral dir", readAmount - i);
        delete[] buf;
        return -1;
    }
    //endOfCentralDir.dump();

    if (endOfCentralDir.mDiskNumber != 0 || endOfCentralDir.mDiskWithCentralDir != 0 ||
        endOfCentralDir.mNumEntries != endOfCentralDir.mTotalNumEntries)
    {
        MY_ERROR("archive spanning not supported\n");
        delete[] buf;
        return -1;
    }

    /*
     * So far so good.  "mCentralDirSize" is the size in bytes of the
     * central directory, so we can just seek back that far to find it.
     * We can also seek forward mCentralDirOffset bytes from the
     * start of the file.
     *
     * We're not guaranteed to have the rest of the central dir in the
     * buffer, nor are we guaranteed that the central dir will have any
     * sort of convenient size.  We need to skip to the start of it and
     * read the header, then the other goodies.
     *
     * The only thing we really need right now is the file comment, which
     * we're hoping to preserve.
     */
    if (fseek(fp, endOfCentralDir.mCentralDirOffset, SEEK_SET) != 0) {
        MY_ERROR("failure seeking to central dir offset %ld\n", endOfCentralDir.mCentralDirOffset);
        delete[] buf;
        return -1;
    }

    /*
     * Loop through and read the central dir entries.
     */
    int entry;
    for (entry = 0; entry < endOfCentralDir.mTotalNumEntries; entry++) {
        ZipEntry* pEntry = new ZipEntry;

        result = pEntry->initFromCDE(fp);
        if (result != NO_ERROR) {
            MY_ERROR("initFromCDE failed\n");
            delete pEntry;
            delete[] buf;
            return -1;
        }
		
		const char* filename = pEntry->getFileName();
		int name_len = strlen(filename);
		bool is_directory = name_len > 0 ? filename[name_len - 1] == '/' : true;
		if (is_directory)
		{
			MY_METHOD("ignore directory:%s", filename);
            delete pEntry;
			continue;
		}

        _all_entries.push_back(pEntry);
    }

    /*
     * If all went well, we should now be back at the EOCD.
     */
    {
        unsigned char checkBuf[4];
        if (fread(checkBuf, 1, 4, fp) != 4) {
            MY_ERROR("EOCD check read failed");
            delete[] buf;
            return -1;
        }
        if (ZipEntry::getLongLE(checkBuf) != EndOfCentralDir::kSignature) {
            MY_ERROR("EOCD read check failed");
            delete[] buf;
            return -1;
        }
    }
    delete[] buf;
	MY_LOG("got %zu entries[%u] in file %s", _all_entries.size(), (unsigned)sizeof(ZipEntry), _path);
    return 0;
}

void clean_file_entries_map(std::vector<std::vector<ZipEntry*> > & _entries_in_zip_file)
{
    std::vector<std::vector<ZipEntry*> >::iterator it = _entries_in_zip_file.begin();
    for(; it != _entries_in_zip_file.end(); it++)
    {
        std::vector<ZipEntry*>& entries = *it;
        for(int i = 0; i < entries.size(); i++) {
            delete entries[i];
        }
        entries.clear();
    }
    _entries_in_zip_file.clear();
}

static void add_entry_to_partition(ZipEntry* entry, 
    int& pre_file_index,
    uint64_t& pre_file_stop,
    uint64_t& pre_shadow_stop,
    std::vector<FilePartitionInfo>& patch_partitions, 
	FILE* changed_entries_header_patch_file)
{
    int file_index = entry->mUserData1;
    uint64_t file_start = entry->getEntryBegin();
	int padding = 0;
	const char* filename = entry->getFileName();
	int name_len = strlen(filename);
	bool is_directory = name_len > 0 ? filename[name_len - 1] == '/' : true;
	if (!entry->isCompressed() && !is_directory)
	{
		uint64_t shadow_zip_raw_file_offset = pre_shadow_stop + ZipEntry::LocalFileHeader::kLFHLen + entry->mLFH.mFileNameLength + entry->mLFH.mExtraFieldLength;
		const uint alignment = 4; 
		padding = (alignment - (shadow_zip_raw_file_offset % alignment)) % alignment;
	}
	
    bool can_merge = (file_index == pre_file_index) && ( file_start == pre_file_stop ) && padding == 0;

    pre_file_index = file_index;
    pre_file_stop = entry->getEntryEnd();
    int32_t entry_size = pre_file_stop - file_start;

    if (can_merge)
    {
        FilePartitionInfo& partition = patch_partitions.back(); 
        partition.shadow_stop_ += entry_size;
        partition.stop_in_file_ += entry_size;
		pre_shadow_stop += entry_size;
    }
	else if (padding != 0){
		uint64_t raw_file_offset = entry->getFileOffset();
		if (entry->addPadding(padding) != 0){
			MY_ERROR("entry add padding failed:%s", entry->getFileName());
			exit(-1);	
		}
		
		int header_file_start = ::ftell(changed_entries_header_patch_file);
		if (entry->mLFH.write(changed_entries_header_patch_file) != 0)
		{			
			MY_ERROR("write entry local file header failed:%s", entry->getFileName());
			exit(-1);
		}
		int header_file_stop = ::ftell(changed_entries_header_patch_file);
		
		//local header changed
		{
			FilePartitionInfo partition(pre_shadow_stop, pre_shadow_stop + header_file_stop - header_file_start, 1, header_file_start, header_file_stop); 
			patch_partitions.push_back(partition);		
			pre_shadow_stop += header_file_stop - header_file_start;
		}
		MY_METHOD("padding %d for %s, header size:%d, file begin:0x%08zx", padding, entry->getFileName(), header_file_stop - header_file_start, (size_t)pre_shadow_stop);
		
		{
			FilePartitionInfo partition(pre_shadow_stop, pre_shadow_stop + pre_file_stop - raw_file_offset, file_index, raw_file_offset, pre_file_stop); 
			patch_partitions.push_back(partition);
			pre_shadow_stop += pre_file_stop - raw_file_offset;	
		}
	}
    else{
        FilePartitionInfo partition(pre_shadow_stop, pre_shadow_stop + entry_size, file_index, file_start, pre_file_stop); 
        patch_partitions.push_back(partition);
		pre_shadow_stop += entry_size;
    }
    
}

void ShadowZip::output_apk(const char* _patch_dir)
{
	ShadowZip test;
    FILE* fr = test.fopen();
	
    char test_apk_path[512] = {0};
    snprintf( test_apk_path, sizeof(test_apk_path), "%s/test.apk", _patch_dir );
    FILE* fw = ::fopen( test_apk_path, "wb" );
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
	{
		MY_LOG("copy end at %ld, %llu", ::ftell(fw), (unsigned long long)test.end_of_file_);
	}
    test.fclose(fr);
    ::fclose(fw);
}

int ShadowZip::init(const char* _patch_dir, const char* _sys_apk_file, ShadowZipGlobalData* global_data)
{
	PthreadWriteGuard(global_data->mutex);
    global_data->patch_partitions_.clear();
    global_data->all_files_.clear();
    global_data->all_files_.push_back(_sys_apk_file);
    global_data->end_of_file_ = 0;
	
	char changed_entries_header_path[512] = {0};
    snprintf( changed_entries_header_path, sizeof(changed_entries_header_path), "%s/.entries_header.data", _patch_dir );
    global_data->all_files_.push_back(changed_entries_header_path);
    FILE* changed_entries_header_patch_file = ::fopen(changed_entries_header_path, "wb");
	FileHandleReleaser autoDel(changed_entries_header_patch_file);  

    //set before get files
	int persist_file_count = global_data->all_files_.size();
	
    //find all patch files
    char apk_patch_path[512] = {0};
    snprintf( apk_patch_path, sizeof(apk_patch_path), "%s/assets_bin_Data", _patch_dir );
    get_files( apk_patch_path, global_data->all_files_ );
    if( global_data->all_files_.size() <= persist_file_count ){
        MY_INFO("no apk patches:[%s/assets_bin_Data]", _patch_dir);
        return -1;
    }
	
	//all removed files
    char removed_files_path[512] = {0};
    snprintf(removed_files_path, sizeof(removed_files_path), "%s/removed_files.txt", _patch_dir );
	std::set<std::string> removed_files;
	std::ifstream rmfiles_stream(removed_files_path);
	if (rmfiles_stream.good()){
		std::string line;
		while( std::getline( rmfiles_stream, line ) ) {
			line.erase(std::remove_if(line.begin(), line.end(), is_illegal_path_char), line.end());
			if (line.length() == 0){continue;}	
			MY_INFO("remove file: [%s]", line.c_str());
			removed_files.insert(line);
		}
		rmfiles_stream.close();
	}
	

    //find all entries in patches
    std::vector<std::vector<ZipEntry*> > entries_in_zip_file(global_data->all_files_.size());
    std::map<std::string, ZipEntry*> filename_2_entry;
    for(int i = 2; i < global_data->all_files_.size(); i++) {
        std::string& zip_path = global_data->all_files_[i];
        std::vector<ZipEntry*> zip_entries;
        int ret = parse_apk(zip_path.c_str(), zip_entries);
        entries_in_zip_file[i] = zip_entries;
        if (ret != 0){
            MY_ERROR("parse file failed:%s", zip_path.c_str());
            clean_file_entries_map(entries_in_zip_file);
            return -1;
        }
        for(int j = 0; j < zip_entries.size(); j++)
        {			
            ZipEntry* entry = zip_entries[j];
            entry->mUserData1 = i;
            std::string filename = entry->getFileName();
            MY_LOG("find patch:%s in %s", filename.c_str(), zip_path.c_str());
            if (filename_2_entry.find(filename) != filename_2_entry.end()) {
                MY_ERROR("dup patch file failed:%s", filename.c_str());
                clean_file_entries_map(entries_in_zip_file);
                return -1;
            }
			
			if (removed_files.find(filename) != removed_files.end()){
				MY_LOG("rm file:%s in %s", filename.c_str(), zip_path.c_str());
				continue;
			}
            filename_2_entry[filename] = entry;
        }
    }

    // find all entries in apk
    std::vector<ZipEntry*> apk_entries;
    int ret = parse_apk(_sys_apk_file, apk_entries);
    entries_in_zip_file[0] = apk_entries;
    if (ret != 0){
        MY_ERROR("parse file failed:%s", _sys_apk_file);
        clean_file_entries_map(entries_in_zip_file);
        return -1;
    }

    //entries partition
    global_data->patch_partitions_.clear();
    std::vector<ZipEntry*> all_entries;
    int pre_file_index = -1;
    uint64_t pre_file_stop = 0;
    uint64_t pre_shadow_stop = 0;
    for(int i = 0; i < apk_entries.size(); i++)
    {
        ZipEntry* entry = apk_entries[i];
        std::string name(entry->getFileName());	
		if (removed_files.find(name) != removed_files.end()){
			MY_LOG("rm file:%s in apk", name.c_str());
			continue;
		}
        std::map<std::string, ZipEntry*>::iterator it = filename_2_entry.find(name);
        if (it != filename_2_entry.end()){ entry = it->second; }
        entry->mUserData2 = 1;
        uint64_t entry_new_start = pre_shadow_stop;
        add_entry_to_partition(entry, pre_file_index, pre_file_stop, pre_shadow_stop, global_data->patch_partitions_, changed_entries_header_patch_file);
        entry->setLFHOffset((off_t)entry_new_start);
        all_entries.push_back(entry);
    }
    for(int i = 1; i < entries_in_zip_file.size(); i++)
    {
        std::vector<ZipEntry*>& entries = entries_in_zip_file[i];
        for(int j = 0; j < entries.size(); j++) {
            ZipEntry* entry = entries[j];
            if (entry->mUserData2 != 0) continue;
            uint64_t entry_new_start = pre_shadow_stop;
            add_entry_to_partition(entry, pre_file_index, pre_file_stop, pre_shadow_stop, global_data->patch_partitions_, changed_entries_header_patch_file);
            entry->setLFHOffset((off_t)entry_new_start);
            all_entries.push_back(entry);
        }
    }
    
    uint64_t cd_offset = pre_shadow_stop; 
    char end_patch_path[512] = {0};
    snprintf( end_patch_path, sizeof(end_patch_path), "%s/.patch.data", _patch_dir );
    global_data->all_files_.push_back(end_patch_path);
    FILE* end_patch_file = ::fopen(end_patch_path, "wb");
    for(int i = 0; i < all_entries.size(); i++) {
        ZipEntry* entry = all_entries[i];
		MY_LOG("mapping [%08lx-%08lx-%08lx-%08lx] %s, method:%d, ", entry->getEntryBegin(), entry->getFileOffset(), entry->getFileOffset() + entry->getCompressedLen(), entry->getEntryEnd(), 
			entry->getFileName(), entry->mCDE.mCompressionMethod);
        entry->mCDE.write(end_patch_file);
    }
    EndOfCentralDir end_of_cd;
    end_of_cd.mNumEntries = all_entries.size();
    end_of_cd.mTotalNumEntries = all_entries.size();
    end_of_cd.mCentralDirSize = ::ftell(end_patch_file);
    end_of_cd.mCentralDirOffset = pre_shadow_stop;
    end_of_cd.write( end_patch_file );
    uint64_t end_size = ::ftell(end_patch_file);
    ::fclose(end_patch_file);

    global_data->end_of_file_ = cd_offset + end_size;
    FilePartitionInfo partition(cd_offset, global_data->end_of_file_, global_data->all_files_.size() - 1, 0, end_size); 
    global_data->patch_partitions_.push_back(partition);


    log(global_data);
    clean_file_entries_map(entries_in_zip_file);
	
    return 0;
}

void ShadowZip::log(ShadowZipGlobalData* global_data)
{
	for(int i = 0; i < global_data->patch_partitions_.size(); i++)
    {
        FilePartitionInfo& partition = global_data->patch_partitions_[i]; 
        MY_LOG("0x%08llx - 0x%08llx file:%lld, [0x%08llx - 0x%08llx] %s", 
			(unsigned long long)partition.shadow_start_, 
			(unsigned long long)partition.shadow_stop_, 
			(unsigned long long)partition.file_index_, 
			(unsigned long long)partition.start_in_file_, 
			(unsigned long long)partition.stop_in_file_,
			global_data->all_files_[partition.file_index_].c_str());
    }
	MY_LOG("eof_pos 0x%08llx", (unsigned long long)global_data->end_of_file_);
}

uint64_t ShadowZip::get_eof_pos()
{
	PthreadWriteGuard(g_shadowzip_global_data->mutex);
	MY_METHOD("get_eof_pos -> 0x%08llx", (unsigned long long)g_shadowzip_global_data->end_of_file_);
	return g_shadowzip_global_data->end_of_file_;
}


bool ShadowZip::contains_path(const char* _apk_file, const char* _check_path)
{
	bool ret = false;
	std::vector<ZipEntry*> zip_entries;
	if (parse_apk(_apk_file, zip_entries) != 0){
		MY_ERROR("parse file failed:%s", _apk_file);
		return ret;
	}
	for(int j = 0; j < zip_entries.size(); j++)
	{
		ZipEntry* entry = zip_entries[j];
		std::string filename = entry->getFileName();
		if (memcmp(filename.c_str(), _check_path, strlen(_check_path)) == 0){
			ret = true;
			break;
		}
	}
	
	for(int i = 0; i < zip_entries.size(); i++) {
        delete zip_entries[i];
	}
	zip_entries.clear();
	
	MY_INFO("%s%s contains path:%s", _apk_file, ret ? "": " doesn't", _check_path);
	return ret;
}

ShadowZip::ShadowZip()
{
	PthreadWriteGuard(g_shadowzip_global_data->mutex);
	patch_partitions_ = g_shadowzip_global_data->patch_partitions_;
    all_files_ = g_shadowzip_global_data->all_files_;
    end_of_file_ = g_shadowzip_global_data->end_of_file_;
}

FILE* ShadowZip::fopen()
{
    pos_ = 0;
	fp_array_.clear();
	for(int i = 0; i < all_files_.size(); i++)
	{
		fp_array_.push_back(NULL);
	}
    FILE* fp = prepare_file(0);
	MY_METHOD("fopen -> 0x%08zx", (size_t)fp);
	return fp;
}

off64_t ShadowZip::fseek(FILE *stream, off64_t offset, int whence)
{	
	MY_METHOD("fseek -> 0x%08zx at 0x%08llx with type %d", (size_t)stream, (unsigned long long)offset, whence);
    int64_t cur_pos = pos_;
    if (whence == SEEK_SET){
        pos_ = offset;
    }
    else if (whence == SEEK_CUR){
        pos_ += offset;
    }
    else if (whence == SEEK_END){
        pos_ = end_of_file_ + offset;
    }
    if (pos_ < 0 || pos_ > end_of_file_){
        pos_ = cur_pos;
		MY_ERROR("fseek failed: pos:0x%08llx, end_of_file:0x%08llx", (unsigned long long)pos_, (unsigned long long)end_of_file_);
        return -1;
    }
    return 0;
}

long ShadowZip::ftell(FILE *stream)
{
	MY_METHOD("ftell -> 0x%08zx at 0x%08llx", (size_t)stream, (unsigned long long)pos_);
    return (long) pos_;
}

void ShadowZip::rewind(FILE *stream)
{
    pos_ = 0;
}

size_t ShadowZip::fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	MY_METHOD("fread -> 0x%08zx at 0x%08llx, size:%zu, n:%zu", (size_t)stream, (unsigned long long)pos_, size, nmemb);
	if (((int)nmemb) <= 0){return 0;}
	
    uint64_t begin = pos_;
    uint64_t end = pos_ + size * nmemb;
    size_t ret = 0;
	
    void* write_ptr = ptr;
    for(int i = 0; i < patch_partitions_.size(); i++)
    {
        FilePartitionInfo& info = patch_partitions_[i];
        if (begin >= info.shadow_stop_){
            continue;
        }
        assert(begin >= info.shadow_start_);

        uint64_t start_in_file = begin - info.shadow_start_ + info.start_in_file_;
        uint64_t stop_in_file = (end >= info.shadow_stop_) ? info.stop_in_file_ : (end - info.shadow_start_ + info.start_in_file_);
        int64_t read_size = stop_in_file - start_in_file;
        if (read_size > nmemb){
            MY_LOG("p:%d, start:[%zx,%zx) read size:%zu", i, (size_t)start_in_file, (size_t)stop_in_file,  (size_t)read_size);
            MY_LOG("shadow:[%zx,%zx) pos:%zx", (size_t)begin, (size_t)end, (size_t)pos_);
        }
        FILE* fp = prepare_file(info.file_index_);
        assert(fp != NULL);
        ::fseek(fp, start_in_file, SEEK_SET);
        ::fread(write_ptr, 1, read_size, fp); 
        ret += read_size;
        pos_ += read_size;
        write_ptr = (char*)write_ptr + read_size;
        begin += read_size;
        if (begin >= end){return ret;}
    }
    return ret;

}

char* ShadowZip::fgets(char *s, int size, FILE *stream)
{
	MY_METHOD("fgets -> 0x%08zx at 0x%08llx, size:%d", (size_t)stream, (unsigned long long)pos_, size);
    uint64_t begin = pos_;
    uint64_t end = pos_ + size;

    void* write_ptr = s;
    for(int i = 0; i < patch_partitions_.size(); i++)
    {
        FilePartitionInfo& info = patch_partitions_[i];
        if (begin >= info.shadow_stop_){
            continue;
        }
        assert(begin >= info.shadow_start_);

        uint64_t start_in_file = begin - info.shadow_start_ + info.start_in_file_;
        uint64_t stop_in_file = (end >= info.shadow_stop_) ? info.stop_in_file_ : (end - info.shadow_start_ + info.start_in_file_);
        int64_t read_size = stop_in_file - start_in_file;
        if (read_size > size){
            MY_LOG("p:%d, start:[%zx,%zx) read size:%zu", i, (size_t)start_in_file, (size_t)stop_in_file,  (size_t)read_size);
            MY_LOG("shadow:[%zx,%zx) pos:%zx", (size_t)begin, (size_t)end, (size_t)pos_);
        }
        FILE* fp = prepare_file(info.file_index_);
        assert(fp != NULL);
        ::fseek(fp, start_in_file, SEEK_SET);
        char* ret = ::fgets(s, size, fp); 
		long pos_after_fgets = ::ftell(fp);
		read_size = pos_after_fgets - start_in_file;
        pos_ += read_size;
        write_ptr = (char*)write_ptr + read_size;
        begin += read_size;
        return ret;
    }
    return NULL;
}

int ShadowZip::fclose(FILE* stream)
{
	MY_METHOD("fclose -> 0x%08zx at 0x%08llx", (size_t)stream, (unsigned long long)pos_);
    for(int i = 0; i < fp_array_.size(); i++) 
    {
		FILE* fp = fp_array_[i];
		if (fp) {		
			MY_METHOD("fclose -> 0x%08zx fp%d at 0x%08lx", (size_t)stream, i, ::ftell(fp));
			::fclose(fp);
		}
    }
    fp_array_.clear();
    return 0;
}

FILE* ShadowZip::prepare_file(int _file_index)
{
    if (fp_array_[_file_index] != NULL ){
        return fp_array_[_file_index];
    }
	
	//in cace of too many file opened, close all except base.apk
	//but if you have only one patch file including everything, you can comment these to get a better performance. The max open files may reach to 100.
	for(int i = 1; i < fp_array_.size(); i++) 
    {
		FILE* fp = fp_array_[i];
		if (fp) {		
			MY_METHOD("fclose -> 0x%08zx fp:%d at 0x%08lx", (size_t)fp, i, ::ftell(fp));
			::fclose(fp);
			fp_array_[i] = NULL;
		}
    }

    std::string& path = all_files_[_file_index];
    FILE* fp = ::fopen(path.c_str(), "rb");
    if (fp == NULL){
        MY_LOG("can't open file:%s", path.c_str());
        return NULL; 
    }
	MY_METHOD("prepare_file %s -> 0x%08zx", path.c_str(), (size_t)fp);
    fp_array_[_file_index] = fp;
	
	//if you have only one patch file including everything, the reading index can be random each time. a smaller buffer size performs better.
	//for others, 1024 is also fair enough.
	setvbuf( fp , NULL , _IOFBF , 1024);
    return fp;
}


