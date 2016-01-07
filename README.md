icassp2013
==========

Source code based on work published in ICASSP 2013. Paper available at http://ieeexplore.ieee.org/xpls/icp.jsp?arnumber=6638125

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

- RunMR-AIM.cpp is the code that runs MR-AIM (model proposed in ICASSP 2013)

- We also provide an open-source C++ version of the original AIM model (proposed in JOV 2009) for comparison purposes. Original MATLAB code is available at http://www-sop.inria.fr/members/Neil.Bruce/AIM.zip

----------------
Getting Started
----------------

- Navigate to Debug folder
> cd ./code/Debug

- Compile code using make (tested on MacOS X Yosemite) 
> make

- Run MR-AIM 
> ./cvMR-AIM image.jpg

- Final Saliency map is generated as MR-AIMInfomap_image.jpg

- Run AIM 
> ./cvMR-AIM image.jpg 0

- Final Saliency map is generated as AIMInfomap_image.jpg

----------------
Dependencies
----------------

OpenCV 3.0 (makefile assumes Libraries are in /usr/local/lib and Includes in /usr/local/include)

----------------
License
----------------

This code is published under the MIT License.
