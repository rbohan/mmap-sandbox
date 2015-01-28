# mmap-sandbox

THIS COMES WITH NO WARRANTY OF ANY KIND - NOR IS IT OFFICIALLY ENDORSED OR SUPPORTED BY MONGODB INC, MONGODB LTD.

It contains source taken from the MongoDB open source project so like that is AGPL licensed. see https://github.com/mongodb/mongo/blob/master/GNU-AGPL-3.0.txt

### Create the source file:
`dd if=/dev/zero of=/tmp/1GB bs=1073741824 count=1`

### Compile:
`g++ mmap.cpp -o mmap`

### Run:
`echo 3 | sudo tee /proc/sys/vm/drop_caches # drop filesystem cache`
`./mmap`

Choose an option to perform the task. A confirmation message will appear in each case - hit enter again to proceed.

### Interesting things to note:
`while true; do ps aux | grep mmap | grep -v grep; done`
`top -p $PID_OF_MMAP`
`free -m -s 0.5`
