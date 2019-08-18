#include "il2cpp.h"
#include <fstream>
#include <sstream>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "log.h"
#include "zip/shadow_zip.h"
#include "xhook/xhook.h"
#include "io_github_noodle1983_Boostrap.h"

struct GlobalData
{	
	std::map<int, FILE*> g_fd_to_file;	
	std::map<FILE*, ShadowZip*> g_file_to_shadowzip;
	PthreadRwMutex g_file_to_shadowzip_mutex;
};
#define g_global_data (LeakSingleton<GlobalData, 0>::instance())


static inline char * dupstr(const char* const str)
{
	if (str == NULL){ return NULL; }
	
	int len = strlen(str) + 1;
	char* ret = new char[len];
	memset(ret, 0, len);
	strncpy(ret, str, len);
	return ret;
}

const char* SPLITER = ";";
const char* g_data_file_path = NULL;
const char* g_apk_file_path = NULL;
static void bootstrap();
std::string get_apk_path(const std::string& bundle_id);

__attribute__ ((visibility ("default")))
JNIEXPORT void JNICALL Java_io_github_noodle1983_Boostrap_init
  (JNIEnv * jenv, jclass cls, jstring path)
{
	const char* data_file_path = jenv->GetStringUTFChars(path, NULL); 
	g_data_file_path = dupstr(data_file_path); // never delete, ok with only one
	jenv->ReleaseStringUTFChars(path, data_file_path);
	MY_INFO("data file path:%s", g_data_file_path);
	
	LeakSingleton<GlobalData, 0>::init();
    bootstrap();
}

static std::string get_bundle_id()
{
	pid_t pid = getpid();
	char bundle_id_filename[256] = {0};
	snprintf(bundle_id_filename, sizeof(bundle_id_filename), "/proc/%d/cmdline", pid);
	
	std::fstream bundle_id_file(bundle_id_filename, std::fstream::in);
	if (!bundle_id_file.is_open())
	{
		MY_ERROR("failed to open %s ", bundle_id_filename);
		exit(-1);
		return "";
	}
	
	char bundle_id[256] = {0};
	bundle_id_file.getline(bundle_id, sizeof(bundle_id));
	bundle_id_file.close();
	return std::string(bundle_id);
}

__attribute__ ((visibility ("default")))
char* get_arch_abi()
{
	return dupstr(TARGET_ARCH_ABI);
}

static char* g_use_data_path = NULL;
__attribute__ ((visibility ("default")))
char* use_data_dir(const char* data_path, const char* apk_path)
{
	if (strlen(data_path) > 0){
		DIR* dir = opendir(data_path);
		if (dir == NULL)
		{
			MY_ERROR("can't access data path:%s", data_path);
			return strdup("can't access data dir!");		
		}
		closedir(dir);
	}
	
	std::string bundle_id = get_bundle_id();
	char patch_info_path[256] = {0};
	snprintf(patch_info_path, sizeof(patch_info_path), "%s/user.db",  g_data_file_path);
	
	std::fstream patch_info_file(patch_info_path, std::fstream::out|std::fstream::trunc);
	if (!patch_info_file.is_open())
	{	
		char error_str[256] = {0};
		snprintf(error_str, sizeof(error_str), "can't store data path. error:%s", strerror(errno));
		MY_ERROR("can't access to %s. %s", patch_info_path, error_str);
		return dupstr(error_str);
	}
	patch_info_file.write(data_path, strlen(data_path));	
	patch_info_file.write(SPLITER, strlen(SPLITER));	
	patch_info_file.write(apk_path, strlen(apk_path));	
	patch_info_file.close();
	return NULL;
}

static bool pre_process_so_lib(const char* const so_path, const char* const so_name, const std::string& bundle_id)
{
	char link_file[256] = {0};
	snprintf(link_file, sizeof(link_file), "%s/%s", g_data_file_path, so_name);
	
	MY_LOG("link %s to %s", so_path, link_file);
	
	//symlink(so_path, link_file);
	unlink(link_file);
	
	int fd_to, fd_from;
    char buf[4096];
    ssize_t nread;
    int saved_errno;

    fd_from = open(so_path, O_RDONLY);
    if (fd_from < 0){
		MY_ERROR("can't access to %s.", so_path);
        return false;
	}

    fd_to = open(link_file, O_WRONLY | O_CREAT | O_EXCL, 0755);
    if (fd_to < 0){		
		MY_ERROR("can't access to %s.", so_path);
		close( fd_from );
        return false;
	}

    while ((nread = read(fd_from, buf, sizeof buf))> 0) {
        char *out_ptr = buf;
        ssize_t nwritten;
        do {
            nwritten = write(fd_to, out_ptr, nread);
            if (nwritten >= 0) {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR) {	
				MY_ERROR("can't write to %s. errno:%d", so_path, errno);
				close( fd_from );
				close( fd_to );
                return false;
            }
        } while (nread > 0);
    }
	close( fd_from );
	close( fd_to );
	return true;
}

