

#include "oversampling.h"


double IZero(double x_sq)
{
	long i;
	double new_term = 1;
	double b_func = 1;
	
	for (i = 1; new_term; i++)	// Gives Maximum Accuracy
	{
		new_term = new_term * x_sq * (1.0 / (4.0 * (double) i * (double) i));
		b_func += new_term;
	}
	
	return b_func;
}


float *generate_filter (long filter_length, double alpha, double cf)
{
	float *filter = malloc(sizeof(float) * filter_length);
	
	double sinc_arg;
	double x_sq, alpha_bessel_recip, val;
	
	long half_filter_length = filter_length >> 1;
	long i;
	
	// Check memory
	
	if (!filter)
		return 0;
	
	// First find bessel function of alpha
	
	alpha_bessel_recip = 1 / IZero(alpha * alpha);
	
	// Calculate first half of filter
	
	for (i = 0; i < half_filter_length; i++)
	{
		// Kaiser Window
		
		val = ((double) i) - ((double) half_filter_length) / half_filter_length;
		x_sq = (1 - val * val) * alpha * alpha;		
		val = IZero(x_sq) * alpha_bessel_recip;
		
		// Multiply with Sinc Function
		
		sinc_arg = M_PI * (double) (i - half_filter_length);			
		filter[i] = (sin (2 * cf * sinc_arg) / sinc_arg) * val;
	}
	
	// Limit Value
	
	filter[i++] = 2 * cf;
	
	// Copy symmetrical other half
	
	for ( ;i < filter_length; i++)
		filter[i] = filter[filter_length - i];
		
	return filter;
}		


void oversampling_free(t_oversampling *x)
{
	if (!x)
		return;
	
	partition_convolve_free(x->filter);
	free(x);
}


t_oversampling *oversampling_new(long fft_size, long filter_length, double alpha, double cf)
{
	t_oversampling *x = malloc(sizeof(t_oversampling));
	float *filter_temp;
	
	if (!x)
		return 0;

	x->filter = partition_convolve_new(fft_size, filter_length, 0, filter_length);
	alloc_memory_swap(&x->temporary_memory, 0, 0);
	filter_temp = generate_filter(filter_length, alpha, cf);
	
	if (!x->filter || filter_temp)
	{
		oversampling_free(x);
		return 0;
	}
	
	partition_convolve_set(x->filter, filter_temp, filter_length);
	free(filter_temp);
		
	return (x);
}

void oversampling_process_float(t_oversampling *x, float *in, vFloat *out, long vec_size)
{
	long oversampling_amount = x->oversampling_amount;
	long i;
	float *temporary_memory = grow_memory_swap(&x->temporary_memory, vec_size * sizeof(float), vec_size);
	
	if (temporary_memory)
	{
		for (i = 0; i < vec_size * oversampling_amount; i++)
			temporary_memory[i] = 0;
	
		for (i = 0; i < vec_size; i++)
			temporary_memory[i * oversampling_amount] = in[i];
	
		partition_convolve_process(x->filter, (vFloat *)temporary_memory, out, vec_size);
	}
}


void oversampling_process_double(t_oversampling *x, double *in, vFloat *out, long vec_size)
{
	long oversampling_amount = x->oversampling_amount;
	long i;
	float *temporary_memory = grow_memory_swap(&x->temporary_memory, vec_size * sizeof(float), vec_size);
	
	if (temporary_memory)
	{
		for (i = 0; i < vec_size * oversampling_amount; i++)
			temporary_memory[i] = 0;
		
		for (i = 0; i < vec_size; i++)
			temporary_memory[i * oversampling_amount] = in[i];
		
		partition_convolve_process(x->filter, (vFloat *)temporary_memory, out, vec_size);
	}	
}
