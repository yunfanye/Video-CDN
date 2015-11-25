#!/usr/bin/expect

set local_dir "/Users/Kobe/Downloads/15-441-project-3"
set guest_ip "128.237.141.162"
set password "proj3"

spawn rsync -av $local_dir proj3@$guest_ip:~/
expect "password:"
send "$password\r"
interact