static bool pre_process_all_so_lib(const char* const data_path, const std::string& bundle_id)
{
	DIR* dir = opendir(data_path);
	if (dir == NULL)
	{
		MY_ERROR("can't access data path:%s", data_path);
		return false;		
	}
	
	char filepath_buffer[256] = {0};
	struct dirent *ent = NULL;
	while((ent = readdir(dir)) != NULL)
    {
        if(ent->d_type & DT_DIR) { continue; }
		
        const char* filename = ent->d_name;
		int len = strlen(filename);
		snprintf(filepath_buffer, sizeof(filepath_buffer), "%s/%s", data_path, filename);
		
		//rename
		int new_file_postfix_len = strlen(".new");
        if (len > new_file_postfix_len && memcmp(filename + len - new_file_postfix_len, ".new", new_file_postfix_len) == 0)
		{
			std::string so_name = std::string(filename).substr(0, len - new_file_postfix_len);
			std::string new_filepath = filepath_buffer;
			std::string old_filepath = new_filepath.substr( 0, new_filepath.length() - new_file_postfix_len );
			unlink(old_filepath.c_str());
			rename(new_filepath.c_str(), old_filepath.c_str());
			chmod(old_filepath.c_str(), 0755);
			MY_LOG("rename %s to %s", new_filepath.c_str(), old_filepath.c_str());
			
			pre_process_so_lib(old_filepath.c_str(), so_name.c_str(), bundle_id);
			continue;
		}
		
		//link
		int so_file_postfix_len = strlen(".so");
        if (len > so_file_postfix_len && memcmp(filename + len - so_file_postfix_len, ".so", so_file_postfix_len) == 0)
		{			
			chmod(filepath_buffer, 0755);
			pre_process_so_lib(filepath_buffer, filename, bundle_id);
			continue;
		}	
    }
	closedir(dir);
	return true;
	
}

static dev_t g_apk_device_id = -1;
static ino_t g_apk_ino = -1;
static bool extract_patch_info(const std::string& bundle_id, std::string& default_path, std::string& patch_path)
{
	char default_il2cpp_path[256] = {0};
	snprintf(default_il2cpp_path, sizeof(default_il2cpp_path), "%s/../lib/libil2cpp.so",  g_data_file_path);
	default_path = std::string(default_il2cpp_path);
	patch_path.clear();
	
	char patch_info_path[256] = {0};
	snprintf(patch_info_path, sizeof(patch_info_path), "%s/user.db",  g_data_file_path);
	
	//get patch date
	struct stat user_stat;
	memset(&user_stat, 0, sizeof(struct stat));
	if (stat(patch_info_path, &user_stat) != 0) {
		MY_LOG("can't read file:%d[%s]", errno, patch_info_path);
		return false;
	}
	
	//get patch config
	std::fstream patch_info_file(patch_info_path, std::fstream::in);
	if (!patch_info_file.is_open())
	{	
		if (g_use_data_path != NULL){delete[] g_use_data_path; g_use_data_path = NULL;}
		return false;
	}	
	char file_content[4096] = {0};
	patch_info_file.getline(file_content, sizeof(file_content));
	if (strlen(file_content) == 0)
	{
		MY_ERROR("empty file string");
		return false;
	}
	patch_info_file.close();
	
	//split data_path
	char* split_pos = strstr( file_content, SPLITER );
	if(split_pos != NULL)
	{
		int spliter_len = strlen(SPLITER);
		memset(split_pos, 0, spliter_len);
	}
	
	char* data_path = file_content;
	DIR* dir = opendir(data_path);
	if (dir == NULL)
	{
		MY_ERROR("can't access data path:%s", data_path);
		return false;		
	}
	closedir(dir);
	
	//get and save apk file id
	std::string apk_path_string = get_apk_path(bundle_id);	
	const char* apk_path = apk_path_string.c_str();
	struct stat apk_stat;
	memset(&apk_stat, 0, sizeof(struct stat));
	if (stat(apk_path, &apk_stat) != 0) {
		MY_ERROR("can't read file:%d[%s]", errno, apk_path);
		return false;
	}
	g_apk_device_id = apk_stat.st_dev;
	g_apk_ino = apk_stat.st_ino;
		
	//if we have newer apk file, then no need to load patch
	if (apk_stat.st_mtime > user_stat.st_mtime){	
		MY_ERROR("newer apk file:%s, no need to patch", apk_path);
		unlink(patch_info_path);
		return false;
	}
	
	if (!pre_process_all_so_lib(data_path, bundle_id))
	{
		MY_ERROR("can't pre_process_all_so_lib");
		return false;
	}
	
	char link_file[256] = {0};
	snprintf(link_file, sizeof(link_file), "%s/libil2cpp.so", g_data_file_path);
	std::string il2cpp_path(link_file);	
	FILE *fp = fopen (il2cpp_path.c_str(), "r");
	if (fp == NULL) 
	{
		MY_ERROR("can't libil2cpp:%s", il2cpp_path.c_str());
		return false;		   
	}
	fclose (fp);
	
	if (g_use_data_path != NULL){delete[] g_use_data_path;}
	g_use_data_path = dupstr(data_path);
	if (g_apk_file_path != NULL){delete[] g_apk_file_path;}
	g_apk_file_path = dupstr(apk_path);
	patch_path = il2cpp_path;
	return true;
}

