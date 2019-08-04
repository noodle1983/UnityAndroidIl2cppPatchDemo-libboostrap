#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <android/log.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>


#include <vector>
#include <map>
#include <string>

class Il2CppChar;
class Il2CppMemoryCallbacks;
class Il2CppImage;
class Il2CppMethod;
typedef void* Il2CppMethodPointer;
class Il2CppClass;
class Il2CppImage;
class Il2CppAssembly;
class Il2CppArray;
typedef int32_t  il2cpp_array_size_t;
typedef uint32_t _BOOL4;
class FieldInfo;
class Il2CppType;
class Il2CppChar;
class Il2CppDomain;
class Il2CppException;
class Il2CppObject;
class Il2CppReflectionType;
class EventInfo;
class PropertyInfo;
class MethodInfo;
enum Il2CppStat { 	IL2CPP_STAT_NEW_OBJECT_COUNT, };
typedef void* il2cpp_register_object_callback;
typedef void* il2cpp_WorldChangedCallback;
class Il2CppReflectionMethod;
class Il2CppReflectionType;
class Il2CppProfiler;
typedef void* Il2CppProfileFunc;
enum Il2CppGCEvent { IL2CPP_GC_EVENT_START,};
enum Il2CppProfileFlags {IL2CPP_PROFILE_NONE = 0,};
class Il2CppString;
class Il2CppThread;
typedef void* Il2CppFrameWalkFunc;
enum Il2CppRuntimeUnhandledExceptionPolicy { IL2CPP_UNHANDLED_POLICY_LEGACY,};
class Il2CppStackFrameInfo;
class Il2CppManagedMemorySnapshot;
typedef void* Il2CppLogCallback;
typedef void* Il2CppSetFindPlugInCallback;
extern "C" {
	char* get_arch_abi();
	char* get_data_dir();
	char* use_data_dir(const char* data_path, const char* apk_path);
	
	void il2cpp_init(const char *domain_name);
	void il2cpp_init_utf16();
	void il2cpp_shutdown();
	void il2cpp_set_config_dir(const char *config_path);
	void il2cpp_set_data_dir(const char *data_path);
	void il2cpp_set_temp_dir(const char *temp_dir);
	void il2cpp_set_commandline_arguments(int argc, const char *const *argv);
	void il2cpp_set_commandline_arguments_utf16(const Il2CppChar *const *a1, int a2);
	void il2cpp_set_config_utf16(const Il2CppChar *executablePath);
	void il2cpp_set_config(const char *executablePath);
	void il2cpp_set_memory_callbacks(Il2CppMemoryCallbacks *callbacks);
	Il2CppImage *il2cpp_get_corlib();
	void il2cpp_add_internal_call(const char *name, Il2CppMethodPointer method);
	Il2CppMethodPointer il2cpp_resolve_icall(const char *a1);
	void *il2cpp_alloc(size_t size);
	void il2cpp_free(void *ptr);
	Il2CppClass *il2cpp_array_class_get(Il2CppClass *element_class, uint32_t rank);
	uint32_t il2cpp_array_length(Il2CppArray *array);
	uint32_t il2cpp_array_get_byte_length(Il2CppArray *array);
	Il2CppArray *il2cpp_array_new(Il2CppClass *elementTypeInfo, il2cpp_array_size_t length);
	Il2CppArray *il2cpp_array_new_specific(Il2CppClass *a1, il2cpp_array_size_t a2);
	Il2CppArray *il2cpp_array_new_full(Il2CppClass *array_class, il2cpp_array_size_t *lengths, il2cpp_array_size_t *lower_bounds);
	Il2CppClass *il2cpp_bounded_array_class_get(Il2CppClass *element_class, uint32_t rank, bool bounded);
	int32_t il2cpp_array_element_size(Il2CppClass *ac);
	Il2CppImage *il2cpp_assembly_get_image(const Il2CppAssembly *assembly);
	const Il2CppType *il2cpp_class_enum_basetype(Il2CppClass *klass);
	_BOOL4 il2cpp_class_is_generic(const Il2CppClass *klass);
	_BOOL4 il2cpp_class_is_inflated(const Il2CppClass *klass);
	_BOOL4 il2cpp_class_is_assignable_from(Il2CppClass *a1, Il2CppClass *a2);
	_BOOL4 il2cpp_class_is_subclass_of(Il2CppClass *klass, Il2CppClass *klassc, bool check_interfaces);
	_BOOL4 il2cpp_class_has_parent(Il2CppClass *a1, Il2CppClass *a2);
	Il2CppClass *il2cpp_class_from_il2cpp_type(const Il2CppType *a1);
	Il2CppClass *il2cpp_class_from_name(const Il2CppImage *image, const char *namespaze, const char *name);
	Il2CppClass *il2cpp_class_from_system_type(Il2CppReflectionType *type);
	Il2CppClass *il2cpp_class_get_element_class(Il2CppClass *klass);
	const EventInfo *il2cpp_class_get_events(Il2CppClass *a1, void **a2);
	FieldInfo *il2cpp_class_get_fields(Il2CppClass *a1, void **a2);
	Il2CppClass *il2cpp_class_get_nested_types(Il2CppClass *a1, void **a2);
	Il2CppClass *il2cpp_class_get_interfaces(Il2CppClass *a1, void **a2);
	const PropertyInfo *il2cpp_class_get_properties(Il2CppClass *a1, void **a2);
	const PropertyInfo *il2cpp_class_get_property_from_name(Il2CppClass *klass, const char *name);
	FieldInfo *il2cpp_class_get_field_from_name(Il2CppClass *klass, const char *name);
	const MethodInfo *il2cpp_class_get_methods(Il2CppClass *a1, void **a2);
	const MethodInfo *il2cpp_class_get_method_from_name(Il2CppClass *klass, const char *name, int argsCount);
	const char *il2cpp_class_get_name(Il2CppClass *klass);
	const char *il2cpp_class_get_namespace(Il2CppClass *klass);
	Il2CppClass *il2cpp_class_get_parent(Il2CppClass *klass);
	Il2CppClass *il2cpp_class_get_declaring_type(Il2CppClass *klass);
	int32_t il2cpp_class_instance_size(Il2CppClass *klass);
	size_t il2cpp_class_num_fields(const Il2CppClass *klass);
	_BOOL4 il2cpp_class_is_valuetype(const Il2CppClass *klass);
	int32_t il2cpp_class_value_size(Il2CppClass *a1, uint32_t *a2);
	_BOOL4 il2cpp_class_is_blittable(const Il2CppClass *klass);
	int il2cpp_class_get_flags(const Il2CppClass *klass);
	_BOOL4 il2cpp_class_is_abstract(const Il2CppClass *klass);
	_BOOL4 il2cpp_class_is_interface(const Il2CppClass *klass);
	int il2cpp_class_array_element_size(const Il2CppClass *a1);
	Il2CppClass *il2cpp_class_from_type(const Il2CppType *a1);
	const Il2CppType *il2cpp_class_get_type(Il2CppClass *klass);
	_BOOL4 il2cpp_class_has_attribute(Il2CppClass *klass, Il2CppClass *attr_class);
	_BOOL4 il2cpp_class_has_references(Il2CppClass *a1);
	_BOOL4 il2cpp_class_is_enum(const Il2CppClass *klass);
	const Il2CppImage *il2cpp_class_get_image(Il2CppClass *klass);
	const char *il2cpp_class_get_assemblyname(const Il2CppClass *klass);
	size_t il2cpp_class_get_bitmap_size(const Il2CppClass *klass);
	void il2cpp_class_get_bitmap(Il2CppClass *klass, size_t *bitmap);
	signed int il2cpp_stats_dump_to_file(const char *path);
	int il2cpp_stats_get_value(Il2CppStat stat);
	Il2CppDomain *il2cpp_domain_get();
	const Il2CppAssembly *il2cpp_domain_assembly_open(void*, const char *a1);
	std::vector<const Il2CppAssembly *,std::allocator<const Il2CppAssembly *> >::pointer il2cpp_domain_get_assemblies(const Il2CppDomain *domain, size_t *size);
	void __noreturn il2cpp_raise_exception( Il2CppException *a1);
	Il2CppException *il2cpp_exception_from_name_msg(const Il2CppImage *image, const char *name_space, const char *name, const char *msg);
	Il2CppException *il2cpp_get_exception_argument_null(const char *a1);
	void il2cpp_format_exception(const Il2CppException *ex, char *message, int message_size);
	void il2cpp_format_stack_trace(const Il2CppException *ex, char *output, int output_size);
	void il2cpp_unhandled_exception(void *a1);
	int il2cpp_field_get_flags(FieldInfo *field);
	const char *il2cpp_field_get_name(FieldInfo *field);
	Il2CppClass *il2cpp_field_get_parent(FieldInfo *field);
	size_t il2cpp_field_get_offset(FieldInfo *field);
	const Il2CppType *il2cpp_field_get_type(FieldInfo *field);
	void il2cpp_field_get_value(Il2CppObject *obj, FieldInfo *field, void *value);
	Il2CppObject *il2cpp_field_get_value_object(Il2CppObject *a1, FieldInfo *a2);
	_BOOL4 il2cpp_field_has_attribute(FieldInfo *field, Il2CppClass *attr_class);
	void il2cpp_field_set_value(Il2CppObject *obj, FieldInfo *field, void *value);
	void il2cpp_field_static_get_value(FieldInfo *a1, void *a2);
	void il2cpp_field_static_set_value(void *a1, FieldInfo *a2);
	void il2cpp_field_set_value_object(Il2CppObject *objectInstance, FieldInfo *field, Il2CppObject *value);
	void il2cpp_gc_collect(int maxGenerations);
	int32_t il2cpp_gc_collect_a_little();
	void il2cpp_gc_disable();
	void il2cpp_gc_enable();
	int64_t il2cpp_gc_get_used_size();
	int64_t il2cpp_gc_get_heap_size();
	uint32_t il2cpp_gchandle_new(Il2CppObject *obj, bool pinned);
	uint32_t il2cpp_gchandle_new_weakref(Il2CppObject *obj, bool track_resurrection);
	Il2CppObject *il2cpp_gchandle_get_target(uint32_t a1);
	void il2cpp_gchandle_free(uint32_t a1);
	void *il2cpp_unity_liveness_calculation_begin(Il2CppClass *filter, int max_object_count, il2cpp_register_object_callback callback, void *userdata, il2cpp_WorldChangedCallback onWorldStarted, il2cpp_WorldChangedCallback onWorldStopped);
	void il2cpp_unity_liveness_calculation_end(void *a1);
	void il2cpp_unity_liveness_calculation_from_root(Il2CppObject *root, void *state);
	void il2cpp_unity_liveness_calculation_from_statics(void *a1);
	const Il2CppType *il2cpp_method_get_return_type(const MethodInfo *method);
	Il2CppClass *il2cpp_method_get_declaring_type(const MethodInfo *method);
	const char *il2cpp_method_get_name(const MethodInfo *method);
	Il2CppReflectionMethod *il2cpp_method_get_object(const MethodInfo *a1, Il2CppClass *a2);
	_BOOL4 il2cpp_method_is_generic(const MethodInfo *method);
	_BOOL4 il2cpp_method_is_inflated(const MethodInfo *method);
	_BOOL4 il2cpp_method_is_instance(const MethodInfo *method);
	uint32_t il2cpp_method_get_param_count(const MethodInfo *method);
	const Il2CppType * il2cpp_method_get_param(uint32_t a1, const MethodInfo *a2);
	Il2CppClass *il2cpp_method_get_class(const MethodInfo *method);
	_BOOL4 il2cpp_method_has_attribute(const MethodInfo *method, Il2CppClass *attr_class);
	uint32_t il2cpp_method_get_flags(MethodInfo *a3, uint32_t *a4);
	uint32_t il2cpp_method_get_token(const MethodInfo *method);
	const char * il2cpp_method_get_param_name(uint32_t a1, const MethodInfo *a2);
	void il2cpp_profiler_install(Il2CppProfiler *prof, Il2CppProfileFunc shutdown_callback);
	void  il2cpp_profiler_set_events(int a1, Il2CppProfileFlags a2);
	void il2cpp_profiler_install_enter_leave(void (*a1)(Il2CppProfiler *, const MethodInfo *), void (*a2)(Il2CppProfiler *, const MethodInfo *));
	void  il2cpp_profiler_install_allocation(int a1, void (*a2)(Il2CppProfiler *, Il2CppObject *, Il2CppClass *));
	void il2cpp_profiler_install_gc(void (*a1)(Il2CppProfiler *, int64_t), void (*a2)(Il2CppProfiler *, Il2CppGCEvent, int));
	uint32_t il2cpp_property_get_flags(PropertyInfo *prop);
	const MethodInfo *il2cpp_property_get_get_method(PropertyInfo *prop);
	const MethodInfo *il2cpp_property_get_set_method(PropertyInfo *prop);
	const char *il2cpp_property_get_name(PropertyInfo *prop);
	Il2CppClass *il2cpp_property_get_parent(PropertyInfo *prop);
	Il2CppClass *il2cpp_object_get_class(Il2CppObject *obj);
	uint32_t il2cpp_object_get_size(Il2CppObject *a1);
	const MethodInfo *il2cpp_object_get_virtual_method(Il2CppObject *a1, const MethodInfo *a2);
	Il2CppObject *il2cpp_object_new(const Il2CppClass *klass);
	void *il2cpp_object_unbox(Il2CppObject *obj);
	Il2CppObject *il2cpp_value_box(void *a1, Il2CppClass *a2);
	void il2cpp_monitor_enter(Il2CppObject *obj);
	_BOOL4 il2cpp_monitor_try_enter(Il2CppObject *obj, uint32_t timeout);
	void il2cpp_monitor_exit(Il2CppObject *a1);
	void il2cpp_monitor_pulse(Il2CppObject *obj);
	void il2cpp_monitor_pulse_all(Il2CppObject *obj);
	void il2cpp_monitor_wait(Il2CppObject *obj);
	_BOOL4 il2cpp_monitor_try_wait(Il2CppObject *obj, uint32_t timeout);
	Il2CppObject *il2cpp_runtime_invoke(const MethodInfo *method, void *obj, void **params, Il2CppObject **exc);
	Il2CppObject *il2cpp_runtime_invoke_convert_args(const MethodInfo *method, void *obj, Il2CppObject **params, int paramCount, Il2CppException **exc);
	void il2cpp_runtime_class_init(Il2CppClass *a1);
	void il2cpp_runtime_object_init(Il2CppObject *obj);
	void il2cpp_runtime_object_init_exception(Il2CppException **a1, Il2CppObject *a2);
	void il2cpp_runtime_unhandled_exception_policy_set(Il2CppRuntimeUnhandledExceptionPolicy value);
	int32_t il2cpp_string_length(Il2CppString *str);
	Il2CppChar *il2cpp_string_chars(Il2CppString *str);
	Il2CppString *il2cpp_string_new(const char *str);
	Il2CppString *il2cpp_string_new_len(const char *str, uint32_t length);
	Il2CppString *il2cpp_string_new_utf16(const Il2CppChar *text, int32_t len);
	Il2CppString *il2cpp_string_new_wrapper(const char *str);
	Il2CppString *il2cpp_string_intern(Il2CppString *a1);
	Il2CppString * il2cpp_string_is_interned(Il2CppString *a1);
	char *il2cpp_thread_get_name(Il2CppThread *thread, uint32_t *len);
	Il2CppThread *il2cpp_thread_current();
	Il2CppThread *il2cpp_thread_attach(Il2CppDomain *domain);
	void il2cpp_thread_detach(Il2CppThread *thread);
	Il2CppThread **il2cpp_thread_get_all_attached_threads(size_t *size);
	_BOOL4 il2cpp_is_vm_thread(Il2CppThread *thread);
	void il2cpp_current_thread_walk_frame_stack(Il2CppFrameWalkFunc func, void *user_data);
	void il2cpp_thread_walk_frame_stack(Il2CppThread *thread, Il2CppFrameWalkFunc func, void *user_data);
	_BOOL4 il2cpp_current_thread_get_top_frame(Il2CppStackFrameInfo *frame);
	_BOOL4 il2cpp_thread_get_top_frame(Il2CppThread *thread, Il2CppStackFrameInfo *frame);
	_BOOL4 il2cpp_current_thread_get_frame_at(int32_t offset, Il2CppStackFrameInfo *frame);
	_BOOL4 il2cpp_thread_get_frame_at(Il2CppThread *thread, int32_t offset, Il2CppStackFrameInfo *frame);
	int32_t il2cpp_current_thread_get_stack_depth();
	int32_t il2cpp_thread_get_stack_depth(Il2CppThread *thread);
	Il2CppReflectionType *il2cpp_type_get_object(const Il2CppType *type);
	int il2cpp_type_get_type(const Il2CppType *type);
	Il2CppClass *il2cpp_type_get_class_or_element_class(const Il2CppType *type);
	char *il2cpp_type_get_name(const Il2CppType *type);
	const Il2CppAssembly *il2cpp_image_get_assembly(const Il2CppImage *image);
	const char *il2cpp_image_get_name(const Il2CppImage *image);
	const MethodInfo *il2cpp_image_get_entry_point(const Il2CppImage *image);
	Il2CppManagedMemorySnapshot *il2cpp_capture_memory_snapshot();
	void il2cpp_free_captured_memory_snapshot(Il2CppManagedMemorySnapshot *a1);
	void il2cpp_set_find_plugin_callback(Il2CppSetFindPlugInCallback method);
	void il2cpp_register_log_callback(Il2CppLogCallback method);
	const char *il2cpp_image_get_filename(const Il2CppImage *image); 
	
	//2018
	int il2cpp_class_get_type_token(int a1);
	int il2cpp_class_get_rank(int a1);
	int il2cpp_method_get_from_reflection(int a1, int a2, int a3, int a4);
	int il2cpp_profiler_install_fileio(int a1, int a2, int a3, int a4);
	int il2cpp_type_is_byref(int a1);
	int il2cpp_type_get_attrs(int a1);
	int il2cpp_type_equals();
	void *il2cpp_type_get_assembly_qualified_name();
	void il2cpp_debugger_set_agent_options();
	int il2cpp_is_debugger_attached();
	int il2cpp_unity_install_unitytls_interface(int a1);
	
	const char* const g_all_bridge_name[] = {
		"il2cpp_init",
		"il2cpp_init_utf16",
		"il2cpp_shutdown",
		"il2cpp_set_config_dir",
		"il2cpp_set_data_dir",
		"il2cpp_set_temp_dir",
		"il2cpp_set_commandline_arguments",
		"il2cpp_set_commandline_arguments_utf16",
		"il2cpp_set_config_utf16",
		"il2cpp_set_config",
		"il2cpp_set_memory_callbacks",
		"il2cpp_get_corlib",
		"il2cpp_add_internal_call",
		"il2cpp_resolve_icall",
		"il2cpp_alloc",
		"il2cpp_free",
		"il2cpp_array_class_get",
		"il2cpp_array_length",
		"il2cpp_array_get_byte_length",
		"il2cpp_array_new",
		"il2cpp_array_new_specific",
		"il2cpp_array_new_full",
		"il2cpp_bounded_array_class_get",
		"il2cpp_array_element_size",
		"il2cpp_assembly_get_image",
		"il2cpp_class_enum_basetype",
		"il2cpp_class_is_generic",
		"il2cpp_class_is_inflated",
		"il2cpp_class_is_assignable_from",
		"il2cpp_class_is_subclass_of",
		"il2cpp_class_has_parent",
		"il2cpp_class_from_il2cpp_type",
		"il2cpp_class_from_name",
		"il2cpp_class_from_system_type",
		"il2cpp_class_get_element_class",
		"il2cpp_class_get_events",
		"il2cpp_class_get_fields",
		"il2cpp_class_get_nested_types",
		"il2cpp_class_get_interfaces",
		"il2cpp_class_get_properties",
		"il2cpp_class_get_property_from_name",
		"il2cpp_class_get_field_from_name",
		"il2cpp_class_get_methods",
		"il2cpp_class_get_method_from_name",
		"il2cpp_class_get_name",
		"il2cpp_class_get_namespace",
		"il2cpp_class_get_parent",
		"il2cpp_class_get_declaring_type",
		"il2cpp_class_instance_size",
		"il2cpp_class_num_fields",
		"il2cpp_class_is_valuetype",
		"il2cpp_class_value_size",
		"il2cpp_class_is_blittable",
		"il2cpp_class_get_flags",
		"il2cpp_class_is_abstract",
		"il2cpp_class_is_interface",
		"il2cpp_class_array_element_size",
		"il2cpp_class_from_type",
		"il2cpp_class_get_type",
		"il2cpp_class_has_attribute",
		"il2cpp_class_has_references",
		"il2cpp_class_is_enum",
		"il2cpp_class_get_image",
		"il2cpp_class_get_assemblyname",
		"il2cpp_class_get_bitmap_size",
		"il2cpp_class_get_bitmap",
		"il2cpp_stats_dump_to_file",
		"il2cpp_stats_get_value",
		"il2cpp_domain_get",
		"il2cpp_domain_assembly_open",
		"il2cpp_domain_get_assemblies",
		"il2cpp_raise_exception",
		"il2cpp_exception_from_name_msg",
		"il2cpp_get_exception_argument_null",
		"il2cpp_format_exception",
		"il2cpp_format_stack_trace",
		"il2cpp_unhandled_exception",
		"il2cpp_field_get_flags",
		"il2cpp_field_get_name",
		"il2cpp_field_get_parent",
		"il2cpp_field_get_offset",
		"il2cpp_field_get_type",
		"il2cpp_field_get_value",
		"il2cpp_field_get_value_object",
		"il2cpp_field_has_attribute",
		"il2cpp_field_set_value",
		"il2cpp_field_static_get_value",
		"il2cpp_field_static_set_value",
		"il2cpp_field_set_value_object",
		"il2cpp_gc_collect",
		"il2cpp_gc_collect_a_little",
		"il2cpp_gc_disable",
		"il2cpp_gc_enable",
		"il2cpp_gc_get_used_size",
		"il2cpp_gc_get_heap_size",
		"il2cpp_gchandle_new",
		"il2cpp_gchandle_new_weakref",
		"il2cpp_gchandle_get_target",
		"il2cpp_gchandle_free",
		"il2cpp_unity_liveness_calculation_begin",
		"il2cpp_unity_liveness_calculation_end",
		"il2cpp_unity_liveness_calculation_from_root",
		"il2cpp_unity_liveness_calculation_from_statics",
		"il2cpp_method_get_return_type",
		"il2cpp_method_get_declaring_type",
		"il2cpp_method_get_name",
		"il2cpp_method_get_object",
		"il2cpp_method_is_generic",
		"il2cpp_method_is_inflated",
		"il2cpp_method_is_instance",
		"il2cpp_method_get_param_count",
		"il2cpp_method_get_param",
		"il2cpp_method_get_class",
		"il2cpp_method_has_attribute",
		"il2cpp_method_get_flags",
		"il2cpp_method_get_token",
		"il2cpp_method_get_param_name",
		"il2cpp_profiler_install",
		"il2cpp_profiler_set_events",
		"il2cpp_profiler_install_enter_leave",
		"il2cpp_profiler_install_allocation",
		"il2cpp_profiler_install_gc",
		"il2cpp_property_get_flags",
		"il2cpp_property_get_get_method",
		"il2cpp_property_get_set_method",
		"il2cpp_property_get_name",
		"il2cpp_property_get_parent",
		"il2cpp_object_get_class",
		"il2cpp_object_get_size",
		"il2cpp_object_get_virtual_method",
		"il2cpp_object_new",
		"il2cpp_object_unbox",
		"il2cpp_value_box",
		"il2cpp_monitor_enter",
		"il2cpp_monitor_try_enter",
		"il2cpp_monitor_exit",
		"il2cpp_monitor_pulse",
		"il2cpp_monitor_pulse_all",
		"il2cpp_monitor_wait",
		"il2cpp_monitor_try_wait",
		"il2cpp_runtime_invoke",
		"il2cpp_runtime_invoke_convert_args",
		"il2cpp_runtime_class_init",
		"il2cpp_runtime_object_init",
		"il2cpp_runtime_object_init_exception",
		"il2cpp_runtime_unhandled_exception_policy_set",
		"il2cpp_string_length",
		"il2cpp_string_chars",
		"il2cpp_string_new",
		"il2cpp_string_new_len",
		"il2cpp_string_new_utf16",
		"il2cpp_string_new_wrapper",
		"il2cpp_string_intern",
		"il2cpp_string_is_interned",
		"il2cpp_thread_get_name",
		"il2cpp_thread_current",
		"il2cpp_thread_attach",
		"il2cpp_thread_detach",
		"il2cpp_thread_get_all_attached_threads",
		"il2cpp_is_vm_thread",
		"il2cpp_current_thread_walk_frame_stack",
		"il2cpp_thread_walk_frame_stack",
		"il2cpp_current_thread_get_top_frame",
		"il2cpp_thread_get_top_frame",
		"il2cpp_current_thread_get_frame_at",
		"il2cpp_thread_get_frame_at",
		"il2cpp_current_thread_get_stack_depth",
		"il2cpp_thread_get_stack_depth",
		"il2cpp_type_get_object",
		"il2cpp_type_get_type",
		"il2cpp_type_get_class_or_element_class",
		"il2cpp_type_get_name",
		"il2cpp_image_get_assembly",
		"il2cpp_image_get_name",
		"il2cpp_image_get_entry_point",
		"il2cpp_capture_memory_snapshot",
		"il2cpp_free_captured_memory_snapshot",
		"il2cpp_set_find_plugin_callback",
		"il2cpp_register_log_callback",
		"il2cpp_image_get_filename",
		
		//2018
		"il2cpp_class_get_type_token",
		"il2cpp_class_get_rank",
		"il2cpp_method_get_from_reflection",
		"il2cpp_profiler_install_fileio",
		"il2cpp_type_is_byref",
		"il2cpp_type_get_attrs",
		"il2cpp_type_equals",
		"il2cpp_type_get_assembly_qualified_name",
		"il2cpp_debugger_set_agent_options",
		"il2cpp_is_debugger_attached",
		"il2cpp_unity_install_unitytls_interface",
	};
}


