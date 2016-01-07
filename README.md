MR-AIM
==========

Source code for MR-AIM based on work published in ICASSP 2013. Paper available at http://ieeexplore.ieee.org/xpls/icp.jsp?arnumber=6638125

If you use this code for evaluation and/or benchmarking, we appreciate if you cite an appropriate subset of the following papers:

@INPROCEEDINGS{icassp2013,
author={Advani, S. and Sustersic, J. and Irick, K. and Narayanan, V.},
booktitle={Acoustics, Speech and Signal Processing (ICASSP), 2013 IEEE International Conference on},
title={A multi-resolution saliency framework to drive foveation},
year={2013},
month={May},
pages={2596-2600},
}

@ARTICLE{AIM,
author = {Bruce, N. and Tsotsos, J.},
journal = {Journal of Vision},
title = {{Saliency, Attention, and Visual Search: An Information Theoretic Approach}},
year = {2009},
}

-------------
Contents
-------------

This code package contains the following files:

- RunMITBenchmark.cpp is the code that runs MR-AIM (model proposed in ICASSP 2013) on the MIT Benchmark

- We also provide an open-source C++ version of the original AIM model (proposed in JOV 2009) for comparison purposes. Original MATLAB code is available at http://www-sop.inria.fr/members/Neil.Bruce/AIM.zip

----------------
Getting Started
----------------

- Navigate to Debug folder
> cd ./source/Debug

- Compile code using make (tested on MacOS X Yosemite) 
> make

- Download the dataset from http://saliency.mit.edu/results_mit300.html
- >./cvMR-AIM BenchmarkIMAGES/ BenchmarkOUTPUT/

Saliency maps are generated in the directory BenchmarkOUTPUT

- To run baseline AIM model >./cvMR-AIM BenchmarkIMAGES/ BenchmarkOUTPUT/ 0

----------------
Dependencies
----------------

OpenCV 3.0 (makefile assumes Libraries are in /usr/local/lib and Includes in /usr/local/include)

----------------
License
----------------

This code is published under the MIT License.
