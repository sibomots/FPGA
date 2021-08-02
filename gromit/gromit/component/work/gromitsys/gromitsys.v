//////////////////////////////////////////////////////////////////////
// Created by SmartDesign Sat Jul 31 14:06:25 2021
// Version: v11.8 SP3 11.8.4.8
//////////////////////////////////////////////////////////////////////

`timescale 1ns / 100ps

// gromitsys
module gromitsys(
    // Inputs
    DEVRST_N,
    MMUART_0_RXD_F2M,
    MMUART_1_RXD_F2M,
    // Outputs
    GPIO_0_M2F,
    GPIO_1_M2F,
    GPIO_2_M2F,
    GPIO_3_M2F,
    GPIO_4_M2F,
    GPIO_5_M2F,
    GPIO_6_M2F,
    GPIO_7_M2F,
    MMUART_0_TXD_M2F,
    MMUART_1_TXD_M2F,
    // Inouts
    I2C0_SCL,
    I2C0_SDA
);

//--------------------------------------------------------------------
// Input
//--------------------------------------------------------------------
input  DEVRST_N;
input  MMUART_0_RXD_F2M;
input  MMUART_1_RXD_F2M;
//--------------------------------------------------------------------
// Output
//--------------------------------------------------------------------
output GPIO_0_M2F;
output GPIO_1_M2F;
output GPIO_2_M2F;
output GPIO_3_M2F;
output GPIO_4_M2F;
output GPIO_5_M2F;
output GPIO_6_M2F;
output GPIO_7_M2F;
output MMUART_0_TXD_M2F;
output MMUART_1_TXD_M2F;
//--------------------------------------------------------------------
// Inout
//--------------------------------------------------------------------
inout  I2C0_SCL;
inout  I2C0_SDA;
//--------------------------------------------------------------------
// Nets
//--------------------------------------------------------------------
wire   BIBUF_0_Y;
wire   BIBUF_1_Y;
wire   DEVRST_N;
wire   GPIO_0_M2F_net_0;
wire   GPIO_1_M2F_net_0;
wire   GPIO_2_M2F_net_0;
wire   GPIO_3_M2F_net_0;
wire   GPIO_4_M2F_net_0;
wire   GPIO_5_M2F_net_0;
wire   GPIO_6_M2F_net_0;
wire   GPIO_7_M2F_net_0;
wire   gromitsys_sb_0_I2C_0_SCL_M2F;
wire   gromitsys_sb_0_I2C_0_SCL_M2F_OE;
wire   gromitsys_sb_0_I2C_0_SDA_M2F;
wire   gromitsys_sb_0_I2C_0_SDA_M2F_OE;
wire   gromitsys_sb_0_POWER_ON_RESET_N;
wire   I2C0_SCL;
wire   I2C0_SDA;
wire   MMUART_0_RXD_F2M;
wire   MMUART_0_TXD_M2F_net_0;
wire   MMUART_1_RXD_F2M;
wire   MMUART_1_TXD_M2F_net_0;
wire   GPIO_0_M2F_net_1;
wire   GPIO_1_M2F_net_1;
wire   GPIO_2_M2F_net_1;
wire   GPIO_3_M2F_net_1;
wire   GPIO_4_M2F_net_1;
wire   GPIO_5_M2F_net_1;
wire   GPIO_6_M2F_net_1;
wire   GPIO_7_M2F_net_1;
wire   MMUART_0_TXD_M2F_net_1;
wire   MMUART_1_TXD_M2F_net_1;
//--------------------------------------------------------------------
// Top level output port assignments
//--------------------------------------------------------------------
assign GPIO_0_M2F_net_1       = GPIO_0_M2F_net_0;
assign GPIO_0_M2F             = GPIO_0_M2F_net_1;
assign GPIO_1_M2F_net_1       = GPIO_1_M2F_net_0;
assign GPIO_1_M2F             = GPIO_1_M2F_net_1;
assign GPIO_2_M2F_net_1       = GPIO_2_M2F_net_0;
assign GPIO_2_M2F             = GPIO_2_M2F_net_1;
assign GPIO_3_M2F_net_1       = GPIO_3_M2F_net_0;
assign GPIO_3_M2F             = GPIO_3_M2F_net_1;
assign GPIO_4_M2F_net_1       = GPIO_4_M2F_net_0;
assign GPIO_4_M2F             = GPIO_4_M2F_net_1;
assign GPIO_5_M2F_net_1       = GPIO_5_M2F_net_0;
assign GPIO_5_M2F             = GPIO_5_M2F_net_1;
assign GPIO_6_M2F_net_1       = GPIO_6_M2F_net_0;
assign GPIO_6_M2F             = GPIO_6_M2F_net_1;
assign GPIO_7_M2F_net_1       = GPIO_7_M2F_net_0;
assign GPIO_7_M2F             = GPIO_7_M2F_net_1;
assign MMUART_0_TXD_M2F_net_1 = MMUART_0_TXD_M2F_net_0;
assign MMUART_0_TXD_M2F       = MMUART_0_TXD_M2F_net_1;
assign MMUART_1_TXD_M2F_net_1 = MMUART_1_TXD_M2F_net_0;
assign MMUART_1_TXD_M2F       = MMUART_1_TXD_M2F_net_1;
//--------------------------------------------------------------------
// Component instances
//--------------------------------------------------------------------
//--------BIBUF
BIBUF BIBUF_0(
        // Inputs
        .D   ( gromitsys_sb_0_I2C_0_SCL_M2F ),
        .E   ( gromitsys_sb_0_I2C_0_SCL_M2F_OE ),
        // Outputs
        .Y   ( BIBUF_0_Y ),
        // Inouts
        .PAD ( I2C0_SCL ) 
        );

//--------BIBUF
BIBUF BIBUF_1(
        // Inputs
        .D   ( gromitsys_sb_0_I2C_0_SDA_M2F ),
        .E   ( gromitsys_sb_0_I2C_0_SDA_M2F_OE ),
        // Outputs
        .Y   ( BIBUF_1_Y ),
        // Inouts
        .PAD ( I2C0_SDA ) 
        );

//--------gromitsys_sb
gromitsys_sb gromitsys_sb_0(
        // Inputs
        .FAB_RESET_N      ( gromitsys_sb_0_POWER_ON_RESET_N ),
        .DEVRST_N         ( DEVRST_N ),
        .MMUART_0_RXD_F2M ( MMUART_0_RXD_F2M ),
        .MMUART_1_RXD_F2M ( MMUART_1_RXD_F2M ),
        .I2C_0_SDA_F2M    ( BIBUF_1_Y ),
        .I2C_0_SCL_F2M    ( BIBUF_0_Y ),
        // Outputs
        .POWER_ON_RESET_N ( gromitsys_sb_0_POWER_ON_RESET_N ),
        .INIT_DONE        (  ),
        .FAB_CCC_GL0      (  ),
        .FAB_CCC_LOCK     (  ),
        .MSS_READY        (  ),
        .MMUART_0_TXD_M2F ( MMUART_0_TXD_M2F_net_0 ),
        .MMUART_1_TXD_M2F ( MMUART_1_TXD_M2F_net_0 ),
        .I2C_0_SDA_M2F    ( gromitsys_sb_0_I2C_0_SDA_M2F ),
        .I2C_0_SDA_M2F_OE ( gromitsys_sb_0_I2C_0_SDA_M2F_OE ),
        .I2C_0_SCL_M2F    ( gromitsys_sb_0_I2C_0_SCL_M2F ),
        .I2C_0_SCL_M2F_OE ( gromitsys_sb_0_I2C_0_SCL_M2F_OE ),
        .GPIO_0_M2F       ( GPIO_0_M2F_net_0 ),
        .GPIO_1_M2F       ( GPIO_1_M2F_net_0 ),
        .GPIO_2_M2F       ( GPIO_2_M2F_net_0 ),
        .GPIO_3_M2F       ( GPIO_3_M2F_net_0 ),
        .GPIO_4_M2F       ( GPIO_4_M2F_net_0 ),
        .GPIO_5_M2F       ( GPIO_5_M2F_net_0 ),
        .GPIO_6_M2F       ( GPIO_6_M2F_net_0 ),
        .GPIO_7_M2F       ( GPIO_7_M2F_net_0 ) 
        );


endmodule
