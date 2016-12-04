#pragma once

#ifdef EXPORT_Log
# define Log_API __declspec(dllexport)
#else
# define Log_API __declspec(dllimport)
#endif