
#ifdef __cplusplus
extern "C" {
	struct t_oversampling;
#else
	
	
#include <partition_convolve.h>
#include <AH_Headers/AH_Generic_Memory_Swap.h>


typedef struct _oversampling
{
	t_partition_convolve *filter;	
	t_memory_swap temporary_memory;
	
	long oversampling_amount;
	
} t_oversampling;

#endif

void oversampling_free(t_oversampling *x);
t_oversampling *oversampling_new(long fft_size, long filter_length, double alpha, double cf);

void oversampling_process_float(t_oversampling *x, float *in, vFloat *out, long vec_size);
void oversampling_process_double(t_oversampling *x, double *in, vFloat *out, long vec_size);

#ifdef __cplusplus
}  /* this brace matches the one on the extern "C" line */
#endif