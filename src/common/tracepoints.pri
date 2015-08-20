TRACEPOINT_FILE = tracepoints.tp
tracepointgen.name = LTTng UST 2.0 tracepoint code generator
tracepointgen.input = TRACEPOINT_FILE
tracepointgen.output = ${QMAKE_FILE_BASE}.c
tracepointgen.commands = lttng-gen-tp ${QMAKE_FILE_IN} -o ${QMAKE_FILE_BASE}.c -o ${QMAKE_FILE_BASE}.h

QMAKE_EXTRA_COMPILERS += tracepointgen
tracepointgen.variable_out = SOURCES

PKGCONFIG += lttng-ust
