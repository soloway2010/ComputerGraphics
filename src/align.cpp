#include "align.h"
#include <string>

using std::string;
using std::cout;
using std::endl;
using std::get;

const int SHIFT = 15;

class convert
{
public:
	convert(Matrix<double> k):kernel(k){}

    std::tuple<uint, uint, uint> operator () (const Image &m) const
    {
        uint size = 2 * radius + 1;

        std::tuple<uint, uint, uint> res;

        for (uint i = 0; i < size; ++i) {
            for (uint j = 0; j < size; ++j) {
                get<0>(res) += kernel(i, j)*get<0>(m(i, j));
                get<1>(res) += kernel(i, j)*get<1>(m(i, j));
                get<2>(res) += kernel(i, j)*get<2>(m(i, j));
            }
        }

        return res;
    }
    // Radius of neighbourhoud, which is passed to that operator
    static int radius;

    Matrix<double> kernel;
};

int convert::radius = 1;

Image one_dim_convert(Image src_image, Matrix<double> kernel, int radius, int dir){
	const int start_i = radius;
    const int n = (dir ? src_image.n_cols : src_image.n_rows);
    const int end_i = (dir ? src_image.n_rows : src_image.n_cols) - radius;

    Image dst_image(src_image.n_rows, src_image.n_cols);

    for(int j = 0; j < n; j++)
    	for(int i = start_i; i < end_i; i++){
    		std::tuple<uint, uint, uint> sum = std::make_tuple(0, 0, 0);
    		for(int k = -radius; k <= radius; k++){
    			get<0>(sum) += (dir ? get<0>(src_image(i + k, j)) : get<0>(src_image(j, i + k)))*kernel(k + radius, 0);
    			get<1>(sum) += (dir ? get<1>(src_image(i + k, j)) : get<1>(src_image(j, i + k)))*kernel(k + radius, 0);
    			get<2>(sum) += (dir ? get<2>(src_image(i + k, j)) : get<2>(src_image(j, i + k)))*kernel(k + radius, 0);
    		}
    		(dir ? dst_image(i, j) : dst_image(j, i)) = sum;
    	}

    return dst_image;
}

uint MSE(Image image1, Image image2, int shift){

	uint sum = 0;

	if(shift >= 0){
		for(uint i = 0; i < image1.n_rows - shift; i++)
			for(uint j = 0; j < image1.n_cols; j++)
				sum += (get<0>(image2(i, j)) - get<0>(image1(i + shift, j))) * (get<0>(image2(i, j)) - get<0>(image1(i + shift, j)));
	}else{
		for(uint i = 0; i < image1.n_rows + shift; i++)
			for(uint j = 0; j < image1.n_cols; j++)
				sum += (get<0>(image1(i, j)) - get<0>(image2(i - shift, j))) * (get<0>(image1(i, j)) - get<0>(image2(i - shift, j)));
	}
	sum /= (image1.n_rows - abs(shift)) * image1.n_cols;

	return sum;
}

uint COR(Image image1, Image image2, int shift){

	uint sum = 0;

	if(shift >= 0){
		for(uint i = 0; i < image1.n_rows - shift; i++)
			for(uint j = 0; j < image1.n_cols; j++)
				sum += get<0>(image2(i, j)) * get<0>(image1(i + shift, j));
	}else{
		for(uint i = 0; i < image1.n_rows + shift; i++)
			for(uint j = 0; j < image1.n_cols; j++)
				sum += get<0>(image1(i, j)) * get<0>(image2(i - shift, j));
	}
	
	return sum;
}

