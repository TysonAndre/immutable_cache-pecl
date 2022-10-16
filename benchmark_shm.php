<?php

function test_apcu(int $n, $value) {
    for ($i = 0; $i < 16; $i++) {
        apcu_store('key' . $i, $value);
    }
    if (!apcu_store('key', $value)) {
        var_export(apcu_sma_info());
        throw new RuntimeException("Failed to store value\n");
    }
    $ser = serialize($value);
    $start = hrtime(true);
    for ($i = 0; $i < $n; $i++) {

        // apcu_store throughput empty new stdClass()
        //   100000 per second with 8  processes (4 real cores with 2 threads - effectively 4 since all of this is CPU-bound)
        //   230000 per second with 4  processes
        //   490000 per second with 2  processes
        //  3070000 per second with 1  process
        //apcu_store('key', $value);  // using different keys doesn't have much difference

        // unserialize only, no apcu (cpu benchmarking only)
        // 1420000 per second with 8 processes
        // 3080000 per second with 4 processes
        // 3680000 per second with 2 processes
        // 3770000 per second with 2 processes
        //unserialize($ser);

        // apcu_fetch throughput empty new stdClass(), default serializer
        //   350000 per second with 32
        //   860000 per second with 8 processes (97 no lock)
        //  1290000 per second with 4 processes (145 no lock, 170 no stats, 190 separate keys)
        //  1760000 per second with 2 processes (230 no lock)
        //  2660000 per second with 1 process   (260 no lock)

        // apcu_fetch throughput value=42, storing raw value
        //  1200000 per second with 8 processes
        //  1790000 per second with 4 processes (4100000 no lock)
        //  3000000 per second with 2 processes
        //  9850000 per second with 1 process
        //  I guess there's a bottleneck of 10 million apcu fetches per second. Can we hit that with a 128-core machine?
        //
        // This can be avoided if you run multiple apache server pools.
        apcu_fetch('key' . ($i & 15), $value);
    }
    $end = hrtime(true);
    $elapsed = ($end - $start)/1e9;
    printf("APCu Elapsed: %.6f throughput %10.0f\n", $elapsed,  (1/$elapsed * $n));
    //printf("count=%d\n", count((array)apcu_fetch('key', $value)));
}
const N = 4000000;
const NUM_PROCESSES = 4;
$value = 42;//new stdClass();

printf("Testing with %d processes\n", NUM_PROCESSES);
$pids = [];
for ($i = 0; $i < NUM_PROCESSES; $i++) {
    $result = pcntl_fork();
    if ($result <= 0) {
        //ob_start();
        // $outname = "out-" . getmypid() . ".out";
        //file_put_contents($outname, ob_get_clean());
        //test_shm(N, new stdClass());
        test_apcu(N, $value);
        //file_put_contents($outname, ob_get_clean());
        exit(0);
    } else {
        $pids[] = $result;
        echo "Child pid is $result\n";
    }
}
foreach ($pids as $pid) {
    pcntl_waitpid($pid, $status);
    echo "status of $pid is $status\n";
}
