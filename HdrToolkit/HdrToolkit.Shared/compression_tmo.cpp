#include "pch.h"
#include <math.h>
#include <algorithm>
#include <iostream>

#include "psftmo_def.h"

#include "compression_tmo.h"

#include <vccorlib.h>
#include <fstream>

using namespace std;
#ifndef DEGEN_CASE
#define DEGEN_CASE
#endif
#undef DEGEN_CASE

/**
 * Lookup table on a uniform array & interpolation
 *
 * x_i must be at least two elements
 * y_i must be initialized after creating an object
 */
class UniformArrayLUT
{
    double start_v;
    size_t lut_size;


    bool own_y_i;
public:
    double *y_i;
	double delta;

    UniformArrayLUT( double from, double to, int lut_size, double *y_i = NULL  ) :
        start_v(from), lut_size( lut_size ), delta( (to-from)/(double)lut_size )
    {
        if( y_i == NULL ) {
            this->y_i = new double[lut_size];
            own_y_i = true;
        } else {
            this->y_i = y_i;
            own_y_i = false;
        }
    }

    UniformArrayLUT() : y_i(NULL), lut_size( 0 ), delta( 0. ), own_y_i( false ) {}

    UniformArrayLUT(const UniformArrayLUT& other) : start_v( other.start_v ), lut_size( other.lut_size ), delta( other.delta )
    {
        this->y_i = new double[lut_size];
        own_y_i = true;
        memcpy(this->y_i, other.y_i, lut_size * sizeof(double));
    }

    UniformArrayLUT& operator = (const UniformArrayLUT& other)
    {
        this->lut_size = other.lut_size;
        this->delta = other.delta;
        this->start_v = other.start_v;
        this->y_i = new double[lut_size];
        own_y_i = true;
        memcpy(this->y_i, other.y_i, lut_size * sizeof(double));
        return *this;
    }

    ~UniformArrayLUT()
    {
        if( own_y_i )
            delete []y_i;
    }

    double interp( double x )
    {
        const double ind_f = (x - start_v)/delta;
        const size_t ind_low = (size_t)(ind_f);
        const size_t ind_hi = (size_t)ceil(ind_f);

        if( unlikely(ind_f < 0) )           // Out of range checks
            return y_i[0];
        if( unlikely(ind_hi >= lut_size) )
            return y_i[lut_size-1];

        if( unlikely(ind_low == ind_hi) )
            return y_i[ind_low];      // No interpolation necessary
		double val = y_i[ind_low] + (y_i[ind_hi] - y_i[ind_low])*(ind_f - (double)ind_low); // Interpolation

		return val;// y_i[ind_low] + (y_i[ind_hi] - y_i[ind_low])*(ind_f - (double)ind_low); // Interpolation
    }

};


class ImgHistogram
{
public:
    const float L_min, L_max;
    const float delta;

	float img_L_min, img_L_max;
    int *bins;
    double *p;
    int bin_count;

	int zeroval_bin_index; // the bin for value zero


    ImgHistogram() : L_min( -6.f ), L_max( 9.f ), delta( 0.1 ), bins( NULL ), p( NULL )
    {
        bin_count = (int)ceil((L_max-L_min)/delta);
        bins = new int[bin_count];
        p = new double[bin_count];

		img_L_min = numeric_limits<float>::max();
		img_L_max = -img_L_min;
		zeroval_bin_index = 0; // set to zero by default
    }

    ~ImgHistogram()
    {
        delete [] bins;
        delete [] p;
    }

    void compute( const float *img, size_t pixel_count )
    {
		// Get L max from the image...

        std::fill( bins, bins + bin_count, 0 );


        int pp_count = 0;
        for( size_t pp = 0; pp < pixel_count; pp++ )
        {
            int bin_index = (img[pp]-L_min)/delta;


			img_L_min = min(img[pp], img_L_min);
			img_L_max = max(img[pp], img_L_max);

            // ignore anything outside the range
            if( bin_index < 0 || bin_index >= bin_count )
                continue;
            bins[bin_index]++;
            pp_count++;

        }

        for( int bb = 0; bb < bin_count; bb++ ) {
            p[bb] = (double)bins[bb] / (double)pp_count;
        }
    }

