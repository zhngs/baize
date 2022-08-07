SCRIPT_DIR=$(cd `dirname $0`; pwd)
PROJECT_DIR=`cd $SCRIPT_DIR/..; pwd`

function clang_format()
{
    echo "format files in $1"
    cd $1

    for file in `find -name "*.h"`
    do
        echo "format $file"
        # clang-format -style=file -i $file
    done

    for file in `find -name "*.cc"`
    do
        echo "format $file"
        # clang-format -style=file -i $file
    done
}

clang_format $PROJECT_DIR/kernel
clang_format $PROJECT_DIR/quic