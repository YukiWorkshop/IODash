Benchmarks
=========

### Test Platform
- CPU: Intel(R) Core(TM) i7-9750H CPU @ 2.60GHz **(Turbo off)**
- RAM: Micron DDR4 32GB 3200MHz x 2
- Kernel: 5.4.15, `CONFIG_HZ_250`, `PREEMPT`
- Test command: `wrk -t 2 -c 10000 -d 10s`
- Memory profile command: `valgrind --tool=massif`
- Compiler: `gcc version 9.2.1 20191008 (Ubuntu 9.2.1-9ubuntu2)`
- Compile flags: `-O3 -march=native`

### Results
- [IODash](./IODash_HTTP.cpp)
```
$ ./wrk -t 2 -c 10000 -d 10s http://127.0.0.1:8082
Running 10s test @ http://127.0.0.1:8082
  2 threads and 10000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    44.83ms   47.22ms   1.45s    96.32%
    Req/Sec    22.29k   827.31    24.86k    76.76%
  434435 requests in 10.15s, 67.12MB read
  Socket errors: connect 0, read 136, write 0, timeout 0
Requests/sec:  42806.55
Transfer/sec:      6.61MB
```

![Memory usage](https://user-images.githubusercontent.com/34613827/81383314-6feea000-9142-11ea-949a-653d14d9afc2.png)

- [Boost](./boost_HTTP.cpp)
```
$ ./wrk -t 2 -c 10000 -d 10s http://127.0.0.1:8083 
Running 10s test @ http://127.0.0.1:8083
  2 threads and 10000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    33.99ms  143.76ms   1.98s    95.26%
    Req/Sec    18.32k     1.37k   30.39k    91.49%
  351446 requests in 10.08s, 54.30MB read
  Socket errors: connect 0, read 0, write 0, timeout 1065
Requests/sec:  34882.48
Transfer/sec:      5.39MB
```

![Memory usage](https://user-images.githubusercontent.com/34613827/81383650-ed1a1500-9142-11ea-9a58-e79c8b1c6b15.png)


- [Node.js](./node_HTTP.js)

node version: `v14.2.0` installed by `n`
```
$ ./wrk -t 2 -c 10000 -d 10s http://127.0.0.1:8080
Running 10s test @ http://127.0.0.1:8080
  2 threads and 10000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   389.06ms  439.87ms   2.00s    70.86%
    Req/Sec     5.62k     6.10k   20.80k    80.56%
  61751 requests in 10.46s, 9.54MB read
  Socket errors: connect 0, read 0, write 0, timeout 1251
Requests/sec:   5904.13
Transfer/sec:      0.91MB
```

- [libuv](./libuv_HTTP.c)
```
$ ./wrk -t 2 -c 10000 -d 10s http://127.0.0.1:10000
Running 10s test @ http://127.0.0.1:10000
  2 threads and 10000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   749.85ms  598.27ms   1.98s    53.53%
    Req/Sec     4.73k     3.27k   11.73k    77.42%
  60663 requests in 10.88s, 0.00B read
  Socket errors: connect 0, read 60663, write 0, timeout 0
Requests/sec:   5576.72
Transfer/sec:       0.00B
```


## Help Wanted

The libuv test is based on an example randomly found on the Internet and is nearly broken. If you are good at libuv stuff, you could improve it.