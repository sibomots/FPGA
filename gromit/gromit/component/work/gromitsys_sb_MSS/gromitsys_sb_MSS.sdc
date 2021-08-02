set_component gromitsys_sb_MSS
# Microsemi Corp.
# Date: 2021-Jul-31 13:38:30
#

create_clock -period 40 [ get_pins { MSS_ADLIB_INST/CLK_CONFIG_APB } ]
set_false_path -ignore_errors -through [ get_pins { MSS_ADLIB_INST/CONFIG_PRESET_N } ]
