# rwtools
Read / Write tools, from disk/volume/file to disk/volume/file. <br/>
Simple tool, Easy use.
<br/>
<br/>
# usage
``` batch
rwtools /from:value /offset1:value /to:value /offset2:value /block:value /size:value /nobuffer
   Usage:
     from       read data from
     offset1    offset of data source, default 0
     to         write data to, default console
     offset2    offset of data target, default 0
     size       read/write total size
     block      size of byte once, defalut 1MB
     nobuffer   open with no buffering
   
   Eg:
     rwtools /from:\\.\Z: /offset1:0 /block:4096
     rwtools /from:\\.\PHYSICALDRIVE1 /offset1:0 /to:D:\data.dat
     rwtools /from:Z:\test.dat /offset1:0 /to:\\.\Z: /block:4096 /nobuffer
     rwtools /from:\\.\Z: /offset1:0 /to:D:\data.dat /offset2:4096 /block:8192 /size:16384
```
