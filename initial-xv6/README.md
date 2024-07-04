# Testing system calls

## Running Tests for getreadcount

Just do the following frominside the `initial-xv6` directory:

```sh
prompt> ./test-getreadcounts.sh
```
You can suppress the repeated building of xv6 in the tests with the
`-s` flag. This should make repeated testing faster:

```sh
prompt> ./test-getreadcounts.sh -s
```

If the implementation works correctly, you should get some notification
that the tests passed.

The tests assume that xv6 source code is found in the `src/` subdirectory.
If it's not there, the script will complain.

---

## Running Tests for sigalarm and sigreturn

**After implementing both sigalarm and sigreturn**, do the following:
- Make the entry for `alarmtest` in `src/Makefile` inside `UPROGS`
- Run the command inside xv6:
    ```sh
    prompt> alarmtest
    ```

---

## Getting runtimes and waittimes for your schedulers
- Run the following command in xv6:
    ```sh
    prompt> schedulertest
    ```  
---

## Running tests for entire xv6 OS
- Run the following command in xv6:
    ```sh
    prompt> usertests
    ```

---