	void computeWithZeroBackgrnd(const float *img, float nonzero_min, size_t pixel_count)
	{
		// Get L max from the image...

		std::fill(bins, bins + bin_count, 0);
		float invDelta = 1.0f / delta;

		int nonZeroMinBinInd = (nonzero_min - L_min) * invDelta;
		// Actually set the zero value's bin index
		zeroval_bin_index = max(0, nonZeroMinBinInd - 1);

		int pp_count = 0;
		for (size_t pp = 0; pp < pixel_count; pp++)
		{
			// If we've got the zero value, use the valZero_BinIndex!
			int bin_index = (img[pp] == L_min) ? zeroval_bin_index : (img[pp] - L_min) * invDelta;

			img_L_min = min(img[pp], img_L_min);
			img_L_max = max(img[pp], img_L_max);

			// ignore anything outside the range
			if (bin_index == zeroval_bin_index || bin_index < 0 || bin_index >= bin_count)
				continue;
			bins[bin_index]++;
			pp_count++;

		}

		// Compute probability
		for (int bb = 0; bb < bin_count; bb++) {
			p[bb] = (double)bins[bb] / (double)pp_count;
		}
	}

};

inline float safelog10f( float x )
{
	if (x <= 0.0f)
		return -6.f;
  return log10f( x );
}



