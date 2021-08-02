set_family {SmartFusion2}
read_verilog -mode verilog_2k {C:\ghf\FPGA\gromit\gromit\component\Actel\DirectCore\CoreResetP\7.1.100\rtl\vlog\core\coreresetp_pcie_hotreset.v}
read_verilog -mode verilog_2k {C:\ghf\FPGA\gromit\gromit\component\Actel\DirectCore\CoreResetP\7.1.100\rtl\vlog\core\coreresetp.v}
read_verilog -mode verilog_2k {C:\ghf\FPGA\gromit\gromit\component\work\gromitsys_sb\CCC_0\gromitsys_sb_CCC_0_FCCC.v}
read_verilog -mode verilog_2k {C:\ghf\FPGA\gromit\gromit\component\work\gromitsys_sb\FABOSC_0\gromitsys_sb_FABOSC_0_OSC.v}
read_verilog -mode verilog_2k {C:\ghf\FPGA\gromit\gromit\component\work\gromitsys_sb_MSS\gromitsys_sb_MSS.v}
read_verilog -mode verilog_2k {C:\ghf\FPGA\gromit\gromit\component\work\gromitsys_sb\gromitsys_sb.v}
read_verilog -mode verilog_2k {C:\ghf\FPGA\gromit\gromit\component\work\gromitsys\gromitsys.v}
set_top_level {gromitsys}
map_netlist
check_constraints {C:\ghf\FPGA\gromit\gromit\constraint\synthesis_sdc_errors.log}
write_fdc {C:\ghf\FPGA\gromit\gromit\designer\gromitsys\synthesis.fdc}
