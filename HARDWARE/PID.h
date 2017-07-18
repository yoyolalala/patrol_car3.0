
void setKp(float kp);

/// Set the integral term. Use it AFTER setRefreshInterval or setRefreshRate
void setKi(float ki);

/// Set the derivative term. Use it AFTER setRefreshInterval or setRefreshRate
void setKd(float kd);

/// Set the P, I, D terms respectively. Use it AFTER setRefreshInterval or setRefreshRate
void setWeights(float kp, float ki, float kd);

/// Set the refresh interval of the controller in seconds.
/// If you don't know the interval you can set the frequency of update using setRefreshRate
void setRefreshInterval(float refresh_interval);

/// Set the refresh frequency of the controller in hertz.
/// If you don't know the refresh frequency you can set the refresh interval using setRefreshRate
void setRefreshRate(float refresh_rate);

/// Set the minimun error for computation of the PID loop. The default is 0.0
/// It can be set to prevent integral windup or updates with minimal errors


/// Set the lower limit of the output. Output will be clamped to this value.
/// If clamped, the integral will not be computed to prevent integral windup.
/// To set the upper limit use setOutputUpperLimit
/// YOU NEED TO SET IT!
void setOutputLowerLimit(float lower_limit);

/// Set the upper limit of the output. Output will be clamped to this value.
/// If clamped, the integral will not be computed to prevent integral windup.
/// To set the lower limit use setOutputLowerLimit
/// YOU NEED TO SET IT!
void setOutputUpperLimit(float upper_limit);

/// Set the desired point of the output.
/// This is the output the PID loop will try to reach.
/// error will be computed subtracting the feedback input from this value
void setDesiredPoint(float desired_point);

/// The refresh funcion that make all the PID computation.
/// Call it at specific rate, setted by setRefreshRate or setRefreshInterval (read descriptions).
/// (use a timer interrupt if used on a embedded microcontroller)
/// The feedback_input is the value from the feedback.
/// The returned value is the output value of the filter.
float refresh(float feedback_input);

/// ÷ÿ÷√PID
void reset();

void pidInit(float refresh_interval, float upper_limit, float lower_limit);