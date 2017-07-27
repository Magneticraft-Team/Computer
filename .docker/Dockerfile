#https://github.com/cout970/mips-emulator/issues/1
#https://hub.docker.com/_/gcc/
FROM gcc:7.1.0

ENV PROJECT_DIR=/app
ENV PREFIX=$PROJECT_DIR/compiler
ENV TARGET="mipsel-none-elf"
ENV PATH="$PREFIX/bin:$PATH"

WORKDIR $PREFIX

RUN wget http://mirrors-usa.go-parts.com/gcc/releases/gcc-5.3.0/gcc-5.3.0.tar.gz
RUN tar xzvf gcc-5.3.0.tar.gz

### GCC

# install gcc prerequisites
RUN cd gcc-5.3.0 \
	&& ./contrib/download_prerequisites \
	&& cd ..
	
# configure
RUN mkdir build-gcc \
	&& cd build-gcc \
	&& ../gcc-5.3.0/configure --disable-multilib \
	&& cd ..
	
# compile and install
RUN cd build-gcc \
	&& make -j8 \
	&& make install \
	&& cd ..
	
### BINUTILS

RUN wget https://ftp.gnu.org/gnu/binutils/binutils-2.25.1.tar.gz
RUN tar xzvf binutils-2.25.1.tar.gz

# configure
RUN mkdir build-binutils \
	&& cd build-binutils \
	&& ../binutils-2.25.1/configure --target=$TARGET --prefix="$PREFIX" --disable-multilib --disable-nls --enable-lto --with-sysroot --disable-werror \
	&& cd ..
	
# compiler and install	
RUN cd build-binutils &&\
	make -j8 \
	&& make install \
	&& cd ..
	
# gcc configure
RUN mkdir xgcc-mips \
	&& cd xgcc-mips \
	&& ../gcc-5.3.0/configure --target=mipsel-none-elf --disable-nls --enable-lto --enable-languages=c,c++ \
	&& cd ..

	
# gcc compile and install
RUN cd xgcc-mips \
	&& make -j8 all-gcc \
	&& make install-gcc \
	&& cd ..

### NEWLIB

RUN wget ftp://sourceware.org/pub/newlib/newlib-2.3.0.20160104.tar.gz
RUN tar xzvf newlib-2.3.0.20160104.tar.gz

# configure
RUN mkdir xnewlib-mips \
	&& cd xnewlib-mips \
	&& ../newlib-2.3.0.20160104/configure --target=mipsel-none-elf --enable-lto \
	&& cd ..

# compile and install	
RUN cd xnewlib-mips \
	&& make -j8 \
	&& make install \
	&& cd ..

# final gcc
RUN cd xgcc-mips \
	&& make -j8 \
	&& make install \
	&& cd ..

COPY . $PROJECT_DIR/src
WORKDIR $PROJECT_DIR/src
