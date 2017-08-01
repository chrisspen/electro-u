setpoint = 0.34
samples = [
    #input,output
    [0.4,49],
    [0.33,58],
    [0.3,66],
]
kp=850
ki=0.5
kd=2

first = True
for input, expected_output in samples:
    if not first:
        error = setpoint - input
        dInput = (input - lastInput)
        outputSum = 0
        outputSum += ki * error
        outputSum -= kp * dInput;        
        actual_output = outputSum - kd * dInput
        print('output:', actual_output, expected_output)
    lastInput = input
    first = False
