//
//
//#include "SimpleOpt.h"
//#include "JPEGPyramid.h"
//#include "JPEGImage.h"
#include "patchwork.hpp"
#include "pyramid_stitcher.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <assert.h>
using namespace std;
//
//TODO: have a pyramid stitch class?

//  @param planeDim == width == height of planes to cover with images (optional)
//            if planeDim <= 0, then ignore planeDim and compute plane size based on input image dims

Patchwork stitch_pyramid(string file, int img_minWidth, int img_minHeight,
                         int padding, int nscales, int planeDim, cv::Scalar mean_pixel)
{
    cv::Mat image = cv::imread(file);
    if (image.empty()) {
        cerr << "\nInvalid image " << file << endl;
    }
    return stitch_pyramid(image, img_minWidth, img_minHeight, padding, nscales, planeDim, mean_pixel);
}


Patchwork stitch_pyramid(cv::Mat image, int img_minWidth, int img_minHeight,
                         int padding, int nscales, int planeDim, cv::Scalar mean_pixel){

  return stitch_pyramid(image, img_minWidth, img_minHeight, padding, nscales, planeDim, planeDim, mean_pixel);
}

Patchwork stitch_pyramid(string file, int img_minWidth, int img_minHeight,
                         int padding, int nscales, int planeWidth, int planeHeight, cv::Scalar mean_pixel){
    cv::Mat image = cv::imread(file);
    if (image.empty()) {
        cerr << "\nInvalid image " << file << endl;
    }
    return stitch_pyramid(image, img_minWidth, img_minHeight, padding, nscales, planeWidth,  planeHeight, mean_pixel);
}


Patchwork stitch_pyramid(cv::Mat image, int img_minWidth, int img_minHeight,
                         int padding, int nscales, int planeWidth, int planeHeight, cv::Scalar mean_pixel)
{
    //image.convertTo(image, CV_32F);
    int upsampleFactor = 1; //TODO: make this an input param?
    //int upsampleFactor = 3;

  // Compute the downsample+stitch
    ImagePyramid pyramid(image, padding, padding, nscales, 0.85, upsampleFactor, mean_pixel); //multiscale DOWNSAMPLE with (padx == pady == padding)
    if (pyramid.empty()) {
        cerr << "\nInvalid image."<< endl;
    }

    int nlevels = pyramid.levels().size();
    if(planeWidth <= 0 || planeHeight <=0){
        planeWidth = (pyramid.levels()[0].cols + 15) & ~15;
        planeHeight = (pyramid.levels()[0].rows + 15) & ~15;
        planeWidth = max(planeWidth, planeHeight);  //SQUARE planes for Caffe convnet
        planeHeight = max(planeWidth, planeHeight);
    }

    Patchwork::Init(planeHeight, planeWidth, img_minWidth, img_minHeight);
    const Patchwork patchwork(pyramid, mean_pixel); //STITCH

    return patchwork;
}

//@param sbin = difference between input image dim and convnet feature dim
//         e.g. if input img is 200x200 and conv5 is 25x25 ... 200/25=8 -> 8x downsampling in convnet
//JPEGPyramid unstitch_pyramid(Patchwork image_patchwork, float* convnet_planes, int sbin){
vector<ScaleLocation> unstitch_pyramid_locations(Patchwork &patchwork,
                                                 int sbin)
{
    int nbScales = patchwork.nbScales;
    vector<ScaleLocation> scaleLocations(nbScales);
    int planeWidth = patchwork.MaxCols();
    int planeHeight = patchwork.MaxRows();

    for(int i=0; i<nbScales; i++)
    {
        scaleLocations[i].xMin  = patchwork.rectangles_[i].first.x() / sbin;
        scaleLocations[i].width = patchwork.rectangles_[i].first.width() / sbin;
        scaleLocations[i].xMax = scaleLocations[i].width + scaleLocations[i].xMin; //already accounts for subsampling ratio
        //scaleLocations[i].xMin  = (patchwork.rectangles_[i].first.x() + patchwork.rectangles_[i].first.width()) / sbin;

        scaleLocations[i].yMin = patchwork.rectangles_[i].first.y() / sbin;
        scaleLocations[i].height = patchwork.rectangles_[i].first.height() / sbin;
        scaleLocations[i].yMax = scaleLocations[i].height + scaleLocations[i].yMin; //already accounts for subsampling ratio
        //scaleLocations[i].yMax = (patchwork.rectangles_[i].first.y() + patchwork.rectangles_[i].first.height()) / sbin;

        scaleLocations[i].planeID = patchwork.rectangles_[i].second;

        //printf(" scale idx = %d \n", i);
        //printf("    image xMax = %d, planeWidth = %d \n", scaleLocations[i].xMax, planeWidth/sbin);
        //printf("    image yMax = %d, planeHeight = %d \n", scaleLocations[i].yMax, planeHeight/sbin);
        //printf("    yMax, in rgb space = %d \n", (patchwork.rectangles_[i].first.y() + patchwork.rectangles_[i].first.height()));

        assert( scaleLocations[i].xMax <= (planeWidth/sbin) );
	    assert( scaleLocations[i].yMax <= (planeHeight/sbin) );
    }

    return scaleLocations;
}

