set_family {SmartFusion2}
read_adl {C:\ghf\FPGA\gromit\gromit\designer\gromitsys\gromitsys.adl}
map_netlist
check_constraints {C:\ghf\FPGA\gromit\gromit\designer\gromitsys\placer_coverage.log}
write_sdc -strict {C:\ghf\FPGA\gromit\gromit\designer\gromitsys\place_route.sdc}
