#! /bin/bash
# Chan-Vese segmentation example BASH shell script

# Echo shell commands
set -v

# Perform Chan-Vese segmentation on wrech.bmp with edge penalty mu = 0.2.
# The output animation.gif shows an animation of the curve evolution, and
# final.bmp shows the final segmentation.
./chanvese mu:0.2 wrench.bmp animation.gif final.bmp
