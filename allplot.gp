set terminal png
unset key
set yrange [-0.05:1.05]
set xrange [0:210]
call "plot.gp" ARG1 GRAY 0
set xrange [0:180]
call "plot.gp" ARG1 GRAY 90
set xrange [0:250]
call "plot.gp" ARG1 LIGHTGRAY 0
set xrange [0:160]
call "plot.gp" ARG1 LIGHTGRAY 90
set xrange [0:110]
call "plot.gp" ARG1 DARKGRAY 0
set xrange [0:120]
call "plot.gp" ARG1 DARKGRAY 90
set xrange [0:150]
call "plot.gp" ARG1 RED 0
set xrange [0:100]
call "plot.gp" ARG1 RED 90
set xrange [0:90]
call "plot.gp" ARG1 GREEN 0
set xrange [0:120]
call "plot.gp" ARG1 GREEN 90
set xrange [0:110]
call "plot.gp" ARG1 BLUE 0
set xrange [0:130]
call "plot.gp" ARG1 BLUE 90
