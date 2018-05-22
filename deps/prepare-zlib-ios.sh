export ARCH=arm64 # armv7 or arm64
export CHOST=aarch64-apple-darwin* # arm-apple-darwin* or aarch64-apple-darwin*
export SDK=iphoneos

export SDKVERSION=9 #$(xcrun --sdk $SDK --show-sdk-version) # current version
export SDKROOT=$(xcrun --sdk $SDK --show-sdk-path) # current version
export PREFIX="/opt/$SDK-$SDKVERSION/$ARCH"

export CC=$(xcrun --sdk $SDK --find gcc)" -fembed-bitcode"
export CPP=$(xcrun --sdk $SDK --find gcc)" -E"
export CXX=$(xcrun --sdk $SDK --find g++)
export LD=$(xcrun --sdk $SDK --find ld)

export CFLAGS="$CFLAGS -arch $ARCH -isysroot $SDKROOT -I$PREFIX/include -miphoneos-version-min=$SDKVERSION"
export CPPFLAGS="$CPPFLAGS -arch $ARCH -isysroot $SDKROOT -I$PREFIX/include -miphoneos-version-min=$SDKVERSION"
export CXXFLAGS="$CXXFLAGS -arch $ARCH -isysroot $SDKROOT -I$PREFIX/include"
export LDFLAGS="$LDFLAGS -arch $ARCH -isysroot $SDKROOT -L$PREFIX/lib"
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH":"$SDKROOT/usr/lib/pkgconfig":"$PREFIX/lib/pkgconfig"

tar xvf zlib-1.2.11-noexample.tar.gz
cd zlib-1.2.11
./configure
make
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp zlib.h ../include/
cp zconf.h ../include/
cp libz.a ../lib/
cd ../
rm -rf zlib-1.2.11

