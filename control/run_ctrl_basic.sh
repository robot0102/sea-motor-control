# Get test_function:
if [ $# -gt 0 ]
then
    TEST_FUNC_NAME=$1
else
    TEST_FUNC_NAME=""
    echo "Must give name of real-time executable."
    exit 1 
fi

EXEC_NAME="bin/test_ctrl_basic" 

sudo ./init_can.sh 
echo $(pwd) 
sudo ./$EXEC_NAME $TEST_FUNC_NAME 