#include <elfio/elfio.hpp> 
using namespace ELFIO;
static void elf_hook(char const *so_file, void const* so_map_addr, std::vector<const char*>& symbol_names, std::vector<void const *>& new_func, std::vector<void **> & old_func)
{
    static size_t pagesize = sysconf(_SC_PAGESIZE);
	
    elfio reader;
	if (!reader.load(so_file)) {        
		MY_ERROR("can't access path:%s", so_file);;
		return;
	}
	
	size_t bias_addr = -1;
	const section* dynsym = NULL;
	const section* relplt = NULL;
	Elf_Half sec_num = reader.sections.size(); 
	for ( int i = 0; i < sec_num; ++i ) {
		const section* psec = reader.sections[i]; 
		if(bias_addr == -1 && (psec->get_type() & PT_LOAD)){
			bias_addr = (size_t)so_map_addr + psec->get_offset() - psec->get_address();
			MY_LOG("fix addr 0x%zx ->0x%zx", (size_t)so_map_addr, bias_addr);
		}
		
		if (strcmp(psec->get_name().c_str(), ".dynsym") == 0){dynsym = psec;}
		else if (strcmp(psec->get_name().c_str(), ".rel.plt") == 0){relplt = psec;}
		else if (strcmp(psec->get_name().c_str(), ".rela.plt") == 0){relplt = psec;}
		
		if (dynsym != NULL && relplt != NULL){break;}
	}
	if (bias_addr == -1){
		bias_addr = (size_t)so_map_addr;
		MY_LOG("force addr 0x%zx ->0x%zx", (size_t)so_map_addr, bias_addr);
	}
	
	if (dynsym == NULL && relplt == NULL) {    
		MY_ERROR("can't get section, dynsym:0x%zx, relplt:0x%zx", (size_t)dynsym, (size_t)relplt);;
		return;		
	}
	
	std::map<const char*, size_t> name2index;
	const_symbol_section_accessor dynamic_symbols( reader, dynsym );
	for ( unsigned int j = 0; j < dynamic_symbols.get_symbols_num(); ++j ) { 
		std::string   name;
		Elf64_Addr    value   = 0;
		Elf_Xword     size    = 0;
		unsigned char bind    = 0;
		unsigned char type    = 0;
		Elf_Half      section = 0;
		unsigned char other   = 0;
        dynamic_symbols.get_symbol( j, name, value, size, bind, type, section, other );
        for(int i = 0; i < symbol_names.size(); i++)
		{
			const char* find_name = symbol_names[i];
			if (strcmp(find_name, name.c_str()) == 0){
				name2index[find_name] = j;
				MY_LOG("found %s:%d", name.c_str(), j);
				if (name2index.size() == symbol_names.size()) {break;}
			}
		}
		if (name2index.size() == symbol_names.size()) {break;}	
    }
	
	if (name2index.size() != symbol_names.size()) {
		MY_ERROR("name2index not completed");
		return;
	}	
	
	const_relocation_section_accessor rel_entries( reader, relplt );
	for(int j = 0; j < rel_entries.get_entries_num(); j++)
	{
		Elf64_Addr offset;
		Elf_Word   symbol;
		Elf_Word   type;
		Elf_Sxword addend;
		bool ret = rel_entries.get_entry( j, offset, symbol, type, addend );
		if (!ret){
			MY_ERROR("can't get sym offset, dynsym:%d", j);
			return;
		}
		
		//MY_LOG("rel %d:[0x%zx][%d][%d][%d]", j, (size_t)offset, symbol, type, (size_t)addend);
		std::map<const char*, size_t>::iterator it = name2index.begin();
		for(int i = 0; it != name2index.end(); it++, i++)
		{
			
			int index = it->second;
			if (index == symbol)
			{
				size_t* ori_func_addr = (size_t *)(((size_t)bias_addr) + offset);
				void* original = (void *)*ori_func_addr;
				MY_LOG("%s:(0x%zx + 0x%zx) = 0x%zx, old:0x%zx -> new:0x%zx", it->first, (size_t)bias_addr, (size_t)offset, (size_t)ori_func_addr, 
					(size_t)original, (size_t)new_func[i]);
				*old_func[i] = original;
				//if (bias_addr == (size_t)so_map_addr)
				{
					*ori_func_addr = (size_t)new_func[i];
				}				
			}
		}
	}
}

