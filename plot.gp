set terminal png
set output ARG1."/".ARG2."_".ARG3.".png"
plot ARG1."/".ARG2."_".ARG3.".txt" lc rgb "red" with lines