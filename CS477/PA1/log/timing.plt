set terminal png small
set output "timing.png"
set title "Time Elapse"
plot "timing.log" smooth bezier
