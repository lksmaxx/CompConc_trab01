#!/bin/bash
# formato:
# file_name.out image_name, (double)zoom = 0, (double)x_offset = 0, (double)y_offset = 0
echo "Alguns exemplos:"
echo "----------------"
./mandelbrot.out visao_geral 0.5
./mandelbrot.out exemplo01 200 -0.748 0.101
./mandelbrot.out exemplo02 5000 -1.23451 0.10485
./mandelbrot.out exemplo03_1 13000 0.34901 0.09015
./mandelbrot.out exemplo03_2 20000 0.34897 0.09015
