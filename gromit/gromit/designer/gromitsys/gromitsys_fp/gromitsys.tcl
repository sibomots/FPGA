open_project -project {C:\ghf\FPGA\gromit\gromit\designer\gromitsys\gromitsys_fp\gromitsys.pro}
set_programming_file -name {M2S010} -file {C:\ghf\FPGA\gromit\gromit\designer\gromitsys\gromitsys.ipd}
enable_device -name {M2S010} -enable 1
set_programming_action -action {PROGRAM} -name {M2S010} 
run_selected_actions
set_programming_file -name {M2S010} -no_file
save_project
close_project
