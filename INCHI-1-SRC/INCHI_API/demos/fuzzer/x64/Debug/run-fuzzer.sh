prog_exec=./fuzzer.exe
test_name="pocs"
input_dir=.

date > $test_name.log
for f in `ls poc-*`; do
    ls $f 2>> $test_name.log
    $prog_exec $f >>$test_name.log 2>&1
done
