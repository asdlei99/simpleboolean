import os

TEST_APP = "Debug/test.app/Contents/MacOS/test"

testTotal = 0
testSuccess = 0

def runTest(a, b):
    global testTotal
    global testSuccess
    testTotal += 1
    cmd = TEST_APP + " ../models/" + a + " ../models/" + b + ""
    print(cmd)
    errorCode = os.system(cmd)
    if (0 == errorCode):
        testSuccess += 1
        print("Test " + a + " " + b + " [PASS]")
    else:
        print("Test " + a + " " + b + " [FAIL]")

runTest("box.obj", "subdived-box.obj")
runTest("addax.obj", "meerkat.obj")

if (testSuccess == testTotal):
    print("All tests [PASS]")
    exit(0)
else:
    print(str(testSuccess) + "/" + str(testTotal) + " tests pass")
    exit(1)