static void* get_module_base( const char* module_name )
{
	FILE *fp;
	size_t addr = 0;
	char *pch;
	char filename[32];
	char line[1024];
	char* store_ptr = NULL;
	
	fp = fopen( "/proc/self/maps", "r" );
	if ( fp != NULL ){
		while ( fgets( line, sizeof(line), fp ) ){
			if ( strstr( line, module_name ) ){
				pch = strtok_r( line, "-", &store_ptr);
				addr = strtoull( pch, NULL, 16 );
				break;
			}
		}
		fclose( fp ) ;
	}
	return (void *)addr;
}

static void* get_module_base( char* module_buffer, int len )
{
	FILE *fp;
	size_t addr = 0;
	char *pch;
	char line[1024];
	char* matched_pos = NULL;
	char* store_ptr = NULL;
	
	fp = fopen( "/proc/self/maps", "r" );
	if ( fp != NULL ){
		while ( fgets( line, sizeof(line), fp ) ){
			if ((matched_pos = strstr( line, module_buffer )) != NULL){
				pch = strtok_r( line, "-", &store_ptr );
				addr = strtoull( pch, NULL, 16 );
				
				char* fullpath = matched_pos;
				while(fullpath > line && (*fullpath) != ' ' && (*fullpath) != '\t'){ fullpath--; }
				strncpy(module_buffer, fullpath + 1, len);
				for(int i = 0; i < len; i++){
					if(module_buffer[i] == ' ' || module_buffer[i] == '\r'|| module_buffer[i] == '\n'|| module_buffer[i] == '\t' || module_buffer[i] == '\0') {
						module_buffer[i] = 0;
						break;
					}
				}				
				break;
			}
		}
		fclose( fp ) ;
	}
	return (void *)addr;
}

static void dump_maps( )
{
	FILE *fp;
	size_t addr = 0;
	char *pch;
	char filename[32];
	char line[1024];
	
	fp = fopen( "/proc/self/maps", "r" );
	if ( fp != NULL ){
		while ( fgets( line, sizeof(line), fp ) ){
			MY_ERROR("%s", line);
		}
		fclose( fp ) ;
	}
}

std::string get_apk_path(const std::string& bundle_id)
{
	FILE *fp;
	char *found_path = NULL;
	char filename[32];
	char line[1024];
	const char* TAIL = ".apk";
	int TAIL_LEN = strlen(TAIL);
	std::string ret;
	
	fp = fopen( "/proc/self/maps", "r" );
	if ( fp != NULL ){
		while ( fgets( line, sizeof(line), fp ) ){	
			int line_len = strlen(line);
			while(line[line_len - 1] == ' ' || line[line_len - 1] == '\r' || line[line_len - 1] == '\n' || line[line_len - 1] == '\t') {line[line_len - 1] = '\0'; line_len--;}
			if ( strstr( line, bundle_id.c_str()) && (memcmp(line + strlen(line) - TAIL_LEN, TAIL, TAIL_LEN) == 0) ){
				found_path = strchr( line, '/' );
				if (ShadowZip::contains_path(found_path, "assets/bin/Data/"))
				{
					ret = std::string(found_path);
					break;
				}
			}
		}
		fclose( fp ) ;
	}
	return ret;
}

static void check_set_old_function_to_shadow_zip();

typedef int (* StatType)(const char *path, struct stat *file_stat);
static StatType old_stat = NULL;
static int my_stat(const char *path, struct stat *file_stat)
{
	if (old_stat == NULL)
	{
		MY_ERROR("old_stat is NULL");
		return -1;
	}
	
	check_set_old_function_to_shadow_zip();
	
	memset(file_stat, 0, sizeof(struct stat));
	int ret = old_stat(path, file_stat);
	if (ret != 0) {
		return ret;
	}
	
	if (g_apk_device_id == file_stat->st_dev && g_apk_ino == file_stat->st_ino)
	{
		uint64_t size = ShadowZip::get_eof_pos();
		if (size > 0){
			file_stat->st_size = (off_t)size;
		}
		return ret;
	}
	
	return ret;
}

static ShadowZip* get_cached_shadowzip(FILE *stream)
{
	PthreadReadGuard(g_global_data->g_file_to_shadowzip_mutex);
	std::map<FILE*, ShadowZip*>::iterator it = g_global_data->g_file_to_shadowzip.find(stream);
	ShadowZip* shadow_zip = (it == g_global_data->g_file_to_shadowzip.end()) ? NULL : it->second;
	return shadow_zip;
}

