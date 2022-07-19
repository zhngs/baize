APP_DIR=$(cd `dirname $0`/../; pwd)
echo "format files in $APP_DIR"

for file in `find $APP_DIR -name "*.h"`
do
    echo "format $file"
    clang-format -style=file -i $file
done

for file in `find $APP_DIR -name "*.cc"`
do
    echo "format $file"
    clang-format -style=file -i $file
done
