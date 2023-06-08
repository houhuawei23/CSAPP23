filename=$1
make $filename
echo "" >> res.txt
echo $filename >> res.txt
./$filename ./traces/gedit.trace.zst >> res.txt
echo "" >> res.txt