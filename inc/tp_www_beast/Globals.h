#ifndef tp_www_beast_Globals_h
#define tp_www_beast_Globals_h

#include "tp_utils/Globals.h" // IWYU pragma: keep

#if defined(TP_WWW_BEAST_LIBRARY)
#  define TP_WWW_BEAST_EXPORT TP_EXPORT
#else
#  define TP_WWW_BEAST_EXPORT TP_IMPORT
#endif

//##################################################################################################
//! Serve tp_www using Boost Beast.
namespace tp_www_beast
{

}

#endif