void mai11_tonemap( const float *R_in, const float *G_in, float *B_in, int width, int height,
                              float *R_out, float *G_out, float *B_out, const float *L_in,
                              pfstmo_progress_callback progress_cb )
{

    const size_t pix_count = width*height;

    // Compute log of Luminance
    float *logL = new float[pix_count];
//    std::unique_ptr<float[]> logL(new float[pix_count]);
	float nonzero_min = numeric_limits<float>::max(); // the minimum non-zero value in the image!
	float val_delta = 0.3f;
    for( size_t pp = 0; pp < pix_count; pp++ ) {
		if(L_in[pp] > 0.0)
			nonzero_min = min(nonzero_min, safelog10f(L_in[pp])); // smallest non-zero val

        logL[pp] = safelog10f( L_in[pp] + val_delta);
    }

	// Get the histogram bin 

    ImgHistogram H;
    //H.compute(logL, pix_count );
	
	// Use our zero-background version!
	H.computeWithZeroBackgrnd(logL, nonzero_min, pix_count);

    //Compute slopes
	auto installFolder = Windows::Storage::ApplicationData::Current->LocalFolder;
	wchar_t fileName[512];
	static int cnt = 0;
	int cx = swprintf_s(fileName, 512, L"tmoLut%i.txt", int(cnt));
	Platform::String^ sFilename = ref new Platform::String(fileName);

	Platform::String^ fullName = installFolder->Path + L"\\" + sFilename;
	ofstream ofLut(fullName->Data());
	cnt++;

//    std::unique_ptr<double[]> s(new double[H.bin_count]);
    double *s = new double[H.bin_count];
	double d = 0;
	double e = 1. / 3.;
    {
        for( int bb = 0; bb < H.bin_count; bb++ ) {
            d += pow( H.p[bb], e );
        }
        d *= H.delta;
		ofLut << "Original S = " << endl;
        for( int bb = 0; bb < H.bin_count; bb++ ) {
            s[bb] = pow( H.p[bb], e )/d;

			ofLut << s[bb] << endl;
        }

    }

#ifdef DEGEN_CASE
    // TODO: Handling of degenerated cases, e.g. when an image contains uniform color
    const double s_max = 1.0 / safelog10f(1.01); // Maximum slope, to avoid enhancing noise
    double s_renorm = 0;
    for( int bb = 0; bb < H.bin_count; bb++ ) {
        if( s[bb] >= s_max ) {
            s[bb] = s_max;
            s_renorm += s_max * H.delta * pow(H.p[bb], e);
        }
    }
	ofLut << "Renormalized S = " << endl;
    for( int bb = 0; bb < H.bin_count; bb++ ) {
        if( s[bb] < s_max ) {
            //s[bb] = s_max;
			s[bb] = s[bb] - s_renorm / d;
			//s[bb] = s_renorm;

			ofLut << s[bb] << endl;
        }
    }

#endif
    //progress_cb( 50 );

    //Create a tone-curve 
	// CAREFUL: Make sure we cover the WHOLE color map range by using the proper starting bin of H!
    //UniformArrayLUT lut( H.L_min, H.L_max, H.bin_count );
	// Use the effective bin counts
	int effectiveBinCount = H.bin_count;// - H.zeroval_bin_index;

	UniformArrayLUT lut( H.L_min, H.L_max, H.bin_count );
	//UniformArrayLUT lut(nonzero_min, H.L_max, effectiveBinCount);
    lut.y_i[0] = 0;
	float delta = H.delta;// lut.delta;
    for( int bb = 1; bb < effectiveBinCount; bb++ ) {
		int effect_s_ind = bb - 1;// +H.zeroval_bin_index;
        lut.y_i[bb] = lut.y_i[bb-1] + s[effect_s_ind] * delta;
		ofLut << "y: " << lut.y_i[bb - 1] << "; s: " << s[effect_s_ind] << "; Hdelta: " << delta << endl;
    }
	ofLut.close();
	

    // Apply the tone-curve
	float min_nz_R = numeric_limits<float>::max();
	float min_nz_G = numeric_limits<float>::max();
	float min_nz_B = numeric_limits<float>::max();

	float max_nz_R = -min_nz_R;
	float max_nz_G = -min_nz_G;
	float max_nz_B = -min_nz_B;


    for( int pp = 0; pp < pix_count; pp++ ) {
		if (R_in[pp] == 0.0f)
			R_out[pp] = 0.f;
		else
		{
			R_out[pp] = lut.interp( safelog10f(R_in[pp] + val_delta) );
			//R_out[pp] = lut.interp(safelog10f(R_in[pp]));
			min_nz_R = min(R_out[pp], min_nz_R);
			max_nz_R = max(R_out[pp], max_nz_R);
		}


		if (G_in[pp] == 0.0f)
			G_out[pp] = 0.f;
		else
		{

			G_out[pp] = lut.interp( safelog10f(G_in[pp] + val_delta) );
			//G_out[pp] = lut.interp(safelog10f(G_in[pp]));
			min_nz_G = min(G_out[pp], min_nz_G);
			max_nz_G = max(G_out[pp], max_nz_G);
		}


		if (B_in[pp] == 0.0f)
			B_out[pp] = 0.f;
		else
		{
			B_out[pp] = lut.interp( safelog10f(B_in[pp] + val_delta) );
			//B_out[pp] = lut.interp(safelog10f(B_in[pp]));
			min_nz_B = min(B_out[pp], min_nz_B);
			max_nz_B = max(B_out[pp], max_nz_B);
		}
    }

	// Renormalize for better colormapping result!
	float invR = 1.0f / (max_nz_R - min_nz_R);
	float invG = 1.0f / (max_nz_G - min_nz_G);
	float invB = 1.0f / (max_nz_B - min_nz_B);

	float CM_bin_val = 1.0f / 255.0f;

	for (int pp = 0; pp < pix_count; pp++) {
		if (R_in[pp] == 0.0f)
			R_out[pp] = 0.f;
		else
		{
			R_out[pp] = invR * (R_out[pp] - min_nz_R) + CM_bin_val;
		}


		if (G_in[pp] == 0.0f)
			G_out[pp] = 0.f;
		else
		{
			G_out[pp] = invG * (G_out[pp] - min_nz_G) + CM_bin_val;
		}


		if (B_in[pp] == 0.0f)
			B_out[pp] = 0.f;
		else
		{
			B_out[pp] = invB * (B_out[pp] - min_nz_B) + CM_bin_val;
		}
	}


    delete [] s;
    delete [] logL;

}
