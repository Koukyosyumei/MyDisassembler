add_test([=[decoder.NOP]=]  /mnt/c/Users/kanka/Desktop/Dev/MyDisassembler/build/test/myplan-test [==[--gtest_filter=decoder.NOP]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[decoder.NOP]=]  PROPERTIES WORKING_DIRECTORY /mnt/c/Users/kanka/Desktop/Dev/MyDisassembler/build/test SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
set(  myplan-test_TESTS decoder.NOP)