static ShadowZip* get_cached_shadowzip(int fd)
{
	PthreadReadGuard(g_global_data->g_file_to_shadowzip_mutex);
	std::map<int, FILE*>::iterator it_fd = g_global_data->g_fd_to_file.find(fd);
	FILE* stream = (it_fd == g_global_data->g_fd_to_file.end()) ? NULL : it_fd->second;
	if (stream == NULL){return NULL;}
	std::map<FILE*, ShadowZip*>::iterator it = g_global_data->g_file_to_shadowzip.find(stream);
	ShadowZip* shadow_zip = (it == g_global_data->g_file_to_shadowzip.end()) ? NULL : it->second;
	return shadow_zip;
}

typedef FILE *(* FOpenType)(const char *path, const char *mode);
static FOpenType old_fopen = NULL;
static FILE *my_fopen(const char *path, const char *mode)
{	
	MY_METHOD("fopen([%s],[%s]) old:%zx :%zx", path, mode, (size_t)old_fopen, (size_t)(ShadowZip::old_fopen));
	if (old_fopen == NULL)
	{
		MY_ERROR("fopen is NULL");
		return old_fopen(path, mode);
	}
	
	check_set_old_function_to_shadow_zip();
	
	struct stat file_stat;
	memset(&file_stat, 0, sizeof(struct stat));
	if (old_stat(path, &file_stat) != 0) {
		
		MY_METHOD("not exist([%s],[%s])", path, mode);
		return old_fopen(path, mode);
	}
	
	if (g_apk_device_id == file_stat.st_dev && g_apk_ino == file_stat.st_ino)
	{
		ShadowZip* shadow_zip = new ShadowZip();
		FILE* fp = shadow_zip->fopen();
		if (fp == NULL){	
			MY_ERROR("something bad happens!");
			delete shadow_zip;
			return old_fopen(path, mode); 
		}	
		
		MY_LOG("shadow apk: %s", path);
		PthreadWriteGuard(g_global_data->g_file_to_shadowzip_mutex);
		g_global_data->g_file_to_shadowzip[fp] = shadow_zip;
		return fp;
	}
	
	MY_METHOD("not apk([%s],[%s])", path, mode);
	return old_fopen(path, mode);
}


typedef int (*FseekType)(FILE *stream, long offset, int whence);
static FseekType old_fseek = NULL;
static int my_fseek(FILE *stream, long offset, int whence)
{
	
	check_set_old_function_to_shadow_zip();
	
	ShadowZip* shadow_zip = get_cached_shadowzip(stream);
	if (shadow_zip == NULL){
		return old_fseek(stream, offset, whence);
	}else{
		return shadow_zip->fseek(stream, offset, whence);
	}
}

typedef long (*FtellType)(FILE *stream);
static FtellType old_ftell = NULL;
static int my_ftell(FILE *stream)
{
	
	check_set_old_function_to_shadow_zip();
	
	ShadowZip* shadow_zip = get_cached_shadowzip(stream);
	if (shadow_zip == NULL){
		return old_ftell(stream);
	}else{
		return shadow_zip->ftell(stream);
	}
}

typedef void (*RewindType)(FILE *stream);
static RewindType old_rewind = NULL;
static void myrewind(FILE *stream)
{
	
	check_set_old_function_to_shadow_zip();
	
	ShadowZip* shadow_zip = get_cached_shadowzip(stream);
	if (shadow_zip == NULL){
		old_rewind(stream);
	}else{
		shadow_zip->rewind(stream);
	}
}

typedef size_t (*FreadType)(void *ptr, size_t size, size_t nmemb, FILE *stream);
static FreadType old_fread = NULL;
static size_t my_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	
	check_set_old_function_to_shadow_zip();
	
	ShadowZip* shadow_zip = get_cached_shadowzip(stream);
	if (shadow_zip == NULL){
		return old_fread(ptr, size, nmemb, stream);
	}else{
		return shadow_zip->fread(ptr, size, nmemb, stream);
	}
}

typedef char * (*Fgets)(char *s, int size, FILE *stream);
static Fgets old_fgets = NULL;
static char * my_fgets(char *s, int size, FILE *stream)
{
	
	check_set_old_function_to_shadow_zip();
	
	ShadowZip* shadow_zip = get_cached_shadowzip(stream);
	if (shadow_zip == NULL){
		return old_fgets(s, size, stream);
	}else{
		return shadow_zip->fgets(s, size, stream);
	}
}

typedef int (*FcloseType)(FILE* _fp);
static FcloseType old_fclose = NULL;
static int my_fclose(FILE* stream)
{
	
	check_set_old_function_to_shadow_zip();
	
	ShadowZip* shadow_zip = NULL;
	{
		PthreadWriteGuard(g_global_data->g_file_to_shadowzip_mutex);
		std::map<FILE*, ShadowZip*>::iterator it = g_global_data->g_file_to_shadowzip.find(stream);
		shadow_zip = (it == g_global_data->g_file_to_shadowzip.end()) ? NULL : it->second;
		if (it != g_global_data->g_file_to_shadowzip.end()){
			g_global_data->g_file_to_shadowzip.erase(it);
		}	
	}
	
	if (shadow_zip == NULL){
		return old_fclose(stream);
	}else{
		return shadow_zip->fclose(stream);
	}
}

