new_project \
         -name {gromitsys} \
         -location {C:\ghf\FPGA\gromit\gromit\designer\gromitsys\gromitsys_fp} \
         -mode {chain} \
         -connect_programmers {FALSE}
add_actel_device \
         -device {M2S010} \
         -name {M2S010}
enable_device \
         -name {M2S010} \
         -enable {TRUE}
save_project
close_project
