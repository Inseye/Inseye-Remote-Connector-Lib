#if !defined(LIB_EXPORT)
#define LIB_EXPORT /* NOTHING */

#if defined(WIN32) || defined(WIN64)
#undef LIB_EXPORT
#if defined(Remote_Connector_Dlib_EXPORTS)
#define LIB_EXPORT __declspec(dllexport)
#else
#define LIB_EXPORT __declspec(dllimport)
#endif // defined(Remote_Connector_Dlib_EXPORTS)
#endif // defined(WIN32) || defined(WIN64)

#if defined(__GNUC__) || defined(__APPLE__) || defined(LINUX)
#if defined(Remote_Connector_Dlib_EXPORTS)
			#undef LIB_EXPORT
			#define LIB_EXPORT __attribute__((visibility("default")))
		#endif // defined(DLib_EXPORTS)
#endif // defined(__GNUC__) || defined(__APPLE__) || defined(LINUX)

#endif // !defined(LIB_EXPORT)