typedef void *(*DlopenType)(const char *filename, int flags);
static DlopenType old_dlopen = NULL;
static void *my_dlopen(const char *filename, int flags)
{
	
	check_set_old_function_to_shadow_zip();
	
	int il2cpp_postfix_len = strlen("libil2cpp.so");
	int len = strlen(filename);
	if (len > il2cpp_postfix_len && memcmp(filename + len - il2cpp_postfix_len, "libil2cpp.so", il2cpp_postfix_len) == 0)
	{		
		std::string bundle_id = get_bundle_id();	
		char link_file[256] = {0};
		snprintf(link_file, sizeof(link_file), "%s/libil2cpp.so", g_data_file_path);
		MY_LOG("redirect to %s", link_file);
		return old_dlopen(link_file, flags);
	}
	return old_dlopen(filename, flags);
}

typedef int (*OpenType)(const char *path, int flags, ...);
static OpenType old_open = NULL;
static int my_open(const char *path, int flags, ...)
{	
	if (old_open == NULL)
	{
		MY_ERROR("open is NULL");
		return -1;
	}
		
	check_set_old_function_to_shadow_zip();
	
	mode_t mode = -1;
	int has_mode = ((flags & O_CREAT) == O_CREAT) || ((flags & 020000000) == 020000000);
	if (has_mode)
	{
		va_list args;
		va_start(args, flags);
		mode = static_cast<mode_t>(va_arg(args, int));
		va_end(args);
	}
	
	struct stat file_stat;
	memset(&file_stat, 0, sizeof(struct stat));
	if (old_stat(path, &file_stat) != 0) {
		int ret = has_mode ? old_open(path, flags, mode) : old_open(path, flags);
		MY_METHOD("open: %s -> fd:0x%08x", path, ret);
		return ret;
	}
	
	if (g_apk_device_id == file_stat.st_dev && g_apk_ino == file_stat.st_ino)
	{
		ShadowZip* shadow_zip = new ShadowZip();
		FILE* fp = shadow_zip->fopen();
		if (fp == NULL){	
			MY_ERROR("something bad happens!");
			delete shadow_zip;
			int ret = has_mode ? old_open(path, flags, mode) : old_open(path, flags);
			MY_METHOD("open: %s -> fd:0x%08x", path, ret);
			return ret;
		}	
		int fd = fileno(fp);
		
		MY_LOG("shadow apk: %s, fd:0x%08x,", path, fd);
		PthreadWriteGuard(g_global_data->g_file_to_shadowzip_mutex);
		g_global_data->g_fd_to_file[fd] = fp;
		g_global_data->g_file_to_shadowzip[fp] = shadow_zip;
		return fd;
	}
	
	int ret = has_mode ? old_open(path, flags, mode) : old_open(path, flags);
	MY_METHOD("open: %s -> fd:0x%08x", path, ret);
	return ret;
}

typedef ssize_t(*ReadType)(int fd, void *buf, size_t nbyte);
static ReadType old_read = NULL;
static ssize_t my_read(int fd, void *buf, size_t nbyte)
{
	MY_METHOD("read: 0x%08x, %zu", fd, nbyte);
	
	check_set_old_function_to_shadow_zip();
	
	ssize_t ret = 0;
	ShadowZip* shadow_zip = get_cached_shadowzip(fd);
	if (shadow_zip == NULL){
		ret = old_read(fd, buf, nbyte);
	}else{
		ret = shadow_zip->fread(buf, 1, nbyte, (FILE*)(size_t)fd);
	}
	MY_METHOD("readed: 0x%08x, %zd", fd, ret);
	return ret;
}

typedef off_t (*LseekType)(int fd, off_t offset, int whence);
static LseekType old_lseek = NULL;
off_t my_lseek(int fd, off_t offset, int whence)
{
	MY_METHOD("lseek: 0x%08x, offset: 0x%08lx, whence: %d", fd, offset, whence);
	
	check_set_old_function_to_shadow_zip();
	
	off_t ret = 0;
	ShadowZip* shadow_zip = get_cached_shadowzip(fd);
	if (shadow_zip == NULL){
		ret = old_lseek(fd, offset, whence);
	}else{
		ret = shadow_zip->fseek((FILE*)(size_t)fd, offset, whence);
		if (ret == 0){ret = shadow_zip->ftell((FILE*)(size_t)fd);}
	}
	MY_METHOD("lseek: 0x%08x, return: %ld", fd, ret);
	return ret;
}

