
# redis-benchmark

* Date: 2020-02-08
* Commit: 5eb9fb5344bde1318201aa2d4abad7dddf07de20

### redis-server

```
$ redis-benchmark -r 1000000 -n 4000000 -t ping,set,get -P 16 -q 
PING_INLINE: 1267427.12 requests per second
PING_BULK: 1695633.75 requests per second
SET: 906412.88 requests per second
GET: 970402.69 requests per second
```

```
$ redis-benchmark -r 1000000 -n 4000000 -t ping,get -P 16 -q 
PING_INLINE: 1281640.50 requests per second
PING_BULK: 1706484.62 requests per second
GET: 1514577.75 requests per second
```

### protodb1, 1 threads

```
$ redis-benchmark -r 1000000 -n 4000000 -t ping,set,get -P 16 -q
PING_INLINE: 1869158.75 requests per second
PING_BULK: 1845869.88 requests per second
SET: 905592.06 requests per second
GET: 844416.31 requests per second
```

```
$ redis-benchmark -r 1000000 -n 10000000 -t ping,get -P 16 -q 
PING_INLINE: 1825150.50 requests per second
PING_BULK: 1833180.62 requests per second
GET: 1789549.12 requests per second
```

### protodb1, 2 threads

```
$ redis-benchmark -r 1000000 -n 4000000 -t ping,get -P 16 -q 
PING_INLINE: 1817355.75 requests per second
PING_BULK: 1806684.75 requests per second
GET: 1789709.25 requests per second
```
