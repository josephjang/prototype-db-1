
# Benchmarks

## Benchmarks without Pipelined Requests

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

```
$ ./memtier_benchmark --hide-histogram --threads 4 –data-size 8 
[...]
=========================================================================
Type         Ops/sec     Hits/sec   Misses/sec      Latency       KB/sec 
-------------------------------------------------------------------------
Sets        13850.07          ---          ---      1.37800      1066.70 
Gets       138348.50         0.00    138348.50      1.35900      5389.27 
Waits           0.00          ---          ---      0.00000          --- 
Totals     152198.57         0.00    138348.50      1.36100      6455.97 
```

### protodb1, 2 threads

```
$ redis-benchmark -r 1000000 -n 4000000 -t ping,get -P 16 -q 
PING_INLINE: 1817355.75 requests per second
PING_BULK: 1806684.75 requests per second
GET: 1789709.25 requests per second
```

```
$ ./memtier_benchmark --hide-histogram --threads 4 –data-size 8 
[...]
=========================================================================
Type         Ops/sec     Hits/sec   Misses/sec      Latency       KB/sec 
-------------------------------------------------------------------------
Sets        24070.41          ---          ---      0.76700      1853.84 
Gets       240439.62         0.00    240439.62      0.75400      9366.16 
Waits           0.00          ---          ---      0.00000          --- 
Totals     264510.03         0.00    240439.62      0.75500     11220.00 
```

### protodb1, 4 threads

```
$ ./memtier_benchmark --hide-histogram --threads 4 –data-size 8
[...]
=========================================================================
Type         Ops/sec     Hits/sec   Misses/sec      Latency       KB/sec 
-------------------------------------------------------------------------
Sets        40841.95          ---          ---      0.45600      3145.55 
Gets       407970.64         0.00    407970.64      0.44200     15892.22 
Waits           0.00          ---          ---      0.00000          --- 
Totals     448812.59         0.00    407970.64      0.44300     19037.76 
```

## Benchmarks with Pipelined Requests

* Date: 2020-02-13
* Commit: c460ace415494c564f797cc3fab816a6c1f43f21

### protodb1, 1 threads, 16 pipeline

```
 $ ./memtier_benchmark --hide-histogram --threads 4 –data-size 8 --pipeline=16
[...]
=========================================================================
Type         Ops/sec     Hits/sec   Misses/sec      Latency       KB/sec 
-------------------------------------------------------------------------
Sets       147994.79          ---          ---      1.95800     11398.19 
Gets      1478321.61         0.00   1478321.61      1.96200     57587.01 
Waits           0.00          ---          ---      0.00000          --- 
Totals    1626316.40         0.00   1478321.61      1.96200     68985.20 
```

### protodb1, 2 threads, 16 pipeline

```
$ ./memtier_benchmark --hide-histogram --threads 4 –data-size 8 --pipeline=16
[...]
=========================================================================
Type         Ops/sec     Hits/sec   Misses/sec      Latency       KB/sec 
-------------------------------------------------------------------------
Sets       291407.02          ---          ---      1.01600     22443.44 
Gets      2910867.88         0.00   2910867.88      1.01700    113390.87 
Waits           0.00          ---          ---      0.00000          --- 
Totals    3202274.90         0.00   2910867.88      1.01700    135834.31 
```

### protodb1, 4 threads, 16 pipeline

```
$ ./memtier_benchmark --hide-histogram --threads 4 –data-size 8 --pipeline=16
[...]
=========================================================================
Type         Ops/sec     Hits/sec   Misses/sec      Latency       KB/sec 
-------------------------------------------------------------------------
Sets       355271.69          ---          ---      0.81500     27362.14 
Gets      3548812.77         0.00   3548812.77      0.81500    138241.57 
Waits           0.00          ---          ---      0.00000          --- 
Totals    3904084.45         0.00   3548812.77      0.81500    165603.71 
```