typedef off64_t (*Lseek64Type)(int fd, off64_t offset, int whence);
static Lseek64Type old_lseek64 = NULL;
off64_t my_lseek64(int fd, off64_t offset, int whence)
{
	MY_METHOD("lseek64: 0x%08x, offset: 0x%08llx, whence: %d", fd, (unsigned long long)offset, whence);
	
	check_set_old_function_to_shadow_zip();
	
	off64_t ret = 0;
	ShadowZip* shadow_zip = get_cached_shadowzip(fd);
	if (shadow_zip == NULL){
		ret =  old_lseek64(fd, offset, whence);
	}else{
		ret =  shadow_zip->fseek((FILE*)(size_t)fd, offset, whence);
		if (ret == 0){ret = shadow_zip->ftell((FILE*)(size_t)fd);}
	}
	MY_METHOD("lseek64: 0x%08x, return: %lld", fd, (unsigned long long)ret);
	return ret;
}

typedef int (*CloseType)(int fd);
static CloseType old_close = NULL;
static int my_close(int fd)
{
	MY_METHOD("my_close: 0x%08x", fd);
	
	check_set_old_function_to_shadow_zip();
	
	ShadowZip* shadow_zip = NULL;
	{
		PthreadWriteGuard(g_global_data->g_file_to_shadowzip_mutex);
		std::map<int, FILE*>::iterator it_fd = g_global_data->g_fd_to_file.find(fd);
		FILE* stream = (it_fd == g_global_data->g_fd_to_file.end()) ? NULL : it_fd->second;		
		if (stream == NULL){return old_close(fd);}
		g_global_data->g_fd_to_file.erase(it_fd);
		
		std::map<FILE*, ShadowZip*>::iterator it = g_global_data->g_file_to_shadowzip.find(stream);
		shadow_zip = (it == g_global_data->g_file_to_shadowzip.end()) ? NULL : it->second;
		if (it != g_global_data->g_file_to_shadowzip.end()){
			g_global_data->g_file_to_shadowzip.erase(it);
		}	
	}
	
	if (shadow_zip == NULL){
		return old_close(fd);
	}else{
		return shadow_zip->fclose((FILE*)(size_t)fd);
	}
}

