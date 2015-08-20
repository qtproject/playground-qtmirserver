#ifdef QT_MIR_NO_TRACEPOINTS
# define tracepoint(...)
#else
# include "tracepoints.h" // generated from tracepoints.tp
#endif
