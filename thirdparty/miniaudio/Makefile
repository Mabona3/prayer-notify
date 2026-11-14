AR=ar
AR_FLAGS=rcs 
BUILD_DIR?=build
LIB_NAME=miniaudio
CFLAGS+=-DMINIAUDIO_IMPLEMENTATION -xc

${BUILD_DIR}/lib${LIB_NAME}.a:${BUILD_DIR}/${LIB_NAME}.o
	${AR} ${AR_FLAGS} $@ $^ 

${BUILD_DIR}/${LIB_NAME}.o:${LIB_NAME}.h
	mkdir -p ${BUILD_DIR}
	${CC} ${CFLAGS} -c $^ -o $@

clean:
	${RM} -r ${BUILD_DIR}
