#undef DllSub
#undef DllVar
#undef ExtSub
#undef ExtVar

#ifdef LIB_USER

#define DllSub DllImport
#define DllVar DllImport

#else

#define DllSub DllExport
#define DllVar extern DllExport

#endif

#ifdef __cplusplus//ss
#define ExtSub extern "C" DllSub
#define ExtVar extern "C" DllVar
#else
#define ExtSub extern DllSub
#define ExtVar extern DllVar
#endif