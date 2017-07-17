#include "PID.h"
float error, interval, set_point, output_lower_limit, output_upper_limit, last_error, last_output, error_threshold, integral, Ki, Kd, Kp;
void setKp(float kp) 
{
	Kp = kp;
}

void setKi(float ki)
{
	Ki=ki*interval;
}

void setKd(float kd)
{
	Kd = kd / interval;
}

void setWeights(float kp, float ki, float kd)
{
	setKp(kp);
	setKi(ki);
	setKd(kd);
}

void setRefreshInterval(float refresh_interval) 
{
	interval = refresh_interval;
}

void setRefreshRate(float refresh_rate) {
	interval = 1.f / refresh_rate;
}

void setErrorThreshold(float threshold) 
{
	error_threshold = threshold;
}

void setOutputLowerLimit(float lower_limit) 
{
	output_lower_limit = lower_limit;
}

void setOutputUpperLimit(float upper_limit)
{
	output_upper_limit = upper_limit;
}

void setDesiredPoint(float desired_point) 
{
	set_point = desired_point;
}

float refresh( float feedback_input) 
{
	error = set_point - feedback_input;
	last_output = Kp*error + Ki*integral + Kd*(error - last_error);
	if (last_output > output_upper_limit) 
	{
		last_output = output_upper_limit;
		if (integral / error < 0.f) 
			{
				integral += (error + last_error) / 2.f;
				last_error = error;
			}
		return output_upper_limit;
	}
	if (last_output < output_lower_limit)
	{
		if (integral / error < 0.f) 
			{
				integral += (error + last_error) / 2.f;
				last_error = error;
			}
		last_output = output_lower_limit;
		return output_lower_limit;
	}
	integral += (error + last_error) / 2.f;
	last_error = error;
	return last_output;
}

void reset()
{
	last_error = 0;
	last_output = 0;
	integral = 0;
	error = 0;
}
void pidInit(float refresh_interval, float threshold, float upper_limit, float lower_limit)
{
	setRefreshInterval(refresh_interval);
	setErrorThreshold(threshold);
	setOutputUpperLimit(upper_limit);
	setOutputLowerLimit(lower_limit);
}