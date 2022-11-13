<?php

function test_apcu(int $n, $value) {
    for ($i = 0; $i < 16; $i++) {
        if (!apcu_store('key' . $i, $value)) {
            var_export(apcu_sma_info());
            throw new RuntimeException("Failed to store value\n");
        }
    }
    $start = hrtime(true);
    for ($i = 0; $i < $n; $i++) {
        apcu_fetch('key' . ($i & 15), $value);
    }
    $end = hrtime(true);
    $elapsed = ($end - $start)/1e9;
    printf("APCu            Elapsed: %.6f throughput %10.0f / second\n", $elapsed,  (1/$elapsed * $n));
    // printf("count=%d\n", count((array)immutable_cache_fetch('key0', $value)));
}
function test_immutablecache(int $n, $value) {
    for ($i = 0; $i < 16; $i++) {
        if (!immutable_cache_add('imm' . $i, $value)) {
            // expected, since there are multiple processes from pcntl and only the first will succeed
        }
    }
    $start = hrtime(true);
    for ($i = 0; $i < $n; $i++) {
        immutable_cache_fetch('key' . ($i & 15), $value);
    }
    $end = hrtime(true);
    $elapsed = ($end - $start)/1e9;
    printf("immutable_cache Elapsed: %.6f throughput %10.0f / second\n", $elapsed,  (1/$elapsed * $n));
    // printf("count=%d\n", count((array)apcu_fetch('key0', $value)));
}
define('N', (int)($_ENV['BENCHMARK_N'] ?? 400000));
define('NUM_PROCESSES', (int)($_ENV['BENCHMARK_NUM_PROCESSES'] ?? 4));
define('ARRAY_SIZE', (int)($_ENV['BENCHMARK_ARRAY_SIZE'] ?? 8));
define('USE_STRING_INSTEAD', (int)($_ENV['BENCHMARK_USE_STRING_INSTEAD'] ?? 0));
define('USE_SCALAR_INSTEAD', (int)($_ENV['BENCHMARK_USE_SCALAR_INSTEAD'] ?? 0));

if (USE_SCALAR_INSTEAD) {
    $value = 42;
} else if (USE_STRING_INSTEAD) {
    $value = str_repeat('.', ARRAY_SIZE);
} else {
    $value = [];
    for ($i = 0; $i < ARRAY_SIZE; $i++) {
        $value["key$i"] = "myValue$i";
    }
}

printf("Testing string->string array of size %d with %d forked processes\n", ARRAY_SIZE, NUM_PROCESSES);
echo "Note that the immutable array and strings part of shared memory in immutable_cache, where apcu must make copies of strings and arrays to account for eviction (and php must free them)\n";
echo json_encode($value), "\n\n";
printf("apc.serializer = %s\nimmutable_cache.serializer = %s\n", ini_get('apc.serializer'), ini_get('immutable_cache.serializer'));

$pids = [];
for ($i = 0; $i < NUM_PROCESSES; $i++) {
    $result = pcntl_fork();
    if ($result <= 0) {
        test_immutablecache(N, $value);
        if (extension_loaded('apcu')) {
            test_apcu(N, $value);
        }
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
/*
E.g. to retrieve multiple versions of the fake cached config
{"key0":"myValue0","key1":"myValue1","key2":"myValue2","key3":"myValue3","key4":"myValue4","key5":"myValue5","key6":"myValue6","key7":"myValue7"}
as a php array with 4 processes, repeatedly (with immutable_cache.enable_cli=1, immutable_cache.enabled=1, etc).

APCu            Elapsed: 0.339511 throughput    1178166 / second
APCu            Elapsed: 0.342087 throughput    1169294 / second
APCu            Elapsed: 0.424223 throughput     942901 / second
APCu            Elapsed: 0.424893 throughput     941413 / second
immutable_cache Elapsed: 0.109808 throughput    3642719 / second
immutable_cache Elapsed: 0.117099 throughput    3415902 / second
immutable_cache Elapsed: 0.099313 throughput    4027680 / second
immutable_cache Elapsed: 0.105851 throughput    3778904 / second
 */
