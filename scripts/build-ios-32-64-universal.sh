set -e

if [ "$1" != "Debug" ] && [ "$1" != "Release" ]; then
    echo "Please indicate build type: Debug or Release"
    exit 1
fi

echo "Building universal (OpenGL ES for arm64 and armv7) for iOS"

rm -rf tmp1
rm -rf tmp2

./build-ios.sh ios32 $1 opengles
mkdir tmp1
mv ../build/lib tmp1/
rm -rf ../build

./build-ios.sh ios $1 opengles
mkdir tmp2
mv ../build/lib tmp2/

mkdir ../build/lib

for f in tmp1/lib/*.a ; do
    lipo -create $f tmp2/lib/$(basename $f) -output ../build/lib/$(basename $f)
    Echo "Created universal (arm64 & armv7) $f." ;
done

cp tmp1/lib/interop.m ../build/lib/

rm -rf tmp1
rm -rf tmp2
rm -rf ../deps/lib
cp -rf ../build/lib ../deps/

echo Universal libraries built successfully and placed in build/lib