Image align(Image srcImage, bool isPostprocessing, std::string postprocessingType, double fraction, bool isMirror, 
            bool isInterp, bool isSubpixel, double subScale)
{
	/*Image image1 = srcImage.submatrix(0, 0, srcImage.n_rows/3, srcImage.n_cols);
	Image image2 = srcImage.submatrix(srcImage.n_rows/3, 0, srcImage.n_rows/3, srcImage.n_cols);
	Image image3 = srcImage.submatrix(2*srcImage.n_rows/3, 0, srcImage.n_rows/3, srcImage.n_cols);

	Image dstImage(image1.n_rows + 2*SHIFT, image2.n_cols);

	for(uint i = SHIFT; i < SHIFT + image1.n_rows; i++)
		for(uint j = 0; j < dstImage.n_cols; j++)
			get<2>(dstImage(i, j)) = get<0>(image1(i - SHIFT, j));

	uint sum = MSE(image1, image2, -SHIFT + 1);
	int shift = -SHIFT + 1;

	for(int i = -SHIFT + 2; i < SHIFT; i++){
		uint tmp = MSE(image1, image2, i);
		if(tmp < sum){
			sum = tmp;
			shift = i;
		}
	}

	std::cout << shift << endl;

	for(uint i = SHIFT + shift; i < SHIFT + shift + image1.n_rows; i++)
		for(uint j = 0; j < dstImage.n_cols; j++)
			get<1>(dstImage(i, j)) = get<0>(image2(i - SHIFT - shift, j));

	sum = MSE(image1, image3, -SHIFT + 1);
	int shift1 = -SHIFT + 1;

	for(int i = -SHIFT + 2; i < SHIFT; i++){
		uint tmp = MSE(image1, image3, i);
		if(tmp < sum){
			sum = tmp;
			shift1 = i;
		}
	}

	sum = MSE(image2, image3, -SHIFT + 1);
	int shift2 = -SHIFT + 1;

	for(int i = -SHIFT + 2; i < SHIFT; i++){
		uint tmp = MSE(image2, image3, i);
		if(tmp < sum){
			sum = tmp;
			shift2 = i;
		}
	}

	std::cout << shift1 << " " << shift2 << endl;

	shift = (shift1 + shift2)/2;

	for(uint i = SHIFT + shift; i < SHIFT + shift + image1.n_rows; i++)
		for(uint j = 0; j < dstImage.n_cols; j++)
			get<0>(dstImage(i, j)) = get<0>(image3(i - SHIFT - shift, j));*/

    return canny(srcImage, 0, 0);
}

Image sobel_x(Image src_image) {
    Matrix<double> kernel = {{-1, 0, 1},
                             {-2, 0, 2},
                             {-1, 0, 1}};
    return custom(src_image, kernel);
}

Image sobel_y(Image src_image) {
    Matrix<double> kernel = {{ 1,  2,  1},
                             { 0,  0,  0},
                             {-1, -2, -1}};
    return custom(src_image, kernel);
}

Image unsharp(Image src_image) {
    return src_image;
}

Image gray_world(Image src_image) {
    return src_image;
}

Image resize(Image src_image, double scale) {
    return src_image;
}

Image custom(Image src_image, Matrix<double> kernel) {
    Image dst_image = src_image.unary_map(convert(kernel));

    return dst_image;
}

Image autocontrast(Image src_image, double fraction) {
    return src_image;
}

Image gaussian(Image src_image, double sigma, int radius)  {
	int size = 2*radius + 1;
	Matrix<double> kernel(size, size);
	double sum = 0;

	for(int i = -size/2; i < size/2 + 1; i++)
		for(int j = -size/2; j < size/2 + 1; j++){
			kernel(i + size/2, j + size/2) = exp(-(i*i + j*j)/(2*sigma*sigma))/(2*3.14*sigma*sigma);
			sum += kernel(i + size/2, j + size/2);
		}

	for(int i = 0; i < size; i++)
		for(int j = 0; j < size; j++)
			kernel(i, j) /= sum;

	convert::radius = radius;
	Image dst_image = custom(src_image, kernel);

    return dst_image;
}

Image gaussian_separable(Image src_image, double sigma, int radius) {
	int size = 2*radius + 1;
	Matrix<double> kernel(size, 1);
	double sum = 0;

	for(int i = -size/2; i < size/2 + 1; i++){
		kernel(i + size/2, 0) = exp(-(i*i)/(2*sigma*sigma))/sqrt((2*3.14*sigma*sigma));
		sum += kernel(i + size/2, 0);
	}

	for(int i = 0; i < size; i++)
		kernel(i, 0) /= sum;

	Image dst_image = one_dim_convert(src_image, kernel, radius, 0);
	dst_image = one_dim_convert(dst_image, kernel, radius, 1);

    return dst_image;
}

Image median(Image src_image, int radius) {
    return src_image;
}

Image median_linear(Image src_image, int radius) {
    return src_image;
}

Image median_const(Image src_image, int radius) {
    return src_image;
}

Image canny(Image src_image, int threshold1, int threshold2) {
	Image dst_image = gaussian_separable(src_image, 1.4, 2);

    return dst_image;
}