static int init_hook(const std::string& bundle_id)
{
	/*
	char path[512] = {0};
	snprintf((char*)path, sizeof(path), "libunity.so");
	void const* addr = get_module_base(path, sizeof(path)-1);
	MY_INFO("libunity addr:0x%zx, path:%s", (size_t)addr, path);
	if (addr == NULL){	
		MY_ERROR("failed to load libunity info");
		return -1;
	}
	
	void *handle = dlopen(path, RTLD_LAZY);
	if (!handle) {
		MY_ERROR("failed to load libunity:%s, error:%s", path, dlerror());
		return -1;
	}
	
	std::vector<const char*> symbol_names;
	symbol_names.push_back("fopen");
	symbol_names.push_back("fseek");
	symbol_names.push_back("ftell");
	symbol_names.push_back("fread");
	symbol_names.push_back("fgets");
	symbol_names.push_back("fclose");
	symbol_names.push_back("stat");
	symbol_names.push_back("dlopen");
	symbol_names.push_back("open");
	symbol_names.push_back("read");
	symbol_names.push_back("lseek");
	symbol_names.push_back("lseek64");
	symbol_names.push_back("close");
	
	std::vector<void const *> new_func;
	new_func.push_back((void const *)my_fopen);
	new_func.push_back((void const *)my_fseek);
	new_func.push_back((void const *)my_ftell);
	new_func.push_back((void const *)my_fread);
	new_func.push_back((void const *)my_fgets);
	new_func.push_back((void const *)my_fclose);
	new_func.push_back((void const *)my_stat);
	new_func.push_back((void const *)my_dlopen);
	new_func.push_back((void const *)my_open);
	new_func.push_back((void const *)my_read);
	new_func.push_back((void const *)my_lseek);
	new_func.push_back((void const *)my_lseek64);
	new_func.push_back((void const *)my_close);
	
	std::vector<void **> old_func;
	old_func.push_back((void**)&old_fopen);
	old_func.push_back((void**)&old_fseek);
	old_func.push_back((void**)&old_ftell);
	old_func.push_back((void**)&old_fread);
	old_func.push_back((void**)&old_fgets);
	old_func.push_back((void**)&old_fclose);
	old_func.push_back((void**)&old_stat);
	old_func.push_back((void**)&old_dlopen);
	old_func.push_back((void**)&old_open);
	old_func.push_back((void**)&old_read);
	old_func.push_back((void**)&old_lseek);
	old_func.push_back((void**)&old_lseek64);
	old_func.push_back((void**)&old_close);
	
	elf_hook(path, addr, symbol_names, new_func, old_func);
	
	//dlclose(handle);
	*/
#define HOOK(lib, symbol) \
	if(0 != xhook_register(lib, #symbol, (void*)my_##symbol, (void **)&old_##symbol)){ \
		MY_ERROR("failed to find function:%s in %s", #symbol, lib); \
		return -1; \
	}

	//xhook_enable_debug(1);

	HOOK("libunity.so", fopen);
	HOOK("libunity.so", fseek);
	HOOK("libunity.so", ftell);
	HOOK("libunity.so", fread);
	HOOK("libunity.so", fgets);
	HOOK("libunity.so", fclose);
	HOOK("libunity.so", stat);
	HOOK("libunity.so", dlopen);
	HOOK("libunity.so", open);
	HOOK("libunity.so", read);
	HOOK("libunity.so", lseek);
	HOOK("libunity.so", lseek64);
	HOOK("libunity.so", close);
	
	if(0 != xhook_refresh(1)){
		MY_ERROR("failed to find replace function"); 
		return -1; 
	}	
	
	return 0;
}

static DlopenType art_old_dlopen = NULL;
static void *my_art_dlopen(const char *filename, int flags)
{
	MY_LOG("art redirect to %s", filename);
	int il2cpp_postfix_len = strlen("libil2cpp.so");
	int len = strlen(filename);
	if (len > il2cpp_postfix_len && memcmp(filename + len - il2cpp_postfix_len, "libil2cpp.so", il2cpp_postfix_len) == 0)
	{		
		std::string bundle_id = get_bundle_id();	
		char link_file[256] = {0};
		snprintf(link_file, sizeof(link_file), "%s/libil2cpp.so", g_data_file_path);
		MY_LOG("art redirect to %s", link_file);
		return art_old_dlopen(link_file, flags);
	}
	return art_old_dlopen(filename, flags);
}

static int init_art_hook()
{
//	char path[512] = {0};
//	snprintf((char*)path, sizeof(path), "libart.so");
//	void const* addr = get_module_base(path, sizeof(path)-1);
//	MY_LOG("libart addr:0x%zx, path:%s", (size_t)addr, path);
//	if (addr == NULL){	
//		MY_ERROR("failed to load libart info");
//		return -1;
//	}
//	
//	void *handle = dlopen(path, RTLD_LAZY);
//	if (!handle) {
//		MY_ERROR("failed to load libart:%s", dlerror());
//		return -1;
//	}
//	
//	std::vector<const char*> symbol_names;
//	symbol_names.push_back("dlopen");
//	
//	std::vector<void const *> new_func;
//	new_func.push_back((void const *)my_art_dlopen);
//	
//	std::vector<void **> old_func;
//	old_func.push_back((void**)&art_old_dlopen);
//	
//	elf_hook(path, addr, symbol_names, new_func, old_func);
//	
//	//dlclose(handle);
	return 0;
}

static void check_set_old_function_to_shadow_zip()
{
	if (NULL == ShadowZip::old_fopen){ShadowZip::old_fopen = old_fopen;}
	if (NULL == ShadowZip::old_fseek){ShadowZip::old_fseek = old_fseek;}
	if (NULL == ShadowZip::old_ftell){ShadowZip::old_ftell = old_ftell;}
	if (NULL == ShadowZip::old_fread){ShadowZip::old_fread = old_fread;}
	if (NULL == ShadowZip::old_fclose){ShadowZip::old_fclose = old_fclose;}
	if (NULL == ShadowZip::old_fgets){ShadowZip::old_fgets = old_fgets;}
}

static void bootstrap()
{
	std::string bundle_id = get_bundle_id();
	if (!verify_bundle_id( bundle_id.c_str() )){		
		MY_ERROR("bundle id not matched:" BUNDLE_ID);
		return;
	}
	
	std::string default_il2cpp_path;
	std::string patch_il2cpp_path;
	bool use_patch = extract_patch_info(bundle_id, default_il2cpp_path, patch_il2cpp_path);
	if (!verify_bundle_id( bundle_id.c_str() )){		
		MY_ERROR("bootstrap running failed.");
		return;
	}
	
	if (use_patch){
		MY_INFO("bootstrap running %s with apk_path:%s", TARGET_ARCH_ABI, g_apk_file_path);	
		bool success = (0 == ShadowZip::init(g_use_data_path, g_apk_file_path)) && (0 == init_hook(bundle_id)) && (0 == init_art_hook());
		if (success)
		{			
			static void *handle = dlopen(patch_il2cpp_path.c_str(), RTLD_NOW);
			if (!handle) {
				MY_ERROR("failed to load libil2cpp:%s, must exit", dlerror());
				_exit(-1);
				return;
			}
			
			check_set_old_function_to_shadow_zip();
			
			MY_INFO("bootstrap running with patch:%s", patch_il2cpp_path.c_str());
			//ShadowZip::output_apk(g_use_data_path);
		}// else we still do so hook
		else
		{		
			MY_INFO("bootstrap running failed with patch");
		}
	}
	else
	{
		MY_INFO("bootstrap running without patch");
	}
}

//static void entrance() __attribute__((constructor));
//void entrance() {
//}



